#include "common/save_file.h"

/**
 * @brief Compute the checksum of the save header and saved game data.
 * @param save_data Start of at least SAVE_FILE_CHECKSUM_BYTES readable bytes.
 * @return Twice the byte sum plus SAVE_FILE_CHECKSUM_BIAS.
 * @note The stored checksum, magic, and unused tail bytes are excluded.
 */
s32 compute_save_checksum(const void* save_data)
{
    const u8* cursor;
    s32 byte_sum;
    u32 byte_count;

    cursor = save_data;
    byte_sum = 0;
    byte_count = 0;
    while (byte_count < SAVE_FILE_CHECKSUM_BYTES)
    {
        byte_count++;
        byte_sum += *cursor;
        cursor++;
    }
    return byte_sum * SAVE_FILE_CHECKSUM_MULTIPLIER + SAVE_FILE_CHECKSUM_BIAS;
}
