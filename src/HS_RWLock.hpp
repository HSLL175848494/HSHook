#pragma once
#include "HS_Type.h"
#include <atomic>
#include <thread>

namespace HSLL
{
	namespace INNER
	{
		constexpr signedP HS_SPINRWLOCK_MAXSLOTS = 32;
		constexpr signedP HS_SPINRWLOCK_MAXREADER = (sizeof(signedP) == 4 ? (1 << 30) : (1LL << 62));

		class HSSpinRWLock
		{
		private:

			class alignas(64) InnerLock
			{
			private:
				std::atomic<signedP> uCount;

			public:

				InnerLock() noexcept : uCount(0) {}

				void LockRead() noexcept
				{
					signedP uOld = uCount.fetch_add(1, std::memory_order_acquire);

					while (uOld < 0)
					{
						uCount.fetch_sub(1, std::memory_order_relaxed);

						std::this_thread::yield();

						while (uCount.load(std::memory_order_relaxed) < 0)
							std::this_thread::yield();

						uOld = uCount.fetch_add(1, std::memory_order_acquire);
					}
				}

				void UnlockRead() noexcept
				{
					uCount.fetch_sub(1, std::memory_order_relaxed);
				}

				bool MarkWrite() noexcept
				{
					return !uCount.fetch_sub(HS_SPINRWLOCK_MAXREADER, std::memory_order_relaxed);
				}

				void UnmarkWrite(bool bReady) noexcept
				{
					if (bReady)
						uCount.store(0, std::memory_order_relaxed);
					else
						uCount.fetch_add(HS_SPINRWLOCK_MAXREADER, std::memory_order_relaxed);
				}

				void UnlockWrite() noexcept
				{
					uCount.store(0, std::memory_order_release);
				}

				bool IsWriteReady() noexcept
				{
					return uCount.load(std::memory_order_relaxed) == -HS_SPINRWLOCK_MAXREADER;
				}
			};

			class LocalReadLock
			{
				InnerLock& m_oLocalLock;

			public:

				explicit LocalReadLock(InnerLock& oLocalLock) noexcept : m_oLocalLock(oLocalLock) {}

				void LockRead() noexcept
				{
					m_oLocalLock.LockRead();
				}

				void UnlockRead() noexcept
				{
					m_oLocalLock.UnlockRead();
				}
			};

			std::atomic<bool> m_bFlag;
			InnerLock m_oRWLocks[HS_SPINRWLOCK_MAXSLOTS];

			thread_local static signedP m_uLocalIndex;
			static std::atomic<signedP> m_uGlobalIndex;

			inline LocalReadLock GetLocalLock() noexcept
			{
				signedP uIndex = m_uLocalIndex;

				if (uIndex == -1)
					uIndex = m_uLocalIndex = m_uGlobalIndex.fetch_add(1, std::memory_order_relaxed) % HS_SPINRWLOCK_MAXSLOTS;

				return LocalReadLock(m_oRWLocks[uIndex]);
			}

			bool TryMarkWrite(bool pFlagArray[HS_SPINRWLOCK_MAXSLOTS]) noexcept
			{
				bool uOld = true;

				if (!m_bFlag.compare_exchange_strong(uOld, false, std::memory_order_acquire, std::memory_order_relaxed))
					return false;

				for (signedP i = 0; i < HS_SPINRWLOCK_MAXSLOTS; ++i)
					pFlagArray[i] = m_oRWLocks[i].MarkWrite();

				return true;
			}

			bool TryMarkWriteCheckBefore(bool pFlagArray[HS_SPINRWLOCK_MAXSLOTS]) noexcept
			{
				if (!m_bFlag.load(std::memory_order_relaxed))
					return false;

				return TryMarkWrite(pFlagArray);
			}

			void MarkWrite(bool pFlagArray[HS_SPINRWLOCK_MAXSLOTS]) noexcept
			{
				if (TryMarkWrite(pFlagArray))
					return;

				std::this_thread::yield();

				while (!TryMarkWriteCheckBefore(pFlagArray))
					std::this_thread::yield();
			}

			void UnmarkWrite(bool pFlagArray[HS_SPINRWLOCK_MAXSLOTS]) noexcept
			{
				for (signedP i = 0; i < HS_SPINRWLOCK_MAXSLOTS; ++i)
					m_oRWLocks[i].UnmarkWrite(pFlagArray[i]);

				m_bFlag.store(true, std::memory_order_relaxed);
			}

			signedP ReadyCount(signedP uStartIndex, bool pFlagArray[HS_SPINRWLOCK_MAXSLOTS]) noexcept
			{
				signedP uIndex = HS_SPINRWLOCK_MAXSLOTS;

				for (signedP i = uStartIndex; i < HS_SPINRWLOCK_MAXSLOTS; ++i)
				{
					if (pFlagArray[i])
						continue;

					pFlagArray[i] = m_oRWLocks[i].IsWriteReady();

					if (!pFlagArray[i] && i < uIndex)
						uIndex = i;
				}

				return uIndex;
			}

		public:

			HSSpinRWLock() noexcept : m_bFlag(true) {}

			void LockRead() noexcept
			{
				GetLocalLock().LockRead();
			}

			void UnlockRead() noexcept
			{
				GetLocalLock().UnlockRead();
			}

			void LockWrite() noexcept
			{
				bool aFlagArray[HS_SPINRWLOCK_MAXSLOTS];

				MarkWrite(aFlagArray);

				signedP uNextCheckIndex = 0;

				while ((uNextCheckIndex = ReadyCount(uNextCheckIndex, aFlagArray)) != HS_SPINRWLOCK_MAXSLOTS)
					std::this_thread::yield();
			}

			void UnlockWrite() noexcept
			{
				for (signedP i = 0; i < HS_SPINRWLOCK_MAXSLOTS; ++i)
					m_oRWLocks[i].UnlockWrite();

				m_bFlag.store(true, std::memory_order_release);
			}

			HSSpinRWLock(const HSSpinRWLock&) = delete;
			HSSpinRWLock& operator=(const HSSpinRWLock&) = delete;
		};

		std::atomic<signedP> HSSpinRWLock::m_uGlobalIndex{ 0 };
		thread_local signedP HSSpinRWLock::m_uLocalIndex{ -1 };

		class HSReadLockGuard
		{
		private:

			HSSpinRWLock& oLock;

		public:

			explicit HSReadLockGuard(HSSpinRWLock& oLock) noexcept : oLock(oLock)
			{
				oLock.LockRead();
			}

			~HSReadLockGuard() noexcept
			{
				oLock.UnlockRead();
			}

			HSReadLockGuard(const HSReadLockGuard&) = delete;
			HSReadLockGuard& operator=(const HSReadLockGuard&) = delete;
		};

		class HSWriteLockGuard
		{
		private:

			HSSpinRWLock& oLock;

		public:

			explicit HSWriteLockGuard(HSSpinRWLock& oLock) noexcept : oLock(oLock)
			{
				oLock.LockWrite();
			}

			~HSWriteLockGuard() noexcept
			{
				oLock.UnlockWrite();
			}

			HSWriteLockGuard(const HSWriteLockGuard&) = delete;
			HSWriteLockGuard& operator=(const HSWriteLockGuard&) = delete;
		};
	}

	using INNER::HSSpinRWLock;
	using INNER::HSReadLockGuard;
	using INNER::HSWriteLockGuard;
}