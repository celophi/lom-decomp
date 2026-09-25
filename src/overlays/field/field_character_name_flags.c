/** @file field_character_name_flags.c
 * @brief Recognise memory-card saves of other games by their product code.
 */

#include "common.h"
#include "main.h"
#include "sdk/strings.h"

/** @brief Number of entries in the known product-code table. */
#define FIELD_KNOWN_SAVE_CODE_COUNT 11

/** @brief Length of one product code (region prefix plus product id, no terminator). */
#define FIELD_KNOWN_SAVE_CODE_LENGTH 12

/** @brief Memory-card file-name prefixes of the recognised saves. */
extern char g_field_known_save_codes[FIELD_KNOWN_SAVE_CODE_COUNT][FIELD_KNOWN_SAVE_CODE_LENGTH];

/**
 * @brief Set the PadContext.known_save_flags bit of every product code that @p file_name starts with.
 * @param file_name Memory-card file name of one directory entry.
 */
void field_flag_known_save(char* file_name)
{
    s32 i;

    for (i = 0; i < FIELD_KNOWN_SAVE_CODE_COUNT; i++)
    {
        if (strncmp(file_name, g_field_known_save_codes[i], FIELD_KNOWN_SAVE_CODE_LENGTH) == 0)
        {
            g_pad_ctx->known_save_flags |= 1 << i;
        }
    }
}
