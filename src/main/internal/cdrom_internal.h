#ifndef CDROM_INTERNAL_H
#define CDROM_INTERNAL_H

#include "main/cdrom.h"
#include <libetc.h>

/*
 * Streamed resources are received into a four-sector ring. The first
 * CD_STREAM_DECOMPRESS_GUARD_SIZE bytes are kept free so an unread tail can be
 * moved in front of a wrapped sector.
 */
#define CD_STREAM_BUFFER_START ((u8*)CD_STREAM_BUFFER_ADDRESS)
#define CD_STREAM_BUFFER_SECTORS 4
#define CD_STREAM_BUFFER_LIMIT (CD_STREAM_BUFFER_START + (CD_STREAM_BUFFER_SECTORS * CD_DATA_SECTOR_SIZE))
#define CD_STREAM_DECOMPRESS_GUARD_SIZE 280
#define CD_STREAM_WRAP_START (CD_STREAM_BUFFER_START + CD_STREAM_DECOMPRESS_GUARD_SIZE)
#define CD_DECOMPRESS_UNBOUNDED_END ((u8*)0xFFFFFFFCU)
#define CD_BYTES_PER_WORD 4
#define CD_BYTES_PER_WORD_SHIFT 2
#define CD_BYTES_PER_WORD_MASK (CD_BYTES_PER_WORD - 1)

/** @brief Byte and word views of an aligned stream-copy cursor. */
typedef union
{
    u8* bytes;
    u32* words;
} CdStreamCopyCursor;

/**
 * @brief Scratchpad handoff between CD sector delivery and decompression.
 * @note data_ready and input_complete are written by the sector callback in interrupt context.
 */
typedef struct
{
    volatile u8 data_ready;
    volatile u8 input_complete;
    u8 _pad02[2];
    u8* buffer_start;
    u8* input_cursor;
    s32 bytes_buffered;
    s32 wrap_overflow;
    s32 bytes_consumed;
    s32 deferred_sectors;
} CdStreamState;

#define CD_STREAM_STATE (*(CdStreamState*)getScratchAddr(0))

s32 cdrom_decompress_data(u8** src_cursor, u8** dst_cursor, u8* src_end, u8* dst_end);
u8* cdrom_handle_stream_data(s32 byte_count, u32 bytes_remaining);
void cdrom_clear_data_ready(volatile u8* data_ready);

#endif
