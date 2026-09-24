/** @file field_character_name_flags.c
 * @brief Set named-character flags by looking up the fixed character-name table.
 */

#include "common.h"

/** @brief Number of entries in the character-name table. */
#define FIELD_CHARACTER_NAME_COUNT 11

/** @brief Length of one fixed-width character-name entry, in bytes. */
#define FIELD_CHARACTER_NAME_LENGTH 12

/** @brief Byte offset of the named-character flag word in the pad context. */
#define FIELD_CHARACTER_NAME_FLAGS_OFFSET 0x204

extern u8* g_pad_ctx;
extern char D_800ECFDC[FIELD_CHARACTER_NAME_COUNT][FIELD_CHARACTER_NAME_LENGTH];

/**
 * @brief Set the flag bit of every character-name table entry that equals @p name.
 * @param name Name to compare, up to FIELD_CHARACTER_NAME_LENGTH bytes.
 */
void func_800B0170(char* name)
{
    s32 i;

    for (i = 0; i < FIELD_CHARACTER_NAME_COUNT; i++)
    {
        if (strncmp(name, D_800ECFDC[i], FIELD_CHARACTER_NAME_LENGTH) == 0)
        {
            *(u32*)(g_pad_ctx + FIELD_CHARACTER_NAME_FLAGS_OFFSET) |= 1 << i;
        }
    }
}
