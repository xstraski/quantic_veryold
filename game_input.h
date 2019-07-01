#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "game_platform.h"
#include "game_math.h"
#include "game_steamworks.h"

// NOTE(ivan): Generic input button state.
struct input_button_state {
	b32 WasDown;
	b32 IsDown;
	b32 IsNew; // NOTE(ivan): True if the state has been set in THIS frame. On next frame this will be false.
};

inline b32
IsNewlyPressed(input_button_state *Button) {
	Assert(Button);
	return !Button->WasDown && Button->IsDown && Button->IsNew;
}

// NOTE(ivan): Xbox controllers maximum count.
// NOTE(ivan): The value is equal to XUSER_MAX_COUNT, but this is defined in MS-DirectX's xinput.h header file,
// which is Win32-specific header and must not be used globally, so the maximum count is defined here like that.
#define MAX_XBOX_CONTROLLERS_COUNT 4 // NOTE(ivan): XUSER_MAX_COUNT.

// NOTE(ivan): Xbox controller battery type.
enum xbox_controller_battery_type {
	XboxControllerBatteryType_Disconnected,
	XboxControllerBatteryType_Wired,
	XboxControllerBatteryType_Alkaline,
	XboxControllerBatteryType_Nimh,
	XboxControllerBatteryType_Unknown
};

// NOTE(ivan): Xbox controller battery charge level.
enum xbox_controller_battery_level {
	XboxControllerBatteryLevel_Empty,
	XboxControllerBatteryLevel_Low,
	XboxControllerBatteryLevel_Medium,
	XboxControllerBatteryLevel_Full,
	XboxControllerBatteryLevel_Unknown
};

// NOTE(ivan): Xbox controller state.
struct xbox_controller_state {
	b32 IsConnected;

	xbox_controller_battery_type BatteryType;
	xbox_controller_battery_level BatteryLevel;
	
	input_button_state Start;
	input_button_state Back;

	input_button_state A;
	input_button_state B;
	input_button_state X;
	input_button_state Y;

	input_button_state Up;
	input_button_state Down;
	input_button_state Left;
	input_button_state Right;

	input_button_state LeftBumper;
	input_button_state RightBumper;

	input_button_state LeftStick;
	input_button_state RightStick;

	u8 LeftTrigger;
	u8 RightTrigger;

	v2 LeftStickPos;
	v2 RightStickPos;

	// NOTE(ivan): Set this to non-zero to vibrate the controller.
	// NOTE(ivan): After actual vibration these will be zeroed again.
	u16 DoLeftVibration;
	u16 DoRightVibration;
};

// NOTE(ivan): Steam maximum controllers count.
// NOTE(ivan): STEAM_INPUT_MAX_COUNT is defined in Steamworks API's isteaminput.h header file.
#define MAX_STEAM_CONTROLLERS_COUNT STEAM_INPUT_MAX_COUNT

// NOTE(ivan): Steam controller state.
// TODO(ivan): Still not complete Steam controller representation!
struct steam_controller_state {
	b32 IsConnected;

	input_button_state Start;
	input_button_state Back;
	
	input_button_state A;
	input_button_state B;
	input_button_state X;
	input_button_state Y;

	input_button_state Stick;
	v2 StickPos;
	
	// NOTE(ivan): Set this to non-zero to vibrate the controller.
	// NOTE(ivan): After actual vibration these will be zeroed again.
	u16 DoLeftVibration;
	u16 DoRightVibration;

	// NOTE(ivan): These allows to set the LED color.
	// NOTE(ivan): These will remain unchanged by the platform layer.
	v3 SetLEDColor;
};

#endif // #ifndef GAME_INPUT_H
