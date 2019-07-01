#ifndef GAME_MATH_H
#define GAME_MATH_H

#include "game_platform.h"

#define Percentage(Percent, Value) ((u32)((f32)Value * (f32)Percent / 100.0f))

#define Square(Value) ((Value) * (Value))
#define Cube(Value) ((Value) * (Value) * (Value))

// NOTE(ivan): 2D point.
struct point {
	s32 X;
	s32 Y;
};

// NOTE(ivan): 2D rectangle.
struct rectangle {
	s32 Width;
	s32 Height;
};

// NOTE(ivan): 2D vector.
struct v2 {
	union {
		struct {
			f32 X;
			f32 Y;
		};
		struct {
			f32 U;
			f32 V;
		};
		f32 E[2];
	};
};

// NOTE(ivan): 3D vector.
struct v3 {
	union {
		struct {
			f32 X;
			f32 Y;
			f32 Z;
		};
		struct {
			f32 R;
			f32 G;
			f32 B;
		};
		f32 E[3];
	};
};

#endif // #ifndef GAME_MATH_H
