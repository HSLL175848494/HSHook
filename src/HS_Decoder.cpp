#include "HS_Decoder.h"
#if defined(_M_IX86) || defined(__i386__)

#include <string.h>

namespace HSLL
{
	enum HSX86Flag
	{
		X86Flag_Modrm = 1,
		X86Flag_PlusR = 2,
		X86Flag_RegOP = 4,
		X86Flag_Imm8 = 8,
		X86Flag_Imm16 = 16,
		X86Flag_Imm32 = 32,
		X86Flag_Imm64 = 64,
		X86Flag_Reloc = 128
	};

	enum HSX86InsType
	{
		InsType_Normal = 0,
		InsType_Jump = 1,
		InsType_Return = 2,
		InsType_Call = 3
	};

	constexpr unsigned8 HS_X86_PREFIXES[] = { 0xF0, 0xF2, 0xF3, 0x2E, 0x36, 0x3E, 0x26, 0x64, 0x65, 0x66, 0x67 };
	constexpr unsigned8 HS_X86_REX_START = 0x40;
	constexpr unsigned8 HS_X86_REX_END = 0x4F;
	constexpr HSOpcodeInfo HS_X86_OPCODES[] =
	{
		/* ADD AL, imm8      */ {0x04, 0, X86Flag_Imm8, InsType_Normal},
		/* ADD EAX, imm32    */ {0x05, 0, X86Flag_Imm32, InsType_Normal},
		/* ADD RAX, imm32    */ {0x05, 0, X86Flag_Imm32, InsType_Normal},
		/* ADD r/m8, imm8    */ {0x80, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* ADD r/m32, imm32  */ {0x81, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* ADD r/m64, imm32  */ {0x81, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* ADD r/m32, imm8   */ {0x83, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* ADD r/m64, imm8   */ {0x83, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* ADD r/m8, r8      */ {0x00, 0, X86Flag_Modrm, InsType_Normal},
		/* ADD r/m32, r32    */ {0x01, 0, X86Flag_Modrm, InsType_Normal},
		/* ADD r/m64, r64    */ {0x01, 0, X86Flag_Modrm, InsType_Normal},
		/* ADD r8, r/m8      */ {0x02, 0, X86Flag_Modrm, InsType_Normal},
		/* ADD r32, r/m32    */ {0x03, 0, X86Flag_Modrm, InsType_Normal},
		/* ADD r64, r/m64    */ {0x03, 0, X86Flag_Modrm, InsType_Normal},

		/* AND AL, imm8      */ {0x24, 0, X86Flag_Imm8, InsType_Normal},
		/* AND EAX, imm32    */ {0x25, 0, X86Flag_Imm32, InsType_Normal},
		/* AND RAX, imm32    */ {0x25, 0, X86Flag_Imm32, InsType_Normal},
		/* AND r/m8, imm8    */ {0x80, 4, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* AND r/m32, imm32  */ {0x81, 4, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* AND r/m64, imm32  */ {0x81, 4, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* AND r/m32, imm8   */ {0x83, 4, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* AND r/m64, imm8   */ {0x83, 4, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* AND r/m8, r8      */ {0x20, 0, X86Flag_Modrm, InsType_Normal},
		/* AND r/m32, r32    */ {0x21, 0, X86Flag_Modrm, InsType_Normal},
		/* AND r/m64, r64    */ {0x21, 0, X86Flag_Modrm, InsType_Normal},
		/* AND r8, r/m8      */ {0x22, 0, X86Flag_Modrm, InsType_Normal},
		/* AND r32, r/m32    */ {0x23, 0, X86Flag_Modrm, InsType_Normal},
		/* AND r64, r/m64    */ {0x23, 0, X86Flag_Modrm, InsType_Normal},

		/* CALL rel32        */ {0xE8, 0, X86Flag_Imm32 | X86Flag_Reloc, InsType_Call},
		/* CALL r/m32        */ {0xFF, 2, X86Flag_Modrm | X86Flag_RegOP, InsType_Call},
		/* CALL r/m64        */ {0xFF, 2, X86Flag_Modrm | X86Flag_RegOP, InsType_Call},

		/* CMP r/m32, imm8   */ {0x83, 7, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* CMP r/m64, imm8   */ {0x83, 7, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* CMP r/m32, r32    */ {0x39, 0, X86Flag_Modrm, InsType_Normal},
		/* CMP r/m64, r64    */ {0x39, 0, X86Flag_Modrm, InsType_Normal},
		/* CMP imm16/32      */ {0x3D, 0, X86Flag_Imm32, InsType_Normal},

		/* DEC r/m32         */ {0xFF, 1, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* DEC r/m64         */ {0xFF, 1, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* DEC r32           */ {0x48, 0, X86Flag_PlusR, InsType_Normal},
		/* DEC r64           */ {0x48, 0, X86Flag_PlusR, InsType_Normal},

		/* ENTER imm16, imm8 */ {0xC8, 0, X86Flag_Imm16 | X86Flag_Imm8, InsType_Normal},
		/* FLD m32fp         */ {0xD9, 0, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* FLD m64fp         */ {0xDD, 0, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* FLD m80fp         */ {0xDB, 5, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* INT 3             */ {0xCC, 0, 0, InsType_Normal},

		/* JMP rel8          */ {0xEB, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JO rel8           */ {0x70, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JNO rel8          */ {0x71, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JB/JNAE/JC rel8   */ {0x72, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JNB/JAE/JNC rel8  */ {0x73, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JZ/JE rel8        */ {0x74, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JNZ/JNE rel8      */ {0x75, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JBE/JNA rel8      */ {0x76, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JNBE/JA rel8      */ {0x77, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JS rel8           */ {0x78, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JNS rel8          */ {0x79, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JP/JPE rel8       */ {0x7A, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JNP/JPO rel8      */ {0x7B, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JL/JNGE rel8      */ {0x7C, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JNL/JGE rel8      */ {0x7D, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JLE/JNG rel8      */ {0x7E, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},
		/* JNLE/JG rel8      */ {0x7F, 0, X86Flag_Imm8 | X86Flag_Reloc, InsType_Jump},

		/* JMP rel32         */ {0xE9, 0, X86Flag_Imm32 | X86Flag_Reloc, InsType_Jump},
		/* JMP r/m32         */ {0xFF, 4, X86Flag_Modrm | X86Flag_RegOP, InsType_Jump},
		/* JMP r/m64         */ {0xFF, 4, X86Flag_Modrm | X86Flag_RegOP, InsType_Jump},

		/* LEA r32,m         */ {0x8D, 0, X86Flag_Modrm, InsType_Normal},
		/* LEA r64,m         */ {0x8D, 0, X86Flag_Modrm, InsType_Normal},
		/* LEAVE             */ {0xC9, 0, 0, InsType_Normal},

		/* MOV r/m8,r8       */ {0x88, 0, X86Flag_Modrm, InsType_Normal},
		/* MOV r/m32,r32     */ {0x89, 0, X86Flag_Modrm, InsType_Normal},
		/* MOV r/m64,r64     */ {0x89, 0, X86Flag_Modrm, InsType_Normal},
		/* MOV r8,r/m8       */ {0x8A, 0, X86Flag_Modrm, InsType_Normal},
		/* MOV r32,r/m32     */ {0x8B, 0, X86Flag_Modrm, InsType_Normal},
		/* MOV r64,r/m64     */ {0x8B, 0, X86Flag_Modrm, InsType_Normal},
		/* MOV r/m16,Sreg    */ {0x8C, 0, X86Flag_Modrm, InsType_Normal},
		/* MOV Sreg,r/m16    */ {0x8E, 0, X86Flag_Modrm, InsType_Normal},
		/* MOV AL,moffs8     */ {0xA0, 0, X86Flag_Imm32, InsType_Normal},
		/* MOV EAX,moffs32   */ {0xA1, 0, X86Flag_Imm32, InsType_Normal},
		/* MOV RAX,moffs64   */ {0xA1, 0, X86Flag_Imm32, InsType_Normal},
		/* MOV moffs8,AL     */ {0xA2, 0, X86Flag_Imm32, InsType_Normal},
		/* MOV moffs32,EAX   */ {0xA3, 0, X86Flag_Imm32, InsType_Normal},
		/* MOV moffs64,RAX   */ {0xA3, 0, X86Flag_Imm32, InsType_Normal},
		/* MOV r8, imm8      */ {0xB0, 0, X86Flag_PlusR | X86Flag_Imm8, InsType_Normal},
		/* MOV r32, imm32    */ {0xB8, 0, X86Flag_PlusR | X86Flag_Imm32, InsType_Normal},
		/* MOV r64, imm64    */ {0xB8, 0, X86Flag_PlusR | X86Flag_Imm64, InsType_Normal},
		/* MOV r/m8, imm8    */ {0xC6, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* MOV r/m32, imm32  */ {0xC7, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* MOV r/m64, imm32  */ {0xC7, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},

		/* NOP               */ {0x90, 0, 0, InsType_Normal},

		/* OR AL, imm8       */ {0x0C, 0, X86Flag_Imm8, InsType_Normal},
		/* OR EAX, imm32     */ {0x0D, 0, X86Flag_Imm32, InsType_Normal},
		/* OR RAX, imm32     */ {0x0D, 0, X86Flag_Imm32, InsType_Normal},
		/* OR r/m8, imm8     */ {0x80, 1, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* OR r/m32, imm32   */ {0x81, 1, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* OR r/m64, imm32   */ {0x81, 1, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* OR r/m32, imm8    */ {0x83, 1, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* OR r/m64, imm8    */ {0x83, 1, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* OR r/m8, r8       */ {0x08, 0, X86Flag_Modrm, InsType_Normal},
		/* OR r/m32, r32     */ {0x09, 0, X86Flag_Modrm, InsType_Normal},
		/* OR r/m64, r64     */ {0x09, 0, X86Flag_Modrm, InsType_Normal},
		/* OR r8, r/m8       */ {0x0A, 0, X86Flag_Modrm, InsType_Normal},
		/* OR r32, r/m32     */ {0x0B, 0, X86Flag_Modrm, InsType_Normal},
		/* OR r64, r/m64     */ {0x0B, 0, X86Flag_Modrm, InsType_Normal},

		/* POP r/m32         */ {0x8F, 0, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* POP r/m64         */ {0x8F, 0, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* POP r32           */ {0x58, 0, X86Flag_PlusR, InsType_Normal},
		/* POP r64           */ {0x58, 0, X86Flag_PlusR, InsType_Normal},
		/* PUSH r/m32        */ {0xFF, 6, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* PUSH r/m64        */ {0xFF, 6, X86Flag_Modrm | X86Flag_RegOP, InsType_Normal},
		/* PUSH r32          */ {0x50, 0, X86Flag_PlusR, InsType_Normal},
		/* PUSH r64          */ {0x50, 0, X86Flag_PlusR, InsType_Normal},
		/* PUSH imm8         */ {0x6A, 0, X86Flag_Imm8, InsType_Normal},
		/* PUSH imm32        */ {0x68, 0, X86Flag_Imm32, InsType_Normal},

		/* RET               */ {0xC3, 0, 0, InsType_Return},
		/* RET imm16         */ {0xC2, 0, X86Flag_Imm16, InsType_Return},

		/* SUB AL, imm8      */ {0x2C, 0, X86Flag_Imm8, InsType_Normal},
		/* SUB EAX, imm32    */ {0x2D, 0, X86Flag_Imm32, InsType_Normal},
		/* SUB RAX, imm32    */ {0x2D, 0, X86Flag_Imm32, InsType_Normal},
		/* SUB r/m8, imm8    */ {0x80, 5, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* SUB r/m32, imm32  */ {0x81, 5, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* SUB r/m64, imm32  */ {0x81, 5, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* SUB r/m32, imm8   */ {0x83, 5, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* SUB r/m64, imm8   */ {0x83, 5, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* SUB r/m8, r8      */ {0x28, 0, X86Flag_Modrm, InsType_Normal},
		/* SUB r/m32, r32    */ {0x29, 0, X86Flag_Modrm, InsType_Normal},
		/* SUB r/m64, r64    */ {0x29, 0, X86Flag_Modrm, InsType_Normal},
		/* SUB r8, r/m8      */ {0x2A, 0, X86Flag_Modrm, InsType_Normal},
		/* SUB r32, r/m32    */ {0x2B, 0, X86Flag_Modrm, InsType_Normal},
		/* SUB r64, r/m64    */ {0x2B, 0, X86Flag_Modrm, InsType_Normal},

		/* TEST AL, imm8     */ {0xA8, 0, X86Flag_Imm8, InsType_Normal},
		/* TEST EAX, imm32   */ {0xA9, 0, X86Flag_Imm32, InsType_Normal},
		/* TEST RAX, imm32   */ {0xA9, 0, X86Flag_Imm32, InsType_Normal},
		/* TEST r/m8, imm8   */ {0xF6, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* TEST r/m32, imm32 */ {0xF7, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* TEST r/m64, imm32 */ {0xF7, 0, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* TEST r/m8, r8     */ {0x84, 0, X86Flag_Modrm, InsType_Normal},
		/* TEST r/m32, r32   */ {0x85, 0, X86Flag_Modrm, InsType_Normal},
		/* TEST r/m64, r64   */ {0x85, 0, X86Flag_Modrm, InsType_Normal},

		/* XOR AL, imm8      */ {0x34, 0, X86Flag_Imm8, InsType_Normal},
		/* XOR EAX, imm32    */ {0x35, 0, X86Flag_Imm32, InsType_Normal},
		/* XOR RAX, imm32    */ {0x35, 0, X86Flag_Imm32, InsType_Normal},
		/* XOR r/m8, imm8    */ {0x80, 6, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* XOR r/m32, imm32  */ {0x81, 6, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* XOR r/m64, imm32  */ {0x81, 6, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm32, InsType_Normal},
		/* XOR r/m32, imm8   */ {0x83, 6, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* XOR r/m64, imm8   */ {0x83, 6, X86Flag_Modrm | X86Flag_RegOP | X86Flag_Imm8, InsType_Normal},
		/* XOR r/m8, r8      */ {0x30, 0, X86Flag_Modrm, InsType_Normal},
		/* XOR r/m32, r32    */ {0x31, 0, X86Flag_Modrm, InsType_Normal},
		/* XOR r/m64, r64    */ {0x31, 0, X86Flag_Modrm, InsType_Normal},
		/* XOR r8, r/m8      */ {0x32, 0, X86Flag_Modrm, InsType_Normal},
		/* XOR r32, r/m32    */ {0x33, 0, X86Flag_Modrm, InsType_Normal},
		/* XOR r64, r/m64    */ {0x33, 0, X86Flag_Modrm, InsType_Normal} };

	bool HSx86Decoder::CheckBounds(signedP uCurrent, signedP sIncrement, signedP uMaxLen)
	{
		return (uCurrent >= 0) && (sIncrement >= 0) && ((uCurrent + sIncrement) <= uMaxLen);
	}

	signedP HSx86Decoder::ParsePrefixes(const ptrU8 pCode, signedP& sLen, signedP& sOperandSize)
	{
		signedP startLen = sLen;

		while (CheckBounds(sLen, 1, 15))
		{
			bool bFoundPrefix = false;

			for (size_t i = 0; i < sizeof(HS_X86_PREFIXES) / sizeof(HS_X86_PREFIXES[0]); i++)
			{
				if (pCode[sLen] == HS_X86_PREFIXES[i])
				{
					bFoundPrefix = true;

					if (HS_X86_PREFIXES[i] == 0x66)
					{
						sOperandSize = 2;
					}

					sLen++;
					break;
				}
			}

			if (!bFoundPrefix)
			{
				break;
			}
		}

		return sLen - startLen;
	}

	signedP HSx86Decoder::ParseREXPrefix(const ptrU8 pCode, signedP& sLen, signedP& sOperandSize)
	{
		signedP startLen = sLen;

#if defined(_M_X64) || defined(__x86_64__)
		if (CheckBounds(sLen, 1, 15) && pCode[sLen] >= HS_X86_REX_START && pCode[sLen] <= HS_X86_REX_END)
		{
			unsigned8 uRex = pCode[sLen++];

			if (uRex & 0x8) // REX.W = 1
			{
				sOperandSize = 8;
			}
		}
#endif

		return sLen - startLen;
	}

	bool HSx86Decoder::MatchOpcode(const ptrU8 pCode, signedP& sLen, signedP& sOpcodeIndex, signedP sOperandSize)
	{
		if (!CheckBounds(sLen, 0, 15))
		{
			return false;
		}

		for (unsignedP i = 0; i < sizeof(HS_X86_OPCODES) / sizeof(HS_X86_OPCODES[0]); i++)
		{
			bool bFoundOpcode = false;

			if (pCode[sLen] == HS_X86_OPCODES[i].uOpcode)
			{
				if (HS_X86_OPCODES[i].uFlags & X86Flag_RegOP)
				{
					if (!CheckBounds(sLen, 2, 15))
					{
						continue;
					}

					unsigned8 uModRM = pCode[sLen + 1];
					unsigned8 uRegOp = (uModRM >> 3) & 0x07;
					bFoundOpcode = (uRegOp == HS_X86_OPCODES[i].uRegOpcode);
				}
				else
				{
					bFoundOpcode = true;
				}
			}

			if (!bFoundOpcode && (HS_X86_OPCODES[i].uFlags & X86Flag_PlusR))
			{
				if ((pCode[sLen] & 0xF8) == HS_X86_OPCODES[i].uOpcode)
				{
					bFoundOpcode = true;
				}
			}

			if (bFoundOpcode)
			{
#if defined(_M_X64) || defined(__x86_64__)
				if ((HS_X86_OPCODES[i].uFlags & X86Flag_Imm64) && sOperandSize != 8)
					continue;
#endif
				sOpcodeIndex = static_cast<signedP>(i);
				sLen++;
				return true;
			}
		}
		return false;
	}

	void HSx86Decoder::SetRelocationInfo(HSInsInfo& stInsInfo, const HSOpcodeInfo& stOpcodeInfo, signedP& sLen, signedP sOperandSize)
	{
		if (stOpcodeInfo.uFlags & X86Flag_Reloc)
		{
			stInsInfo.bNeedReloc = true;
			stInsInfo.sRelocOffset = sLen;

			if (stOpcodeInfo.uFlags & X86Flag_Imm8)
			{
				stInsInfo.sRelocSize = 1;
			}
			else if (stOpcodeInfo.uFlags & X86Flag_Imm32)
			{
				stInsInfo.sRelocSize = 4;
			}
			else if (stOpcodeInfo.uFlags & X86Flag_Imm16)
			{
				stInsInfo.sRelocSize = 2;
			}
			else if (stOpcodeInfo.uFlags & X86Flag_Imm64)
			{
				stInsInfo.sRelocSize = 8;
			}
		}
	}

	void HSx86Decoder::SetInstructionType(HSInsInfo& stInsInfo, const HSOpcodeInfo& stOpcodeInfo)
	{
		stInsInfo.bIsJmp = (stOpcodeInfo.uInsType == InsType_Jump);
		stInsInfo.bIsRet = (stOpcodeInfo.uInsType == InsType_Return);
		stInsInfo.bIsCall = (stOpcodeInfo.uInsType == InsType_Call);
	}

	bool HSx86Decoder::ParseModRM(const ptrU8 pCode, signedP& sLen, HSInsInfo& stInsInfo, signedP sOperandSize)
	{
		stInsInfo.bHasModrm = true;
		stInsInfo.sModrmOffset = sLen;

		if (!CheckBounds(sLen, 1, 15))
		{
			return false;
		}

		unsigned8 uModrm = pCode[sLen++];
		unsigned8 uMod = uModrm >> 6;
		unsigned8 uRM = uModrm & 0x07;

		if (uMod == 3)
		{
			return true;
		}

#if defined(_M_X64) || defined(__x86_64__)
		if (uMod == 0 && uRM == 5)
		{
			if (!CheckBounds(sLen, 4, 15))
				return false;
			sLen += 4;
			return true;
		}
#endif

		if (uRM == 4)
		{
			if (!CheckBounds(sLen, 1, 15))
			{
				return false;
			}

			unsigned8 uSib = pCode[sLen++];
			unsigned8 uBase = uSib & 0x07;
			unsigned8 uIndex = (uSib >> 3) & 0x07;
			unsigned8 uScale = (uSib >> 6) & 0x03;

			if (uMod == 0)
			{
				if (uBase == 5)
				{
					if (!CheckBounds(sLen, 4, 15))
						return false;
					sLen += 4;
				}
			}
			else if (uMod == 1)
			{
				if (!CheckBounds(sLen, 1, 15))
					return false;
				sLen += 1;
			}
			else if (uMod == 2)
			{
				if (!CheckBounds(sLen, 4, 15))
					return false;
				sLen += 4;
			}
		}
		else
		{
			if (uMod == 1)
			{
				if (!CheckBounds(sLen, 1, 15))
					return false;
				sLen += 1;
			}
			else if (uMod == 2)
			{
				if (!CheckBounds(sLen, 4, 15))
					return false;
				sLen += 4;
			}
		}

		return true;
	}

	void HSx86Decoder::ParseImmediate(const HSOpcodeInfo& stOpcodeInfo, signedP& sLen, signedP sOperandSize, HSInsInfo& pInfo)
	{
		if (stOpcodeInfo.uFlags & X86Flag_Imm8)
		{
			if (CheckBounds(sLen, 1, 15))
			{
				pInfo.bHasImmediate = true;
				pInfo.sImmOffset = sLen;
				pInfo.sImmSize = 1;
				sLen += 1;
			}
		}
		else if (stOpcodeInfo.uFlags & X86Flag_Imm16)
		{
			if (CheckBounds(sLen, 2, 15))
			{
				pInfo.bHasImmediate = true;
				pInfo.sImmOffset = sLen;
				pInfo.sImmSize = 2;
				sLen += 2;
			}
		}
		else if (stOpcodeInfo.uFlags & X86Flag_Imm32)
		{
			signedP immSize = 4;

			if (CheckBounds(sLen, immSize, 15))
			{
				pInfo.bHasImmediate = true;
				pInfo.sImmOffset = sLen;
				pInfo.sImmSize = immSize;
				sLen += immSize;
			}
		}
		else if (stOpcodeInfo.uFlags & X86Flag_Imm64)
		{
			if (CheckBounds(sLen, 8, 15))
			{
				pInfo.bHasImmediate = true;
				pInfo.sImmOffset = sLen;
				pInfo.sImmSize = 8;
				sLen += 8;
			}
		}
	}

	void HSx86Decoder::InitializeInsInfo(HSInsInfo& stInsInfo)
	{
		stInsInfo.sTotalSize = 0;
		stInsInfo.bNeedReloc = false;
		stInsInfo.sRelocOffset = -1;
		stInsInfo.sRelocSize = 0;
		stInsInfo.bHasImmediate = false;
		stInsInfo.sImmOffset = -1;
		stInsInfo.sImmSize = 0;
		stInsInfo.bHasModrm = false;
		stInsInfo.sModrmOffset = -1;
		stInsInfo.bIsJmp = false;
		stInsInfo.bIsRet = false;
		stInsInfo.bIsCall = false;
	}

	bool HSx86Decoder::ParseCode(const ptrAny pIns, HSInsInfo& stInsInfo)
	{
		if (pIns == nullptr)
		{
			return false;
		}

		signedP sLen = 0;
		signedP sOperandSize =
#if defined(_M_X64) || defined(__x86_64__)
			8;
#else
			4;
#endif
		signedP sOpcodeIndex = -1;
		const ptrU8 pCode = (ptrU8)pIns;

		InitializeInsInfo(stInsInfo);
		ParsePrefixes(pCode, sLen, sOperandSize);
		ParseREXPrefix(pCode, sLen, sOperandSize);

		if (!MatchOpcode(pCode, sLen, sOpcodeIndex, sOperandSize))
		{
			return false;
		}

		if (sOpcodeIndex < 0 || static_cast<size_t>(sOpcodeIndex) >= sizeof(HS_X86_OPCODES) / sizeof(HS_X86_OPCODES[0]))
		{
			return false;
		}

		const HSOpcodeInfo& stMatchedOpcode = HS_X86_OPCODES[sOpcodeIndex];
		SetInstructionType(stInsInfo, stMatchedOpcode);
		SetRelocationInfo(stInsInfo, stMatchedOpcode, sLen, sOperandSize);

		if (stMatchedOpcode.uFlags & X86Flag_Modrm)
		{
			if (!ParseModRM(pCode, sLen, stInsInfo, sOperandSize))
			{
				return false;
			}
		}

		ParseImmediate(stMatchedOpcode, sLen, sOperandSize, stInsInfo);

		if (sLen > 15 || sLen <= 0)
		{
			return false;
		}

		stInsInfo.sTotalSize = sLen;
		return true;
	}

	bool HSx86Decoder::CallJmpConvert(const HSInsInfo& stInfoBefore, unsignedP uPosBefore, ptrU8 pInsBefore,
		HSInsInfo& stInfoAfter, unsignedP uPosAfter, ptrU8 pInsAfter)
	{
		memcpy(pInsAfter, pInsBefore, stInfoBefore.sTotalSize);
		stInfoAfter = stInfoBefore;

		if (!stInfoBefore.bIsJmp && !stInfoBefore.bIsCall)
		{
			return false;
		}

		if (!stInfoBefore.bHasImmediate && !stInfoBefore.bHasModrm)
		{
			return false;
		}

		unsignedP uTargetAddr = 0;

		if (stInfoBefore.bHasImmediate)
		{
			switch (stInfoBefore.sImmSize)
			{
			case 1:
			{
				signed8 sOffset = *(signed8*)(pInsBefore + stInfoBefore.sImmOffset);
				uTargetAddr = uPosBefore + stInfoBefore.sTotalSize + sOffset;
				break;
			}
			case 2:
			{
				signed16 sOffset = *(signed16*)(pInsBefore + stInfoBefore.sImmOffset);
				uTargetAddr = uPosBefore + stInfoBefore.sTotalSize + sOffset;
				break;
			}
			case 4:
			{
				signed32 sOffset = *(signed32*)(pInsBefore + stInfoBefore.sImmOffset);
#if defined(_M_X64) || defined(__x86_64__)
				uTargetAddr = uPosBefore + stInfoBefore.sTotalSize + (signed64)sOffset;
#else
				uTargetAddr = uPosBefore + stInfoBefore.sTotalSize + sOffset;
#endif
				break;
			}
			case 8:
			{
				uTargetAddr = *(unsignedP*)(pInsBefore + stInfoBefore.sImmOffset);
				break;
			}
			default:
				return false;
			}
		}
		else if (stInfoBefore.bHasModrm)
		{
			return false;
		}

		unsignedP uNewInstructionEnd = uPosAfter + 5;
#if defined(_M_X64) || defined(__x86_64__)
		signed64 sNewOffset = (signed64)uTargetAddr - (signed64)uNewInstructionEnd;
		bool bCanUseRelative = (sNewOffset >= -0x7FFFFFFFLL && sNewOffset <= 0x7FFFFFFFLL);
#else
		signed32 sNewOffset = (signed32)uTargetAddr - (signed32)uNewInstructionEnd;
		bool bCanUseRelative = true;
#endif

		if (!bCanUseRelative)
		{
			return false;
		}

		if (stInfoBefore.bIsJmp)
		{
			pInsAfter[0] = 0xE9; // JMP rel32
		}
		else if (stInfoBefore.bIsCall)
		{
			pInsAfter[0] = 0xE8; // CALL rel32
		}
		else
		{
			return false;
		}

		*(signed32*)(pInsAfter + 1) = (signed32)sNewOffset;

		InitializeInsInfo(stInfoAfter);
		stInfoAfter.bNeedReloc = true;
		stInfoAfter.bHasImmediate = true;
		stInfoAfter.bIsJmp = stInfoBefore.bIsJmp;
		stInfoAfter.bIsCall = stInfoBefore.bIsCall;
		stInfoAfter.sTotalSize = 5;
		stInfoAfter.sRelocOffset = 1;
		stInfoAfter.sRelocSize = 4;
		stInfoAfter.sImmOffset = 1;
		stInfoAfter.sImmSize = 4;

		return true;
	}
}

#endif