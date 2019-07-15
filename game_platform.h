#ifndef GAME_PLATFORM_H
#define GAME_PLATFORM_H

// NOTE(ivan): Compiler detection.
#if defined(_MSC_VER)
#    define MSVC 1
#else
#    error Unsupported compiler!
#endif

// NOTE(ivan): Target CPU architecture detection.
#if MSVC
#    if defined(_M_IX86) || defined(_M_X64) || defined(_M_AMD64)
#        define INTEL86 1
#        define INTELORDER 1
#        define AMIGAORDER 0
#        if defined(_M_X64) || defined(_M_AMD64)
#            define X32CPU 0
#            define X64CPU 1
#        else
#            define X32CPU 1
#            define X64CPU 0
#        endif
#    else
#        error Unsupported target CPU architecture!
#    endif
#endif

#if X32CPU
#    define IsTargetCPU32Bit() true
#    define IsTargetCPU64Bit() false
#elif X64CPU
#    define IsTargetCPU32Bit() false
#    define IsTargetCPU64Bit() true
#endif
#if INTERNAL
#    define IsInternal() true
#else
#    define IsInternal() false
#endif
#if SLOWCODE
#    define IsSlowCode() true
#else
#    define IsSlowCode() false
#endif

// NOTE(ivan): C standard includes.
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>

// NOTE(ivan): C intrinsics.
#if MSVC
#    include <intrin.h>
#endif

// NOTE(ivan): General types.
typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long int u32;
typedef unsigned long long u64;

typedef signed char s8;
typedef signed short int s16;
typedef signed long int s32;
typedef signed long long s64;

#if X32CPU
typedef u32 uptr;
typedef s32 sptr;
#elif X64CPU
typedef u64 uptr;
typedef s64 sptr;
#endif

typedef float f32;
typedef double f64;

typedef u32 b32;

// NOTE(ivan): Tells the compiler not to cry about a given variable that is really unused.
#define UnusedParam(Param) (Param)

// NOTE(ivan): Collects multiple arguments into a single buffer.
#define CollectArgs(Buffer, Format)										\
	do {va_list Args; va_start(Args, Format); vsprintf(Buffer, Format, Args); va_end(Args);} while(0)
#define CollectArgsN(Buffer, BufferSize, Format)						\
	do {va_list Args; va_start(Args, Format); vsnprintf(Buffer, BufferSize, Format, Args); va_end(Args);} while(0)

// NOTE(ivan): Calculates a given array's elements count.
template <typename T, u32 N> inline constexpr u32
ArraySize(T (&Array)[N]) {
	UnusedParam(Array);
	return N;
}

// NOTE(ivan): Min/max.
template <typename T> inline T
Min(T A, T B) {
	return A <= B ? A : B;
}
template <typename T> inline T
Max(T A, T B) {
	return A > B ? A : B;
}

template <typename T> inline T
Min3(T A, T B, T C) {
	return Min(Min(A, B), C);
}
template <typename T> inline T
Max3(T A, T B, T C) {
	return Max(Max(A, B), C);
}

// NOTE(ivan): Clamp.
template <typename T> inline T
Clamp(T MinValue, T MaxValue, T Value) {
	return Value < MinValue ? MinValue : Value > MaxValue ? MaxValue : Value;
}

// NOTE(ivan): Swaps data.
template <typename T> inline void
Swap(T *A, T *B) {
	T Tmp = *A;
	*A = *B;
	*B = Tmp;
}

// NOTE(ivan): Indicates that something is missing or the operation cannot be completed.
// NOTE(ivan): Usually is returned by functions that returns a signed integer value or a "handle".
#define NOTFOUND ((s32)-1)

// NOTE(ivan): Causes access violation exception which allows to break into the debugger if any.
#define BreakDebugger() do {*((s32 *)0) = 1;} while(0)

// NOTE(ivan): Debug assertions.
#if SLOWCODE
#    define Assert(Expression) do {if (!(Expression)) BreakDebugger();} while(0)
#else
#    define Assert(Expression)
#endif
#if SLOWCODE
#    define Verify(Expression) Assert(Expression)
#else
#    define Verify(Expression) (Expression)
#endif

// NOTE(ivan): Does not allow to compile the program in public release builds.
#if INTERNAL
#    define NotImplemented() Assert(!"Not implemented!!!")
#else
#    define NotImplemented() NotImplemented!!!!!!!!!!!!!
#endif

// NOTE(ivan): Helper macros for code pieces that are not intended to run at all.
#if INTERNAL
#    define InvalidCodePath() Assert(!"Invalid code path!!!")
#else
#    define InvalidCodePath() do {} while(0)
#endif
#define InvalidDefaultCase default: {InvalidCodePath();} break

// NOTE(ivan): Safe truncation.
inline u32
SafeTruncateU64(u64 Value) {
	Assert(Value <= 0xFFFFFFFF);
	return (u32)Value;
}
inline u16
SafeTruncateU32(u32 Value) {
	Assert(Value <= 0xFFFF);
	return (u16)Value;
}
inline u8
SafeTruncateU16(u16 Value) {
	Assert(Value <= 0xFF);
	return (u8)Value;
}

// NOTE(ivan): Measurings.
#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

// NOTE(ivan): Is power of two?
template <typename T> inline b32
IsPow2(T Value) {
	return ((Value & ~(Value - 1)) == Value);
}

// NOTE(ivan): Values alignment.
template <typename T> inline T
AlignPow2(T Value, T Alignment) {
	Assert(IsPow2(Alignment));
	return ((Value + (Alignment - 1)) & ~(Alignment - 1));
}
#define Align4(Value) (((Value) + 3) & ~3)
#define Align8(Value) (((Value) + 7) & ~7)
#define Align16(Value) (((Value) + 15) & ~15)

// NOTE(ivan): Bit scan result.
struct bit_scan_result {
	b32 IsFound;
	u32 Index;
};

// NOTE(ivan): Bit scan.
inline bit_scan_result
FindLeastSignificantBit(u32 Value)
{
	bit_scan_result Result = {};
	
#if MSVC
	Result.IsFound = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
	for (u32 Test = 0; Test < 32; Test++) {
		if (Value & (1 << Test)) {
			Result.IsFound = true;
			Result.Index = Test;
			break;
		}
	}
#endif	

	return Result;
}
inline bit_scan_result
FindMostSignificantBit(u32 Value)
{
	bit_scan_result Result = {};

#if MSVC
	Result.IsFound = _BitScanReverse((unsigned long *)&Result.Index, Value);
#else
	for (s32 Test = 31; Test >= 0; Test++) {
		if (Value & (1 << Test)) {
			Result.IsFound = true;
			Result.Index = Test;
			break;
		}
	}
#endif

	return Result;
}

// NOTE(ivan): Counts set bits in a mask.
inline u32
CountSetBits(uptr Mask) {
	u32 Result = 0;
	u32 LeftShift = sizeof(uptr) * 8 - 1;
	uptr BitTest = ((uptr)1 << LeftShift);

	for (u32 Index = 0; Index <= LeftShift; Index++) {
		Result += ((Mask & BitTest) ? 1 : 0);
		BitTest /= 2;
	}

	return Result;
}

// NOTE(ivan): Generic memory piece.
struct piece {
	u8 *Base;
	uptr Size;
};

// NOTE(ivan): Consumes a chunk from a memory piece.
#define ConsumeType(Piece, Type) (Type *)ConsumeSize(Piece, sizeof(Type))
#define ConsumeTypeArray(Piece, Type, Count) (Type *)ConsumeSize(Piece, sizeof(Type) * Count)
inline u8 * ConsumeSize(piece *Piece, uptr Size) {
	Assert(Piece);
	Assert(Piece->Size >= Size);
	Assert(Size);
	
	u8 *Result = Piece->Base;
	Piece->Base = (Piece->Base + Size);
	Piece->Size -= Size;

	return Result;
}

// NOTE(ivan): Endian-ness utilities.
inline void
SwapEndianU32(u32 *Value) {
	Assert(Value);

#if MSVC
	*Value = _byteswap_ulong(*Value);
#else	
	u32 V = *Value;
	*Value = ((V << 24) | ((V & 0xFF00) << 8) | ((V >> 8) & 0xFF00) | (V >> 24));
#endif	
}
inline void
SwapEndianU16(u16 *Value) {
	Assert(Value);

#if MSVC
	*Value = _byteswap_ushort(*Value);
#else
	u16 V = *Value;
	*Value = ((V << 8) || (V  >> 8));
#endif
}

// NOTE(ivan): 4-character-code.
#define FourCC(String) ((u32)((String[3] << 0) | (String[2] << 8) | (String[1] << 16) | (String[0] << 24)))
#define FastFourCC(String) (*(u32 *)(String)) // NOTE(ivan): Does not work with switch/case.

// NOTE(ivan): Memory barriers.
#if MSVC
inline void CompleteWritesBeforeFutureWrites(void) {_WriteBarrier(); _mm_sfence();}
inline void CompleteReadsBeforeFutureReads(void) {_ReadBarrier(); _mm_lfence();}
#endif

// NOTE(ivan): Interlocked operations.
#if MSVC
inline u32 AtomicIncrementU32(volatile u32 *Value) {return _InterlockedIncrement((volatile long *)Value);}
inline u64 AtomicIncrementU64(volatile u64 *Value) {return _InterlockedIncrement64((volatile __int64 *)Value);}
inline u32 AtomicDecrementU32(volatile u32 *Value) {return _InterlockedDecrement((volatile long *)Value);}
inline u64 AtomicDecrementU64(volatile u64 *Value) {return _InterlockedDecrement64((volatile __int64 *)Value);}
inline u32 AtomicExchangeU32(volatile u32 *Target, u32 Value) {return _InterlockedExchange((volatile long *)Target, Value);}
inline u64 AtomicExchangeU64(volatile u64 *Target, u64 Value) {return _InterlockedExchange64((volatile __int64 *)Target, Value);}
inline u32 AtomicCompareExchangeU32(volatile u32 *Value, u32 NewValue, u32 Exp) {return _InterlockedCompareExchange((volatile long *)Value, NewValue, Exp);}
inline u64 AtomicCompareExchangeU64(volatile u64 *Value, u64 NewValue, u64 Exp) {return _InterlockedCompareExchange64((volatile __int64 *)Value, NewValue, Exp);}
#endif

// NOTE(ivan): Yield processor, give its time to other threads.
#if MSVC
inline void YieldProcessor(void) {_mm_pause();}
#endif

// NOTE(ivan): Cross-platform ticket-mutex.
// NOTE(ivan): Any instance of this structure MUST be ZERO-initialized for proper
// functioning of EnterTicketMutex()/LeaveTicketMutex() functions.
struct ticket_mutex {
	volatile u64 Ticket;
	volatile u64 Serving;
};

// NOTE(ivan): Ticket-mutex locking/unlocking.
inline void
EnterTicketMutex(ticket_mutex *Mutex) {
	Assert(Mutex);
	AtomicIncrementU64(&Mutex->Ticket);
	
	u64 Ticket = Mutex->Ticket - 1;
	while (Ticket != Mutex->Serving)
		YieldProcessor();
}
inline void
LeaveTicketMutex(ticket_mutex *Mutex) {
	Assert(Mutex);
	AtomicIncrementU64(&Mutex->Serving);
}

// NOTE(ivan): CPU information.
struct cpu_info {
	b32 IsIntel;
	b32 IsAMD;

	// NOTE(ivan): Identification strings.
	char VendorName[13];
	char BrandName[49];

	f32 ClockSpeed; // NOTE(ivan): In GHz.

	// NOTE(ivan): Features and instruction sets.
	b32 SupportsMMX;
	b32 SupportsMMXExt;
	b32 Supports3DNow;
	b32 Supports3DNowExt;
	b32 SupportsSSE;
	b32 SupportsSSE2;
	b32 SupportsSSE3;
	b32 SupportsSSSE3;
	b32 SupportsSSE4_1;
	b32 SupportsSSE4_2;
	b32 SupportsSSE4A;
	b32 SupportsHT;

	// NOTE(ivan): Cores information.
	u32 NumCores;
	u32 NumCoreThreads;

	// NOTE(ivan): Cache information.
	u32 NumL1;
	u32 NumL2;
	u32 NumL3;

	u32 NumNUMA;
};

// NOTE(ivan): File handle.
typedef s32 file_handle; // NOTE(ivan): In case of fail file_handle-returning functions return NOTFOUND.

// NOTE(ivan): File access type.
enum file_access_type {
    FileAccessType_OpenForReading = (1 << 0),
	FileAccessType_OpenForWriting = (1 << 1)
};	

// NOTE(ivan): File seek origin.
enum file_seek_origin {
    FileSeekOrigin_Begin,
	FileSeekOrigin_Current,
	FileSeekOrigin_End
};

// NOTE(ivan): Platform-specific interface prototypes.
#define PLATFORM_CHECK_PARAM(Name) s32 Name(const char *Param)
typedef PLATFORM_CHECK_PARAM(platform_check_param);

#define PLATFORM_CHECK_PARAM_VALUE(Name) const char * Name(const char *Param)
typedef PLATFORM_CHECK_PARAM_VALUE(platform_check_param_value);

#define PLATFORM_OUTF(Name) void Name(const char *Format, ...)
typedef PLATFORM_OUTF(platform_outf);

#define PLATFORM_CRASHF(Name) void Name(const char *Format, ...)
typedef PLATFORM_CRASHF(platform_crashf);

#define PLATFORM_FOPEN(Name) file_handle Name(const char *FileName, file_access_type AccessType)
typedef PLATFORM_FOPEN(platform_fopen);

#define PLATFORM_FCLOSE(Name) void Name(file_handle FileHandle)
typedef PLATFORM_FCLOSE(platform_fclose);

#define PLATFORM_FREAD(Name) u32 Name(file_handle FileHandle, void *Buffer, u32 Size)
typedef PLATFORM_FREAD(platform_fread);

#define PLATFORM_FWRITE(Name) u32 Name(file_handle FileHandle, void *Buffer, u32 Size)
typedef PLATFORM_FWRITE(platform_fwrite);

#define PLATFORM_FSEEK(Name) b32 Name(file_handle FileHandle, uptr Size, file_seek_origin SeekOrigin, uptr *NewPos)
typedef PLATFORM_FSEEK(platform_fseek);

#define PLATFORM_FFLUSH(Name) void Name(file_handle FileHandle)
typedef PLATFORM_FFLUSH(platform_fflush);

// NOTE(ivan): Platform-specific interface.
struct platform_api {
	// NOTE(ivan): Generic-purpose methods.
	platform_check_param *CheckParam; // NOTE(ivan): Returns NOTFOUND in case a given parameter is missing.
	platform_check_param_value *CheckParamValue;
	platform_outf *Outf;
	platform_crashf *Crashf;

	// NOTE(ivan): File I/O methods.
	platform_fopen *FOpen;
	platform_fclose *FClose;
	platform_fread *FRead;
	platform_fwrite *FWrite;
	platform_fseek *FSeek;
	platform_fflush *FFlush;

	// NOTE(ivan): Quit flags (corresponding functions QuitGame() and RestartGame() are located in game.h header file).
	b32 QuitRequested; // NOTE(ivan): Set to true to quit from primary loop at the end of current frame.
	s32 QuitReturnCode;
	b32 QuitToRestart; // NOTE(ivan): Set to true to start again the program after quit.

	// NOTE(ivan): Executable's file name in various forms.
	const char *ExecutableName;      // NOTE(ivan): File name with extension.
	const char *ExecutableNameNoExt; // NOTE(ivan): File name without extension.
	const char *ExecutablePath;      // NOTE(ivan): File path without name.

	// NOTE(ivan): "Shared name": all game modules contains this shared piece of name.
	// F.e: if the shared name is "quantic", then the executable is "runquantic", game core module is "quantic",
	// game entities module is "quantic_ents", and game editor module is "quantic_ed".
	const char *SharedName;

	// NOTE(ivan): Current working directory.
	const char *CurrentPath;

	// NOTE(ivan): CPU information.
	cpu_info CPUInfo;
};	

#endif // #ifndef GAME_PLATFORM_H
