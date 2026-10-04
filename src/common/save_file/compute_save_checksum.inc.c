/*
 * Shared save-file function; see include/common/save_file.h. Included by each
 * overlay that has it, at the point where it sits in that overlay's binary.
 */
#include "common/save_file.h"

/**
 * @brief Compute a save file's checksum over its first SAVE_FILE_CHECKSUM_BYTES bytes.
 * @param data Start of the save file.
 * @return Twice the byte sum plus SAVE_FILE_CHECKSUM_BIAS.
 */
s32 compute_save_checksum(void* data)
{
    u8* cursor;
    s32 sum;
    u32 byte_count;

    cursor = data;
    sum = 0;
    byte_count = 0;
    do
    {
        byte_count++;
        sum += *cursor;
        cursor++;
    } while (byte_count < SAVE_FILE_CHECKSUM_BYTES);
    return sum * 2 + SAVE_FILE_CHECKSUM_BIAS;
}
