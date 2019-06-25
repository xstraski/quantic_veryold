#ifndef GAME_MISC_H
#define GAME_MISC_H

#include "game_platform.h"

// NOTE(ivan): String tokenizer, splits a given string to a seperate tokens using a specified delimiters array.
char ** TokenizeString(const char *String, u32 *NumTokens, const char *Delims);
void FreeTokenizedString(char **Tokens, u32 NumTokens);

#endif // #ifndef GAME_MISC_H
