#ifndef GAME_PLATFORM_WIN32_H
#define GAME_PLATFORM_WIN32_H

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

// NOTE(ivan): Win32-specific renderer initialization parameters.
struct renderer_init_platform_specific {
	HWND TargetWindow;
};

#endif // #ifndef GAME_PLATFORM_WIN32_H
