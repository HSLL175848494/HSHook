#include "HS_Hook.h"
#if defined(_M_IX86) || defined(__i386__)

#include "HS_Decoder.h"
#include "HS_Context.h"
#include "HS_RWLock.hpp"
#include <string.h>

namespace HSLL
{
	struct HSStaticContext
	{
		ptrAny pMem;
		ptrAny pCover;
		unsigned32 uSize;
	};

	struct HSRuntimeContext
	{
		ptrAny pRet;
	};

	static HSSpinRWLock g_oHookLock;
	static HSContextManager<HSStaticContext> g_oStaticManager;
	thread_local HSContextManager<HSRuntimeContext> g_oRuntimeManager;

	unsigned32 HSHook::GetInsSize(HSInsInfo* pInfo, unsigned32 uNum)
	{
		unsigned32 uSize = 0;

		for (unsigned32 i = 0; i < uNum; i++)
		{
			uSize += pInfo[i].sTotalSize;
		}

		return uSize;
	}

	bool HSHook::GetBackupIns(ptrAny pIns, HSInsInfo* pInfo, unsigned32& uNum)
	{
		uNum = 0;
		unsigned32 uNowSize = 0;
		ptrU8 pInsPtr = (ptrU8)pIns;

		while (true)
		{
			if (!HSLL::HSx86Decoder::ParseCode(pInsPtr, pInfo[uNum]))
			{
				return false;
			}

			pInsPtr += pInfo[uNum].sTotalSize;
			uNowSize += pInfo[uNum].sTotalSize;
			uNum++;

			if (uNowSize >= 5)
			{
				return true;
			}

			if (pInfo[uNum - 1].bIsRet)
			{
				return false;
			}
		}
	}

	bool HSHook::GetFixedIns(ptrAny pIns, HSInsInfo* pInfo, unsigned32 uNum,
		ptrAny pFixedIns, HSInsInfo* pFixedInfo)
	{
		ptrU8 pInsPtr = (ptrU8)pIns;
		ptrU8 pFixedPtr = (ptrU8)pFixedIns;

		for (unsigned32 i = 0; i < uNum; i++)
		{
			if (pInfo[i].bIsJmp || pInfo[i].bIsCall)
			{
				if (!HSx86Decoder::CallJmpConvert(pInfo[i], (unsignedP)pInsPtr, pInsPtr, pFixedInfo[i], (unsignedP)pFixedPtr, pFixedPtr))
				{
					return false;
				}
			}
			else
			{
				pFixedInfo[i] = pInfo[i];
				memcpy(pFixedPtr, pInsPtr, pInfo[i].sTotalSize);
			}

			pInsPtr += pInfo[i].sTotalSize;
			pFixedPtr += pFixedInfo[i].sTotalSize;
		}

		return true;
	}

	bool HSHook::IsHookFull()
	{
		return g_oStaticManager.IsFull();
	}

	void HSHook::StoreHook(ptrAny pSrc, ptrAny pMem, ptrAny pBackup, unsigned32 uSize)
	{
		g_oStaticManager.SetContext((unsignedP)pSrc, HSStaticContext{ pMem, pBackup, uSize });
	}

	ptrAny HSHook::FindHookSrc(ptrAny pSrc)
	{
		HSReadLockGuard oLock(g_oHookLock);
		HSStaticContext* pContext = FindHook(pSrc);

		if (!pContext)
		{
			return nullptr;
		}

		return pContext->pMem;
	}

	HSStaticContext* HSHook::FindHook(ptrAny pSrc)
	{
		return g_oStaticManager.FindContext((unsignedP)pSrc);
	}

	HSStaticContext* HSHook::RemoveHook(ptrAny pSrc)
	{
		return g_oStaticManager.RemoveContext((unsignedP)pSrc);
	}
}

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
namespace HSLL
{
	enum HSMemProtection
	{
		HSMemProtection_None = PAGE_NOACCESS,
		HSMemProtection_Read = PAGE_READONLY,
		HSMemProtection_ReadWrite = PAGE_READWRITE,
		HSMemProtection_Execute = PAGE_EXECUTE,
		HSMemProtection_ReadExecute = PAGE_EXECUTE_READ,
		HSMemProtection_ReadWriteExecute = PAGE_EXECUTE_READWRITE
	};

	bool HSHook::SetProt(ptrAny pMem, unsigned32 uSize, unsigned32 uProt)
	{
		if (pMem == nullptr || uSize == 0)
		{
			return false;
		}

		DWORD uOldProtect;
		return VirtualProtect(pMem, uSize, uProt, &uOldProtect) != 0;
	}

	ptrAny HSHook::MemAlloc(unsigned32 uSize, unsigned32 uProt)
	{
		if (uSize == 0)
		{
			return nullptr;
		}

		return VirtualAlloc(nullptr, uSize, MEM_COMMIT | MEM_RESERVE, uProt);
	}

	bool HSHook::MemFree(ptrAny pMem)
	{
		if (!pMem)
		{
			return false;
		}

		return VirtualFree(pMem, 0, MEM_RELEASE) != 0;
	}
}
#elif defined(__unix__)
#include <sys/mman.h>
#include <unistd.h>

namespace HSLL
{
	enum HSMemProtection
	{
		HSMemProtection_None = PROT_NONE,
		HSMemProtection_Read = PROT_READ,
		HSMemProtection_ReadWrite = PROT_READ | PROT_WRITE,
		HSMemProtection_Execute = PROT_EXEC,
		HSMemProtection_ReadExecute = PROT_READ | PROT_EXEC,
		HSMemProtection_ReadWriteExecute = PROT_READ | PROT_WRITE | PROT_EXEC
	};

	bool HSHook::SetProt(ptrAny pBuf, unsignedP uSize, unsigned32 uProt)
	{
		if (pBuf == nullptr || uSize == 0)
		{
			return false;
		}

		long uPageSize = sysconf(_SC_PAGESIZE);
		if (uPageSize == -1)
		{
			return false;
		}

		ptrU8 pAlignedAddr = (ptrU8)((unsignedP)pBuf & ~(uPageSize - 1));
		ptrU8 pEnd = (ptrU8)pBuf + uSize;
		ptrU8 pAlignedEnd = (ptrU8)(((unsignedP)pEnd + uPageSize - 1) & ~(uPageSize - 1));
		unsignedP uNewSize = pAlignedEnd - pAlignedAddr;

		if (uNewSize == 0)
		{
			return false;
		}

		return mprotect(pAlignedAddr, uNewSize, uProt) == 0;
	}

	ptrAny HSHook::MemAlloc(unsignedP uSize, unsigned32 uProt)
	{
		if (uSize == 0)
		{
			return nullptr;
		}

		const unsignedP uHeaderSize = sizeof(unsignedP);
		unsignedP uTotalSize = uSize + uHeaderSize;

		if (uTotalSize < uSize)
		{
			return nullptr;
		}

		ptrAny pBuf = mmap(nullptr, uTotalSize, uProt, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
		if (pBuf == MAP_FAILED)
		{
			return nullptr;
		}

		*((unsignedP*)pBuf) = uTotalSize;
		return (ptrAny)((ptrU8)pBuf + uHeaderSize);
	}

	bool HSHook::MemFree(ptrAny pBuf)
	{
		if (!pBuf)
		{
			return false;
		}

		const unsignedP uHeaderSize = sizeof(unsignedP);
		ptrAny pActualAddr = (ptrAny)((ptrU8)pBuf - uHeaderSize);
		unsignedP uTotalSize = *((unsignedP*)pActualAddr);

		if (uTotalSize < uHeaderSize)
		{
			return false;
		}

		return munmap(pActualAddr, uTotalSize) == 0;
	}
}
#endif

namespace HSLL
{
	void HSHook::WriteJmp(ptrAny pBuf, ptrAny pDst)
	{
		*(ptrU8)pBuf = 0xE9;
		*(ptrS32)((ptrU8)pBuf + 1) = (signed32)pDst - (signed32)pBuf - 5;
	}

	bool HSHook::Install(ptrAny pSrc, ptrAny pDst)
	{
		if (pSrc == nullptr || pDst == nullptr || pSrc == pDst)
		{
			return false;
		}

		HSWriteLockGuard oLock(g_oHookLock);

		if (IsHookFull())
		{
			return false;
		}

		if (FindHook(pSrc))
		{
			return false;
		}

		unsigned32 uNum;
		HSInsInfo pFixedInfo[64];
		HSInsInfo pBackupInfo[64];
		ptrU8 pBuf = (ptrU8)MemAlloc(4096, HSMemProtection_ReadWriteExecute);

		if (pBuf == nullptr)
		{
			return false;
		}

		if (!GetBackupIns(pSrc, pBackupInfo, uNum))
		{
			MemFree(pBuf);
			return false;
		}

		if (!GetFixedIns(pSrc, pBackupInfo, uNum, pBuf, pFixedInfo))
		{
			MemFree(pBuf);
			return false;
		}

		unsigned32 uFixedSize = GetInsSize(pFixedInfo, uNum);
		unsigned32 uBackUpSize = GetInsSize(pBackupInfo, uNum);

		if (!SetProt((ptrU8)pSrc, uBackUpSize, HSMemProtection_ReadWriteExecute))
		{
			MemFree(pBuf);
			return false;
		}

		WriteJmp(pBuf + uFixedSize, (ptrU8)pSrc + uBackUpSize);
		memcpy(pBuf + uFixedSize + 5, pSrc, uBackUpSize);
		WriteJmp(pSrc, pDst);
		StoreHook(pSrc, pBuf, pBuf + uFixedSize + 5, uBackUpSize);
		return true;
	}

	bool HSHook::Remove(ptrAny pSrc)
	{
		HSWriteLockGuard oLock(g_oHookLock);
		HSStaticContext* pContext = FindHook(pSrc);

		if (pContext == nullptr)
		{
			return false;
		}

		memcpy(pSrc, pContext->pCover, pContext->uSize);
		MemFree(pContext->pMem);
		RemoveHook(pSrc);
		return true;
	}
}

#endif