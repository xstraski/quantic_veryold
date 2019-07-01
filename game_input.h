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

// NOTE(ivan): Returns new only if the button was pressed at exactly current frame, not earlier.
inline b32
IsNewlyPressed(input_button_state *Button) {
	Assert(Button);
	return !Button->WasDown && Button->IsDown && Button->IsNew;
}

// NOTE(ivan): Generic controller DPAD state.
struct controller_dpad_state {
	input_button_state Up;
	input_button_state Down;
	input_button_state Left;
	input_button_state Right;
};

// NOTE(ivan): Generic controller vibration.
struct controller_vibration {
	u16 LeftMotorSpeed;
	u16 RightMotorSpeed;
};

// NOTE(ivan): Xbox controllers maximum count.
// NOTE(ivan): The value is equal to XUSER_MAX_COUNT, but this is defined in MS-DirectX's xinput.h header file,
// which is Win32-specific header and must not be used globally, so the maximum count is defined here like that.
#define MAX_XBOX_CONTROLLERS_COUNT 4 // NOTE(ivan): XUSER_MAX_COUNT.

// NOTE(ivan): Xbox controller stick state.
struct xbox_controller_stick_state {
	input_button_state Button;
	v2 Pos;
};

// NOTE(ivan): Xbox controller state.
struct xbox_controller_state {
	b32 IsConnected;
	
	input_button_state Start;
	input_button_state Back;

	input_button_state A;
	input_button_state B;
	input_button_state X;
	input_button_state Y;

	controller_dpad_state DPad;

	input_button_state LeftBumper;
	input_button_state RightBumper;

	u8 LeftTrigger;
	u8 RightTrigger;

	xbox_controller_stick_state LeftStick;
	xbox_controller_stick_state RightStick;

	// NOTE(ivan): Set this to non-zero to vibrate the controller.
	// NOTE(ivan): After actual vibration these will be zeroed again.
	controller_vibration DoVibration;
};

// NOTE(ivan): Steam maximum controllers count.
// NOTE(ivan): STEAM_CONTROLLER_MAX_COUNT is defined in Steamworks API's isteamcontroller.h header file.
#define MAX_STEAM_CONTROLLERS_COUNT STEAM_CONTROLLER_MAX_COUNT

// NOTE(ivan): Steam controller pad state.
struct steam_controller_pad_state {
	controller_dpad_state DPad;
	input_button_state Button;

	b32 IsSwipeHappened;
	b32 IsTouchHappened;
};

// NOTE(ivan): Steam controller trigger state.
struct steam_controller_trigger_state {
	input_button_state Button;
	u32 PullForce;
};

// NOTE(ivan): Steam controller stick state.
struct steam_controller_stick_state {
	// NOTE(ivan): Stick position.
    controller_dpad_state DPad;
	v2 Pos;

	// NOTE(ivan): Stick as a button.
	input_button_state Button;
};

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

	input_button_state LeftBumper;
	input_button_state RightBumper;

	steam_controller_pad_state LeftPad;
	steam_controller_pad_state RightPad;

	steam_controller_trigger_state LeftTrigger;
	steam_controller_trigger_state RightTrigger;

	// NOTE(ivan): Left stick, the only one on the controller.
	steam_controller_stick_state LeftStick;
	
	// NOTE(ivan): Set this to non-zero to vibrate the controller.
	// NOTE(ivan): After actual vibration these will be zeroed again.
	controller_vibration DoVibration;

	// NOTE(ivan): This allows to set the LED color of the controller.
	// NOTE(ivan): The color will remain unchanged by the platform layer.
	v3 SetLEDColor;
};

#endif // #ifndef GAME_INPUT_H
