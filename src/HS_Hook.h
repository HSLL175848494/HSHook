#pragma once
#if defined(_M_IX86) || defined(__i386__)
#include "HS_Type.h"

#if defined(_MSC_VER)
#define HS_NOINLINE __declspec(noinline)
#define HS_CDECL __cdecl
#elif defined(__GNUC__) || defined(__clang__)
#define HS_NOINLINE __attribute__((noinline))
#define HS_CDECL __attribute__((cdecl))
#endif

namespace HSLL
{
	struct HSInsInfo;
	struct HSStaticContext;

	class HSHook
	{
	public:
		static bool Install(ptrAny pSrc, ptrAny pDst);

		static bool Remove(ptrAny pSrc);

		template<class T>
		static T* Original(T* pSrc)
		{
			return (T*)FindHookSrc(pSrc);
		}

	private:
		static bool SetProt(ptrAny pMem, unsigned32 uSize, unsigned32 uProt);

		static bool MemFree(ptrAny pMem);

		static ptrAny MemAlloc(unsigned32 uSize, unsigned32 uProt);

	private:
		static unsigned32 GetInsSize(HSInsInfo* pInfo, unsigned32 uNum);

		static bool GetBackupIns(ptrAny pIns, HSInsInfo* pInfo, unsigned32& uNum);

		static bool GetFixedIns(ptrAny pIns, HSInsInfo* pInfo, unsigned32 uNum, ptrAny pFixedIns, HSInsInfo* pFixedInfo);

		static void WriteJmp(ptrAny pBuf, ptrAny pDst);

	private:
		static bool IsHookFull();

		static void StoreHook(ptrAny pSrc, ptrAny pMem, ptrAny pBackup, unsigned32 uSize);

		static ptrAny FindHookSrc(ptrAny pSrc);

		static HSStaticContext* FindHook(ptrAny pSrc);

		static HSStaticContext* RemoveHook(ptrAny pSrc);
	};
}

#endif