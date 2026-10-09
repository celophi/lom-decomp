#include "main/cdrom_decompress.h"
#include "internal/cdrom_internal.h"

#define CD_STREAM_PAYLOAD_START (CD_STREAM_BUFFER_START + 1)
#define CD_DECOMPRESS_LOW_NIBBLE_MASK 0x0F
#define CD_DECOMPRESS_HIGH_NIBBLE_MASK 0xF0
/** @brief Shortest copy CD_DECOMPRESS_COPY_8_BIT can encode. */
#define CD_DECOMPRESS_COPY_8_BIT_BASE_LENGTH 20

/**
 * @brief Control bytes in the compressed resource format.
 *
 * Bytes 0x00 to 0xEF start a literal run of (byte + 1) bytes. In the formats
 * below, n is the count byte that follows the opcode. Back-reference distances
 * count from the previous output byte, so distance 0 repeats that byte.
 */
typedef enum CdDecompressOpcode
{
    CD_DECOMPRESS_REPEAT_NIBBLE = 0xF0,      /**< [b]: high nibble of b, (low nibble + 3) times. */
    CD_DECOMPRESS_REPEAT_BYTE = 0xF1,        /**< [n, v]: v, n + 4 times. */
    CD_DECOMPRESS_REPEAT_NIBBLE_PAIR = 0xF2, /**< [n, b]: low nibble then high nibble of b, n + 2 times. */
    CD_DECOMPRESS_REPEAT_PAIR = 0xF3,        /**< [n, a, b]: a, b, n + 2 times. */
    CD_DECOMPRESS_REPEAT_TRIPLET = 0xF4,     /**< [n, a, b, c]: a, b, c, n + 2 times. */
    CD_DECOMPRESS_INTERLEAVE_BYTE = 0xF5,    /**< [n, a, then n + 4 literals]: a before each literal. */
    CD_DECOMPRESS_INTERLEAVE_PAIR = 0xF6,    /**< [n, a, b, then n + 3 literals]: a, b before each literal. */
    CD_DECOMPRESS_INTERLEAVE_TRIPLET = 0xF7, /**< [n, a, b, c, then n + 2 literals]: a, b, c before each literal. */
    CD_DECOMPRESS_ASCENDING_RUN = 0xF8,      /**< [n, v]: n + 4 bytes counting up from v. */
    CD_DECOMPRESS_DESCENDING_RUN = 0xF9,     /**< [n, v]: n + 4 bytes counting down from v. */
    CD_DECOMPRESS_STEPPED_RUN = 0xFA,        /**< [n, v, step]: n + 5 bytes from v, adding step each time. */
    CD_DECOMPRESS_PAIR_DELTA_RUN = 0xFB,     /**< [n, lo, hi, d]: n + 3 little-endian halfwords from hi:lo, adding signed d. */
    CD_DECOMPRESS_COPY_12_BIT = 0xFC,        /**< [lo, b]: copy (high nibble of b) + 4 bytes from distance (low nibble of b):lo. */
    CD_DECOMPRESS_COPY_8_BIT = 0xFD,         /**< [d, n]: copy n + 20 bytes from distance d. */
    CD_DECOMPRESS_COPY_NIBBLE = 0xFE,        /**< [b]: copy (low nibble + 3) bytes from distance (high nibble * 8) + 7. */
    CD_DECOMPRESS_END = 0xFF,                /**< End of the stream. */
} CdDecompressOpcode;

/**
 * @brief Decompresses a custom bytecode-encoded data stream.
 *
 * Processes opcodes from a source buffer and emits uncompressed bytes into a
 * destination buffer. Both pointers are updated in-place so the caller can
 * resume across multiple calls.
 *
 * Bounds are checked between opcode expansions. Back-references require
 * the preceding output to remain available.
 *
 * @param src_cursor Current source position; updated on return.
 * @param dst_cursor Current destination position; updated on return.
 * @param src_end Exclusive source bound.
 * @param dst_end Exclusive destination bound.
 *
 * @return FALSE at the end marker; TRUE when a buffer bound pauses decoding.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/1Feag
 */
s32 cdrom_decompress_data(u8** src_cursor, u8** dst_cursor, u8* src_end, u8* dst_end)
{
    u8* source;
    u8* destination;
    u32 iterations;
    u32 opcode;

    u8* copy_source;
    u8 count_byte;

    u8 pattern_second;
    u8 pattern_first;
    u8 value_low;
    u8 pattern_third;

    u32 value_high;
    u32 high_word;
    u32 next_value;

    s32 delta;

    u16 distance;

    source = *src_cursor;
    destination = *dst_cursor;

    while (source < src_end && destination < dst_end)
    {
        opcode = *source;

        switch (opcode)
        {
        case CD_DECOMPRESS_REPEAT_NIBBLE:
            pattern_first = source[1];

            source += 2;
            iterations = (pattern_first & CD_DECOMPRESS_LOW_NIBBLE_MASK) + 3;
            pattern_first >>= 4;

            do
            {
                *destination++ = pattern_first;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_REPEAT_BYTE:
            pattern_first = source[2];
            count_byte = source[1];

            source += 3;
            iterations = count_byte + 4;

            do
            {
                *destination++ = pattern_first;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_REPEAT_NIBBLE_PAIR:
            pattern_first = source[2];
            count_byte = source[1];

            source += 3;
            iterations = count_byte + 2;
            value_high = pattern_first >> 4;
            pattern_first = pattern_first & CD_DECOMPRESS_LOW_NIBBLE_MASK;

            do
            {
                destination[0] = pattern_first;
                destination[1] = value_high;
                destination += 2;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_REPEAT_PAIR:
            pattern_first = source[2];
            pattern_second = source[3];
            count_byte = source[1];

            source += 4;
            iterations = count_byte + 2;

            do
            {
                destination[0] = pattern_first;
                destination[1] = pattern_second;
                destination += 2;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_REPEAT_TRIPLET:
            pattern_first = source[2];
            pattern_second = source[3];
            pattern_third = source[4];
            count_byte = source[1];

            source += 5;
            iterations = count_byte + 2;

            do
            {
                destination[0] = pattern_first;
                destination[1] = pattern_second;
                destination[2] = pattern_third;
                destination += 3;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_INTERLEAVE_BYTE:
            pattern_first = source[2];
            count_byte = source[1];

            source += 3;
            iterations = count_byte + 4;

            do
            {
                destination[0] = pattern_first;
                destination[1] = *source++;
                destination += 2;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_INTERLEAVE_PAIR:
            pattern_first = source[2];
            pattern_second = source[3];
            count_byte = source[1];

            source += 4;
            iterations = count_byte + 3;

            do
            {
                destination[0] = pattern_first;
                destination[1] = pattern_second;
                destination[2] = *source++;
                destination += 3;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_INTERLEAVE_TRIPLET:
            pattern_first = source[2];
            pattern_second = source[3];
            pattern_third = source[4];
            count_byte = source[1];

            source += 5;
            iterations = count_byte + 2;

            do
            {
                destination[0] = pattern_first;
                destination[1] = pattern_second;
                destination[2] = pattern_third;
                destination[3] = *source++;
                destination += 4;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_ASCENDING_RUN:
            pattern_first = source[2];
            count_byte = source[1];

            source += 3;
            iterations = count_byte + 4;

            do
            {
                *destination++ = pattern_first;
                pattern_first += 1;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_DESCENDING_RUN:
            pattern_first = source[2];
            count_byte = source[1];

            source += 3;
            iterations = count_byte + 4;

            do
            {
                *destination++ = pattern_first;
                pattern_first -= 1;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_STEPPED_RUN:
            pattern_first = source[2];
            pattern_second = source[3];
            count_byte = source[1];

            source += 4;
            iterations = count_byte + 5;

            do
            {
                *destination++ = pattern_first;
                pattern_first += pattern_second;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_PAIR_DELTA_RUN:
            value_low = source[2];
            value_high = source[3];
            count_byte = source[1];
            pattern_third = source[4];

            iterations = count_byte + 3;
            delta = pattern_third;
            source += 5;

            do
            {
                destination[0] = value_low;
                destination[1] = value_high;
                destination += 2;

                next_value = (s8)delta;

                high_word = value_high << 8;
                next_value += value_low | high_word;

                value_low = next_value;
                value_high = next_value >> 8;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_COPY_12_BIT:
            pattern_first = source[1];
            value_high = source[2];

            source += 3;
            iterations = (value_high >> 4) + 4;

            distance = pattern_first | ((value_high & CD_DECOMPRESS_LOW_NIBBLE_MASK) << 8);
            copy_source = destination - distance;

            do
            {
                *destination++ = copy_source[-1];
                copy_source++;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_COPY_8_BIT:
            value_low = source[1];
            count_byte = source[2];

            source += 3;
            iterations = count_byte + CD_DECOMPRESS_COPY_8_BIT_BASE_LENGTH;
            copy_source = destination - value_low;

            do
            {
                *destination++ = copy_source[-1];
                copy_source++;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_COPY_NIBBLE:
            pattern_first = source[1];

            source += 2;
            iterations = (pattern_first & CD_DECOMPRESS_LOW_NIBBLE_MASK) + 3;
            copy_source = destination - ((pattern_first & CD_DECOMPRESS_HIGH_NIBBLE_MASK) >> 1);

            do
            {
                *destination++ = copy_source[-8];
                copy_source++;
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_END:
            *src_cursor = &source[1];
            *dst_cursor = destination;
            return FALSE;

        default:
            source++;
            iterations = opcode + 1;

            do
            {
                *destination++ = *source++;
            } while (--iterations != 0);
            break;
        }

        *src_cursor = source;
    }

    *dst_cursor = destination;
    return TRUE;
}

/**
 * @brief Supplies ring-buffer destinations for streamed CD sectors.
 *
 * Initializes the scratchpad stream state, compacts unread input after the
 * consumer releases it, and wraps incoming sectors when the upper buffer fills.
 *
 * @param byte_count Bytes delivered before the pending sector; zero for the first sector.
 * @param bytes_remaining Bytes remaining, including the pending sector.
 *
 * @return Next sector destination, or NULL when the sector must be retried.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/UDwSD
 */
u8* cdrom_handle_stream_data(s32 byte_count, u32 bytes_remaining)
{
    s32 alignment_padding;
    s32 word_count;
    CdStreamCopyCursor destination;
    CdStreamCopyCursor copy_source;
    u32 bytes_buffered;
    u32 bytes_consumed;
    u32 wrap_overflow;
    u32 wrap_write_offset;
    u8* wrapped_read_ptr;
    u8* linear_read_ptr;
    u8* current_read_ptr;
    u8* aligned_buffer_start;
    u32 transfer_size;
    u8* next_sector_dst;

    transfer_size = bytes_remaining;
    if (bytes_remaining > CD_DATA_SECTOR_SIZE)
    {
        transfer_size = CD_DATA_SECTOR_SIZE;
    }

    if (byte_count == 0)
    {
        /* The first byte is a stream header; compressed input begins at byte one. */
        CD_STREAM_STATE.data_ready = TRUE;
        CD_STREAM_STATE.input_cursor = CD_STREAM_PAYLOAD_START;
        CD_STREAM_STATE.buffer_start = CD_STREAM_PAYLOAD_START;
        CD_STREAM_STATE.bytes_buffered = transfer_size - 1;
        CD_STREAM_STATE.wrap_overflow = 0;
        return CD_STREAM_BUFFER_START;
    }

    if (!CD_STREAM_STATE.data_ready)
    {
        /* From here on byte_count is the number of bytes the decompressor has not read. */
        byte_count = CD_STREAM_STATE.bytes_buffered - CD_STREAM_STATE.bytes_consumed;
        bytes_consumed = CD_STREAM_STATE.bytes_consumed;
        wrap_overflow = CD_STREAM_STATE.wrap_overflow;
        alignment_padding = (CD_BYTES_PER_WORD - (byte_count & CD_BYTES_PER_WORD_MASK)) & CD_BYTES_PER_WORD_MASK;
        if (wrap_overflow != 0)
        {
            wrapped_read_ptr = CD_STREAM_STATE.buffer_start;
            destination.bytes = CD_STREAM_WRAP_START - byte_count;
            CD_STREAM_STATE.input_cursor = destination.bytes;
            CD_STREAM_STATE.buffer_start = destination.bytes;
            destination.bytes -= alignment_padding;
            CD_STREAM_STATE.bytes_buffered = (wrap_overflow + byte_count) + transfer_size;
            copy_source.bytes = (wrapped_read_ptr + bytes_consumed) - alignment_padding;
            word_count = (byte_count + CD_BYTES_PER_WORD_MASK) / CD_BYTES_PER_WORD;
            for (word_count--; word_count != -1; word_count--)
            {
                *destination.words = *copy_source.words;
                copy_source.bytes += CD_BYTES_PER_WORD;
                destination.bytes += CD_BYTES_PER_WORD;
            }
            destination.bytes += CD_STREAM_STATE.wrap_overflow;
            CD_STREAM_STATE.wrap_overflow = 0;
        }
        else
        {
            destination.bytes = CD_STREAM_BUFFER_START;
            CD_STREAM_STATE.bytes_buffered = byte_count + transfer_size;
            linear_read_ptr = CD_STREAM_STATE.buffer_start;
            aligned_buffer_start = CD_STREAM_BUFFER_START + alignment_padding;
            CD_STREAM_STATE.input_cursor = aligned_buffer_start;
            CD_STREAM_STATE.buffer_start = aligned_buffer_start;
            copy_source.bytes = (linear_read_ptr + bytes_consumed) - alignment_padding;
            word_count = (byte_count + CD_BYTES_PER_WORD_MASK) / CD_BYTES_PER_WORD;
            for (word_count--; word_count != -1; word_count--)
            {
                *destination.words = *copy_source.words;
                copy_source.bytes += CD_BYTES_PER_WORD;
                destination.bytes += CD_BYTES_PER_WORD;
            }
        }
        CD_STREAM_STATE.data_ready = TRUE;
        return destination.bytes;
    }

    current_read_ptr = CD_STREAM_STATE.buffer_start;
    bytes_buffered = CD_STREAM_STATE.bytes_buffered;
    wrap_write_offset = CD_STREAM_STATE.wrap_overflow;
    destination.bytes = current_read_ptr + bytes_buffered;

    if ((wrap_write_offset != 0) || ((destination.bytes + transfer_size) > CD_STREAM_BUFFER_LIMIT))
    {
        destination.bytes = CD_STREAM_WRAP_START + wrap_write_offset;
        if (CD_STREAM_STATE.input_cursor >= (destination.bytes + transfer_size))
        {
            CD_STREAM_STATE.wrap_overflow = wrap_write_offset + transfer_size;
        }
        else
        {
            CD_STREAM_STATE.deferred_sectors++;
            return NULL;
        }
    }
    else
    {
        CD_STREAM_STATE.bytes_buffered = bytes_buffered + transfer_size;
    }

    next_sector_dst = destination.bytes;
    if (bytes_remaining == transfer_size)
    {
        CD_STREAM_STATE.input_complete = TRUE;
        next_sector_dst = destination.bytes;
        return next_sector_dst;
    }
    return next_sector_dst;
}

/**
 * @brief Decompresses a complete bytecode-encoded buffer.
 *
 * Skips the stream header and decodes without source or destination bounds.
 *
 * @param source Compressed stream including its one-byte header.
 * @param destination Output buffer.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/JFLMN
 */
void cdrom_decompress_buffer(u8* source, u8* destination)
{
    source++;
    while (cdrom_decompress_data(&source, &destination, CD_DECOMPRESS_UNBOUNDED_END, CD_DECOMPRESS_UNBOUNDED_END) != FALSE);
}

/**
 * @brief Clear a stream flag shared with the CD sector callback.
 *
 * @param data_ready Flag to clear.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Y4pUH
 */
void cdrom_clear_data_ready(volatile u8* data_ready)
{
    *data_ready = FALSE;
}
