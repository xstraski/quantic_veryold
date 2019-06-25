#include "game_misc.h"

char **
TokenizeString(const char *String, u32 *NumTokens, const char *Delims) {
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
	char **Result = (char **)0;

	return Result;
}
