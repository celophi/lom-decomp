#ifndef CDROM_INTERNAL_H
#define CDROM_INTERNAL_H

#include "cdrom.h"

#define CD_STREAM_WRAP_START ((u8*)0x801DC118)
#define CD_DECOMPRESS_UNBOUNDED_END ((u8*)0xFFFFFFFCU)
#define CD_STREAM_COPY_WORD_SIZE 4
#define CD_STREAM_COPY_WORD_MASK 3
#define CD_DATA_SECTOR_SIZE 0x800

/** @brief Byte and word views of an aligned stream-copy cursor. */
typedef union
{
    u8* bytes;
    u32* words;
} CdStreamCopyCursor;

/** @brief Scratchpad handoff between CD sector delivery and decompression. */
typedef struct
{
    volatile u8 data_ready;
    u8 input_complete;
    u8 pad[2];
    u8* read_ptr;
    u8* write_ptr;
    s32 bytes_buffered;
    s32 wrap_overflow;
    s32 bytes_consumed;
    s32 deferred_sectors;
} CdStreamState;

#define CD_STREAM_STATE (*(CdStreamState*)0x1F800000)

s32 cdrom_decompress_data(u8** src_cursor, u8** dst_cursor, u8* src_end, u8* dst_end);
u8* cdrom_handle_stream_data(s32 bytes_transferred, u32 bytes_remaining);
void cdrom_clear_data_ready(volatile u8* data_ready);

#endif
