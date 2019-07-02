#ifndef GAME_H
#define GAME_H

#include "game_memory.h"
#include "game_keys.h"
#include "game_input.h"

// NOTE(ivan): Game title.
// NOTE(ivan): Should be one single word with no spaces and special symbols.
#define GAMENAME "Quantic"

// NOTE(ivan): Game steam app identifier.
#define GAME_STEAM_APPID k_uAppIdInvalid

// NOTE(ivan): Game memory.
// NOTE(ivan): Game memory structure represents game primary storage which cannot be grown
// and is meant to be partitioned to various memory containers at game initialization. The whole chunk
// of primary storage is always initially zeroed.
struct game_memory {
	piece FreeStorage;     // NOTE(ivan): Storage's starting address of its free space and size of this space in bytes.
	uptr StorageTotalSize; // NOTE(ivan): Storage's total size in bytes.

	// NOTE(ivan): For synchronization, used by EatGameMemory() function.
	ticket_mutex Mutex;
};

// NOTE(ivan): Game clocks and timings for current frame.
// NOTE(ivan): These values will be invalid for the first frame.
struct game_clocks {
	u64 CPUClocksPerFrame;
	f32 SecondsPerFrame;
	f32 FramesPerSecond;
};

// NOTE(ivan): Mouse buttons.
enum mouse_button {
	MouseButton_Left = 0,
	MouseButton_Middle = 1,
	MouseButton_Right = 2,

	MouseButton_X1 = 3,
	MouseButton_X2 = 4,

	MouseButton_MaxCount
};

// NOTE(ivan): Game input state for current frame.
struct game_input {
	// NOTE(ivan): Keyboard and mouse state.
	input_button_state KbButtons[KeyCode_MaxCount];
	input_button_state MouseButtons[MouseButton_MaxCount]; // NOTE(ivan): 0-left, 1-middle, 2-right, 3..4 - extra buttons.
	point MousePos;
	s32 MouseWheel; // NOTE(ivan): Number of scrolls per frame. Negative value says the wheel was rotated backward.

	// NOTE(ivan): Various supported controllers state.
	xbox_controller_state XboxControllers[MAX_XBOX_CONTROLLERS_COUNT];
};

// NOTE(ivan): Command callback function prototype.
#define COMMAND_CALLBACK(Name) b32 Name(char **Params, u32 NumParams)
typedef COMMAND_CALLBACK(command_callback);

// NOTE(ivan): Command. Command is a function that can be called by user at run-time
// using the appropriate interface, such as developer console. Although, it can be called
// directly from the game code.
struct command {
	char Name[128];
	command_callback *Callback;

	command *NextCommand;
	command *PrevCommand;
};

// NOTE(ivan): Commands cache.
struct command_cache {
	command *TopCommand;
	u32 NumCommands;

	ticket_mutex Mutex; // NOTE(ivan): For synchronization.
};

void RegisterCommand(command_cache *Cache, const char *Name, command_callback *Callback);
void UnregisterCommand(command_cache *Cache, const char *Name);
void ExecCommand(command_cache *Cache, const char *Command, ...);

// NOTE(ivan): Setting. A structure that links two strings that tells the name and value.
// NOTE(ivan): Mustly used for working with game configuration settings.
struct setting {
	char Name[128];
	char Value[128];

	setting *NextSetting;
	setting *PrevSetting;
};

// NOTE(ivan): Settings cache.
struct setting_cache {
	setting *TopSetting;
	u32 NumSettings;

	ticket_mutex Mutex; // NOTE(ivan): For synchronization.
};

// NOTE(ivan): Settings load, save, and access.
b32 LoadSettingsFromFile(setting_cache *Cache, const char *FileName);
b32 SaveSettingsToFile(setting_cache *Cache, const char *FileName);
const char * GetSetting(setting_cache *Cache, const char *Name);

// NOTE(ivan): Game globals.
extern struct game_state {
	// NOTE(ivan): Game APIs access.
	platform_api *PlatformAPI;

	// NOTE(ivan): Game data that gets exchanged between platform layer and game module each frame.
	game_memory *GameMemory;
	game_clocks *GameClocks;
	game_input *GameInput;
	
	// NOTE(ivan): Game memory partitions.
	memory_stack PerFrameStack;  // NOTE(ivan): Contains temporary data for one frame, gets cleaned at each new frame.
	memory_stack PermanentStack; // NOTE(ivan): Contains data that will stay online till the program complete shutdown.
	memory_pool CommandsPool;    // NOTE(ivan): Special pool for commands registry.
	memory_pool SettingsPool;    // NOTE(ivan): Special pool for settings cache.

	// NOTE(ivan): Game primary commands and settings caches.
	// NOTE(ivan): Should not be more than once instance of these structure that are meant to be singletons.
	command_cache CommandCache;
	setting_cache SettingCache;
} GameState;

inline void
QuitGame(s32 QuitCode) {
	GameState.PlatformAPI->QuitRequested = true;
	GameState.PlatformAPI->QuitReturnCode = QuitCode;
}

// NOTE(ivan): Game trigger type.
enum game_trigger_type {
	GameTriggerType_Prepare, // NOTE(ivan): Game connection with platform layer, complete initialization.
	GameTriggerType_Release, // NOTE(ivan): Game tear down, all resources release.
	GameTriggerType_Frame    // NOTE(ivan): Game frame update.
};

// NOTE(ivan): Game trigger function prototype.
// NOTE(ivan): Beware that some of the parameters might be 0 depending on trigger type.
#define GAME_TRIGGER(Name) void Name(game_trigger_type TriggerType, \
									 platform_api *PlatformAPI,		\
									 game_memory *GameMemory,		\
									 game_clocks *GameClocks,		\
									 game_input *GameInput)
typedef GAME_TRIGGER(game_trigger);

#endif // #ifndef GAME_H
