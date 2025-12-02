#pragma once
#if defined(_M_IX86) || defined(__i386__)

#include "HS_Type.h"

namespace HSLL
{
	struct HSInsInfo
	{
		bool bNeedReloc;      // Whether relocation is needed
		bool bHasImmediate;   // Whether the instruction has an immediate value
		bool bHasModrm;       // Whether the instruction has a ModR/M byte
		bool bIsJmp;          // Whether the instruction is a jump
		bool bIsRet;          // Whether the instruction is a return
		bool bIsCall;         // Whether the instruction is a call
		signed8 sTotalSize;   // Total size of the instruction in bytes
		signed8 sRelocOffset; // Offset of the relocation operand from the start of the instruction
		signed8 sRelocSize;   // Size of the relocation operand in bytes
		signed8 sImmOffset;   // Offset of the immediate value from the start of the instruction
		signed8 sImmSize;     // Size of the immediate value in bytes
		signed8 sModrmOffset; // Offset of the ModR/M byte
	};

	struct HSOpcodeInfo
	{
		unsigned8 uOpcode;    // Opcode
		unsigned8 uRegOpcode; // Register opcode
		unsigned8 uFlags;     // Flags
		unsigned8 uInsType;   // Instruction type: 0-Normal, 1-Jump, 2-Return, 3-Call
	};

	class HSx86Decoder
	{
	public:
		static bool ParseCode(const ptrAny pIns, HSInsInfo& stInsInfo);
		static bool CallJmpConvert(const HSInsInfo& stInfoBefore, unsignedP uPosBefore, ptrU8 pInsBefore,
			HSInsInfo& stInfoAfter, unsignedP uPosAfter, ptrU8 pInsAfter);

	private:
		static void InitializeInsInfo(HSInsInfo& stInsInfo);
		static signedP ParsePrefixes(const ptrU8 pCode, signedP& sLen, signedP& sOperandSize);
		static signedP ParseREXPrefix(const ptrU8 pCode, signedP& sLen, signedP& sOperandSize);
		static bool ParseModRM(const ptrU8 pCode, signedP& sLen, HSInsInfo& stInsInfo, signedP sOperandSize);
		static void ParseImmediate(const HSOpcodeInfo& stOpcodeInfo, signedP& sLen, signedP sOperandSize, HSInsInfo& pInfo);
		static bool MatchOpcode(const ptrU8 pCode, signedP& sLen, signedP& sOpcodeIndex, signedP sOperandSize);
		static void SetRelocationInfo(HSInsInfo& stInsInfo, const HSOpcodeInfo& stOpcodeInfo, signedP& sLen, signedP sOperandSize);
		static void SetInstructionType(HSInsInfo& stInsInfo, const HSOpcodeInfo& stOpcodeInfo);
		static bool CheckBounds(signedP uCurrent, signedP sIncrement, signedP uMaxLen);
	};
}

#endif