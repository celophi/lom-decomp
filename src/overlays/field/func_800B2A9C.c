#include "common.h"

extern u8 *D_80123FB0;
void akao_set_song_params(s32, s32, s32, s32);

/**
 * @brief Finds a field-state record with the requested byte identifier.
 *
 * Searches eleven 0x68-byte records using the identifier at offset 0x2C and
 * returns the corresponding payload at offset 0x28. A failed search issues an
 * AKAO diagnostic command and returns null.
 *
 * @param value Identifier to find.
 * @return Pointer to the matching record payload, or null when absent.
 * @note 100% match. Loading the base before initializing the offset
 *       reproduces the target load/copy allocation.
 */
u8 *func_800B2A9C(s32 value)
{
    s32 offset;
    s32 index;
    u8 *record;
    u8 *base;

    index = 0;
    base = D_80123FB0;
    offset = 0x28;
    record = base;
    do
    {
        index++;
        if (value != record[0x2C])
        {
            offset += 0x68;
            record += 0x68;
        }
        else
        {
            return base + offset;
        }
    } while (index < 0xB);
    akao_set_song_params(0x8001, 0x68, value, -1);
    return 0;
}
