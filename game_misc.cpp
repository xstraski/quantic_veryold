#include "game_misc.h"

char **
TokenizeString(memory_stack *Stack, const char *String, u32 *NumTokens, const char *Delims) {
	Assert(Stack);
	Assert(String);
	Assert(NumTokens);
	Assert(Delims);

	const char *Ptr = String;
	*NumTokens = 0;

	// NOTE(ivan): Iterate to count tokens.
	b32 WasDelim = true;
	while (true) {
		if (strchr(Delims, *Ptr) || *Ptr == 0) {
			if (!WasDelim)
				(*NumTokens)++;

			WasDelim = true;
		} else {
			WasDelim = false;
		}

		if (*Ptr == 0)
			break;

		Ptr++;
	}

	if ((*NumTokens) == 0)
		return 0;

	// NOTE(ivan): Allocate needed space.
	char **Result = (char **)AllocFromStack(Stack, sizeof(char *) * (*NumTokens));
	if (Result) {
		// NOTE(ivan): Iterate all over again to capture tokens.
		u32 It = 0;
		const char *Last = Ptr = String;

		WasDelim = true;
		while (true) {
			if (strchr(Delims, *Ptr) || *Ptr == 0) {
				if (!WasDelim) {
					u32 Diff = (u32)(Ptr - Last);
		}
	}

	return Result;
}
