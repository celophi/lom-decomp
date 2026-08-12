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
    s32 kind : 4;
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
 * @see decomp.me (99.94%) https://decomp.me/scratch/2OzmD
 */
s32 gosub_handle_combination_dialog(s32 dialog_result)
{
    s32 combination_count;
    GosubPackedRecord* record;
    u32 packed;
    s32 config;
    s32 secondary_value;
    s32 clear_config_mask;

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
            packed = ((dialog_result & 0xFFFF0FFF) | ((D_8016B8E4 & 0xF) << 12) | 3) & 0xFFFF;
            record->word = packed;
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
 * @see decomp.me (95%) https://decomp.me/scratch/pOY6i
 */
s32 gosub_publish_group_selection(void)
{
    s32 selection_count;
    s32 selection_index;
    s32* result;
    s16 row_index;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    selection_index = 0;

    selection_count = g_gosub_selection_count;
    g_gosub_result_count = selection_count;

    if (selection_count != 0)
    {
        result = &g_gosub_result_values[0];

        do
        {
            row_index = g_gosub_rows[g_gosub_selected_rows[selection_index]].index;
            selection_index += 1;
            *result = (s32)row_index;
            result += 1;
        } while (selection_index < selection_count);
    }

    return 1;
}

/**
 * @brief Publish the selected rows' entry indices as result values.
 *
 * @return 1 when at least one row was published, otherwise 0.
 * @see decomp.me (95%) https://decomp.me/scratch/FN7DQ
 */
s32 gosub_publish_selection(void)
{
    s32 selection_count;
    s32 selection_index;
    s32* result;
    s16 row_index;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }

    selection_index = 0;

    selection_count = g_gosub_selection_count;
    g_gosub_result_count = selection_count;

    if (selection_count != 0)
    {
        result = &g_gosub_result_values[0];

        do
        {
            row_index = g_gosub_rows[g_gosub_selected_rows[selection_index]].index;
            selection_index += 1;
            *result = (s32)row_index;
            result += 1;
        } while (selection_index < selection_count);
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