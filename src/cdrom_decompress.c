#include "cdrom_decompress.h"
#include "cdrom_internal.h"

#define CD_STREAM_BUFFER_START ((u8*)0x801DC000)
#define CD_STREAM_PAYLOAD_START ((u8*)0x801DC001)
#define CD_STREAM_BUFFER_LIMIT ((u8*)0x801DE000)
#define CD_DECOMPRESS_LOW_NIBBLE_MASK 0x0F
#define CD_DECOMPRESS_HIGH_NIBBLE_MASK 0xF0
#define CD_DECOMPRESS_COPY_8_BIT_BASE_LENGTH 0x14

/** @brief Control bytes in the compressed resource format. */
typedef enum CdDecompressOpcode
{
    CD_DECOMPRESS_REPEAT_NIBBLE = 0xF0,
    CD_DECOMPRESS_REPEAT_BYTE = 0xF1,
    CD_DECOMPRESS_REPEAT_NIBBLE_PAIR = 0xF2,
    CD_DECOMPRESS_REPEAT_PAIR = 0xF3,
    CD_DECOMPRESS_REPEAT_TRIPLET = 0xF4,
    CD_DECOMPRESS_INTERLEAVE_BYTE = 0xF5,
    CD_DECOMPRESS_INTERLEAVE_PAIR = 0xF6,
    CD_DECOMPRESS_INTERLEAVE_TRIPLET = 0xF7,
    CD_DECOMPRESS_ASCENDING_RUN = 0xF8,
    CD_DECOMPRESS_DESCENDING_RUN = 0xF9,
    CD_DECOMPRESS_STEPPED_RUN = 0xFA,
    CD_DECOMPRESS_PAIR_DELTA_RUN = 0xFB,
    CD_DECOMPRESS_COPY_12_BIT = 0xFC,
    CD_DECOMPRESS_COPY_8_BIT = 0xFD,
    CD_DECOMPRESS_COPY_NIBBLE = 0xFE,
    CD_DECOMPRESS_END = 0xFF,
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
 * @see decomp.me: (100%) https://decomp.me/scratch/1Feag
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

    s32 delta_bits;
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
            iterations = (pattern_first & 0xf) + 3;
            pattern_first = pattern_first >> 4;

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
            pattern_first = pattern_first & 0xf;

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
                *destination = pattern_first;
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
            delta_bits = (u32)pattern_third << 24;
            source += 5;

            do
            {
                destination[0] = value_low;
                destination[1] = value_high;
                destination += 2;

                next_value = delta_bits >> 24;

                high_word = value_high << 8;
                next_value += value_low | high_word;

                value_low = next_value;
                value_high = (next_value >> 8);
            } while (--iterations != 0);
            break;

        case CD_DECOMPRESS_COPY_12_BIT:
            pattern_first = source[1];
            opcode = source[2];

            source += 3;
            iterations = (opcode >> 4) + 4;

            distance = pattern_first | ((opcode & CD_DECOMPRESS_LOW_NIBBLE_MASK) << 8);
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
            iterations = (pattern_first & 0xF) + 3;
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
            return 0;

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
    return 1;
}

/**
 * @brief Supplies ring-buffer destinations for streamed CD sectors.
 *
 * Initializes the scratchpad stream state, compacts unread input after the
 * consumer releases it, and wraps incoming sectors when the upper buffer fills.
 *
 * @param bytes_transferred Bytes delivered before the pending sector.
 * @param bytes_remaining Bytes remaining, including the pending sector.
 *
 * @return Next sector destination, or NULL when the sector must be retried.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/UDwSD
 */
u8* cdrom_handle_stream_data(s32 bytes_transferred, u32 bytes_remaining)
{
    s32 unconsumed_bytes;
    s32 alignment_padding;
    s32 wrapped_word_count;
    s32 linear_word_count;
    CdStreamCopyCursor destination;
    CdStreamCopyCursor wrapped_source;
    CdStreamCopyCursor linear_source;
    u32 bytes_buffered;
    u32 bytes_consumed;
    u32 wrap_overflow;
    u32 wrap_write_offset;
    u8* wrapped_read_ptr;
    u8* linear_read_ptr;
    u8* current_read_ptr;
    u32 previous_wrap_overflow;
    u8* aligned_buffer_start;
    u32 transfer_size;
    u8* next_sector_dst;

    transfer_size = bytes_remaining;
    if (bytes_remaining > CD_DATA_SECTOR_SIZE)
    {
        transfer_size = CD_DATA_SECTOR_SIZE;
    }

    if (bytes_transferred == 0)
    {
        // The first byte is a stream header; compressed input begins at byte one.
        CD_STREAM_STATE.data_ready = TRUE;
        CD_STREAM_STATE.write_ptr = CD_STREAM_PAYLOAD_START;
        CD_STREAM_STATE.read_ptr = CD_STREAM_PAYLOAD_START;
        CD_STREAM_STATE.bytes_buffered = transfer_size - 1;
        CD_STREAM_STATE.wrap_overflow = 0;
        return CD_STREAM_BUFFER_START;
    }

    if (!CD_STREAM_STATE.data_ready)
    {
        unconsumed_bytes = CD_STREAM_STATE.bytes_buffered - CD_STREAM_STATE.bytes_consumed;
        bytes_consumed = CD_STREAM_STATE.bytes_consumed;
        wrap_overflow = CD_STREAM_STATE.wrap_overflow;
        alignment_padding = (CD_STREAM_COPY_WORD_SIZE - (unconsumed_bytes & CD_STREAM_COPY_WORD_MASK)) & CD_STREAM_COPY_WORD_MASK;
        if (wrap_overflow != 0)
        {
            wrapped_read_ptr = CD_STREAM_STATE.read_ptr;
            destination.bytes = CD_STREAM_WRAP_START - unconsumed_bytes;
            CD_STREAM_STATE.write_ptr = destination.bytes;
            CD_STREAM_STATE.read_ptr = destination.bytes;
            destination.bytes -= alignment_padding;
            CD_STREAM_STATE.bytes_buffered = (wrap_overflow + unconsumed_bytes) + transfer_size;
            wrapped_source.bytes = (wrapped_read_ptr + bytes_consumed) - alignment_padding;
            wrapped_word_count = (unconsumed_bytes + CD_STREAM_COPY_WORD_MASK) / CD_STREAM_COPY_WORD_SIZE;
            for (wrapped_word_count--; wrapped_word_count != -1; wrapped_word_count--)
            {
                *destination.words = *wrapped_source.words;
                wrapped_source.bytes += CD_STREAM_COPY_WORD_SIZE;
                destination.bytes += CD_STREAM_COPY_WORD_SIZE;
            }
            previous_wrap_overflow = CD_STREAM_STATE.wrap_overflow;
            CD_STREAM_STATE.wrap_overflow = 0U;
            destination.bytes = destination.bytes + previous_wrap_overflow;
        }
        else
        {
            destination.bytes = CD_STREAM_BUFFER_START;
            CD_STREAM_STATE.bytes_buffered = unconsumed_bytes + transfer_size;
            linear_read_ptr = CD_STREAM_STATE.read_ptr;
            aligned_buffer_start = CD_STREAM_BUFFER_START + alignment_padding;
            CD_STREAM_STATE.write_ptr = aligned_buffer_start;
            CD_STREAM_STATE.read_ptr = aligned_buffer_start;
            linear_source.bytes = (linear_read_ptr + bytes_consumed) - alignment_padding;
            linear_word_count = (unconsumed_bytes + CD_STREAM_COPY_WORD_MASK) / CD_STREAM_COPY_WORD_SIZE;
            for (linear_word_count--; linear_word_count != -1; linear_word_count--)
            {
                *destination.words = *linear_source.words;
                linear_source.bytes += CD_STREAM_COPY_WORD_SIZE;
                destination.bytes += CD_STREAM_COPY_WORD_SIZE;
            }
        }
        CD_STREAM_STATE.data_ready = TRUE;
        return destination.bytes;
    }

    current_read_ptr = CD_STREAM_STATE.read_ptr;
    bytes_buffered = CD_STREAM_STATE.bytes_buffered;
    wrap_write_offset = CD_STREAM_STATE.wrap_overflow;
    destination.bytes = current_read_ptr + bytes_buffered;

    if ((wrap_write_offset != 0) || ((destination.bytes + transfer_size) > CD_STREAM_BUFFER_LIMIT))
    {
        destination.bytes = CD_STREAM_WRAP_START + wrap_write_offset;
        if (CD_STREAM_STATE.write_ptr >= (destination.bytes + transfer_size))
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
        unconsumed_bytes = bytes_buffered;
        CD_STREAM_STATE.bytes_buffered = unconsumed_bytes + transfer_size;
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
 * @see decomp.me: (100%) https://decomp.me/scratch/JFLMN
 */
void cdrom_decompress_buffer(u8* source, u8* destination)
{
    source++;
    while (cdrom_decompress_data(&source, &destination, CD_DECOMPRESS_UNBOUNDED_END, CD_DECOMPRESS_UNBOUNDED_END) != FALSE)
    {
    }
}

/**
 * @brief Clears the stream data-ready flag through volatile access.
 *
 * @param data_ready Flag to clear.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Y4pUH
 */
void cdrom_clear_data_ready(volatile u8* data_ready)
{
    *data_ready = FALSE;
}
