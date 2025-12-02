#include "HS_Hook.h"
#if defined(_M_IX86) || defined(__i386__)

#include "HS_Decoder.h"
#include "HS_Context.h"
#include <string.h>

namespace HSLL
{
	struct HSStaticContext
	{
		ptrAny uMem;
		ptrAny uCover;
		unsigned32 uSize;
	};

	struct HSRuntimeContext
	{
		ptrAny pRet;
	};

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

	void HSHook::SetHook(ptrAny pSrc, ptrAny pMem, ptrAny pBackup, unsigned32 uSize)
	{
		g_oStaticManager.SetContext((unsignedP)pSrc, HSStaticContext{ pMem, pBackup, uSize });
	}

	HSStaticContext* HSHook::FindHook(ptrAny pSrc)
	{
		return g_oStaticManager.FindContext((unsignedP)pSrc);
	}

	HSStaticContext* HSHook::RemoveHook(ptrAny pSrc)
	{
		return g_oStaticManager.RemoveContext((unsignedP)pSrc);
	}

	void HSHook::SetThreadContext(ptrAny pSrc, ptrAny pRet)
	{
		g_oRuntimeManager.SetContext((unsignedP)pSrc, HSRuntimeContext{ pRet });
	}

	ptrAny HSHook::FindThreadContext(ptrAny pSrc)
	{
		return g_oRuntimeManager.FindContext((unsignedP)pSrc);
	}

	ptrAny HSHook::RemoveThreadContext(ptrAny pSrc)
	{
		return g_oRuntimeManager.RemoveContext((unsignedP)pSrc)->pRet;
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
		HSMemProtection_None = 0,
		HSMemProtection_Read = PROT_READ,
		HSMemProtection_ReadWrite = PROT_READ | PROT_WRITE,
		HSMemProtection_Execute = PROT_EXEC,
		HSMemProtection_ReadExecute = PROT_READ | PROT_EXEC,
		HSMemProtection_ReadWriteExecute = PROT_READ | PROT_WRITE | PROT_EXEC
	};

	bool HSHook::SetProt(ptrAny pBuf, unsigned32 uSize, unsigned32 uProt)
	{
		if (pBuf == nullptr || uSize == 0)
		{
			return false;
		}

		unsignedP uPagesize = sysconf(_SC_PAGESIZE);

		if (uPagesize == (unsignedP)-1)
		{
			return false;
		}

		ptrU8 pAlignedAddr = (ptrU8)((unsignedP)pBuf & ~(uPagesize - 1));
		ptrU8 pEnd = (ptrU8)pBuf + uSize;
		unsignedP uNewSize = pEnd - pAlignedAddr;
		return mprotect(pAlignedAddr, uNewSize, uProt) == 0;
	}

	ptrAny HSHook::MemAlloc(unsigned32 uSize, unsigned32 uProt)
	{
		if (uSize == 0)
		{
			return nullptr;
		}

		unsigned32 uTotalSize = uSize + 4;
		ptrAny pBuf = mmap(nullptr, uTotalSize, uProt, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

		if (pBuf == MAP_FAILED)
		{
			return nullptr;
		}

		*((ptrU32)pBuf) = uTotalSize;
		return (ptrAny)((ptrS8)pBuf + 4);
	}

	bool HSHook::MemFree(ptrAny pBuf)
	{
		if (!pBuf)
		{
			return false;
		}

		ptrAny pActualAddr = (ptrAny)((ptrS8)pBuf - 4);
		unsigned32 uTotalSize = *(ptrU32)pActualAddr;
		return munmap(pActualAddr, uTotalSize) == 0;
	}
}
#endif

namespace HSLL
{
	const constexpr unsigned8 HS_HOOK_CODE[] =
	{
		0x60,                     // pushad 
		0x9C,                     // pushfd 
		0x68,0x00,0x00,0x00,0x00, // push key 
		0xE8,0x00,0x00,0x00,0x00, // call FindThreadContext 
		0x83,0xC4,0x04,           // add esp,4
		0x85,0xC0,                // test eax,eax 
		0x75,0x39,                // jne ToSrc 
		0xFF,0x74,0x24,0x24,      // push dword ptr ss:[esp+0x24] 
		0x68,0x00,0x00,0x00,0x00, // push key 
		0xE8,0x00,0x00,0x00,0x00, // call SetThreadContext 
		0x83,0xC4,0x08,           // add esp,8
		0xC7,0x44,0x24,0x24,      // mov dword ptr [esp+24h], ToRet
		0x00,0x00,0x00,0x00,      // ...
		0x9D,                     // popfd 
		0x61,                     // popad 
		0xE9,0x00,0x00,0x00,0x00, // jmp ReplaceFunction 
		0x83,0xEC,0x04,           // sub esp,4 (ToRet) 
		0x60,                     // pushad 
		0x9C,                     // pushfd 
		0x68,0x00,0x00,0x00,0x00, // push key 
		0xE8,0x00,0x00,0x00,0x00, // call ResetThreadContext 
		0x83,0xC4,0x04,           // add esp,4
		0x89,0x44,0x24,0x24,      // mov dword ptr ss:[esp+0x24], eax  
		0x9D,                     // popfd 
		0x61,                     // popad 
		0xC3,                     // ret 
		0x9D,                     // popfd (ToSrc) 
		0x61,                     // popad 
	};

	void HSHook::FillJmp(ptrAny pBuf, ptrAny pDst)
	{
		*(ptrU8)pBuf = 0xE9;
		*(ptrS32)((ptrU8)pBuf + 1) = (signed32)pDst - (signed32)pBuf - 5;
	}

	void HSHook::FillHook(ptrAny pBuf, ptrAny pDst, ptrAny pSrc)
	{
		memcpy(pBuf, HS_HOOK_CODE, sizeof(HS_HOOK_CODE));

		ptrU8 pPtr = (ptrU8)pBuf;
		*(ptrU32)(pPtr + 3) = (unsigned32)pSrc;
		*(ptrS32)(pPtr + 8) = (signed32)((unsigned32)&HSHook::FindThreadContext - ((unsigned32)pBuf + 12));
		*(ptrS32)(pPtr + 24) = (unsigned32)pSrc;
		*(ptrS32)(pPtr + 29) = (signed32)((unsigned32)&HSHook::SetThreadContext - ((unsigned32)pBuf + 33));
		*(ptrU32)(pPtr + 40) = (unsigned32)pBuf + 51;
		*(ptrS32)(pPtr + 47) = (signed32)((unsigned32)pDst - ((unsigned32)pBuf + 51));
		*(ptrS32)(pPtr + 57) = (unsigned32)pSrc;
		*(ptrS32)(pPtr + 62) = (signed32)((unsigned32)&HSHook::RemoveThreadContext - ((unsigned32)pBuf + 66));
		return;
	}

	void HSHook::FillFixed(ptrAny pBuf, ptrAny pFixed, unsigned32 uSize)
	{
		memcpy(pBuf, pFixed, uSize);
	}

	void HSHook::FillBackup(ptrAny pBuf, ptrAny pBackup, unsigned32 uSize)
	{
		memcpy(pBuf, pBackup, uSize);
	}

	bool HSHook::Install(ptrAny pSrc, ptrAny pDst)
	{
		if (pSrc == nullptr || pDst == nullptr || pSrc == pDst)
		{
			return false;
		}

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

		constexpr unsigned32 HOOK_BUFFER_SIZE = 4096;
		ptrAny pBuf = MemAlloc(HOOK_BUFFER_SIZE, HSMemProtection_ReadWriteExecute);

		if (pBuf == nullptr)
		{
			return false;
		}

		if (!GetBackupIns(pSrc, pBackupInfo, uNum))
		{
			return false;
		}

		ptrU8 pFixed = (ptrU8)pBuf + sizeof(HS_HOOK_CODE);

		if (!GetFixedIns(pSrc, pBackupInfo, uNum, pFixed, pFixedInfo))
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

		FillHook(pBuf, pDst, pSrc);
		FillJmp(pFixed + uFixedSize, (ptrU8)pSrc + uBackUpSize);
		FillBackup(pFixed + uFixedSize + 5, pSrc, uBackUpSize);
		FillJmp(pSrc, pBuf);
		SetHook(pSrc, pBuf, pFixed + uFixedSize + 5, uBackUpSize);
		return true;
	}

	bool HSHook::Remove(ptrAny pSrc)
	{
		HSStaticContext* pContext = FindHook(pSrc);

		if (pContext == nullptr)
		{
			return false;
		}

		memcpy(pSrc, pContext->uCover, pContext->uSize);
		MemFree(pContext->uMem);
		RemoveHook(pSrc);
		return true;
	}
}

#endif