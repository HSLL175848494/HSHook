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

		static bool CallJmpConvert(const HSInsInfo& stInfoBefore, unsigned32 uPosBefore,
			ptrU8 pInsBefore, HSInsInfo& stInfoAfter, unsigned32 uPosAfter, ptrU8 pInsAfter);

	private:

		static bool CheckBounds(signed32 uCurrent, signed32 sIncrement, signed32 uMaxLen);

		static signed32 ParsePrefixes(const ptrU8 pCode, signed32& sLen, signed32& sOperandSize);

		static bool MatchOpcode(const ptrU8 pCode, signed32& sLen, signed32& sOpcodeIndex, signed32 sOperandSize);

		static void SetRelocationInfo(HSInsInfo& stInsInfo, const HSOpcodeInfo& stOpcodeInfo, signed32& sLen, signed32 sOperandSize);

		static void SetInstructionType(HSInsInfo& stInsInfo, const HSOpcodeInfo& stOpcodeInfo);

		static bool ParseModRM(const ptrU8 pCode, signed32& sLen, HSInsInfo& stInsInfo, signed32 sOperandSize);

		static void ParseImmediate(const HSOpcodeInfo& stOpcodeInfo, signed32& sLen, signed32 sOperandSize, HSInsInfo& pInfo);

		static void InitializeInsInfo(HSInsInfo& stInsInfo);
	};
}

#endif