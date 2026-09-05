#include "common.h"

/*
 * Golem logic-block records kept in the shared menu-layout buffer.
 *
 * Each record is one packed word. Bits 0-1 hold the logic type the block is
 * assigned to, with 3 meaning unassigned; bits 2-7 the block id; bits 8-11
 * a detail value; bits 12-15 the shape index; bits 17-18 the rotation; and
 * bits 19-23 and 24-28 the grid position once placed. The golem overlay reads
 * these fields with the same layout.
 */

#define LOGIC_BLOCK_TYPE_UNASSIGNED 0x3
#define LOGIC_BLOCK_ID_SHIFT 2
#define LOGIC_BLOCK_ID_MASK 0xFC
#define LOGIC_BLOCK_DETAIL_SHIFT 8
#define LOGIC_BLOCK_DETAIL_MASK 0xF00
#define LOGIC_BLOCK_SHAPE_SHIFT 12
#define LOGIC_BLOCK_SHAPE_MASK 0xF000
/* Cleared on append and by func_800CBE64; its meaning is not yet established. */
#define LOGIC_BLOCK_FLAG_UNK16 0x10000

/** @brief Logic-block table inside the menu-layout buffer. */
typedef struct
{
    u8 pad_0000[0x29D6];
    u8 logic_block_count;
    u8 pad_29D7[5];
    u32 logic_blocks[40];
} GolemLogicBlockTable;

extern GolemLogicBlockTable g_menuLayoutBuffer;

/**
 * @brief Append a new, unassigned logic block to the golem logic-block table.
 * @param block_id Six-bit block id stored in bits 2-7.
 * @param detail Four-bit detail value stored in bits 8-11.
 * @param shape Four-bit shape index stored in bits 12-15.
 */
void golem_logic_block_append(u32 block_id, u32 detail, u32 shape)
{
    g_menuLayoutBuffer.logic_blocks[g_menuLayoutBuffer.logic_block_count] =
        (g_menuLayoutBuffer.logic_blocks[g_menuLayoutBuffer.logic_block_count] & ~LOGIC_BLOCK_ID_MASK) | ((block_id & 0x3F) << LOGIC_BLOCK_ID_SHIFT);
    g_menuLayoutBuffer.logic_blocks[g_menuLayoutBuffer.logic_block_count] =
        (g_menuLayoutBuffer.logic_blocks[g_menuLayoutBuffer.logic_block_count] & ~LOGIC_BLOCK_DETAIL_MASK) | ((detail & 0xF) << LOGIC_BLOCK_DETAIL_SHIFT);
    g_menuLayoutBuffer.logic_blocks[g_menuLayoutBuffer.logic_block_count] =
        (g_menuLayoutBuffer.logic_blocks[g_menuLayoutBuffer.logic_block_count] & ~LOGIC_BLOCK_SHAPE_MASK) | ((shape & 0xF) << LOGIC_BLOCK_SHAPE_SHIFT);
    g_menuLayoutBuffer.logic_blocks[g_menuLayoutBuffer.logic_block_count] |= LOGIC_BLOCK_TYPE_UNASSIGNED;
    g_menuLayoutBuffer.logic_blocks[g_menuLayoutBuffer.logic_block_count] &= ~LOGIC_BLOCK_FLAG_UNK16;
    g_menuLayoutBuffer.logic_block_count++;
}
