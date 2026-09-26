#ifndef SHOP_H
#define SHOP_H

#include "common.h"
#include "main.h"

/** @brief Frame context the host passes to shop_update; its first word is the ordering-table entry. */
typedef struct
{
    u32 ot;
    u8 unknown_0x0004[0x40AE];
    s16 display_buffer_index;
} ShopFrameContext;

/** @brief One of the two double-buffered packet heaps at the start of the shop work buffer. */
typedef struct
{
    u8 packets[0x4000];
    u8* prim_cursor;
} ShopPacketBuffer;

/** @brief Entry id bit marking an entry that refers to an inventory record. */
#define SHOP_ENTRY_RECORD_FLAG 0x8000
#define SHOP_ENTRY_RECORD_INDEX_MASK 0x7FFF

/** @brief One row of the shop list. */
typedef struct ShopEntry
{
    u16 id;    /**< Item type, or SHOP_ENTRY_RECORD_FLAG plus an inventory-record index. */
    u16 count; /**< Remaining stock; zero means unlimited. */
    s32 price;
} ShopEntry;

void shop_init(u8* work, s32 is_buying, s32 entry_count, ShopEntry* entries, InventoryRecord* item_records, s32 title_text_id);
s32 shop_update(ShopFrameContext* ctx);

#endif
