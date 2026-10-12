#ifndef CARD_FILE_H
#define CARD_FILE_H

#include "common.h"

/**
 * @file card_file.h
 * @brief Header format of a PSX memory-card file: magic, icon type, block
 *        count, title, then the icon CLUT and up to three icon frames.
 */

/** @brief Bytes of a full card file header: the title sector and three icon-frame sectors. */
#define CARD_FILE_HEADER_BYTES 512

/** @brief Offset of the title, after the magic, icon type and block count. */
#define CARD_FILE_TITLE_OFFSET 4

/** @brief Length of one of the two title lines, and of both together. */
#define CARD_FILE_TITLE_LINE_BYTES 0x20
#define CARD_FILE_TITLE_BYTES (CARD_FILE_TITLE_LINE_BYTES * 2)

/** @brief Unused bytes between the title and the icon CLUT. */
#define CARD_FILE_RESERVED_BYTES 0x1C

/** @brief Bytes of the icon CLUT: 16 colors of 16 bits. */
#define CARD_FILE_ICON_CLUT_BYTES 0x20

/** @brief Most icon animation frames a file can have, and the bytes of one 16x16 4-bit frame. */
#define CARD_FILE_ICON_MAX_FRAMES 3
#define CARD_FILE_ICON_FRAME_BYTES 0x80

/** @brief Icon type byte of a 16-color icon with @p frames animation frames. */
#define CARD_FILE_ICON_TYPE(frames) (0x10 + (frames))

/** @brief A full card file header with room for all three icon frames. */
typedef struct
{
    char magic[2];
    u8 icon_type;
    u8 block_count;
    /** @brief Shift-JIS title in two lines. */
    u8 title[2][CARD_FILE_TITLE_LINE_BYTES];
    u8 reserved[CARD_FILE_RESERVED_BYTES];
    /** @brief 16-color icon CLUT (16-bit entries), stored as bytes. */
    u8 clut[CARD_FILE_ICON_CLUT_BYTES];
    u8 icon_frames[CARD_FILE_ICON_MAX_FRAMES][CARD_FILE_ICON_FRAME_BYTES];
} CardFileHeader;

#endif
