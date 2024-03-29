#ifndef GAME_MISC_H
#define GAME_MISC_H

#include "game_platform.h"

// NOTE(ivan): String tokenizer, splits a given string to a seperate tokens using a specified delimiters array.
char ** TokenizeString(memory_heap *Heap, const char *String, u32 *NumTokens, const char *Delims);
void FreeTokenizedString(memory_heap *Heap, char **Tokens, u32 NumTokens);

// NOTE(ivan): File I/O utilities.
piece ReadEntireFile(memory_stack *Stack, const char *FileName); // NOTE(ivan): Should be released by PopStack().
b32 WriteEntireFile(const char *FileName, void *Buffer, uptr Size);
uptr GetFileSizeByName(const char *FileName);
uptr GetFileSizeByHandle(file_handle FileHandle);
u32 GetLineFromFile(file_handle FileHandle, char *Buffer, u32 BufferSize);

#endif // #ifndef GAME_MISC_H
