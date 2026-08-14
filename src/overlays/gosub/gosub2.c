typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;

/** @brief Number of entries in the gosub UI element pool. */
#define GOSUB_ELEMENT_COUNT 16

/** @brief Control entries embedded in a gosub screen sequence. */
#define GOSUB_SCREEN_SEQUENCE_END 0xFE
#define GOSUB_SCREEN_SEQUENCE_DIALOG 0xFF

/** @brief Lifecycle states used by a gosub UI element. */
typedef enum GosubElementState
{
    GOSUB_ELEMENT_STATE_INACTIVE = 0,
    GOSUB_ELEMENT_STATE_ENTERING = 1,
    GOSUB_ELEMENT_STATE_ACTIVE = 2,
    GOSUB_ELEMENT_STATE_EXITING = 3
} GosubElementState;

void gosub_build_equipment_list(u32 item_kind); /* extern */
s32 gosub_handle_input(s32 unused);             /* extern */
void gosub_scroll_to_cursor(void);              /* extern */
s32 gosub_toggle_cursor_selection(void);        /* extern */
s32 gosub_advance_screen_sequence(void);        /* extern */
s32 gosub_are_elements_idle(void);              /* extern */
void func_80145F80();                           /* extern */
void func_80143B64();                           /* extern */
void func_80143BB0();                           /* extern */
void func_80143C58();                           /* extern */
void func_80145CEC();                           /* extern */
void func_80146468();                           /* extern */
void func_80146538();                           /* extern */

typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 state : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void* draw_handler;
} GosubElement;

/** @brief Screen-space position pair passed by address to the glyph writer. */
typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    u16 unkE;
} StructS0;

typedef struct
{
    s32 unk0;
    u8 pad4[0x40AE];
    s16 unk40B2;
    u8 pad40B4[4];
    StructS0* unk40B8;
} Arg0Struct;

typedef StructS0* (*Unk6Func)();

/**
 * @brief Ordering-table link word at the head of every GPU packet. Mirrors the
 *        P_TAG layout in include/psyq/libgpu.h.
 */
typedef struct
{
    u32 addr : 24; /* 0x00 next-primitive address (24-bit) */
    u32 len : 8;   /* 0x03 packet word count */
    u_char r0;     /* 0x04 */
    u_char g0;     /* 0x05 */
    u_char b0;     /* 0x06 */
    u_char code;   /* 0x07 */
} GosubTag;

/**
 * @brief Unconnected flat line packet (0x10 bytes, code 0x40). Field names
 *        mirror the LINE_F2 layout in include/psyq/libgpu.h.
 */
typedef struct
{
    u_long tag;  /* 0x00 P_TAG */
    u_char r0;   /* 0x04 */
    u_char g0;   /* 0x05 */
    u_char b0;   /* 0x06 */
    u_char code; /* 0x07 */
    s16 x0;      /* 0x08 */
    s16 y0;      /* 0x0A */
    s16 x1;      /* 0x0C */
    s16 y1;      /* 0x0E */
} GosubLine;     /* 0x10 */

/** @brief libgpu setaddr(): write a packet's 24-bit ordering-table link. */
#define SET_PRIM_ADDR(p, a) (((GosubTag *)(p))->addr = (u_long)(a))
/** @brief libgpu getaddr(): read a packet's 24-bit ordering-table link. */
#define GET_PRIM_ADDR(p) ((u_long)((GosubTag *)(p))->addr)
/** @brief libgpu addPrim(): splice packet @p p in at ordering-table tag @p ot. */
#define ADD_PRIM(ot, p) (SET_PRIM_ADDR(p, GET_PRIM_ADDR(ot)), SET_PRIM_ADDR(ot, p))

s32 func_8001A5D4(s32, void*);                  /* extern */
s32 func_8001C56C(void*, s32, s32, s32, s32);  /* extern */
StructS0* func_801443E4();                      /* extern */
StructS0* func_80144544();                      /* extern */
GosubLine* func_80144764();                      /* extern */
StructS0* func_80146E30();                      /* extern */
StructS0* func_801448EC();                      /* extern */

/** @brief Packed four-byte record stored in the combination table. */
typedef struct
{
    u32 word;
} GosubPackedRecord;

/** @brief One 0x40-byte equipment record in the table at g_pad_ctx + 0xCE0. */
typedef struct
{
    u8 name[0x14];
    union
    {
        u32 word;
        struct
        {
            u16 low;
            u16 material;
        } half;
    } attributes;
    u8 unk18[0xC];
    union
    {
        u16 kind0_value;
        u16 kind1_stats[4];
        struct
        {
            u8 group;
            u8 index;
            u8 value;
        } kind2;
    } data;
    u8 unk2C[0x14];
} GosubEquipmentRecord;

/** @brief Save-data prefix through the 100-record equipment table. */
typedef struct
{
    u8 unk0000[0xCE0];
    GosubEquipmentRecord equipment[100];
} GosubSaveData;

/** @brief Per-row scratch text storage. */
typedef struct
{
    u8 text[0x50];
} GosubTextBuffer;

/** @brief Header of the text archive rooted at D_8014F27C. */
typedef struct
{
    u32 block_offsets[13];
} GosubTextArchive;

#define GOSUB_EQUIPMENT_RECORD(ptr) (&((GosubSaveData*)(ptr))->equipment[0])
#define GOSUB_EQUIPMENT_BASE_FROM_INDEX(index) (g_pad_ctx + (index) * 0x40)
#define GOSUB_EQUIPMENT_FROM_INDEX(index) GOSUB_EQUIPMENT_RECORD((index) * 0x40 + (s32)g_pad_ctx)
#define GOSUB_EQUIPMENT_SOURCE_FROM_INDEX(index) ((u8*)((index) * 0x40 + (s32)g_pad_ctx))
#define GOSUB_EQUIPMENT_AT(index) ((GosubEquipmentRecord*)(g_pad_ctx + ((index) * 0x40 + 0xCE0)))
#define GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(index) ((GosubEquipmentRecord*)(g_pad_ctx + ((index) << 6) + 0xCE0))
#define GOSUB_TEXT_BUFFER(index) (((GosubTextBuffer*)g_gosub_text_buffers)[index].text)
#define GOSUB_TEXT_ARCHIVE ((GosubTextArchive*)&D_8014F27C)
#define GOSUB_EQUIPMENT_KIND(attributes) (((attributes) >> 8) & 3)
#define GOSUB_EQUIPMENT_CATEGORY(attributes) (((attributes) >> 10) & 0x3F)
#define GOSUB_EQUIPMENT_CATEGORY_OFFSET(attributes) (((attributes) >> 9) & 0x7E)
#define GOSUB_KIND2_ARCHIVE_ENTRY(attributes)                                                                                                                  \
    ((u8*)&D_8014F27C + D_8014F288[0] + *(u16*)((u8*)D_8014F288 + D_8014F288[0] + GOSUB_EQUIPMENT_CATEGORY_OFFSET(attributes) + 0x22))

/** @brief UI element pool; element 0 is reserved for the fixed dialog element. */
extern GosubElement g_gosub_elements[GOSUB_ELEMENT_COUNT];
extern u8* g_pad_ctx;
extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];
extern s32 D_8014F29C;
extern s32 D_8014F2A4;
extern s32 D_8014F2C4;
extern s32 g_gosub_cursor_row;
extern s32 g_gosub_finished;
extern s32 g_gosub_row_count;
extern s32 g_gosub_visible_row_count;
extern u8 g_gosub_screen_sequence_index;
extern s32 g_gosub_scroll_frames_remaining;
extern s32 D_8016B8E4;
extern s32 g_gosub_dialog_choice;
extern s32 D_8016B8EC;
extern s32 g_gosub_allow_duplicate_selection;
extern u8 D_8016B8FC;
extern u8 g_gosub_required_selection_count;
extern s32 g_gosub_window_height;
extern s32 g_gosub_window_width;
extern u8 g_gosub_selection_count;
extern s32 D_8017097C;
extern s32 g_gosub_row_height;
extern s32 g_gosub_scroll_y;
extern s32 g_gosub_scroll_target_y;
extern s32 D_801709A4;

/**
 * @brief One row of the item list built by the gosub screen builders.
 *
 * Mirrors the layout documented on GosubListEntry in gosub.c; see that file
 * for why the 0xC word and the 0x1C flag set are spelled the way they are.
 */
typedef struct
{
    u8* name;
    u8* desc;
    s16 value;
    s16 index;
    s32 unkC : 8;
    s32 unkD : 8;
    s32 unkE : 8;
    u32 kind : 4;
    u32 unkC_28 : 4;
    u16 unk10;
    u16 unk12[4];
    u16 unk1A;
    union
    {
        struct
        {
            u32 flag0 : 1;
            u32 flag1 : 1;
            u32 flag2 : 1;
            u32 unk1C_3 : 29;
        } f;
        u16 half;
        u32 word;
    } flags;
} GosubListEntry;

extern s32 D_801228F0;
extern GosubListEntry g_gosub_rows[];
extern u8 g_gosub_selected_rows[];

extern s32 g_pad_input;
extern s32 D_8016B948;
extern s32 D_8016B95C;
extern u8 g_gosub_screen_sequence[20];
extern s32 g_gosub_result_rows[16];
extern s32 (*g_gosub_select_handler)();
extern s32 (*g_gosub_finish_handler)();
extern s32 (*g_gosub_dialog_handler)(s32);

extern u8 D_800EC3E2[];
extern u32 D_8014F27C;
extern u32 D_8014F280;
extern u32 D_8014F288[];
extern s32 D_8016B900;
extern u8 g_gosub_text_buffers[];
extern u8* g_gosub_title_text;

/**
 * @brief A three-element range table indexed by gosub screen group.
 *
 * Held as a struct rather than a bare array because the game copies the whole
 * table from rodata into a local before indexing it.
 */
typedef struct
{
    s32 values[3];
} GosubGroupTable;

extern GosubGroupTable g_gosub_group_first_indices;
extern GosubGroupTable g_gosub_group_counts;

#define GOSUB_MSG_PTR(off) ((u8*)&D_8014F29C - 0x20 + D_8014F29C + *(u16*)((u8*)&D_8014F29C + D_8014F29C + (off)))

/**
 * @brief Resolve a message pointer against a caller-held archive base.
 *
 * Same lookup as GOSUB_MSG_PTR, but the caller keeps the (&D_8014F29C - 0x20)
 * base in a local. ABS reads the offset table through &D_8014F29C, REL reads it
 * through the base itself; the target uses both spellings and they are not
 * interchangeable (they differ by 0x20 in @p off and in which register gcc uses).
 *
 * @param base Archive base, i.e. (u8*)&D_8014F29C - 0x20.
 * @param off  Byte offset of the u16 entry within the message block.
 * @return Pointer to the message text.
 */
#define GOSUB_MSG_ABS(base, off) ((base) + D_8014F29C + *(u16*)((u8*)&D_8014F29C + D_8014F29C + (off)))
#define GOSUB_MSG_REL(base, off) ((base) + D_8014F29C + *(u16*)((base) + D_8014F29C + (off)))

s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode); /* extern */
s32 func_800A8A78(s32* ot, s32 prim, s32 ch, s32 a3, Vec2s* pos, s32 mode);        /* extern */
s32 func_801450D8(s32 prim, s32* ot, s32 row, s32 x, s32 y, s32 count);            /* extern */
s32 func_801466B4(s32 prim, s32* ot, s32 x, s32 y, s32 w, s32 h);                  /* extern */

/**
 * @brief Resolve one entry of a text archive block.
 *
 * The same lookup as gosub.c's ARCHIVE_ENTRY: a block begins with a u16 per
 * entry giving that entry's offset from the archive base, so a lookup is
 * base + block + block[idx].
 *
 * @param blk Block offset word, e.g. D_8014F288[0].
 * @param idx Entry index within the block.
 * @return Pointer to the entry.
 */
#define ARCHIVE_ENTRY(blk, idx) ((u8*)&D_8014F27C + (blk) + *(u16*)((u8*)&D_8014F27C + (blk) + (idx) * 2))

#define GOSUB_MSG(off) func_80145CEC(GOSUB_MSG_PTR(off))

/**
 * @brief Set the top byte of an element's attr word.
 *
 * Must go through the whole word rather than an 8-bit bitfield; see the same
 * macro in gosub.c.
 *
 * @param e Element to update.
 * @param c New top-byte value.
 */
#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

/**
 * @brief Handle the confirmation dialog for creating a two-item combination.
 *
 * @param dialog_result Zero to confirm; nonzero to return to the selection.
 * @return 1 if confirming leaves no equipment rows, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/2OzmD
 */
s32 gosub_handle_combination_dialog(s32 dialog_result)
{
    s32 combination_count;
    GosubPackedRecord* record;
    u32 packed;
    s32 config;
    s32 masked;
    s32 secondary_value;
    s32 clear_config_mask;
    u16 stored_word;

    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        clear_config_mask = ~0xFC;
        combination_count = *(g_pad_ctx + 0x29D6);
        if (combination_count < 0x28)
        {
            record = (GosubPackedRecord*)(g_pad_ctx + combination_count * 4 + 0x29DC);
            config = D_8017097C;
            packed = record->word & clear_config_mask;
            packed = packed | ((config & 0x3F) << 2);
            record->word = packed;
            secondary_value = D_8016B8EC;
            dialog_result = (packed & ~0xF00) | ((secondary_value & 0xF) << 8);
            record->word = dialog_result;
            masked = ((dialog_result & 0xFFFF0FFF) | ((D_8016B8E4 & 0xF) << 12) | 3) & 0xFFFF;
            stored_word = masked;
            record->word = stored_word;
            *(g_pad_ctx + 0x29D6) = *(g_pad_ctx + 0x29D6) + 1;
            GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(g_gosub_result_values[0])->name[0] = 0;
            GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(g_gosub_result_values[1])->name[0] = 0;
            func_800A8FB4();
        }
        if (*(g_pad_ctx + 0x29D6) >= 0x28)
        {
            func_80143B64();
            D_801227F0 = 0;
            GOSUB_MSG(-4);
            return 0;
        }
        g_gosub_scroll_frames_remaining = 0;
        g_gosub_scroll_target_y = 0;
        g_gosub_scroll_y = 0;
        g_gosub_cursor_row = 0;
        g_gosub_allow_duplicate_selection = 0;
        gosub_build_equipment_list(3);
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = 0x10;
        g_gosub_window_width = 0xE8;
        g_gosub_window_height = 0x64;
        D_8016B8E4 = 0;
        D_8017097C = 0;
        D_8016B8EC = 0;
        g_gosub_required_selection_count = 2;
        D_8016B8FC = 2;
        g_gosub_selection_count = 0;
        g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        g_gosub_screen_sequence_index -= 1;
        if (g_gosub_row_count == 0)
        {
            D_801227F0 = 0;
            func_80067F28();
            func_80143B64();
            return 1;
        }
        return 0;
    }
    g_gosub_selection_count -= 1;
    g_gosub_screen_sequence_index -= 1;
    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    return 0;
}

/**
 * @brief Publish the selected group rows as result values.
 *
 * @return 1 when at least one row was published, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/pOY6i
 */
s32 gosub_publish_group_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    g_gosub_result_count = g_gosub_selection_count;

    for (i = 0; i < g_gosub_selection_count; i++)
    {
        g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
    }

    return 1;
}

/**
 * @brief Publish the selected rows' entry indices as result values.
 *
 * @return 1 when at least one row was published, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/FN7DQ
 */
s32 gosub_publish_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    g_gosub_result_count = g_gosub_selection_count;

    for (i = 0; i < g_gosub_selection_count; i++)
    {
        g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
    }

    return 1;
}

/**
 * @brief Test whether a row is absent from the current selection.
 *
 * @param row Row index to test.
 * @return 1 if the row is unselected, otherwise 0.
 * @see decomp.me (100%) https://decomp.me/scratch/lBIH9
 */
s32 gosub_is_row_unselected(s32 row)
{
    s32 i;
    s32 count = g_gosub_selection_count;

    for (i = 0; i < count; i++)
    {
        if (g_gosub_selected_rows[i] == row)
        {
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Build list rows from nonempty equipment records of the requested kind.
 *
 * Kind 3 accepts every record; kind 4 accepts every record except kind 2.
 *
 * @param item_kind Equipment kind filter, or 3/4 for the aggregate filters.
 * @see decomp.me (100%) https://decomp.me/scratch/CJYqj
 */
void gosub_build_equipment_list(u32 item_kind)
{
    s32 item_index;
    s32 stat_index;
    s32 row_count;
    u8* item_base;
    u8* entry;
    u8* item;
    s32 separator_offset;
    GosubEquipmentRecord* record;
    u32 attributes;

    D_8016B900 = 1;
    row_count = 0;

    for (item_index = 0; item_index < 100; item_index++)
    {
        entry = GOSUB_EQUIPMENT_BASE_FROM_INDEX(item_index);
        if (GOSUB_EQUIPMENT_RECORD(entry)->name[0] != 0)
        {
            if ((item_kind == 3) || (GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(entry)->attributes.word) == item_kind) ||
                ((item_kind == 4) && (GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(entry)->attributes.word) != 2)))
            {

                g_gosub_rows[row_count].name = GOSUB_EQUIPMENT_AT(item_index)->name;

                func_80146538(GOSUB_TEXT_BUFFER(row_count),
                              ARCHIVE_ENTRY(D_8014F280, GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(item_index)->attributes.half.material & 0x3F));
                /* A typed table base folds the required D_800EC3E2 - 0x1E relocation. */
                separator_offset = (s32)(D_800EC3E2 - 0x1E) + (D_800EC3E2[1] << 8);
                func_80146468(GOSUB_TEXT_BUFFER(row_count), D_800EC3E2[0] + separator_offset);

                item_base = GOSUB_EQUIPMENT_BASE_FROM_INDEX(item_index);
                g_gosub_rows[row_count].unkC_28 = GOSUB_EQUIPMENT_KIND(GOSUB_EQUIPMENT_RECORD(item_base)->attributes.word);
                attributes = GOSUB_EQUIPMENT_RECORD(item_base)->attributes.word;

                switch (GOSUB_EQUIPMENT_KIND(attributes))
                {
                case 0:
                    func_80146468(GOSUB_TEXT_BUFFER(row_count), ARCHIVE_ENTRY(GOSUB_TEXT_ARCHIVE->block_offsets[3], GOSUB_EQUIPMENT_CATEGORY(attributes)));
                    g_gosub_rows[row_count].unk10 = GOSUB_EQUIPMENT_AT(item_index)->data.kind0_value;
                    break;
                case 1:
                    func_80146468(GOSUB_TEXT_BUFFER(row_count),
                                  ARCHIVE_ENTRY(GOSUB_TEXT_ARCHIVE->block_offsets[3], GOSUB_EQUIPMENT_CATEGORY(attributes) + 0xB));
                    record = GOSUB_EQUIPMENT_FROM_INDEX(item_index);
                    for (stat_index = 0; stat_index < 4; stat_index++)
                    {
                        g_gosub_rows[row_count].unk12[stat_index] = record->data.kind1_stats[stat_index];
                    }
                    break;
                default:
                    item = GOSUB_EQUIPMENT_SOURCE_FROM_INDEX(item_index);
                    record = GOSUB_EQUIPMENT_RECORD(item);
                    g_gosub_rows[row_count].unk10 = record->data.kind2.value;
                    g_gosub_rows[row_count].unk12[0] = record->data.kind2.index + (record->data.kind2.group * 14);
                    func_80146468(GOSUB_TEXT_BUFFER(row_count), GOSUB_KIND2_ARCHIVE_ENTRY(GOSUB_EQUIPMENT_RECORD(item)->attributes.word));
                    break;
                }

                g_gosub_rows[row_count].desc = GOSUB_TEXT_BUFFER(row_count);
                g_gosub_rows[row_count].value = -1;
                g_gosub_rows[row_count].index = item_index;
                g_gosub_rows[row_count].kind = 4;
                row_count++;
            }
        }
    }

    g_gosub_row_count = row_count;
    g_gosub_visible_row_count = 8;

    switch (item_kind)
    {
    case 0:
        g_gosub_title_text = GOSUB_MSG_PTR(0xC);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MSG_PTR(0xE);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MSG_PTR(0x10);
        break;
    case 3:
        g_gosub_title_text = GOSUB_MSG_PTR(0x16);
        break;
    case 4:
        g_gosub_visible_row_count = 7;
        g_gosub_title_text = GOSUB_MSG_PTR(0x16);
        break;
    }

    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = (g_gosub_visible_row_count * 0x10) + 4;
}
/**
 * @brief Build one of the three grouped option lists from the text archive.
 *
 * @param group Option group index, from 0 through 2.
 * @see decomp.me (100%)
 */
void gosub_build_grouped_option_list(s32 group)
{
    s32 option_index;
    GosubGroupTable first_indices = g_gosub_group_first_indices;
    GosubGroupTable counts = g_gosub_group_counts;

    for (option_index = 0; option_index < counts.values[group]; option_index++)
    {
        GosubListEntry* row = &g_gosub_rows[option_index];
        u8* text;

        row->name = ARCHIVE_ENTRY(D_8014F288[0], option_index + first_indices.values[group]);
        text = ARCHIVE_ENTRY(D_8014F288[0], option_index + first_indices.values[group]);
        row->name = text;
        row->value = -1;
        row->index = option_index;
        row->unkC_28 = 0;
        row->desc = text;
        row->kind = 4;
    }

    g_gosub_row_count = counts.values[group];

    switch (group)
    {
    case 0:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1A);
        break;
    case 1:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1C);
        break;
    case 2:
        g_gosub_title_text = GOSUB_MSG_PTR(0x1E);
        break;
    }

    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
}

/**
 * @brief Process input, advance scroll interpolation, and draw the active screen.
 *
 * @param render_ctx Rendering context forwarded to the input and draw handlers.
 * @see decomp.me (100%)
 */
void gosub_update_screen(s32 render_ctx)
{
    gosub_handle_input(render_ctx);

    if (g_gosub_finished == 0)
    {
        if (g_gosub_scroll_frames_remaining != 0)
        {
            g_gosub_scroll_y += (g_gosub_scroll_target_y - g_gosub_scroll_y) / g_gosub_scroll_frames_remaining;
            g_gosub_scroll_frames_remaining -= 1;
        }
        else
        {
            g_gosub_scroll_y = g_gosub_scroll_target_y;
        }

        func_80143BB0(render_ctx);
    }
}

/**
 * @brief Handle dialog, navigation, selection, completion, and cancellation input.
 *
 * @param unused Unused rendering context.
 * @return Undefined; callers ignore the value.
 * @see decomp.me (100%)
 */
s32 gosub_handle_input(s32 unused)
{
    GosubElement* elements;
    s32 steps_remaining;
    s32 restored_row;
    s32 max_scroll_y;

    elements = g_gosub_elements;
    if ((elements[1].attr.word & 7) == 0 && (elements[0].attr.word & 7) == 0)
    {
        g_gosub_finished = 1;
        return;
    }

    if (gosub_are_elements_idle() == 0)
    {
        return;
    }

    if ((g_gosub_elements[0].attr.word & 7) == 2)
    {
        if (D_8016B948 != 0)
        {
            if ((g_pad_input & 0x260) == 0)
            {
                return;
            }
            if (D_8016B95C == 0)
            {
                func_800A3938(0x7D, 0x80);
                func_80067F28();
                func_80143B64();
                return;
            }
            g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
            D_8016B948 = 0;
            return;
        }

        if (g_pad_input & 0x9000)
        {
            func_800A3938(0x7D, 0x80);
            g_gosub_dialog_choice -= 1;
            if (g_gosub_dialog_choice < 0)
            {
                g_gosub_dialog_choice = 0xB;
            }
            return;
        }

        if (g_pad_input & 0x6000)
        {
            func_800A3938(0x7D, 0x80);
            g_gosub_dialog_choice += 1;
            if (g_gosub_dialog_choice == 0xC)
            {
                g_gosub_dialog_choice = 0;
            }
            return;
        }

        if (g_pad_input & 0x220)
        {
            func_800A3938(0x7D, 0x80);
            if (g_gosub_dialog_handler == 0)
            {
                return;
            }
            if (g_gosub_dialog_handler(0) == 0)
            {
                return;
            }
            func_80067F28();
            func_80143B64();
            return;
        }

        if (g_pad_input & 0x40)
        {
            func_800A3938(0x7D, 0x80);
            if (g_gosub_dialog_handler == 0)
            {
                return;
            }
            if (g_gosub_dialog_handler(1) == 0)
            {
                return;
            }
            func_80067F28();
            func_80143B64();
            return;
        }

        return;
    }

    if (g_gosub_scroll_frames_remaining != 0)
    {
        return;
    }

    steps_remaining = 1;
    if (g_pad_input & 8)
    {
        steps_remaining = g_gosub_visible_row_count;
        g_pad_input = 0x4000;
    }
    if (g_pad_input & 4)
    {
        steps_remaining = g_gosub_visible_row_count;
        g_pad_input = 0x1000;
    }

    if (steps_remaining != 0)
    {
        do
        {
            if (g_pad_input & 0x1000)
            {
                g_gosub_cursor_row -= 1;
                if (g_gosub_cursor_row == 0)
                {
                    steps_remaining = 1;
                }
                if (g_gosub_cursor_row < 0)
                {
                    g_gosub_cursor_row = g_gosub_row_count - 1;
                    steps_remaining = 1;
                }
            }
            if (g_pad_input & 0x4000)
            {
                g_gosub_cursor_row += 1;
                if (g_gosub_cursor_row == g_gosub_row_count - 1)
                {
                    steps_remaining = 1;
                }
                if (g_gosub_cursor_row >= g_gosub_row_count)
                {
                    g_gosub_cursor_row = 0;
                    steps_remaining = 1;
                }
            }
            steps_remaining -= 1;
        } while (steps_remaining != 0);
    }

    if (g_pad_input & 0x5000)
    {
        func_800A3938(0x7D, 0x80);
        gosub_scroll_to_cursor();
        return;
    }

    if (g_pad_input & 0x220)
    {
        func_800A3938(0x7D, 0x80);
        if ((g_gosub_rows[g_gosub_cursor_row].kind & 0xF) != 4)
        {
            return;
        }
        if (gosub_toggle_cursor_selection() != 0)
        {
            g_gosub_selected_rows[g_gosub_selection_count] = g_gosub_cursor_row;
            g_gosub_selection_count += 1;
            if (g_gosub_select_handler != 0)
            {
                if (g_gosub_select_handler() == 0)
                {
                    return;
                }
                if (gosub_advance_screen_sequence() == 0)
                {
                    return;
                }
                func_80067F28();
                func_80143B64();
                return;
            }
            if (g_gosub_selection_count != g_gosub_required_selection_count)
            {
                return;
            }
            if (g_gosub_finish_handler == 0)
            {
                return;
            }
            if (g_gosub_finish_handler() == 0)
            {
                return;
            }
            if (gosub_advance_screen_sequence() == 0)
            {
                return;
            }
            func_80067F28();
            func_80143B64();
            return;
        }
        if (g_gosub_select_handler == 0)
        {
            return;
        }
        if (g_gosub_select_handler() == 0)
        {
            return;
        }
        if (gosub_advance_screen_sequence() == 0)
        {
            return;
        }
        func_80067F28();
        func_80143B64();
        return;
    }

    if (g_pad_input & 0x800)
    {
        func_800A3938(0x7D, 0x80);
        if (g_gosub_finish_handler != 0)
        {
            if (g_gosub_finish_handler() == 0)
            {
                return;
            }
            if (gosub_advance_screen_sequence() == 0)
            {
                return;
            }
            func_80067F28();
            func_80143B64();
            return;
        }
        if (gosub_advance_screen_sequence() == 0)
        {
            return;
        }
        func_80067F28();
        func_80143B64();
        return;
    }

    if ((g_pad_input & 0x40) == 0)
    {
        return;
    }

    func_800A3938(0x7F, 0x80);

    if (g_gosub_selection_count != 0)
    {
        g_gosub_selection_count -= 1;
        g_gosub_cursor_row = g_gosub_selected_rows[g_gosub_selection_count];
        gosub_scroll_to_cursor();
        if (g_gosub_select_handler != 0)
        {
            g_gosub_select_handler();
        }
        return;
    }

    if (g_gosub_screen_sequence_index != 0)
    {
        g_gosub_screen_sequence_index -= 1;
        gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index]);
        g_gosub_result_count -= 1;
        restored_row = g_gosub_result_rows[g_gosub_result_count];
        g_gosub_cursor_row = restored_row;
        g_gosub_scroll_y = g_gosub_row_height * restored_row;
        max_scroll_y = (g_gosub_row_count * g_gosub_row_height) - g_gosub_window_height + 4;
        if (max_scroll_y < g_gosub_scroll_y)
        {
            g_gosub_scroll_y = max_scroll_y;
        }
        if (g_gosub_scroll_y < 0)
        {
            g_gosub_scroll_y = 0;
        }
        g_gosub_scroll_frames_remaining = 0;
        g_gosub_scroll_target_y = g_gosub_scroll_y;
        return;
    }

    g_gosub_result_count = 0;
    func_80067F28();
    func_80143B64();
}

/**
 * @brief Scroll the list viewport toward the cursor when it leaves view.
 *
 * @see decomp.me (100%)
 */
void gosub_scroll_to_cursor(void)
{
    s32 cursor_offset;
    s32 cursor_y;

    cursor_y = g_gosub_cursor_row * g_gosub_row_height;
    cursor_offset = cursor_y - g_gosub_scroll_y;

    if ((g_gosub_window_height - g_gosub_row_height) < cursor_offset)
    {
        g_gosub_scroll_frames_remaining = 4;
        g_gosub_scroll_target_y = (g_gosub_cursor_row - (g_gosub_visible_row_count - 1)) * g_gosub_row_height;
    }

    if (cursor_offset < 0)
    {
        g_gosub_scroll_target_y = cursor_y;
        g_gosub_scroll_frames_remaining = 4;
    }
}

/**
 * @brief Remove the cursor row if selected, or permit the caller to add it.
 *
 * @return 0 if the row was removed, otherwise 1.
 * @see decomp.me (100%)
 */
s32 gosub_toggle_cursor_selection(void)
{
    s32 selection_index;
    s32 shift_index;

    if (g_gosub_allow_duplicate_selection != 0)
    {
        return 1;
    }

    for (selection_index = 0; selection_index < g_gosub_selection_count; selection_index++)
    {
        if (g_gosub_selected_rows[selection_index] == g_gosub_cursor_row)
        {
            for (shift_index = selection_index; shift_index < 3; shift_index++)
            {
                g_gosub_selected_rows[shift_index] = g_gosub_selected_rows[shift_index + 1];
            }
            g_gosub_selection_count -= 1;
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Advance to the next screen or open the sequence's final dialog.
 *
 * @return 1 at the sequence terminator, otherwise 0.
 * @see decomp.me (100%)
 */
s32 gosub_advance_screen_sequence(void)
{
    GosubElement* element;

    g_gosub_screen_sequence_index += 1;

    if (g_gosub_screen_sequence[g_gosub_screen_sequence_index] == GOSUB_SCREEN_SEQUENCE_END)
    {
        return 1;
    }

    if (g_gosub_screen_sequence[g_gosub_screen_sequence_index] == GOSUB_SCREEN_SEQUENCE_DIALOG)
    {
        element = &g_gosub_elements[0];
        element->draw_handler = (void*)&func_80145F80;
        g_gosub_dialog_choice = 0;
        element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
        element->attr.f.unk0_3 = 1;
        element->attr.f.x = 0x20;
        element->attr.f.unk0_16 = 0x70;
        element->unk4_0 = 1;
        element->y = 0x24;
        SET_ELEM_CODE(element, 0);
    }
    else
    {
        gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index]);
    }

    return 0;
}

/**
 * @brief Test whether all fixed elements have finished transitioning.
 *
 * @return 1 when all elements are idle, otherwise 0.
 * @see decomp.me (100%)
 */
s32 gosub_are_elements_idle(void)
{
    GosubElement* element;
    s32 i;

    element = g_gosub_elements;

    for (i = 0; i < GOSUB_ELEMENT_COUNT; i++)
    {
        if (element->attr.f.state == GOSUB_ELEMENT_STATE_ENTERING || element->attr.f.state == GOSUB_ELEMENT_STATE_EXITING)
        {
            return 0;
        }
        element++;
    }

    return 1;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/RsBVl
 */
void func_80143B64(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32* var_a0;
    s32 temp;

    var_a0 = &g_gosub_elements;
    var_a1 = 0;
    do
    {
        temp_v1 = *var_a0;
        if (temp_v1 & 7)
        {
            temp = temp_v1 & ~7;
            *var_a0 = (temp & ~0x78) | 0x40;
        }
        var_a1 += 1;
        var_a0 += 3;
    } while (var_a1 < 0x10);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/nVefu
 */
void func_80143BB0(void)
{
    func_80143C58();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/dib6Q
 */
void func_80143BD0(void)
{
    s32 var_a0;
    s32* var_v1;

    var_v1 = &g_gosub_elements;
    var_a0 = 0;
    do
    {
        var_a0 += 1;
        *var_v1 &= ~7;
        var_v1 += 3;
    } while (var_a0 < 0x10);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/X1pXK
 */
s32* func_80143C04(void)
{
    s32 temp_v1;
    s32 var_a1;
    s32* var_a0;

    var_a0 = &D_801709A4;

    for (var_a1 = 1; var_a1 < 0x10; var_a1++, var_a0 += 3)
    {
        temp_v1 = *var_a0;
        if (!(temp_v1 & 7))
        {
            *var_a0 = (temp_v1 & ~7) | 1;
            return var_a0;
        }
    }

    return &g_gosub_elements;
}

/**
 * @see decomp.me (97.70%)
 * @note Remaining differences are isolated CSE and expression-order rows; see working/func_80143C58/status.md.
 */
void func_80143C58(Arg0Struct* arg0)
{
    StructS0* var_s0;
    s32 temp_s1;
    s32 temp_s2;
    Arg0Struct* var_s4;
    u32* var_s5;
    u32 var_s6;
    u32 var_s7;
    s32 sp80;
    s32 sp20[24];
    u32 temp_t0;
    s32 temp_t1;
    u8 temp_a0;
    u32 temp_a1;
    u32 temp_a2;
    u32 temp_a3;
    s32 temp_v0;
    s32 temp_v1;
    s32 temp_mult;
    StructS0* var_a0;
    StructS0* var_s0_2;
    u32 temp_t0_2;
    u32 temp_t0_3;
    u32 temp_a0_2;
    s32 temp_v1_2;
    s32 temp_a0_3;
    s32 var_v1;
    s32 temp_a3_2;
    s32 var_v0;
    s32 temp_a3_3;
    u32 temp_v0_3;
    u32 temp_a0_4;
    s32 temp_a0_5;
    s32 var_v1_2;
    s32 temp_a3_5;
    s32 var_v0_2;
    s32 temp_a3_6;
    u32 temp_v0_5;
    u32 temp_v1_3;

    var_s0 = arg0->unk40B8;
    var_s4 = arg0;

    if (arg0->unk40B2 != 0)
    {
        func_8001C56C(sp20, 0, 0xF0, 0x140, 0xE0);
    }
    else
    {
        func_8001C56C(sp20, 0, 8, 0x140, 0xE0);
    }

    var_s5 = &g_gosub_elements;
    sp80 = 0;
    var_s6 = 0x00FFFFFF;
    var_s7 = 0xFF000000;

    for (; sp80 < 0x10; sp80++)
    {
        temp_a3 = *var_s5;
        if (temp_a3 & 7)
        {
            var_a0 = var_s0;

            if (*(Unk6Func*)((u8*)var_s5 + 8) == (Unk6Func)func_801448EC)
            {
                temp_t0 = *(u32*)((u8*)var_s5 + 4);
                temp_t1 = (temp_t0 >> 1) & 0xFF;

                temp_mult = g_gosub_row_count * g_gosub_row_height;
                if ((g_gosub_scroll_y + temp_t1) < temp_mult)
                {
                    var_s0 = func_801443E4(var_a0, var_s4, (((temp_a3 >> 7) & 0x1FF) + ((temp_a3 >> 24) | ((temp_t0 & 1) << 8))) - 0x10,
                                              (*((u8*)var_s5 + 2)) + temp_t1, 0);
                }
                if (g_gosub_scroll_y != 0)
                {
                    temp_t0_2 = *var_s5;
                    var_s0 = func_801443E4(var_s0, var_s4,
                                              (((temp_t0_2 >> 7) & 0x1FF) + (((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | (temp_t0_2 >> 24))) - 0x10,
                                              (*((u8*)var_s5 + 2)), 1);
                }
                func_8001A5D4((s32)var_s0, sp20);

                var_s0->unk0 = (var_s0->unk0 & var_s7) | (var_s4->unk0 & var_s6);
                var_s4->unk0 = (s32)((var_s4->unk0 & var_s7) | ((s32)var_s0 & var_s6));

                var_s0 = (StructS0*)((u8*)var_s0 + 0x40);

                if (g_gosub_row_count != 0)
                {
                    var_s0->unk4 = 0xFFFF00;
                    ((u8*)var_s0)[3] = 3;
                    ((u8*)var_s0)[7] = 0x60;
                    var_s0->unkC = 6;
                    temp_v0 = (*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF;
                    var_s0->unkE = (u16)((s32)(temp_v0 * (temp_v0 / g_gosub_row_height)) / g_gosub_row_count);
                    temp_v1_2 = *(u32*)((u8*)var_s5 + 4);
                    temp_a0 = (temp_v1_2 >> 1) & 0xFF;
                    if ((s16)var_s0->unkE >= (s16)(temp_a0 - 2))
                    {
                        var_s0->unkE = temp_a0;
                    }
                    var_s0->unk8 = 1;
                    var_s0->unkA =
                        (s16)((s32)(((*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF) * (g_gosub_scroll_y / g_gosub_row_height)) / g_gosub_row_count);
                    var_s0->unk0 = (var_s0->unk0 & var_s7) | (var_s4->unk0 & var_s6);

                    temp_v1 = (s32)var_s0 & var_s6;
                    var_s0 = (StructS0*)((u8*)var_s0 + 0x10);
                    var_s4->unk0 = (s32)((var_s4->unk0 & var_s7) | temp_v1);
                }
                temp_t0_3 = *var_s5;
                var_s0 = func_80144544(var_s0, var_s4,
                                          ((temp_t0_3 >> 7) & 0x1FF) + (((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | (temp_t0_3 >> 24)) + 3,
                                          (*((u8*)var_s5 + 2)), 0xA, (*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF, arg0->unk40B2);
                var_a0 = var_s0;
            }
            func_8001A5D4((s32)var_a0, sp20);
            var_s0->unk0 = (var_s0->unk0 & var_s7) | (var_s4->unk0 & var_s6);
            var_s4->unk0 = (s32)((var_s4->unk0 & var_s7) | ((s32)var_s0 & var_s6));

            temp_a0_2 = *var_s5;
            temp_v1_2 = temp_a0_2 & 7;

            var_s0 = (StructS0*)((u8*)var_s0 + 0x40);

            switch (temp_v1_2)
            {
            case 1:
                temp_a1 = *(u32*)((u8*)var_s5 + 4);
                temp_a2 = ((temp_a1 & 1) << 8) | (temp_a0_2 >> 24);
                temp_a0_3 = (temp_a0_2 >> 3) & 0xF;
                var_v1 = temp_a2 * temp_a0_3;
                if (var_v1 < 0)
                {
                    var_v1 += 7;
                }
                temp_a3_2 = (temp_a1 >> 1) & 0xFF;
                var_v0 = temp_a3_2 * temp_a0_3;
                temp_s1 = var_v1 >> 3;
                if (var_v0 < 0)
                {
                    var_v0 += 7;
                }
                temp_s2 = var_v0 >> 3;
                temp_a3_3 = (s32)(temp_a3_2 - temp_s2);

                var_s0 = func_80144544((*(Unk6Func*)((u8*)var_s5 + 8))(var_s4, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2), var_s4,
                                          ((*var_s5 >> 7) & 0x1FF) +
                                              (s32)((((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | (*var_s5 >> 24)) - temp_s1) / 2,
                                          (*((u8*)var_s5 + 2)) + ((s32)((*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF) - temp_s2) / 2, temp_s1, temp_s2,
                                          arg0->unk40B2);
                temp_v0_3 = *var_s5;
                temp_a0_4 = (temp_v0_3 & ~0x78) | (((((temp_v0_3 >> 3) & 0xF) + 1) & 0xF) * 8);
                *var_s5 = temp_a0_4;
                if (((temp_a0_4 >> 3) & 0xF) == 8)
                {
                    *var_s5 = (temp_a0_4 & ~7) | 2;
                }
                break;

            case 2:
                var_s0 = func_80144544((*(Unk6Func*)((u8*)var_s5 + 8))(var_s4, var_s0, 0, 0), var_s4, (*var_s5 >> 7) & 0x1FF,
                                          (*((u8*)var_s5 + 2)), (*var_s5 >> 24) | ((*(u32*)((u8*)var_s5 + 4) & 1) << 8),
                                          (*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF, arg0->unk40B2);
                break;

            case 3:
                temp_a1 = *(u32*)((u8*)var_s5 + 4);
                temp_a2 = ((temp_a1 & 1) << 8) | (temp_a0_2 >> 24);
                temp_a0_5 = (temp_a0_2 >> 3) & 0xF;
                var_v1_2 = temp_a2 * temp_a0_5;
                if (var_v1_2 < 0)
                {
                    var_v1_2 += 7;
                }
                temp_a3_5 = (temp_a1 >> 1) & 0xFF;
                var_v0_2 = temp_a3_5 * temp_a0_5;
                temp_s1 = var_v1_2 >> 3;
                if (var_v0_2 < 0)
                {
                    var_v0_2 += 7;
                }
                temp_s2 = var_v0_2 >> 3;
                temp_a3_6 = (s32)(temp_a3_5 - temp_s2);

                var_s0 = func_80144544((*(Unk6Func*)((u8*)var_s5 + 8))(var_s4, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2), var_s4,
                                          ((*var_s5 >> 7) & 0x1FF) +
                                              (s32)((((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | (*var_s5 >> 24)) - temp_s1) / 2,
                                          (*((u8*)var_s5 + 2)) + ((s32)((*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF) - temp_s2) / 2, temp_s1, temp_s2,
                                          arg0->unk40B2);
                temp_v0_5 = *var_s5;
                temp_v1_3 = (temp_v0_5 & ~0x78) | (((((temp_v0_5 >> 3) & 0xF) - 1) & 0xF) * 8);
                *var_s5 = temp_v1_3;
                if (!((temp_v1_3 >> 3) & 0xF))
                {
                    *var_s5 = temp_v1_3 & ~7;
                }
                D_8016B948 = 0;
                break;
            }
        }

        var_s5 += 3;
    }

    arg0->unk40B8 = var_s0;
}

/** @brief TODO: as-yet-unnamed main-module global (frame/animation counter);
 *         its low bits drive the marker color. */
extern s32 D_800F22AC;

/**
 * @brief Packed GPU packet built by func_801443E4: a Psy-Q LINE_F4 (0x1C bytes,
 *        code 0x4C, pad 0x55555555) whose first 0x14 bytes are copied into an
 *        immediately-following POLY_F3 (code 0x20). Field names mirror the
 *        LINE_F4 layout in include/psyq/libgpu.h.
 */
typedef struct
{
    u8 addr[3]; /* 0x00 P_TAG addr (24-bit, set via addPrim) */
    u8 len;     /* 0x03 P_TAG len */
    u8 r;       /* 0x04 */
    u8 g;       /* 0x05 */
    u8 b;       /* 0x06 */
    u8 code;    /* 0x07 */
    s16 x0;     /* 0x08 */
    s16 y0;     /* 0x0A */
    s16 x1;     /* 0x0C */
    s16 y1;     /* 0x0E */
    s16 x2;     /* 0x10 */
    s16 y2;     /* 0x12 */
    s16 x3;     /* 0x14 */
    s16 y3;     /* 0x16 */
    u32 mask;   /* 0x18 LINE_F4 pad word (0x55555555) */
} GosubPrim;    /* 0x1C */

/**
 * @brief Build a marker primitive (LINE_F4 quad outline + POLY_F3 fill) and
 *        link both into the ordering table @p ot.
 *
 * The LINE_F4 receives a color derived from the low bits of D_800F22AC; @p flag
 * selects one of two vertical vertex arrangements about center (@p x, @p y).
 * The POLY_F3 is seeded by copying the LINE_F4's first 0x14 bytes, then its
 * len/code/color are overwritten.
 *
 * @param prim Destination packet buffer (LINE_F4 immediately followed by POLY_F3 space).
 * @param ot   Ordering-table tag both primitives are linked into (addPrim idiom).
 * @param x    Center X coordinate.
 * @param y    Center Y coordinate.
 * @param flag Selects the up vs down vertex arrangement.
 * @return Pointer just past the POLY_F3 (next free packet slot).
 *
 * @note WIP - best match 86.02% (gcc 2.7.2 CDK). Residual is a register-alloc
 *       cascade: the target moves the packet pointer to t0 and parks D_800F22AC
 *       in a0 (evicting arg0 from a0), whereas GCC here keeps arg0 in a0 and
 *       D_800F22AC in a1, permuting the whole a0/a1/t0/t1 assignment. The clean
 *       real-type variant (LINE_F4/POLY_F3 + setLineF4/addPrim/SET_BGR0) sits at
 *       78.85%; see working/func_801443E4/ for both and the analysis.
 */
StructS0 *func_801443E4(GosubPrim *prim, s32 *ot, s32 x, s32 y, s32 flag)
{
    s32 color;
    s16 tmp_x;
    s16 tmp_y;
    u32 i;
    u8 *p;
    u8 *dst;
    GosubPrim *prim2;

    prim->len = 6;
    (prim2 = prim)->code = 0x4C;
    prim2->mask = 0x55555555;
    if (D_800F22AC & 0x10)
    {
        color = D_800F22AC & 0xF;
    }
    else
    {
        color = (~D_800F22AC) & 0xF;
    }
    color = (color * 4) + 0x70;
    prim2->b = color;
    prim2->g = color;
    prim2->r = color;
    if (flag != 0)
    {
        prim2->y3 = y - 8;
        prim2->y0 = y - 8;
        tmp_x = x - 6;
        tmp_y = y + 4;
    }
    else
    {
        prim2->y3 = y + 8;
        prim2->y0 = y + 8;
        tmp_x = x - 6;
        tmp_y = y - 4;
    }
    prim2->x1 = tmp_x;
    prim2->x3 = x;
    prim2->x0 = x;
    prim2->y1 = tmp_y;
    prim2->x2 = x + 6;
    prim2->y2 = tmp_y;

    p = (u8 *)prim2;
    prim2 = (GosubPrim *)(p + 0x1C);
    dst = (u8 *)prim2;
    *(u32 *)p = (*(u32 *)p & 0xFF000000) | (*ot & 0xFFFFFF);
    i = 0;
    *ot = (*ot & 0xFF000000) | ((u32)p & 0xFFFFFF);
    do
    {
        i += 1;
        *dst = *p;
        p += 1;
        dst += 1;
    } while (i < 0x14U);

    prim2->len = 4;
    *(u32 *)&prim2->r = 0;
    prim2->code = 0x20;
    *(u32 *)prim2 = (*(u32 *)prim2 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((u32)prim2 & 0xFFFFFF);
    return (StructS0 *)((u8 *)prim2 + 0x14);
}

/**
 * @brief Emit a framed gosub panel: a clipped draw environment, a background,
 *        three nested outline rectangles, a fill tile and a texture-page packet.
 *
 * The draw environment is inset 2 pixels inside (@p x, @p y, @p w, @p h) and
 * biased to the bottom (0xF0) or top (8) half of the frame buffer according to
 * @p flag; it is written into @p prim by func_8001A5D4 and linked into @p ot.
 * The packet cursor then advances 0x40 bytes past that environment and the
 * panel body is appended: func_80146E30 draws the background, then three
 * func_80144764 outlines (white on the panel rect, black inset by one pixel,
 * black outset by one pixel). Finally a TILE (code 0x62, colour 0xC0C0C0)
 * covering the panel rect and an eight-byte DR_MODE (0xE1000045) are linked in.
 *
 * @param prim Destination packet buffer (the draw environment goes here).
 * @param ot   Ordering-table tag every primitive is linked into (addPrim idiom).
 * @param x    Panel left edge.
 * @param y    Panel top edge.
 * @param w    Panel width.
 * @param h    Panel height.
 * @param flag Non-zero selects the bottom (0xF0) frame-buffer half, zero the top (8).
 * @return Pointer just past the DR_MODE packet (next free packet slot).
 *
 * @note WIP - best match 96.43% (gcc 2.7.2 CDK). Two residues remain, both
 *       register allocation. (1) The four entry arg-copies come out in
 *       declaration order (prim, ot, x, y) where the target defers prim's copy
 *       to last (ot, x, y, prim); sched_oracle attributes that ordering to
 *       sched2, i.e. it is downstream of the allocation, not an emit-order
 *       problem. (2) The tail's packet cursor lands in v0 because local-alloc
 *       honours its copy suggestion from the call return, so the target's
 *       `addu a2, v0, zero` is elided and every scratch register in the tail
 *       rotates by one (v0/v1/a0/a1/a2 -> a2/v0/v1/a0/a1). The target's cursor
 *       must reach global-alloc instead (two disjoint live ranges or two basic
 *       blocks); every variable-identity, alias and temp shape probed so far is
 *       delta-exact 0. See working/func_80144544/.
 * @note `var_s0->unkE` is deliberately assigned before `var_s0->unkC`: the
 *       source order shortens h's live range by one insn, which is what puts
 *       h in s5 and y in s6 as the target does (+14 exact rows).
 */
StructS0* func_80144544(StructS0* prim, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 flag)
{
    StructS0* var_s0;
    StructS0* var_a0;
    s32 sp20[24];

    if (flag != 0)
    {
        func_8001C56C(sp20, x + 2, y + 0xF2, w - 4, h - 4);
    }
    else
    {
        func_8001C56C(sp20, x + 2, y + 0xA, w - 4, h - 4);
    }
    func_8001A5D4((s32)prim, sp20);

    prim->unk0 = (prim->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)prim & 0xFFFFFF);

    prim = (StructS0*)((u8*)prim + 0x40);
    var_s0 = func_80146E30(prim, ot, x, y, w, h);
    var_s0 = (StructS0*)func_80144764((GosubLine*)var_s0, ot, x, y, w, h, 0xFFFFFF);
    var_s0 = (StructS0*)func_80144764((GosubLine*)var_s0, ot, x + 1, y + 1, w - 2, h - 2, 0);
    var_s0 = (StructS0*)func_80144764((GosubLine*)var_s0, ot, x - 1, y - 1, w + 2, h + 2, 0);

    var_s0->unk4 = 0xC0C0C0;
    ((u8*)var_s0)[3] = 3;
    ((u8*)var_s0)[7] = 0x62;
    var_s0->unk8 = x;
    var_s0->unkA = y;
    var_s0->unkE = h;
    var_s0->unkC = w;
    var_s0->unk0 = (var_s0->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)var_s0 & 0xFFFFFF);

    var_a0 = (StructS0*)((u8*)var_s0 + 0x10);
    ((u8*)var_a0)[3] = 1;
    var_a0->unk4 = 0xE1000045;
    var_a0->unk0 = (var_a0->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)var_a0 & 0xFFFFFF);
    return (StructS0*)((u8*)var_a0 + 8);
}

/**
 * @brief Emit a rectangle outline as four flat lines linked into @p ot.
 *
 * Each edge is inset four pixels at both ends, so the four lines form an open
 * frame with notched corners: top (left to right), right (top to bottom),
 * bottom (right to left) and left (bottom to top). Every packet takes the same
 * packed RGB @p color.
 *
 * @param p     Destination packet buffer; four LINE_F2 packets are written here.
 * @param ot    Ordering-table tag all four lines are linked into.
 * @param x     Rectangle left edge.
 * @param y     Rectangle top edge.
 * @param w     Rectangle width.
 * @param h     Rectangle height.
 * @param color Packed 0x00BBGGRR colour written to every line.
 * @return Pointer just past the fourth line (next free packet slot).
 *
 * @see decomp.me (100%)
 */
GosubLine *func_80144764(GosubLine *p, s32 *ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    *(u32 *)&p->r0 = color;
    ((GosubTag *)p)->len = 3;
    p->code = 0x40;
    p->x0 = x + 4;
    p->y0 = y;
    p->x1 = (x + w) - 4;
    p->y1 = y;
    ADD_PRIM(ot, p);
    p++;

    *(u32 *)&p->r0 = color;
    ((GosubTag *)p)->len = 3;
    p->code = 0x40;
    p->x0 = x + w;
    p->y0 = y + 4;
    p->x1 = x + w;
    p->y1 = (y + h) - 4;
    ADD_PRIM(ot, p);
    p++;

    *(u32 *)&p->r0 = color;
    ((GosubTag *)p)->len = 3;
    p->code = 0x40;
    p->x0 = (x + w) - 4;
    p->y0 = y + h;
    p->x1 = x + 4;
    p->y1 = y + h;
    ADD_PRIM(ot, p);
    p++;

    *(u32 *)&p->r0 = color;
    ((GosubTag *)p)->len = 3;
    p->code = 0x40;
    p->x0 = x;
    p->y0 = y + 4;
    p->x1 = x;
    p->y1 = (y + h) - 4;
    ADD_PRIM(ot, p);
    return p + 1;
}

/**
 * @brief Draw the gosub item list: one packet run per row, then the cursor
 *        highlight and one highlight per selected row.
 *
 * Rows dispatch on @c value: -3 is a full equipment card (icon strip via
 * func_801450D8 plus three text lines and up to one status glyph), -2 is a
 * combination header (func_801466B4 frame plus a label), anything else is a
 * plain label with an optional trailing glyph. Each row is culled against
 * g_gosub_window_height before any packet is emitted. The tail appends a
 * 0xF080F0 TILE for the cursor row and a 0x808080 TILE per entry of
 * g_gosub_selected_rows.
 *
 * @param ot    Ordering-table tag every packet is linked into.
 * @param prim  Packet cursor.
 * @param x_off Horizontal offset subtracted from every column position.
 * @param y_off Vertical scroll offset subtracted from every row position.
 * @return Packet cursor just past the last highlight tile.
 *
 * @note WIP - best match 73.24% (189/507 exact, gcc 2.7.2 CDK). Frame comes out
 *       -0x58 against the target's -0x60: the target keeps %hi(D_8014F29C) in its
 *       own long-lived pseudo (s7) reused by every in-loop value read, so it needs
 *       one more saved register; here the lui is re-materialized per read and every
 *       later saved register slides down one. That single missing pseudo accounts
 *       for the frame delta and, through it, most of the argdiff rows.
 * @note The row loop is deliberately a label + goto, NOT a do/while - idioms.md
 *       [LOOP-09]. The target shows no loop optimizations at all (in-loop lui per
 *       use, both counters in stack slots, no reduced givs); the do/while form adds
 *       a second induction variable and a +0xC-biased entry pointer. The tail
 *       selection loop IS a real do/while (converting it measures -7 exact).
 * @note `base` must be built in two statements; written as `(u8*)&D_8014F29C - 0x20`
 *       gcc folds it to one %hi(D_8014F29C-0x20) relocation (-10 exact).
 * @see working/func_801448EC/STATUS.md for the measured probe log and retired classes.
 */
StructS0* func_801448EC(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32 row_offset;
    Vec2s* pos_p;
    s32 drawn_count;
    Vec2s pos;
    GosubListEntry* entry;
    s32 row;
    s32 y;
    s32 line_y;
    s32 label_x;
    u8* base;
    s32 blk;
    StructS0* tile;
    StructS0* mark;

    row = 0;
    drawn_count = 0;
    if (g_gosub_row_count > 0)
    {
        label_x = 0x30 - x_off;
        base = (u8*)&D_8014F29C;
        base -= 0x20;
        entry = &g_gosub_rows[0];
        pos_p = &pos;
        row_offset = 0;
    row_loop:
        {
            if (entry->value == -3)
            {
                y = ((row * 0x30) - y_off) - g_gosub_scroll_y;
                if (y >= -0x2F && y < g_gosub_window_height)
                {
                    prim = func_801450D8(prim, ot, row, -x_off, y, drawn_count);
                    prim = func_800A88A0(prim, ot, entry->name, entry->kind, label_x, y, 0);
                    if (entry->flags.f.flag2)
                    {
                        if ((entry->flags.half & 1) == 0)
                        {
                            line_y = y + 0x10;
                            prim = func_800A88A0(prim, ot, GOSUB_MSG_ABS(base, 0x24), entry->kind, label_x, line_y, 0);
                            pos.x = 0x54 - x_off;
                            pos.y = line_y;
                            prim = func_800A8A78(ot, prim, entry->unkD, entry->kind, pos_p, 0);
                            blk = *(s32*)(base + 0x24);
                            prim = func_800A88A0(prim, ot, base + blk + *(u16*)(base + blk + entry->unkC * 2),
                                                 entry->kind, 0x84 - x_off, line_y, 0);
                        }
                        else
                        {
                            prim = func_800A88A0(prim, ot, GOSUB_MSG_ABS(base, entry->unkD * 2 + 0x44), entry->kind, label_x,
                                                 y + 0x10, 0);
                        }
                    }
                    else
                    {
                        blk = D_8014F2A4;
                        prim = func_800A88A0(prim, ot, base + blk + *(u16*)(base + blk + entry->unkC * 2), entry->kind,
                                             label_x, y + 0x10, 0);
                    }
                    line_y = y + 0x20;
                    prim = func_800A88A0(prim, ot, GOSUB_MSG_ABS(base, 0x26), entry->kind, label_x, line_y, 0);
                    pos.x = 0x48 - x_off;
                    pos.y = line_y;
                    prim = func_800A8A78(ot, prim, entry->unk1A, entry->kind, pos_p, 0);
                    prim = func_800A88A0(prim, ot, base + D_8014F29C + *(u16*)((u8*)&D_8014F2C4 + D_8014F29C),
                                         entry->kind, 0x64 - x_off, line_y, 0);
                    pos.x = 0xB0 - x_off;
                    pos.y = line_y;
                    prim = func_800A8A78(ot, prim, entry->unk10, entry->kind, pos_p, 0);
                    if (entry->unkE != 0)
                    {
                        prim = func_800A88A0(prim, ot, GOSUB_MSG_REL(base, 0x4A), entry->kind,
                                             g_gosub_window_width - (x_off + 0xC), line_y, 1);
                    }
                    else if (entry->flags.half & 1)
                    {
                        prim = func_800A88A0(prim, ot, GOSUB_MSG_REL(base, 0x60), entry->kind,
                                             g_gosub_window_width - (x_off + 0xC), line_y, 1);
                    }
                    else if (entry->flags.f.flag1)
                    {
                        prim = func_800A88A0(prim, ot, GOSUB_MSG_REL(base, 0x6E), entry->kind,
                                             g_gosub_window_width - (x_off + 0xC), line_y, 1);
                    }
                    drawn_count += 1;
                }
            }
            else if (entry->value == -2)
            {
                y = (row_offset - y_off) - g_gosub_scroll_y;
                if (y >= -0x1F && y < g_gosub_window_height)
                {
                    line_y = y + 8;
                    prim = func_801466B4(prim, ot, 0xC - x_off, y, entry->unkE, entry->unkD);
                    prim = func_800A88A0(prim, ot, entry->name, entry->kind, 0x4C - x_off, line_y, 0);
                    if (entry->flags.f.flag2)
                    {
                        prim = func_800A88A0(prim, ot, GOSUB_MSG_ABS(base, 0x20), entry->kind, 0x110 - x_off, line_y, 1);
                    }
                }
            }
            else
            {
                y = ((row * g_gosub_row_height) - (y_off - 2)) - g_gosub_scroll_y;
                if (-g_gosub_row_height < y && y < g_gosub_window_height)
                {
                    prim = func_800A88A0(prim, ot, entry->name, entry->kind, 0xC - x_off, y, 0);
                    pos.y = y;
                    pos.x = g_gosub_window_width - (x_off + 0xC);
                    if (entry->value >= 0)
                    {
                        prim = func_800A8A78(ot, prim, entry->value, entry->kind, pos_p, 1);
                    }
                }
            }
            entry += 1;
            row += 1;
            row_offset += 0x20;
        }
        if (row < g_gosub_row_count)
        {
            goto row_loop;
        }
    }

    tile = (StructS0*)prim;
    mark = (StructS0*)((u8*)tile + 0x10);
    ((u8*)tile)[3] = 3;
    tile->unk4 = 0xF080F0;
    ((u8*)tile)[7] = 0x62;
    row = 0;
    tile->unkE = g_gosub_row_height - 1;
    tile->unk8 = 1;
    tile->unkC = g_gosub_window_width;
    y = ((g_gosub_cursor_row * g_gosub_row_height) - (y_off - 2)) - g_gosub_scroll_y;
    tile->unkA = y - 2;
    ADD_PRIM(ot, tile);
    if (g_gosub_selection_count != 0)
    {
        do
        {
            mark->unk8 = 1;
            mark->unk4 = 0x808080;
            ((u8*)mark)[3] = 3;
            ((u8*)mark)[7] = 0x62;
            mark->unkE = g_gosub_row_height - 1;
            mark->unkC = g_gosub_window_width;
            y = ((g_gosub_selected_rows[row] * g_gosub_row_height) - (y_off - 2)) - g_gosub_scroll_y;
            mark->unkA = y - 2;
            row += 1;
            ADD_PRIM(ot, mark);
            mark += 1;
        } while (row < g_gosub_selection_count);
    }
    return mark;
}

/** @brief Sprite primitive (0x14 bytes, code 0x64), SPRT layout. */
typedef struct
{
    u_long tag;  /* 0x00 P_TAG */
    u8 r0;       /* 0x04 */
    u8 g0;       /* 0x05 */
    u8 b0;       /* 0x06 */
    u8 code;     /* 0x07 */
    s16 x0;      /* 0x08 */
    s16 y0;      /* 0x0A */
    u8 u0;       /* 0x0C */
    u8 v0;       /* 0x0D */
    u16 clut;    /* 0x0E */
    s16 w;       /* 0x10 */
    s16 h;       /* 0x12 */
} GosubSprt;     /* 0x14 */

/** @brief libgpu RECT (s16 x, y, w, h). */
typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} GosubRect;

extern s32 g_gosub_frame_parity;
/** @brief Portrait archive: s32 offset table at base, pixels at +0x1C, cluts at -4. */
extern s32 D_80152DF4[];

s32 LoadImage(); /* extern */

/**
 * @brief Upload a row's portrait strip and CLUT, then emit its 48x48 sprite.
 *
 * The portrait index comes from the row's unkC (offset by 0x41 when flag2 is
 * clear); the pixel and CLUT rectangles are double-buffered by
 * g_gosub_frame_parity. Draws nothing once five portraits are on screen.
 *
 * @param prim  Packet cursor.
 * @param ot    Ordering-table tag to link into.
 * @param row   g_gosub_rows index to portray.
 * @param x     Sprite left edge.
 * @param y     Sprite top edge.
 * @param count How many portraits were already emitted this frame.
 * @return Packet cursor past the sprite (func_8014680C's return), or prim
 *         when count is 5 or more.
 *
 * @note WIP - best match 93.92% (99/134 exact rows, gcc 2.7.2 CDK). The
 *       residue is one 5-cycle saved-register rotation (count must color s1;
 *       here it lands s5) plus three instruction-order pairs. Measured
 *       evidence, retired probe classes and next moves are in
 *       working/func_801450D8/STATUS.md.
 */
s32 func_801450D8(s32 prim, s32* ot, s32 row, s32 x, s32 y, s32 count)
{
    GosubSprt* sprt;
    GosubRect rect;
    s32* entry;
    s32 idx;
    s32 cell;

    if (count >= 5)
    {
        return prim;
    }

    if (g_gosub_rows[row].flags.f.flag2)
    {
        idx = g_gosub_rows[row].unkC;
    }
    else
    {
        idx = g_gosub_rows[row].unkC + 0x41;
    }
    cell = count * 3;

    rect.x = cell * 4 + 0x140;
    rect.w = 0xC;
    rect.h = 0x30;
    rect.y = g_gosub_frame_parity * 0x30;
    entry = &D_80152DF4[idx];
    LoadImage(&rect, (u8*)D_80152DF4 + *entry + 0x1C);

    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    count = count * 0x10;
    rect.x = count + g_gosub_frame_parity * 0x50;
    LoadImage(&rect, (u8*)D_80152DF4 + *entry - 4);

    sprt = (GosubSprt*)prim;
    *(u32*)&sprt->r0 = 0x808080;
    ((GosubTag*)sprt)->len = 4;
    sprt->code = 0x64;
    sprt->u0 = cell * 0x10;
    sprt->x0 = x;
    sprt->v0 = g_gosub_frame_parity * 0x30;
    sprt->y0 = y;
    sprt->w = 0x30;
    sprt->h = 0x30;
    count = count + g_gosub_frame_parity * 0x50;
    count = ((count >> 4) & 0x3F) | 0x7C80;
    sprt->clut = count;
    ADD_PRIM(ot, sprt);
    return func_8014680C(prim + 0x14, ot);
}

/** @brief Item-name archive: block offset table at base, entries at base - 0x18. */
extern u32 D_8014F294[];
/** @brief Separator glyph record; the string itself sits at D_800EC3DA - 0x16. */
extern u8 D_800EC3DA[];

/**
 * @brief Draw the combination preview for the cursor row.
 *
 * When at least one row is selected and the cursor sits on a different row,
 * func_800CA480 is asked whether the two rows' item indices combine. It
 * returns the resulting item in D_8017097C and fills D_8016B8EC and
 * D_8016B8E4; a zero result means the pair does not combine and nothing is
 * drawn. Otherwise a frame is emitted, then the result's archive name, with
 * D_8016B8EC's decimal form appended after a separator when it is nonzero.
 *
 * @param ot       Ordering-table tag every packet is linked into.
 * @param arg_prim Packet cursor.
 * @param x_off    Horizontal offset subtracted from every column position.
 * @param y_off    Vertical offset subtracted from every row position.
 * @return Packet cursor past the last packet, or the incoming cursor when
 *         there is no combination to show.
 *
 * @see decomp.me (100%)
 */
s32 func_801452F0(s32* ot, s32 arg_prim, s32 x_off, s32 y_off)
{
    s32 unused[2]; /* never referenced; reserves the leading frame slot */
    u8 buf[0x50];
    u8 tmp[0x50];
    s32 pair[3];
    s32 base;
    u8* p;
    u8* arch;
    s32 blk;
    s32 prim;

    D_8017097C = 0;
    prim = arg_prim;
    if (g_gosub_selection_count != 0)
    {
        if (g_gosub_cursor_row != g_gosub_selected_rows[0])
        {
            pair[0] = g_gosub_rows[g_gosub_selected_rows[0]].index;
            pair[1] = g_gosub_rows[g_gosub_cursor_row].index;
            D_8017097C = func_800CA480(pair, &D_8016B8EC, &D_8016B8E4);
        }
    }
    if (D_8017097C != 0)
    {
        prim = func_801466B4(prim, ot, 0xC - x_off, -y_off, D_8017097C, D_8016B8E4);
        p = buf;
        arch = (u8*)D_8014F294;
        base = (s32)arch;
        base -= 0x18;
        blk = D_8014F294[0];
        func_80146538(p, blk + (*(u16*)(D_8017097C * 2 + blk + base) + base));
        if (D_8016B8EC != 0)
        {
            func_80146468(p, D_800EC3DA - 0x16 + D_800EC3DA[0] + (D_800EC3DA[1] << 8));
            func_800A8B90(tmp, D_8016B8EC, 1);
            func_80146468(p, tmp);
        }
        prim = func_800A88A0(prim, ot, p, 4, 0x4C - x_off, 0xA - y_off, 0);
    }
    return prim;
}

s32 func_80145620(s32); /* extern */

/**
 * @brief Dialog handler for the selected row's action prompt.
 *
 * A nonzero @p dialog_result cancels: the selection count is dropped and the
 * dialog element is deactivated. On confirm, bit 0 of g_gosub_dialog_choice
 * picks the path. When it is clear the work is handed to func_8014595C. When
 * it is set the first selected row decides: a row with flag2 set is rejected
 * with message 0x22 (the selection is dropped and D_8016B95C is raised),
 * otherwise a follow-up dialog is opened with func_80145620 as its handler.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return Always 0.
 * @see decomp.me (100%)
 */
s32 func_801454C4(s32 dialog_result)
{
    GosubElement* element;

    if (dialog_result != 0)
    {
        g_gosub_selection_count = 0;
        g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
        return 0;
    }

    if (g_gosub_dialog_choice & 1)
    {
        if (g_gosub_rows[g_gosub_selected_rows[0]].flags.f.flag2)
        {
            GOSUB_MSG(0x22);
            g_gosub_selection_count = 0;
            D_8016B95C = 1;
            return 0;
        }
        element = &g_gosub_elements[0];
        element->draw_handler = (void*)&func_80145F80;
        g_gosub_dialog_handler = func_80145620;
        g_gosub_dialog_choice = 0;
        element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
        element->attr.f.unk0_3 = 1;
        element->attr.f.x = 0x20;
        element->attr.f.unk0_16 = 0x70;
        element->unk4_0 = 1;
        element->y = 0x24;
        SET_ELEM_CODE(element, 0);
    }
    else
    {
        func_8014595C();
    }
    return 0;
}

/**
 * @brief Dialog handler that drops the cursor row and re-clamps the list.
 *
 * Always deactivates the dialog element. A nonzero @p dialog_result, or bit 0
 * of g_gosub_dialog_choice, cancels: the selection count is set to 1 and
 * nothing else changes. On confirm the cursor row is handed to func_80146908
 * (by entry index) and to func_801469BC (by row), the selection is cleared,
 * and the viewport is re-clamped -- the cursor is pulled back to the last row
 * when it now sits past the end, and the scroll target is clamped to the
 * bottom of the shortened list over a 4-frame scroll.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return 1 when no rows remain, otherwise 0.
 * @see decomp.me (100%)
 */
s32 func_80145620(s32 dialog_result)
{
    s32 scroll_y;
    s32 max_scroll;

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        func_80146908(g_gosub_rows[g_gosub_cursor_row].index);
        func_801469BC(g_gosub_cursor_row);
        g_gosub_selection_count = 0;
        if (g_gosub_row_count == 0)
        {
            return 1;
        }
        if (g_gosub_cursor_row >= g_gosub_row_count)
        {
            g_gosub_cursor_row = g_gosub_row_count - 1;
        }
        scroll_y = g_gosub_scroll_y;
        max_scroll = ((g_gosub_row_count * g_gosub_row_height) - g_gosub_window_height) + 4;
        if (max_scroll < scroll_y)
        {
            scroll_y = max_scroll;
        }
        if (scroll_y < 0)
        {
            scroll_y = 0;
        }
        g_gosub_scroll_target_y = scroll_y;
        g_gosub_scroll_frames_remaining = 4;
        return 0;
    }
    g_gosub_selection_count = 1;
    return 0;
}

/**
 * @brief Dialog handler that backs the screen sequence out one step.
 *
 * Does nothing and reports 1 when the dialog was confirmed with bit 0 of
 * g_gosub_dialog_choice clear. Otherwise it releases one selection slot (only
 * when the selection is already full), steps the screen sequence back, runs
 * g_gosub_select_handler when one is installed, drops the nesting depth in
 * D_801228F0, and puts element 0 into its exit animation.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return 1 when the confirm path is taken, otherwise 0.
 *
 * @note The D_801228F0 decrement is deliberately written out in BOTH arms of
 *       the g_gosub_select_handler test rather than once after it. Do not merge
 *       the two into a single statement - the duplicate is required to match.
 * @see decomp.me (100%)
 */
s32 func_80145744(s32 dialog_result)
{
    if (dialog_result == 0 && (g_gosub_dialog_choice & 1) == 0)
    {
        return 1;
    }

    if (g_gosub_required_selection_count == g_gosub_selection_count)
    {
        g_gosub_selection_count -= 1;
    }

    g_gosub_screen_sequence_index -= 1;
    if (g_gosub_select_handler != 0)
    {
        g_gosub_select_handler();
        D_801228F0 -= 1;
    }
    else
    {
        D_801228F0 -= 1;
    }

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_EXITING;
    g_gosub_elements[0].attr.f.unk0_3 = 8;
    return 0;
}

/** @brief Toggling flag paired with g_gosub_dialog_choice to pick a sound slot. */
extern s32 D_8017098C;

/**
 * @brief Dialog handler that plays the choice's sound and flips its bank.
 *
 * On confirm it hands func_80146AF8 a slot built from D_8017098C's bank bit and
 * the dialog choice reduced modulo 3, clears the selection count, then flips
 * bit 0 of D_8017098C so the next confirm uses the other bank. Cancelling only
 * sets the selection count to 1. Either way element 0 is deactivated.
 *
 * @param dialog_result Zero to confirm; nonzero to cancel.
 * @return Always 0.
 * @see decomp.me (100%)
 */
s32 func_80145800(s32 dialog_result)
{
    if (dialog_result == 0)
    {
        func_80146AF8((D_8017098C << 7) + (g_gosub_dialog_choice % 3));
        g_gosub_selection_count = 0;
        D_8017098C ^= 1;
    }
    else
    {
        g_gosub_selection_count = 1;
    }

    g_gosub_elements[0].attr.f.state = GOSUB_ELEMENT_STATE_INACTIVE;
    return 0;
}

void func_80145A14(); /* extern */

/**
 * @brief Open the wide confirmation dialog and hand it to func_801454C4.
 *
 * Installs func_80145A14 as element 0's draw handler and func_801454C4 as the
 * dialog's result handler, clears the pending choice, then starts the element
 * entering at x 0x80 / y 0x24 with code 0x80. func_800AA02C runs last.
 *
 * @see decomp.me (100%)
 */
void func_801458A4(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&func_80145A14;
    g_gosub_dialog_choice = 0;
    g_gosub_dialog_handler = func_801454C4;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.unk0_3 = 1;
    element->attr.f.x = 0x80;
    element->attr.f.unk0_16 = 0x70;
    element->unk4_0 = 0;
    element->y = 0x24;
    SET_ELEM_CODE(element, 0x80);
    func_800AA02C();
}
