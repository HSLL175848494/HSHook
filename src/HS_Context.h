#pragma once
#include "HS_Type.h"

namespace HSLL
{
	template <typename T>
	class HSContextManager
	{
	public:
		struct Context
		{
			T stValue;
			unsignedP uKey;
			bool bOccupied;
		};

	public:
		HSContextManager()
		{
			uNowCount = 0;

			for (unsigned32 i = 0; i < HS_MAX_CONTEXT_NODE_NUM; ++i)
			{
				pTable[i].bOccupied = false;
			}
		}

	public:
		bool IsFull()
		{
			return uNowCount >= HS_MAX_CONTEXT_NODE_NUM;
		}

		T* FindContext(unsignedP uKey)
		{
			unsigned32 uFoundPos, uEmptyPos;

			if (FindSlot(uKey, uFoundPos, uEmptyPos))
			{
				return &pTable[uFoundPos].stValue;
			}

			return nullptr;
		}

		T* RemoveContext(unsignedP uKey)
		{
			unsigned32 uFoundPos, uEmptyPos;

			if (FindSlot(uKey, uFoundPos, uEmptyPos))
			{
				pTable[uFoundPos].bOccupied = false;
				uNowCount--;
				return &pTable[uFoundPos].stValue;
			}

			return nullptr;
		}

		T* SetContext(unsignedP uKey, const T& stValue)
		{
			unsigned32 uFoundPos, uEmptyPos;

			if (FindSlot(uKey, uFoundPos, uEmptyPos))
			{
				return nullptr;
			}

			if (uNowCount >= HS_MAX_CONTEXT_NODE_NUM)
			{
				return nullptr;
			}

			pTable[uEmptyPos].uKey = uKey;
			pTable[uEmptyPos].stValue = stValue;
			pTable[uEmptyPos].bOccupied = true;
			uNowCount++;
			return &pTable[uEmptyPos].stValue;
		}

	private:
		static constexpr unsigned32 HS_MAX_CONTEXT_NODE_NUM = 512;
		static_assert(HS_MAX_CONTEXT_NODE_NUM > 0, "HS_MAX_CONTEXT_NODE_NUM must be > 0");
		unsigned32 uNowCount;
		Context pTable[HS_MAX_CONTEXT_NODE_NUM];

		unsigned32 Hash(unsignedP uKey) const
		{
			return (uKey * 2654435761u) % HS_MAX_CONTEXT_NODE_NUM;
		}

		unsigned32 NextPosition(unsigned32 uPos) const
		{
			return (uPos + 1) % HS_MAX_CONTEXT_NODE_NUM;
		}

		bool FindSlot(unsignedP uKey, unsigned32& uFoundPos, unsigned32& uEmptyPos)
		{
			unsigned32 uStartPos = Hash(uKey);
			unsigned32 uPos = uStartPos;
			bool bFoundEmpty = false;

			do
			{
				if (!pTable[uPos].bOccupied)
				{
					if (!bFoundEmpty)
					{
						uEmptyPos = uPos;
						bFoundEmpty = true;
					}
				}
				else if (pTable[uPos].uKey == uKey)
				{
					uFoundPos = uPos;
					return true;
				}

				uPos = NextPosition(uPos);

			} while (uPos != uStartPos);

			if (bFoundEmpty)
			{
				return false;
			}

			uPos = uStartPos;

			do
			{
				if (pTable[uPos].uKey == uKey)
				{
					uFoundPos = uPos;
					return true;
				}

				uPos = NextPosition(uPos);

			} while (uPos != uStartPos);

			return false;
		}
	};
}