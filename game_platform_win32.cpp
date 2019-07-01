#include "game.h"

// Win32-specific CRT extensions.
#include <crtdbg.h>

// NOTE(ivan): Win32 visual styles enable.
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// NOTE(ivan): Win32 API versions definitions.
#include <sdkddkver.h>
#undef _WIN32_WINNT
#undef _WIN32_IE
#undef NTDDI_VERSION

// NOTE(ivan): Win32 API target version is: Windows 7.
#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define _WIN32_IE _WIN32_IE_WIN7
#define NTDDI_VERSION NTDDI_VERSION_FROM_WIN32_WINNT(_WIN32_WINNT)

// NOTE(ivan): Win32 API strict mode enable.
#define STRICT

// NOTE(ivan): Win32 API rarely-used routines exclusion.
//
// WIN32_LEAN_AND_MEAN:          keep the api header being a minimal set of mostly-required declarations and includes
// OEMRESOURCE:                  exclude OEM resource values (dunno wtf is that, but never needed them...)
// NOATOM:                       exclude atoms and their api (barely used today obsolete technique of pooling strings)
// NODRAWTEXT:                   exclude DrawText() and DT_* definitions
// NOMETAFILE:                   exclude METAFILEPICT (yet another windows weirdo we don't need)
// NOMINMAX:                     exclude min() & max() macros (we have our own)
// NOOPENFILE:                   exclude OpenFile(), OemToAnsi(), AnsiToOem(), and OF_* definitions (useless for us)
// NOSCROLL:                     exclude SB_* definitions and scrolling routines
// NOSERVICE:                    exclude Service Controller routines, SERVICE_* equates, etc...
// NOSOUND:                      exclude sound driver routines (we'd rather use OpenAL or another thirdparty API)
// NOTEXTMETRIC:                 exclude TEXTMETRIC and associated routines
// NOWH:                         exclude SetWindowsHook() and WH_* defnitions
// NOCOMM:                       exclude COMM driver routines
// NOKANJI:                      exclude Kanji support stuff
// NOHELP:                       exclude help engine interface
// NOPROFILER:                   exclude profiler interface
// NODEFERWINDOWPOS:             exclude DeferWindowPos() routines
// NOMCX:                        exclude Modem Configuration Extensions (modems in 2019, really?)
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#define NOATOM
#define NODRAWTEXT
#define NOMETAFILE
#define NOMINMAX
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

// NOTE(ivan): Win32 API includes.
#include <windows.h>
#include <objbase.h>
#include <commctrl.h>
#include <versionhelpers.h>
#include <mmsystem.h>

// NOTE(ivan): Win32 XInput includes.
#include <xinput.h>

// NOTE(ivan): Win32 XInput prototypes.
#define X_INPUT_GET_BATTERY_INFORMATION(Name) DWORD WINAPI Name(DWORD UserIndex, BYTE DevType, XINPUT_BATTERY_INFORMATION *BatteryInformation)
typedef X_INPUT_GET_BATTERY_INFORMATION(x_input_get_battery_information);

#define X_INPUT_GET_STATE(Name) DWORD WINAPI Name(DWORD UserIndex, XINPUT_STATE *State)
typedef X_INPUT_GET_STATE(x_input_get_state);

#define X_INPUT_SET_STATE(Name) DWORD WINAPI Name(DWORD UserIndex, XINPUT_VIBRATION *Vibration)
typedef X_INPUT_SET_STATE(x_input_set_state);

// NOTE(ivan): Win32 XInput module structure.
struct win32_xinput_module {
	b32 IsValid; // NOTE(ivan): False if something went wrong and the XInput module is not loaded.
	HMODULE XInputLibrary;

	x_input_get_battery_information *GetBatteryInformation;
	x_input_get_state *GetState;
	x_input_set_state *SetState;
};

// NOTE(ivan): Win32-specific Steamworks module structure.
struct win32_steamworks_module {
	b32 IsValid; // NOTE(ivan): False if something went wrong and the XInput module is not loaded.
	HMODULE SteamLibrary;
	
	steamworks_api SteamworksAPI;
};

// NOTE(ivan): Win32-specific game module structure.
struct win32_game_module {
	b32 IsValid; // NOTE(ivan): False if something went wrong and the game module is not loaded. 
	HMODULE GameLibrary;

	game_trigger *GameTrigger;
};

// NOTE(ivan): Win32 globals.
static struct {
	HINSTANCE Instance;

	s32 ArgC;
	char **ArgV;

	u64 PerformanceFrequency;

	UINT QueryCancelAutoplay; // NOTE(ivan): Special event for disabling disc autoplay feaature.

	b32 IsDebugCursor;
	b32 IsDebuggerActive; // NOTE(ivan): Indicates whether the program is running under the debugger.
	b32 IsWindowActive; // NOTE(ivan): Indicates whether the main window is active or not (focused/not focused).

	// NOTE(ivan): Game input, needs to be global to be accessed by Win32WindowProc()'s raw input routines.
	game_input *GameInput;
} Win32State;

// NOTE(ivan): Win32-specific system structure for setting thread name by Win32SetThreadName.
struct win32_thread_name {
	DWORD Type; // NOTE(ivan): Must be 0x1000.
	LPCSTR Name;
	DWORD ThreadId;
	DWORD Flags;
};

static void
Win32SetThreadName(DWORD ThreadId, LPCSTR Name) {
	Assert(Name);

	win32_thread_name ThreadName = {};
	ThreadName.Type = 0x1000;
	ThreadName.Name = Name;
	ThreadName.ThreadId = ThreadId;

#pragma warning(push)
#pragma warning(disable: 6320 6322)
	__try {
		RaiseException(0x406d1388, 0, sizeof(ThreadName) / sizeof(ULONG_PTR), (ULONG_PTR *)&ThreadName);
	} __except(EXCEPTION_EXECUTE_HANDLER) {}
#pragma warning(pop)
}

inline u64
Win32GetClock(void) {
	LARGE_INTEGER Result;
	Verify(QueryPerformanceCounter(&Result));

	return Result.QuadPart;
}

inline f32
Win32GetSecondsElapsed(u64 Start, u64 End) {
	return (f32)((f64)(End - Start) / (f64)Win32State.PerformanceFrequency);
}

static PLATFORM_CHECK_PARAM(Win32CheckParam) {
	Assert(Param);

	for (s32 Index = 0; Index < Win32State.ArgC; Index++) {
		if (strcmp(Win32State.ArgV[Index], Param) == 0)
			return Index;
	}

	return NOTFOUND;
}

static PLATFORM_CHECK_PARAM_VALUE(Win32CheckParamValue) {
	Assert(Param);

	s32 Index = Win32CheckParam(Param);
	if (Index == NOTFOUND)
		return 0;
	if ((Index + 1) > Win32State.ArgC)
		return 0;

	return Win32State.ArgV[Index + 1];
}

static PLATFORM_OUTF(Win32Outf) {
	Assert(Format);
}

static PLATFORM_CRASHF(Win32Outf) {
	Assert(Format);
	ExitProcess(0);
}

static cpu_info
Win32GatherCPUInfo(void) {
	cpu_info CPUInfo = {};
	int CPUId[4] = {}, ExIds = 0;

	const u32 EAX = 0;
	const u32 EBX = 1;
	const u32 ECX = 2;
	const u32 EDX = 3;

	// NOTE(ivan): Obtain vendor name.
	__cpuid(CPUId, 0);

	*((int *)(CPUInfo.VendorName + 0)) = CPUId[EBX];
	*((int *)(CPUInfo.VendorName + 4)) = CPUId[EDX];
	*((int *)(CPUInfo.VendorName + 8)) = CPUId[ECX];

	if (strcmp(CPUInfo.VendorName, "GenuineIntel") == 0)
		CPUInfo.IsIntel = true;
	else if (strcmp(CPUInfo.VendorName, "AuthenticAMD") == 0)
		CPUInfo.IsAMD = true;

	// NOTE(ivan): Obtain brand name.
	__cpuid(CPUId, 0x80000000);
	ExIds = CPUId[EAX];

	if (ExIds >= 0x80000004) {
		for (int Func = 0x80000002, Pos = 0; Func <= 0x80000004; Func++, Pos += 16) {
			__cpuid(CPUId, Func);
			memcpy(CPUInfo.BrandName + Pos, CPUId, sizeof(CPUId));
		}
	} else {
		strcpy(CPUInfo.BrandName, "Unknown");
	}

	// NOTE(ivan): Check features.
	__cpuid(CPUId, 1);
	CPUInfo.SupportsMMX = (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) == TRUE);
	CPUInfo.SupportsSSE = (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) == TRUE);
	CPUInfo.SupportsSSE2 = (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) == TRUE);
	CPUInfo.SupportsSSE3 = (CPUId[ECX] & (1 << 0)) ? true : false;
	CPUInfo.SupportsSSSE3 = (CPUId[ECX] & (1 << 9)) ? true : false;
	CPUInfo.SupportsSSE4_1 = (CPUId[ECX] & (1 << 19)) ? true : false;
	CPUInfo.SupportsSSE4_2 = (CPUId[ECX] & (1 << 20)) ? true : false;
	CPUInfo.SupportsHT = (CPUId[EDX] & (1 << 28)) ? true : false;

	// NOTE(ivan): Check extended features.
	if (ExIds >= 0x80000001) {
		__cpuid(CPUId, 0x80000001);

		CPUInfo.SupportsMMXExt = CPUInfo.IsAMD && ((CPUId[EDX] & (1 << 22)) ? true : false);
		CPUInfo.Supports3DNow = CPUInfo.IsAMD && ((CPUId[EDX] & (1 << 31)) ? true : false);
		CPUInfo.Supports3DNowExt = CPUInfo.IsAMD && ((CPUId[EDX] & (1 << 30)) ? true : false);
		CPUInfo.SupportsSSE4A = CPUInfo.IsAMD && ((CPUId[ECX] & (1 << 6)) ? true : false);
	}

	// NOTE(ivan): Calculate cores/threads/caches count.
	DWORD LogicalLength = 0;
	PSYSTEM_LOGICAL_PROCESSOR_INFORMATION LogicalInfo = 0;
	if (!GetLogicalProcessorInformation(0, &LogicalLength) && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
		LogicalInfo = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)VirtualAlloc(0, LogicalLength, MEM_COMMIT, PAGE_READWRITE);
		if (LogicalInfo)
			GetLogicalProcessorInformation(LogicalInfo, &LogicalLength);
	}

	if (LogicalInfo) {
		PSYSTEM_LOGICAL_PROCESSOR_INFORMATION LogicalPtr = LogicalInfo;
		u32 ByteOffset = 0;
		while ((ByteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION)) <= LogicalLength) {
			switch (LogicalPtr->Relationship) {
			case RelationProcessorCore: {
				CPUInfo.NumCores++;
				CPUInfo.NumCoreThreads += CountSetBits(LogicalPtr->ProcessorMask);
			} break;

			case RelationCache: {
				if (LogicalPtr->Cache.Level == 1)
					CPUInfo.NumL1++;
				else if (LogicalPtr->Cache.Level == 2)
					CPUInfo.NumL2++;
				else if (LogicalPtr->Cache.Level == 3)
					CPUInfo.NumL3++;
			} break;

			case RelationNumaNode: {
				CPUInfo.NumNUMA++;
			} break;
			}

			ByteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
			LogicalPtr++;
		}

		if (LogicalInfo)
			VirtualFree(LogicalInfo, 0, MEM_RELEASE);
	}

	// NOTE(ivan): Calculate clock speed.
	HANDLE CurrentProcess = GetCurrentProcess();
	HANDLE CurrentThread = GetCurrentThread();

	DWORD CurrentPriorityClass = GetPriorityClass(CurrentProcess);
	int CurrentThreadPriority = GetThreadPriority(CurrentThread);

	DWORD_PTR ProcessMask, SystemMask;
	GetProcessAffinityMask(CurrentProcess, &ProcessMask, &SystemMask);

	SetPriorityClass(CurrentProcess, REALTIME_PRIORITY_CLASS);
	SetThreadPriority(CurrentThread, THREAD_PRIORITY_TIME_CRITICAL);
	SetProcessAffinityMask(CurrentProcess, 1);

	// NOTE(ivan): CPU serialization: call the processor to ensure that all other prior called functions are completed now.
	__cpuid(CPUId, 0);

	u64 StartCycle, EndCycle;
	u64 StartClock, EndClock;
		
	StartCycle = Win32GetClock();
	StartClock = __rdtsc();

	Sleep(300); // NOTE(ivan): Sleep time should be as short as possible.

	EndCycle = Win32GetClock();
	EndClock = __rdtsc();

	SetProcessAffinityMask(CurrentProcess, ProcessMask);
	SetThreadPriority(CurrentThread, CurrentThreadPriority);
	SetPriorityClass(CurrentProcess, CurrentPriorityClass);

	f32 SecondsElapsed = Win32GetSecondsElapsed(StartCycle, EndCycle);
	u64 ClocksElapsed = EndClock - StartClock;

	CPUInfo.ClockSpeed = (f32)(((f64)ClocksElapsed / SecondsElapsed) / (f32)(1000 * 1000 * 1000));

	// NOTE(ivan): Complete.
	return CPUInfo;
}

static X_INPUT_GET_BATTERY_INFORMATION(Win32XInputGetBatteryInformationStub) {
	UnusedParam(UserIndex);
	UnusedParam(DevType);
	UnusedParam(BatteryInformation);
	
	return ERROR_DEVICE_NOT_CONNECTED;
}

static X_INPUT_GET_STATE(Win32XInputGetStateStub) {
	UnusedParam(UserIndex);
	UnusedParam(State);
	
	return ERROR_DEVICE_NOT_CONNECTED;
}

static X_INPUT_SET_STATE(Win32XInputSetStateStub) {
	UnusedParam(UserIndex);
	UnusedParam(Vibration);
	
	return ERROR_DEVICE_NOT_CONNECTED;
}

inline win32_xinput_module
Win32LoadXInputModule(void) {
	win32_xinput_module Result = {};

	Win32Outf("Loading XInput module...");

	Win32Outf("...trying xinput1_4.dll");
	Result.XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if (!Result.XInputLibrary) {
		Win32Outf("...trying xinput9_1_0.dll");
		Result.XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}
	if (!Result.XInputLibrary) {
		Win32Outf("...trying xinput1_3.dll");
		Result.XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if (Result.XInputLibrary) {
		Result.GetBatteryInformation
			= (x_input_get_battery_information *)GetProcAddress(Result.XInputLibrary, "XInputGetBatteryInformation");
		Result.GetState = (x_input_get_state *)GetProcAddress(Result.XInputLibrary, "XInputGetState");
		Result.SetState = (x_input_set_state *)GetProcAddress(Result.XInputLibrary, "XInputSetState");

		if (Result.GetBatteryInformation && Result.GetState && Result.SetState) {
			Result.IsValid = true;
			Win32Outf("...success.");
		}
	} else {
		Win32Outf("...fail, file not found!");
	}

	if (!Result.IsValid) {
		Win32Outf("...fail, entries not found, using stubs!");
		
		Result.GetBatteryInformation = Win32XInputGetBatteryInformationStub;
		Result.GetState = Win32XInputGetStateStub;
		Result.SetState = Win32XInputSetStateStub;
	}

	return Result;
}

inline void
Win32SetInputButtonState(input_button_state *Button, b32 IsDown) {
	Assert(Button);

	Button->WasDown = Button->IsDown;
	Button->IsDown = IsDown;
	Button->IsNew = true;
}

inline void
Win32ProcessXInputDigitalButton(input_button_state *Button, DWORD XInputButtonState, DWORD ButtonBit) {
	Assert(Button);

	b32 IsDown = ((XInputButtonState & ButtonBit) == ButtonBit);
	Win32SetInputButtonState(Button, IsDown);
}

inline f32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold) {
	f32 Result = 0.0f;

	if (Value < -DeadZoneThreshold)
		Result = (f32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
	else if (Value > DeadZoneThreshold)
		Result = (f32)((Value - DeadZoneThreshold) / (32767.0f - DeadZoneThreshold));

	return Result;
}

inline b32
Win32ContainsSteamActionOrigin(EControllerActionOrigin *ActionOrigins, u32 NumActionOrigins,
							   EControllerActionOrigin OriginWeSeek) {
	Assert(ActionOrigins);

	b32 Result = false;
	for (u32 Index = 0; Index < NumActionOrigins; Index++) {
		if (ActionOrigins[Index] == OriginWeSeek) {
			Result = true;
			break;
		}
	}

	return Result;
}

inline void
Win32ProcessSteamDigitalButton(input_button_state *Button,
							   EControllerActionOrigin *ActionOrigins, u32 NumActionOrigins,
							   EControllerActionOrigin OriginWeSeek) {
	Assert(Button);
	Assert(ActionOrigins);

	b32 IsDown = Win32ContainsSteamActionOrigin(IncomingOrigins, NumIncomes, OriginWeSeek);
	Win32SetInputButtonState(Button, IsDown);
}

static b32
Win32MapVKToKeyCode(u32 VKCode, u32 ScanCode, b32 IsE0, b32 IsE1, key_code *OutCode) {
	Assert(OutCode);

	key_code KeyCode = {}; // NOTE(ivan): Result of Windows VK -> our keycode conversion.
	b32 KeyFound = false;

	if (VKCode == 255) {
		// NOTE(ivan): Discard "fake keys" which are part of en escaped sequence.
		return false;
	} else if (VKCode == VK_SHIFT) {
		// NOTE(ivan): Correct left-hand / right-hand SHIFT.
		VKCode = MapVirtualKey(ScanCode, MAPVK_VSC_TO_VK_EX);
	} else if (VKCode == VK_NUMLOCK) {
		// NOTE(ivan): Correct PAUSE/BREAK and NUMLOCK silliness, and set the extended bit.
		ScanCode = (MapVirtualKey(VKCode, MAPVK_VK_TO_VSC) | 0x100);
	}

	// NOTE(ivan): E0 and E1 are escape sequences used for certain special keys, such as PRINTSCREEN or PAUSE/BREAK.
	// See: http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

	if (IsE1) {
		// NOTE(ivan): For escaped sequences, turn the virtual key into the correct scan code using MapVirtualKey.
		// However, MapVirtualKey is unable to map VK_PAUSE (this is a known bug), hence we map that by hand.
		if (VKCode == VK_PAUSE)
			ScanCode = 0x45;
		else
			ScanCode = MapVirtualKey(VKCode, MAPVK_VK_TO_VSC);
	}

	switch (VKCode) {
		// NOTE(ivan): Right-hand CONTROL and ALT have their E0 bit set.
	case VK_CONTROL: {
		if (IsE0)
			KeyCode = KeyCode_RightControl;
		else
			KeyCode = KeyCode_LeftControl;
		KeyFound = true;
	} break;

	case VK_MENU: {
		if (IsE0)
			KeyCode = KeyCode_RightAlt;
		else
			KeyCode = KeyCode_LeftAlt;
		KeyFound = true;
	} break;

		// NOTE(ivan): NUM ENTER has its E0 bit set
	case VK_RETURN: {
		if (IsE0)
			KeyCode = KeyCode_NumEnter;
		else
			KeyCode = KeyCode_Enter;
		KeyFound = true;
	} break;

		// NOTE(ivan): The standard INSERT, DELETE, HOME, END, PRIOR and NEXT keys will always have their E0 bit set,
		// but the corresponding NUM keys will not.
	case VK_INSERT: {
		if (!IsE0)
			KeyCode = KeyCode_NumInsert;
		else
			KeyCode = KeyCode_Insert;
		KeyFound = true;
	} break;
	case VK_DELETE: {
		if (!IsE0)
			KeyCode = KeyCode_NumDelete;
		else
			KeyCode = KeyCode_Delete;
		KeyFound = true;
	} break;
	case VK_HOME: {
		if (!IsE0)
			KeyCode = KeyCode_NumHome;
		else
			KeyCode = KeyCode_Home;
		KeyFound = true;
	} break;
	case VK_END: {
		if (!IsE0)
			KeyCode = KeyCode_NumEnd;
		else
			KeyCode = KeyCode_End;
		KeyFound = true;
	} break;
	case VK_PRIOR: {
		if (!IsE0)
			KeyCode = KeyCode_NumPageUp;
		else
			KeyCode = KeyCode_PageUp;
		KeyFound = true;
	} break;
	case VK_NEXT: {
		if (!IsE0)
			KeyCode = KeyCode_NumPageDown;
		else
			KeyCode = KeyCode_PageDown;
		KeyFound = true;
	} break;

		// NOTE(ivan): The standard arrow keys will awlays have their E0 bit set,
		// but the corresponding NUM keys will not.
	case VK_UP: {
		if (!IsE0)
			KeyCode = KeyCode_NumUp;
		else
			KeyCode = KeyCode_Up;
		KeyFound = true;
	} break;
	case VK_DOWN: {
		if (!IsE0)
			KeyCode = KeyCode_NumDown;
		else
			KeyCode = KeyCode_Down;
		KeyFound = true;
	} break;
	case VK_LEFT: {
		if (!IsE0)
			KeyCode = KeyCode_NumLeft;
		else
			KeyCode = KeyCode_Left;
		KeyFound = true;
	} break;
	case VK_RIGHT: {
		if (!IsE0)
			KeyCode = KeyCode_NumRight;
		else
			KeyCode = KeyCode_Right;
		KeyFound = true;
	} break;

		// NOTE(ivan): NUM 5 doesn't have its E0 bit set.
	case VK_CLEAR: {
		if (!IsE0) {
			KeyCode = KeyCode_NumClear;
			KeyFound = true;
		} else {
			return false;
		}
	} break;
	}

#define KeyMap(MapVK, MapKeyCode)				\
	if (VKCode == MapVK) {						\
		KeyCode = MapKeyCode;					\
		KeyFound = true;						\
	}
	KeyMap(VK_TAB, KeyCode_Tab);
	KeyMap(VK_ESCAPE, KeyCode_Escape);
	KeyMap(VK_SPACE, KeyCode_Space);
	KeyMap(VK_BACK, KeyCode_BackSpace);
	KeyMap(VK_LSHIFT, KeyCode_LeftShift);
	KeyMap(VK_RSHIFT, KeyCode_RightShift);
	KeyMap(VK_LMENU, KeyCode_LeftAlt);
	KeyMap(VK_RMENU, KeyCode_RightAlt);
	KeyMap(VK_LCONTROL, KeyCode_LeftControl);
	KeyMap(VK_RCONTROL, KeyCode_RightControl);
	KeyMap(VK_LWIN, KeyCode_LeftSuper);
	KeyMap(VK_RWIN, KeyCode_RightSuper);

	KeyMap(VK_F1, KeyCode_F1);
	KeyMap(VK_F2, KeyCode_F2);
	KeyMap(VK_F3, KeyCode_F3);
	KeyMap(VK_F4, KeyCode_F4);
	KeyMap(VK_F5, KeyCode_F5);
	KeyMap(VK_F6, KeyCode_F6);
	KeyMap(VK_F7, KeyCode_F7);
	KeyMap(VK_F8, KeyCode_F8);
	KeyMap(VK_F9, KeyCode_F9);
	KeyMap(VK_F10, KeyCode_F10);
	KeyMap(VK_F11, KeyCode_F11);
	KeyMap(VK_F12, KeyCode_F12);
	
	KeyMap(VK_NUMLOCK, KeyCode_NumLock);
	KeyMap(VK_CAPITAL, KeyCode_CapsLock);
	KeyMap(VK_SCROLL, KeyCode_ScrollLock);
	
	KeyMap(VK_PRINT, KeyCode_PrintScreen);
	KeyMap(VK_PAUSE, KeyCode_Pause);

	KeyMap(0x41, KeyCode_A);
	KeyMap(0x42, KeyCode_B);
	KeyMap(0x43, KeyCode_C);
	KeyMap(0x44, KeyCode_D);
	KeyMap(0x45, KeyCode_E);
	KeyMap(0x46, KeyCode_F);
	KeyMap(0x47, KeyCode_G);
	KeyMap(0x48, KeyCode_H);
	KeyMap(0x49, KeyCode_I);
	KeyMap(0x4A, KeyCode_J);
	KeyMap(0x4B, KeyCode_K);
	KeyMap(0x4C, KeyCode_L);
	KeyMap(0x4D, KeyCode_M);
	KeyMap(0x4E, KeyCode_N);
	KeyMap(0x4F, KeyCode_O);
	KeyMap(0x50, KeyCode_P);
	KeyMap(0x51, KeyCode_Q);
	KeyMap(0x52, KeyCode_R);
	KeyMap(0x53, KeyCode_S);
	KeyMap(0x54, KeyCode_T);
	KeyMap(0x55, KeyCode_U);
	KeyMap(0x56, KeyCode_V);
	KeyMap(0x57, KeyCode_W);
	KeyMap(0x58, KeyCode_X);
	KeyMap(0x59, KeyCode_Y);
	KeyMap(0x5A, KeyCode_Z);

	KeyMap(0x30, KeyCode_0);
	KeyMap(0x31, KeyCode_1);
	KeyMap(0x32, KeyCode_2);
	KeyMap(0x33, KeyCode_3);
	KeyMap(0x34, KeyCode_4);
	KeyMap(0x35, KeyCode_5);
	KeyMap(0x36, KeyCode_6);
	KeyMap(0x37, KeyCode_7);
	KeyMap(0x38, KeyCode_8);
	KeyMap(0x39, KeyCode_9);

	KeyMap(VK_OEM_4, KeyCode_OpenBracket);
	KeyMap(VK_OEM_6, KeyCode_CloseBracket);
	KeyMap(VK_OEM_1, KeyCode_Semicolon);
	KeyMap(VK_OEM_7, KeyCode_Quote);
	KeyMap(VK_OEM_COMMA, KeyCode_Comma);
	KeyMap(VK_OEM_PERIOD, KeyCode_Period);
	KeyMap(VK_OEM_2, KeyCode_Slash);
	KeyMap(VK_OEM_5, KeyCode_BackSlash);
	KeyMap(VK_OEM_3, KeyCode_Tilde);
	KeyMap(VK_OEM_PLUS, KeyCode_Plus);
	KeyMap(VK_OEM_MINUS, KeyCode_Minus);

	KeyMap(VK_NUMPAD8, KeyCode_Num8);
	KeyMap(VK_NUMPAD2, KeyCode_Num2);
	KeyMap(VK_NUMPAD4, KeyCode_Num4);
	KeyMap(VK_NUMPAD6, KeyCode_Num6);
	KeyMap(VK_NUMPAD7, KeyCode_Num7);
	KeyMap(VK_NUMPAD1, KeyCode_Num1);
	KeyMap(VK_NUMPAD9, KeyCode_Num9);
	KeyMap(VK_NUMPAD3, KeyCode_Num3);
	KeyMap(VK_NUMPAD0, KeyCode_Num0);
	KeyMap(VK_SEPARATOR, KeyCode_NumSeparator);
	KeyMap(VK_MULTIPLY, KeyCode_NumMultiply);
	KeyMap(VK_DIVIDE, KeyCode_NumDivide);
	KeyMap(VK_ADD, KeyCode_NumPlus);
	KeyMap(VK_SUBTRACT, KeyCode_NumMinus);
#undef KeyMap
	if (!KeyFound)
		return false;

	*OutCode = KeyCode;
	return true;
}

inline win32_game_module
Win32LoadGameModule(const char *SharedName) {
	Assert(SharedName);
	
	win32_game_module Result = {};

	char GameLibraryName[1024] = {};
	snprintf(GameLibraryName, ArraySize(GameLibraryName) - 1, "%s.dll", SharedName);

	Win32Outf("Loading game module %s...", GameLibraryName);
	Result.GameLibrary = LoadLibraryA(GameLibraryName);
	if (Result.GameLibrary) {
		Result.GameTrigger = (game_trigger *)GetProcAddress(Result.GameLibrary, "GameTrigger");
		if (Result.GameTrigger)
			Result.IsValid = true;
	}

	return Result;
}

inline win32_steamworks_module
Win32LoadSteamworksModule(void) {
	win32_steamworks_module Result = {};
	
	const char *SteamLibraryName;
	if (IsTargetCPU32Bit())
		SteamLibraryName = "steam_api.dll";
	else if (IsTargetCPU64Bit())
		SteamLibraryName = "steam_api64.dll";

	Win32Outf("Loading Steamworks module %s...", SteamLibraryName);
	Result.SteamLibrary = LoadLibraryA(SteamLibraryFileName);
	if (Result.SteamLibrary) {
		Result.SteamworksAPI.Init = (steam_api_init *)GetProcAddress(Result.SteamLibrary, "SteamAPI_Init");
		Result.SteamworksAPI.Shutdown = (steam_api_shutdown *)GetProcAddress(Result.SteamLibrary, "SteamAPI_Shutdown");
		Result.SteamworksAPI.IsSteamRunning
			= (steam_api_is_steam_running *)GetProcAddress(Result.SteamLibrary, "SteamAPI_IsSteamRunning");
		Result.SteamworksAPI.RestartAppIfNecessary
			= (steam_api_restart_app_if_necessary *)GetProcAddress(Result.SteamLibrary, "SteamAPI_RestartAppIfNecessary");

		Result.SteamworksAPI._ContextInit
			= (steam_internal_context_init *)GetProcAddress(Result.SteamLibrary, "SteamInternal_ContextInit");
		Result.SteamworksAPI._CreateInterface
			= (steam_internal_create_interface *)GetProcAddress(Result.SteamLibrary, "SteamInternal_CreateInterface");

		if (Result.SteamworksAPI.Init &&
			Result.SteamworksAPI.Shutdown &&
			Result.SteamworksAPI.IsSteamRunning &&
			Result.SteamworksAPI.RestartAppIfNecessary &&
			Result.SteamworksAPI._ContextInit &&
			Result.SteamworksAPI._CreateInterface) {
			Result.IsValid = true;
			Win32Outf("...success.");
		} else {
			Win32Outf("...fail, entries not found!");
		}
	} else {
		Win32Outf("...fail, file not found!");
	}

	return Result;
}

static uptr
Win32CalculateDesirableUsableMemorySize(void) {
	uptr Result;
	
	MEMORYSTATUSEX MemStat; // NOTE(ivan): Detect how much memory is available.
	MemStat.dwLength = sizeof(MemStat);
	if (GlobalMemoryStatusEx(&MemStat)) {
		// NOTE(ivan): Capture 80% of free RAM space and leave the rest for internal platform-layer and OS needs.
		// TODO(ivan): Play aroud with this percent to achieve maximum RAM space allocation
		// without stressing the OS and the environment too much.
		Result = (uptr)((f64)MemStat.ullAvailPhys * 0.8);
	} else {
		// NOTE(ivan): Available free RAM detection went wrong
		// for some strange reason - try to guess it approximately.
		if (IsTargetCPU32Bit()) {
			BOOL Is64BitWindows = FALSE;
			if (!IsWow64Process(GetCurrentProcess(), &Is64BitWindows))
				Is64BitWindows = FALSE;

			// TODO(ivan): 64-bit Windows allows 32-bit programs to eat 3 gigabytes of RAM
			// instead of 2 gigabytes as in 32-bit Windows.
			if (Is64BitWindows)
				Result = Gigabytes(3); // NOTE(ivan): Requesting 3Gb of memory for 32-bit program
			// requires it being built with /largeaddressaware MS linker option.
			else
				Result = Gigabytes(2);
		} else if (IsTargetCPU64Bit()) {
			Result = Gigabytes(4);
		}
	}

	return Result;
}

static LRESULT CALLBACK
Win32WindowProc(HWND Window, UINT Msg, WPARAM W, LPARAM L) {
	switch (Msg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
	} break;

	case WM_ACTIVATE: {
		if (W == WA_ACTIVE || W == WA_CLICKACTIVE)
			Win32State.IsWindowActive = true;
		else
			Win32State.IsWindowActive = false;
	} break;

	case WM_INPUT: {
		u8 Buffer[sizeof(RAWINPUT)] = {};
		u32 BufferSize = sizeof(RAWINPUT);
		GetRawInputData((HRAWINPUT)L, RID_INPUT, Buffer, (PUINT)&BufferSize, sizeof(RAWINPUTHEADER));

		RAWINPUT *RawInput = (RAWINPUT *)Buffer;
		if (RawInput->header.dwType == RIM_TYPEKEYBOARD) {
			RAWKEYBOARD *RawKeyboard = &RawInput->data.keyboard;

			key_code KeyCode;
			if (Win32MapVKToKeyCode(RawKeyboard->VKey, RawKeyboard->MakeCode,
									(RawKeyboard->Flags & RI_KEY_E0) != 0,
									(RawKeyboard->Flags & RI_KEY_E1) != 0,
									&KeyCode))
				Win32SetInputButtonState(&Win32State.GameInput->KbButtons[KeyCode],
										 (RawKeyboard->Flags & RI_KEY_BREAK) == 0);
		} else if (RawInput->header.dwType == RIM_TYPEMOUSE) {
			RAWMOUSE *RawMouse = &RawInput->data.mouse;

			if (RawMouse->usFlags == MOUSE_MOVE_ABSOLUTE) {
				Win32State.GameInput->MousePos.X = RawMouse->lLastX;
				Win32State.GameInput->MousePos.Y = RawMouse->lLastY;
			}

			switch (RawMouse->usButtonFlags) {
			case RI_MOUSE_BUTTON_1_DOWN: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_Left], true);
			} break;
			case RI_MOUSE_BUTTON_1_UP: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_Left], false);
			} break;

			case RI_MOUSE_BUTTON_2_DOWN: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_Middle], true);
			} break;
			case RI_MOUSE_BUTTON_2_UP: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_Middle], false);
			} break;

			case RI_MOUSE_BUTTON_3_DOWN: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_Right], true);
			} break;
			case RI_MOUSE_BUTTON_3_UP: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_Right], false);
			} break;

			case RI_MOUSE_BUTTON_4_DOWN: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_X1], true);
			} break;
			case RI_MOUSE_BUTTON_4_UP: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_X1], false);
			} break;

			case RI_MOUSE_BUTTON_5_DOWN: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_X2], true);
			} break;
			case RI_MOUSE_BUTTON_5_UP: {
				Win32SetInputButtonState(&Win32State.GameInput->MouseButtons[MouseButton_X2], false);
			} break;

			case RI_MOUSE_WHEEL: {
				s32 WheelRotations = (s32)RawMouse->usButtonData / WHEEL_DELTA;
				Win32State.GameInput->MouseWheel += WheelRotations;
			} break;
			}
		}
	} break;

	case WM_SETCURSOR: {
		if (Win32State.IsDebugCursor)
			SetCursor(LoadCursorA(0, MAKEINTRESOURCEA(32515))); // NOTE(ivan): IDC_CROSS.
		else
			SetCursor(0);
	} break;

	default: {
		if (Msg == Win32State.QueryCancelAutoplay)
			return TRUE; // NOTE(ivan): Cancel disc autoplay feature.

		return DefWindowProcA(Window, Msg, W, L);
	} break;
	}

	return FALSE;
}

int CALLBACK
WinMain(HINSTANCE Instance, HINSTANCE PrevInstance, LPSTR CommandLine, int ShowCommand) {
	UnusedParam(PrevInstance);
	UnusedParam(CommandLine);

	platform_api Win32API = {};
	
	game_memory GameMemory = {};
	game_clocks GameClocks = {};
	game_input GameInput = {};

	Win32State.Instance = Instance;
	Win32SetThreadName(GetCurrentThreadId(), GAMENAME " primary thread");

	SetCursor(LoadCursorA(0, MAKEINTRESOURCEA(32514))); // NOTE(ivan): IDC_WAIT.

	// NOTE(ivan): C runtime memory optimization for faster execution.
	_CrtSetDbgFlag(0); // NOTE(ivan): Disable heap check every 1024 allocation.
	_CrtSetDebugFillThreshold(0); // NOTE(ivan): Disable buffers filling.

	Win32State.ArgC = __argc;
	Win32State.ArgV = __argv;

	Win32State.GameInput = &GameInput;

	// NOTE(ivan): Debug cursor (cross) is initially visible on the main window in internal build.
#if INTERNAL
	Win32State.IsDebugCursor = true;
#endif

	Win32API.CheckParam = Win32CheckParam;
	Win32API.CheckParamValue = Win32CheckParamValue;
	Win32API.Outf = Win32Outf;
	Win32API.Crashf = Win32Crashf;

	Win32API.FOpen = Win32FOpen;
	Win32API.FClose = Win32FClose;
	Win32API.FRead = Win32FRead;
	Win32API.FWrite = Win32FWrite;
	Win32API.FSeek = Win32FSeek;
	Win32API.FFlush = Win32FFlush;

	// NOTE(ivan): Check whether the host OS is not obsolete.
	if (IsWindows7OrGreater()) {
		// NOTE(ivan): Check debugger presence.
		Win32State.IsDebuggerActive = (IsDebuggerPresent() == TRUE);

		// NOTE(ivan): Initialize and setup COM.
		Verify(CoInitializeEx(0, COINIT_MULTITHREADED) == S_OK);
		Verify(CoInitializeSecurity(0, -1, 0, 0, RPC_C_AUTHN_LEVEL_DEFAULT,
									RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE, 0) == S_OK);

		// NOTE(ivan): Initialize Common Controls for modern visuals.
		INITCOMMONCONTROLSEX InitCC;
		InitCC.dwSize = sizeof(InitCC);
		InitCC.dwICC = ICC_STANDARD_CLASSES;
		Verify(InitCommonControlsEx(&InitCC));

		// NOTE(ivan): Disable disc autoplay feature.
		Win32State.QueryCancelAutoplay = RegisterWindowMessageA("QueryCancelAutoplay");

		// NOTE(ivan): Obtain system high-resolution timer frequency.
		LARGE_INTEGER PerformanceFrequency;
		Verify(QueryPerformanceFrequency(&PerformanceFrequency));
		Win32State.PerformanceFrequency = PerformanceFrequency.QuadPart;

		// NOTE(ivan): Strange, but it is thre only way to set the system's scheduler granularity
		// so our Sleep() calls will be way more accurate.
		b32 IsSleepGranular = (timeBeginPeriod(1) != TIMERR_NOCANDO);

		// NOTE(ivan): Obtain CPU information.
		Win32API.CPUInfo = Win32GatherCPUInfo();

		// NOTE(ivan): Obtain executable's file name, base name and path.
		char ExecPath[1024] = {}, ExecName[1024] = {}, ExecNameNoExt[1024] = {};
		char ModuleName[2048] = {};
		Verify(GetModuleFileNameA(Win32State.Instance, ModuleName, ArraySize(ModuleName) - 1));

		char *PastLastSlash = ModuleName, *Ptr = ModuleName;
		while (*Ptr) {
			if (*Ptr == '\\' || *Ptr == '/')
				PastLastSlash = Ptr + 1;
			Ptr++;
		}
		strcpy(ExecName, PastLastSlash);
		strncpy(ExecPath, ModuleName, PastLastSlash - ModuleName);

		strcpy(ExecNameNoExt, ExecName);
		for (Ptr = ExecNameNoExt; *Ptr; Ptr++) {
			if (*Ptr == '.') {
				*Ptr = 0;
				break;
			}
		}

		Win32API.ExecutableName = ExecName;
		Win32API.ExecutableNameNoExt = ExecNameNoExt;
		Win32API.ExecutablePath = ExecPath;

		// NOTE(ivan): Obtain game "shared name".
		char SharedName[1024] = {};
		strncpy(SharedName, ExecName + 3, ArraySize(SharedName) - 1);

		Win32API.SharedName = SharedName;

		// NOTE(ivan): Set current working directory if necessary.
		const char *ParamCWD = Win32CheckParamValue("-cwd");
		if (ParamCWD)
			SetCurrentDirectoryA(ParamCWD);

		// NOTE(ivan): Create game primary storage.
		GameMemory.FreeStorage.Size = Win32CalculateDesirableUsableMemorySize();
		GameMemory.FreeStorage.Base = (u8 *)VirtualAlloc(0, GameMemory.FreeStorage.Size, MEM_COMMIT, PAGE_READWRITE);
		if (GameMemory.FreeStorage.Base) {
			GameMemory.StorageTotalSize = GameMemorySize;

			// NOTE(ivan): Create main window.
			WNDCLASSA WindowClass = {};
			WindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			WindowClass.lpszClassName = (GAMENAME "Window");
			WindowClass.lpfnWndProc = Win32WindowProc;
			WindowClass.hInstance = Win32State.Instance;

			if (RegisterClassA(&WindowClass)) {
				HWND Window = CreateWindowExA(WS_EX_APPWINDOW,
											  WindowClass.lpszClassName,
											  GAMENAME,
											  WS_OVERLAPPEDWINDOW,
											  CW_USEDEFAULT, CW_USEDEFAULT,
											  CW_USEDEFAULT, CW_USEDEFAULT,
											  0, 0, Win32State.Instance, 0);
				if (Window) {
					HDC WindowDC = GetDC(Window); // NOTE(ivan): CS_OWNDC indicates that a window has single non-shared DC.

					// NOTE(ivan): Obtain monitor refresh rate.
					// TODO(ivan): Should it be updated each new frame to handle monitor settings change while
					// the program is running?
					s32 DisplayFrequency = GetDeviceCaps(WindowDC, VREFRESH);
					if (DisplayFrequency < 2)
						DisplayFrequency = 60; // TODO(ivan): Find a more appropriate way to obtain display frequency.

					// NOTE(ivan): Target seconds to last per one frame.
					f32 GameTargetFramerate = (1.0f / DisplayFrequency);

					// NOTE(ivan): Initialize raw keyboard and mouse input.
					RAWINPUTDEVICE RawDevices[2] = {};

					RawDevices[0].usUsagePage = 0x01;
					RawDevices[0].usUsage = 0x06;
					RawDevices[0].dwFlags = 0;
					RawDevices[0].hwndTarget = Window;

					RawDevices[1].usUsagePage = 0x01;
					RawDevices[1].usUsage = 0x02;
					RawDevices[1].dwFlags = 0;
					RawDevices[1].hwndTarget = Window;

					RegisterRawInputDevices(RawDevices, 2, sizeof(RAWINPUTDEVICE));

					// NOTE(ivan): Connect to XInput for processing xbox controller(s) input.
					win32_xinput_module XInputModule = Win32LoadXInputModule();

					// NOTE(ivan): Connect to Steamworks.
					win32_steamworks_module SteamworksModule = Win32LoadSteamworksModule();
					if (SteamworksModule.IsValid) {
						steamworks_api &SteamworksAPI = SteamworksModule.SteamworksAPI;
						
						// NOTE(ivan): Connect to game module.
						win32_game_module GameModule = Win32LoadGameModule(Win32API.SharedName);
						if (GameModule.IsValid) {
							GameModule.GameTrigger(GameTriggerType_Prepare,
												   &Win32API,
												   &SteamworksAPI,
												   &GameMemory,
												   &GameClocks,
												   &GameInput);

							// NOTE(ivan): Initialize early needed Steamworks interfaces.
							SteamworksAPI.Controller->Init();

							// NOTE(ivan): When all initialization is done, present the window.
							ShowWindow(Window, ShowCommand);
							SetCursor(LoadCursorA(0, MAKEINTRESOURCEA(32512))); // NOTE(ivan): IDC_ARROW.

							// NOTE(ivan): Prepare game clocks and timings.
							u64 LastCPUClockCounter = __rdtsc();
							u64 LastCycleCounter = Win32GetClock();

							// NOTE(ivan): Primary loop.
							b32 IsGameRunning = true;
							while (IsGameRunning) {
								// NOTE(ivan): Process OS messages.
								static MSG Msg;
								while (PeekMessageA(&Msg, 0, 0, 0, PM_REMOVE)) {
									if (Msg.message == WM_QUIT)
										IsGameRunning = false;

									TranslateMessage(&Msg);
									DispatchMessageA(&Msg);
								}
								
								// NOTE(ivan): Do these routines only in case the main window is in focus.
								if (Win32State.IsWindowActive) {
									// NOTE(ivan): Process Xbox controllers state.
									// TODO(ivan): Need to not poll disconnected controllers to avoid
									// XInput frame rate hit on older libraries.
									// TODO(ivan): Should we poll this more frequently?
									const DWORD MaxXboxControllers = XUSER_MAX_COUNT;
									MaxXboxControllers = Min(MaxXboxControllers,
															 ArraySize(GameInput.XboxControllers));
									for (u32 Index = 0; Index < MaxXboxControllerCount; Index++) {
										xbox_controller_state *XboxController = &GameInput.XboxControllers[Index];
										XINPUT_STATE XboxControllerState;
										if (XInputModule.GetState(Index, &XboxControllerState) == ERROR_SUCCESS) {
											XboxController->IsConnected = true;

											// NOTE(ivan): Gather battery state.
											XINPUT_BATTERY_INFORMATION XboxControllerBattery;
											if (XInputModule.GetBatteryInformation(Index, BATTERY_DEVTYPE_GAMEPAD,
																				   &XboxControllerBattery) == ERROR_SUCCESS) {
												switch (XboxControllerBattery.BatteryType) {
												case BATTERY_TYPE_DISCONNNECTED:
													XboxController->Battery.Type = XboxControllerBatteryType_Disconnected;
													break;

												case BATTERY_TYPE_WIRED:
													XboxController->Battery.Type = XboxControllerBatteryType_Wired;
													break;

												case BATTERY_TYPE_ALKALINE:
													XboxController->Battery.Type = XboxControllerBatteryType_Alkaline;
													break;

												case BATTERY_TYPE_NIMH:
													XboxController->Battery.Type = XboxControllerBatteryType_Nimh;
													break;

												case BATTERY_TYPE_UNKNOWN:
												default:
													XboxController->Battery.Type = XboxControllerBatteryType_Unknown;
													break;
												}

												switch (XboxControllerBattery.BatteryLevel) {
												case BATTERY_LEVEL_EMPTY:
													XboxController->Battery.ChargeLevel
														= XboxControllerBatteryChargeLevel_Empty;
													break;

												case BATTERY_LEVEL_LOW:
													XboxController->Battery.ChargeLevel
														= XboxControllerBatteryChargeLevel_Low;
													break;

												case BATTERY_LEVEL_MEDIUM:
													XboxController->Battery.ChargeLevel
														= XboxControllerBatteryChargeLevel_Medium;
													break;

												case BATTERY_LEVEL_FULL:
													XboxController->Battery.ChargeLevel
														= XboxControllerBatteryChargeLevel_Full;
													break;
												}
											} else {
												XboxController->Battery.Type
													= XboxControllerBatteryType_Unknown;
												XboxController->Battery.ChargeLevel
													= XboxControllerBatteryChargeLevel_Unknown;
											}

											// NOTE(ivan): Gather buttons and sticks state.
											// TODO(ivan): See if XboxControllerState.dwPacketNumber
											// increments too rapidly.
											XINPUT_GAMEPAD *XboxGamepad = &XboxControllerState.Gamepad;

											// NOTE(ivan): Process buttons.
											Win32ProcessXInputDigitalButton(&XboxController->Start,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_START);
											Win32ProcessXInputDigitalButton(&XboxController->Back,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_BACK);

											Win32ProcessXInputDigitalButton(&XboxController->A,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_A);
											Win32ProcessXInputDigitalButton(&XboxController->B,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_B);
											Win32ProcessXInputDigitalButton(&XboxController->X,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_X);
											Win32ProcessXInputDigitalButton(&XboxController->Y,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_Y);

											Win32ProcessXInputDigitalButton(&XboxController->DPad.Up,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_DPAD_UP);
											Win32ProcessXInputDigitalButton(&XboxController->DPad.Down,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_DPAD_DOWN);
											Win32ProcessXInputDigitalButton(&XboxController->DPad.Left,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_DPAD_LEFT);
											Win32ProcessXInputDigitalButton(&XboxController->DPad.Right,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_DPAD_RIGHT);

											Win32ProcessXInputDigitalButton(&XboxController->LeftStick.Button,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_LEFT_THUMB);
											Win32ProcessXInputDigitalButton(&XboxController->RightStick.Button,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_RIGHT_THUMB);

											// NOTE(ivan): Process bumpers.
											Win32ProcessXInputDigitalButton(&XboxController->LeftBumper,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_LEFT_SHOULDER);
											Win32ProcessXInputDigitalButton(&XboxController->RightBumper,
																			XboxGamepad->wButtons,
																			XINPUT_GAMEPAD_RIGHT_SHOULDER);

											// NOTE(ivan): Process triggers.
											XboxController->LeftTrigger = XboxGamepad->bLeftTrigger;
											XboxController->RightTrigger = XboxGamepad->bRightTrigger;

											// NOTE(ivan): Process sticks positions.
											XboxController->LeftStick.Pos.X =
												Win32ProcessXInputStickValue(XboxGamepad->sThumbLX,
																			 XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
											XboxController->LeftStick.Pos.Y =
												Win32ProcessXInputStickValue(XboxGamepad->sThumbLY,
																			 XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

											XboxController->RightStick.Pos.X =
												Win32ProcessXInputStickValue(XboxGamepad->sThumbRX,
																			 XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
											XboxController->RightStick.Pos.Y =
												Win32ProcessXInputStickValue(XboxGamepad->sThumbRY,
																			 XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

											// NOTE(ivan): Vibrate if requested.
											if (XboxController->DoVibration.LeftMotorSpeed ||
												XboxController->DoVibration.RightMotorSpeed) {
												XINPUT_VIBRATION XboxControllerVibr;
												XboxControllerVibr.wLeftMotorSpeed
													= XboxController->DoVibration.LeftMotorSpeed;
												XboxControllerVibr.wRightMotorSpeed
													= XboxController->DoVibration.RightMotorSpeed;

												XInputModule.SetState(Index, &XboxControllerVibr);

												XboxController->DoVibration.LeftMotorSpeed = 0;
												XboxController->DoVibration.RightMotorSpeed = 0;
											}
										} else {
											XboxController->IsConnected = false;
										}
									}
									
									// NOTE(ivan): Process Steam controllers state.
									// NOTE(ivan): According to isteamcontroller.h comments, ISteamController::RunFrame()
									// must be called somewhere before ISteamController::GetConnectedControllers().
									SteamworksAPI.Controller->RunFrame();

									static u32 MaxSteamControllers = STEAM_INPUT_MAX_COUNT;
									static ControllerHandle_t SteamControllerHandles[MaxSteamControllers];
									u32 NumConnectedSteamControllers
										= SteamworksAPI.Controller->GetConnectedControllers(SteamControllerHandles);
									NumConnectedSteamControllers = Min(NumConnectedSteamControllers,
																	   MaxSteamControllers);
									for (u32 Index = 0; Index < NumConnectedSteamControllers; Index++) {
										ControllerHandle_t SteamControllerHandle = SteamControllerHandles[Index];
										steam_controller_state *SteamController = &GameInput.SteamControllers[Index];

										SteamController->IsConnected = true;
									  
										// NOTE(ivan): Gather controller digital actions;
										EControllerActionOrigin SteamControllerOrigins[STEAM_CONTROLLER_MAX_ORIGINS];
										int NumSteamControllerOrigins
											= SteamworksAPI.Controller->GetDigitalActionOrigins(SteamControllerHandle,
																								0, 0,
																								&SteamControllerOrigins);
										if (NumSteamControllerOrigins) {
											// NOTE(ivan): Process general buttons.
											Win32ProcessSteamDigitalButton(&SteamController->Start,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_Start);
											Win32ProcessSteamDigitalButton(&SteamController->Back,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_Back);

											Win32ProcessSteamDigitalButton(&SteamController->A,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_A);
											Win32ProcessSteamDigitalButton(&SteamController->B,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_B);
											Win32ProcessSteamDigitalButton(&SteamController->X,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_X);
											Win32ProcessSteamDigitalButton(&SteamController->Y,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_Y);

											Win32ProcessSteamDigitalButton(&SteamConntroller->LeftBumper,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_LeftBumper);
											Win32ProcessSteamDigitalButton(&SteamController->RightBumper,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_RightBumper);

											// NOTE(ivan): Process left stick.
											Win32ProcessSteamDigitalButton(&SteamController->LeftStick.Button,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_LeftStick_Click);

											// NOTE(ivan): Process left pad.
											Win32ProcessSteamDigitalButton(&SteamController->LeftPad.DPad.Up,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_LeftPad_DPadNorth);
											Win32ProcessSteamDigitalButton(&SteamController->LeftPad.DPad.Down,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_LeftPad_DPadSouth);
											Win32ProcessSteamDigitalButton(&SteamController->LeftPad.DPad.Left,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_LeftPad_DPadWest);
											Win32ProcessSteamDigitalButton(&SteamController->LeftPad.DPad.Right,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_LeftPad_DPadEast);

											Win32ProcessSteamDigitalButton(&SteamController->LeftPad.Button,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_LeftPad_Click);

											if (Win32ContainsSteamActionOrigin(SteamControllerOrigins,
																			   NumSteamControllerOrigins,
																			   k_EControllerActionOrigin_LeftPad_Touch))
												SteamController->LeftPad.IsTouchHappening = true;
											else
												SteamController->LeftPad.IsTouchHappening = false;

											if (Win32ContainsSteamActionOrigin(SteamControllerOrigins,
																			   NumSteamControllerOrigins,
																			   k_EControllerActionOrigin_LeftPad_Swipe))
												SteamController->LeftPad.IsSwipeHappening = true;
											else
												SteamController->LeftPad.IsSwipeHappening = false;

											// NOTE(ivan): Process right pad.
											Win32ProcessSteamDigitalButton(&SteamController->RightPad.DPad.Up,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_RightPad_DPadNorth);
											Win32ProcessSteamDigitalButton(&SteamController->RightPad.DPad.Down,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_RightPad_DPadSouth);
											Win32ProcessSteamDigitalButton(&SteamController->RightPad.DPad.Left,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_RightPad_DPadWest);
											Win32ProcessSteamDigitalButton(&SteamController->RightPad.DPad.Right,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_RightPad_DPadEast);

											Win32ProcessSteamDigitalButton(&SteamController->RightPad.Button,
																		   SteamControllerOrigins,
																		   NumSteamControllerOrigins,
																		   k_EControllerActionOrigin_RightPad_Click);

											if (Win32ContainsSteamActionOrigin(SteamControllerOrigins,
																			   NumSteamControllerOrigins,
																			   k_EControllerActionOrigin_RightPad_Touch))
												SteamController->RightPad.IsTouchHappening = true;
											else
												SteamController->RightPad.IsTouchHappening = false;

											if (Win32ContainsSteamActionOrigin(SteamControllerOrigins,
																			   NumSteamControllerOrigins,
																			   k_EControllerActionOrigin_RightPad_Swipe))
												SteamController->RightPad.IsSwipeHappening = true;
											else
												SteamController->RightPad.IsSwipeHappening = false;
										}

										// NOTE(ivan): Gather controller analog actions.
										NumSteamControllerOrigins
											= SteamworksAPI.Controller->GetAnalogActionOrigins(SteamControllerHandle,
																							   0, 0,
																							   &SteamControllerOrigins);
										if (NumSteamControllerOrigins) {
										}
										
										// NOTE(ivan): Set controller LED color.
										u8 SteamControllerLED_R = (u8)((f32)SteamController->SetLEDColor.R * 255.0f);
										u8 SteamControllerLED_G = (u8)((f32)SteamController->SetLEDColor.G * 255.0f);
										u8 SteamControllerLED_B = (u8)((F32)SteamController->SetLEDColor.B * 255.0f);
										SteamworksAPI.Controller->SetLEDColor(SteamControllerHandle,
																			  SteamControllerLED_R,
																			  SteamControllerLED_G,
																			  SteamControllerLED_B,
																			  0);
										
										// NOTE(ivan): Vibrate controller if requested.
										if (SteamController->DoVibration.LeftMotorSpeed ||
											SteamController->DoVibration.RightMotorSpeed) {
											u16 LeftSpeed = SteamController->DoVibration.LeftMotorSpeed;
											u16 RightSpeed = SteamController->DoVibration.RightMotorSpeed;
											
											SteamworksAPI.Controller->TriggerVibration(SteamControllerHandle,
																					   LeftSpeed, RightSpeed);
											
											SteamController->DoVibration.LeftMotorSpeed = 0;
											SteamController->DoVibration.RightMotorSpeed = 0;
										}
									}
									
									// NOTE(ivan): Process Win32-specific input events.
									if (GameInput.KbButtons[KeyCode_F4].IsDown &&
										(GameInput.KbButtons[KeyCode_LeftAlt].IsDown ||
										 GameInput.KbButtons[KeyCode_RightAlt].IsDown))
										IsGameRunning = false;

									if (IsNewlyPressed(&GameInput.KbButtons[KeyCode_F2]))
										Win32State.IsDebugCursor = !Win32State.IsDebugCursor;

									// NOTE(ivan): Update game frame.
									GameModule.GameTrigger(GameTriggerType_Frame, 0, 0, 0, 0, 0);

									// NOTE(ivan): Before the next frame, make all Steam-Controllers disconnected.
									// This is needed because unlike with XInputGetState() we are not able to find
									// out whether a controller with a specified index is connected or not.
									for (u32 Index = 0; Index < MaxSteamControllers; Index++)
										GameInput.SteamController[Index].IsConnected = false;

									// NOTE(ivan): Before the next frame, make all input events obsolete.
									for (u32 Index = 0; Index < ArraySize(GameInput.KbButtons); Index++)
										GameInput.KbButtons[Index].IsNew = false;

									for (u32 Index = 0; Index < ArraySize(GameInput.MouseButtons); Index++)
										GameInput.MouseButtons[Index].IsNew = false;

									for (u32 Index = 0; Index < MaxXboxControllers; Index++) {
										xbox_controller_state *XboxController = &GameInput.XboxControllers[Index];

										XboxController->Start.IsNew = false;
										XboxController->Back.IsNew = false;

										XboxController->A.IsNew = false;
										XboxController->B.IsNew = false;
										XboxController->X.IsNew = false;
										XboxController->Y.IsNew = false;

										XboxController->DPadUp.IsNew = false;
										XboxController->DPadDown.IsNew = false;
										XboxController->DPadLeft.IsNew = false;
										XboxController->DPadRight.IsNew = false;

										XboxController->LeftBumper.IsNew = false;
										XboxController->RightBumper.IsNew = false;

										XboxController->LeftStick.IsNew = false;
										XboxController->RightStick.IsNew = false;
									}

									for (u32 Index = 0; Index < MaxSteamControllers; Index++) {
										steam_controller_state *SteamController = &GameInput.SteamControllers[Index];

										SteamController->Start.IsNew = false;
										SteamController->Back.IsNew = false;

										SteamController->A.IsNew = false;
										SteamController->B.IsNew = false;
										SteamController->X.IsNew = false;
										SteamController->Y.IsNew = false;

										SteamController->LeftPad.DPad.Up.IsNew = false;
										SteamController->LeftPad.DPad.Down.IsNew = false;
										SteamController->LeftPad.DPad.Left.IsNew = false;
										SteamController->LeftPad.DPad.Right.IsNew = false;
										SteamController->LeftPad.Click.IsNew = false;

										SteamController->RightPad.DPad.Up.IsNew = false;
										SteamController->RightPad.DPad.Down.IsNew = false;
										SteamController->RightPad.DPad.Left.IsNew = false;
										SteamController->RightPad.DPad.Right.IsNew = false;
										SteamController->RightPad.Click.IsNew = false;

										SteamController->LeftBumper.IsNew = false;
										SteamController->RightBumper.IsNew = false;

										SteamController->Stick.IsNew = false;
									}

									// NOTE(ivan): Escape primary loop if quit has been requested.
									if (Win32API.QuitRequested)
										IsGameRunning = false;

									// NOTE(ivan): Finalize timings and synchronize framerate.
									u64 EndCycleCounter = Win32GetClock();
									f32 CycleSecondsElapsed =
										Win32GetSecondsElapsed(LastCycleCounter, EndCycleCounter);
										
									if (CycleSecondsElapsed < GameTargetFramerate) {
										while (CycleSecondsElapsed < GameTargetFramerate) {
											if (IsSleepGranular) {
												DWORD SleepMS
													= (DWORD)((GameTargetFramerate - CycleSecondsElapsed) * 1000);
												if (SleepMS) // NOTE(ivan): Wa don't want to call Sleep(0).
													Sleep(SleepMS);
											}

											CycleSecondsElapsed
												= Win32GetSecondsElapsed(LastCycleCounter, Win32GetClock());
										}
									}
									GameClocks.SecondsPerFrame = CycleSecondsElapsed;

									u64 EndCPUClockCounter = __rdtsc();
									GameClocks.CPUClocksPerFrame = EndCPUClockCounter - LastCPUClockCounter;

									EndCycleCounter = Win32GetClock();
									GameClocks.FramesPerSecond =
										(f32)((f64)Win32State.PerformanceFrequency
											  / (EndCycleCounter - LastCycleCounter));

									LastCPUClockCounter = __rdtsc();
									LastCycleCounter = EndCycleCounter;
								}
							}

							// NOTE(ivan): Release early-initialized Steamworks APIs.
							SteamworksAPI.Controller->Shutdown();

							// NOTE(ivan): Release game and its module.
							GameModule.GameTrigger(GameTriggerType_Release, 0, 0, 0, 0, 0);
							FreeLibrary(GameModule.GameLibrary);
						} else {
							// NOTE(ivan): Game module cannot be loaded.
							Win32Crashf(GAMENAME "cannot load game DLL!");
						}

						FreeLibrary(SteamworksModule.SteamLibrary);
					} else {
						// NOTE(ivan): Game cannot load Steamworks.
						Win32Crashf(GAMENAME " cannot load Steamworks DLL!");
					}
					
					FreeLibrary(XInputModule.XInputLibrary);

					RawDevices[0].dwFlags = RIDEV_REMOVE;
					RawDevices[1].dwFlags = RIDEV_REMOVE;
					RegisterRawInputDevices(RawDevices, 2, sizeof(RAWINPUTDEVICE));

					ReleaseDC(Window, WindowDC);
				} else {
					// NOTE(ivan): Game window cannot be created.
					Win32Crashf(GAMENAME " window cannot be created!");
				}

				DestroyWindow(Window);
			} else {
				// NOTE(ivan): Game window class cannot be registered.
				Win32Crashf(GAMENAME " window class cannot be registered!");
			}

			VirtualFree(GameMemory.FreeStorage.Base, 0, MEM_RELEASE);
		} else {
			// NOTE(ivan): Game primary storage cannot be allocated.
			Win32Crashf(GAMENAME " primary storage cannnot be allocated!");
		}

		CoUninitialize();
	} else {
		// NOTE(ivan): Obsolete OS.
		Win32Crashf(GAMENAME " requires Windows 7 or newer OS!");
	}

	return 0;
}
