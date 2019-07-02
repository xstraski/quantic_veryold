#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include "game_platform.h"
#include "game_math.h"

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

// NOTE(ivan): Generic controller DPAD buttons state.
struct controller_dpad_buttons_state {
	input_button_state Up;
	input_button_state Down;
	input_button_state Left;
	input_button_state Right;
};

// NOTE(ivan): Generic controller state.
struct controller_stick_state {
	// NOTE(ivan): Stick position.
	v2 Pos;

	// NOTE(ivan): Stick as a button.
	input_button_state Button;
};

// NOTE(ivan): Generic controller vibration.
struct controller_vibration {
	u16 LeftMotorSpeed;
	u16 RightMotorSpeed;
};

// NOTE(ivan): Xbox controllers maximum count.
// NOTE(ivan): The value is equal to XUSER_MAX_COUNT, but this is defined in MS-DirectX's xinput.h header file,
// which is Win32-specific and must not be used globally, so the maximum count is defined here like that.
#define MAX_XBOX_CONTROLLERS_COUNT 4 // NOTE(ivan): XUSER_MAX_COUNT.

// NOTE(ivan): Xbox controller state.
struct xbox_controller_state {
	b32 IsConnected;
	
	input_button_state Start;
	input_button_state Back;

	input_button_state A;
	input_button_state B;
	input_button_state X;
	input_button_state Y;

	controller_dpad_buttons_state DPad;

	input_button_state LeftBumper;
	input_button_state RightBumper;

	u8 LeftTrigger;
	u8 RightTrigger;

	controller_stick_state LeftStick;
	controller_stick_state RightStick;

	// NOTE(ivan): Set this to non-zero to vibrate the controller or use VibrateXboxController() function.
	// NOTE(ivan): After actual vibration motor speed values will be reset to 0;
	controller_vibration DoVibration;
};

inline void
VibrateXboxController(xbox_controller_state *XboxController, u16 LeftMotorSpeed, u16 RightMotorSpeed) {
	Assert(XboxController);

	XboxController->DoVibration.LeftMotorSpeed = LeftMotorSpeed;
	XboxController->DoVibration.RightMotorSpeed = RightMotorSpeed;
}

#endif // #ifndef GAME_INPUT_H
