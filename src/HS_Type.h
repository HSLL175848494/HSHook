#pragma once

namespace HSLL
{
	/// 8-bit signed integer
	using signed8 = char;
	/// 16-bit signed integer
	using signed16 = short;
	/// 32-bit signed integer
	using signed32 = int;
	/// 64-bit signed integer
	using signed64 = long long;

	/// 8-bit unsigned integer
	using unsigned8 = unsigned char;
	/// 16-bit unsigned integer
	using unsigned16 = unsigned short;
	/// 32-bit unsigned integer
	using unsigned32 = unsigned int;
	/// 64-bit unsigned integer
	using unsigned64 = unsigned long long;

	/// Pointer-sized 8-bit signed integer
	using ptrS8 = signed8 *;
	/// Pointer-sized 16-bit signed integer
	using ptrS16 = signed16 *;
	/// Pointer-sized 32-bit signed integer
	using ptrS32 = signed32 *;
	/// Pointer-sized 64-bit signed integer
	using ptrS64 = signed64 *;

	/// Pointer-sized 8-bit unsigned integer
	using ptrU8 = unsigned8 *;
	/// Pointer-sized 16-bit unsigned integer
	using ptrU16 = unsigned16 *;
	/// Pointer-sized 32-bit unsigned integer
	using ptrU32 = unsigned32 *;
	/// Pointer-sized 64-bit unsigned integer
	using ptrU64 = unsigned64 *;

	/// Generic pointer type (void pointer)
	using ptrAny = void *;

	/**
	 * @brief Template for architecture-dependent integer types
	 * @tparam Arch Architecture bit size (4 or 8)
	 */
	template <int Arch>
	struct ArchTypes
	{
	};

	/**
	 * @brief Specialization for 32-bit architecture
	 */
	template <>
	struct ArchTypes<4>
	{
		using signedP = signed32;
		using unsignedP = unsigned32;
	};

	/**
	 * @brief Specialization for 64-bit architecture
	 */
	template <>
	struct ArchTypes<8>
	{
		using signedP = signed64;
		using unsignedP = unsigned64;
	};

	/// Native pointer-sized signed integer (automatically selected based on architecture)
	using signedP = ArchTypes<sizeof(ptrAny)>::signedP;
	/// Native pointer-sized unsigned integer (automatically selected based on architecture)
	using unsignedP = ArchTypes<sizeof(ptrAny)>::unsignedP;

} // namespace HSLL