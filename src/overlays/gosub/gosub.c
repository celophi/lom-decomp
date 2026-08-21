typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef unsigned char undefined;
typedef unsigned char undefined1;
typedef unsigned short undefined2;
typedef unsigned int undefined4;

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

void field_set_default_fade_target();                   /* extern */
void func_800A8B90();                                   /* extern */
void func_800AA02C();                                   /* extern */
void gosub_load_screen_sequence(s32*);                  /* extern */
void gosub_build_screen_9_elements();                   /* extern */
void gosub_build_screen_10_elements();                  /* extern */
void gosub_build_category_screen_elements();            /* extern */
void gosub_build_list_screen_elements();                /* extern */
void gosub_build_screen_11_elements();                  /* extern */
void gosub_build_compact_list_elements();               /* extern */
void gosub_build_screen_15_item_list();                 /* extern */
void gosub_build_screen_19_item_list();                 /* extern */
void gosub_build_screen_16_item_list();                 /* extern */
void gosub_build_screen_1_item_list();                  /* extern */
void gosub_build_screen_0_item_list();                  /* extern */
void gosub_build_packed_record_list();                  /* extern */
void gosub_build_roster_list();                         /* extern */
s32 gosub_select_row_with_validation();                 /* extern */
s32 gosub_select_row();                                 /* extern */
s32 gosub_validate_pending_pair_selection();            /* extern */
s32 gosub_commit_row_reorder();                         /* extern */
s32 gosub_update_group_selection();                     /* extern */
s32 gosub_publish_two_row_selection();                  /* extern */
s32 gosub_handle_combination_dialog(s32 dialog_result); /* extern */
s32 gosub_publish_group_selection(void);                /* extern */
s32 gosub_publish_selection(void);                      /* extern */
s32 gosub_is_row_unselected(s32 row);                   /* extern */
void gosub_build_equipment_list(u32 item_kind);         /* extern */
void gosub_build_grouped_option_list(s32 group);        /* extern */
void gosub_update_screen(s32 render_ctx);               /* extern */
void func_80143B64();                                   /* extern */
void func_80145CEC();                                   /* extern */
s32 func_80145DA8();                                    /* extern */
s32 func_80145DF8();                                    /* extern */
s32 func_80145EA4();                                    /* extern */
s32 func_80145F80();                                    /* extern */
s32 func_801460D0();                                    /* extern */
void func_80146468();                                   /* extern */
void func_80146538();                                   /* extern */
extern s32 g_gosub_frame_parity;
extern s32 g_gosub_finished;
extern s32 g_gosub_scroll_frames_remaining;
extern s32 g_gosub_scroll_target_y;
extern s32 g_gosub_scroll_y;
extern s32 g_gosub_cursor_row;
extern s32 D_8017098C;
extern u8 g_gosub_screen_sequence_index;
extern s32 g_gosub_result_count;
extern u8 D_80145744;
extern s32 (*g_gosub_dialog_handler)(s32);
extern s32 D_8016B948;
extern u8 g_gosub_screen_sequence[20];
extern u8* g_pad_ctx;
extern s32 D_8014F29C;
extern s32 g_gosub_row_count;
extern s32 g_gosub_visible_row_count;
extern s32 D_8016B8E4;
extern s32 D_8016B8EC;
extern s32 D_8016B8F4;
extern s32 g_gosub_allow_duplicate_selection;
extern s32 (*g_gosub_finish_handler)();
extern u8 D_8016B8FC;
extern u8 g_gosub_required_selection_count;
extern s32 D_8016B900;
extern s32 g_gosub_window_height;
extern s32 (*g_gosub_select_handler)();
extern s32 g_gosub_window_width;
extern s32 D_8016B95C;
extern u8 g_gosub_selected_rows[4];
extern s32 g_gosub_result_rows[16];
extern s32 g_gosub_result_values[];
extern s32 D_8017097C;
extern s32 g_gosub_row_height;
extern u8 D_801448EC;
extern u8 D_801452F0;
extern u8 D_80146418;
extern u8 g_gosub_selection_count;
extern s32 g_gosub_dialog_choice;
extern u8 D_800EC3DA[];
extern u8 D_800EC3EE[];
extern u8 D_800EC3F0[];
extern u8 D_800EC3F2[];
extern u32 D_8014F27C[];
extern u32 D_8014F280[];
extern u32 D_8014F294[];
extern s32 D_8014F2A8;
extern u8 g_gosub_text_buffers[];
extern u8 D_8016B5AC[];
extern u8* g_gosub_title_text;

/**
 * @brief One entry in the gosub screen's element list, as handed out by
 *        func_80143C04.
 *
 * The first word packs several bitfields plus a top byte that the code always
 * rewrites through the whole word (see SET_ELEM_CODE), so it is exposed as a
 * union. The second word carries a flag and the y coordinate.
 *
 * @note Field meanings beyond state/x/y are unconfirmed; the `_N` suffixes
 *       record the bit position each one starts at.
 */
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

/**
 * @brief Set the top byte of an element's attr word.
 *
 * Must go through the whole word rather than an 8-bit bitfield: a bitfield
 * assignment narrows to `sb` at offset 3, which is not what the game does.
 *
 * @param e Element to update.
 * @param c New top-byte value. TODO: meaning unknown; observed 0xE8 and 0x08.
 */
#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

/** @brief UI element pool; element 0 is reserved for the fixed dialog element. */
extern GosubElement g_gosub_elements[GOSUB_ELEMENT_COUNT];

GosubElement* func_80143C04();

/**
 * @brief One row of the item list built by the gosub screen builders.
 *
 * The word at 0xC packs three signed bytes below a 4-bit row kind. The simple
 * slot builders only ever write @c kind, but gosub_build_packed_record_list uses all three
 * bytes, so they are declared as byte-aligned 8-bit bitfields: writes narrow
 * to @c sb and reads to @c lb, which is what the target does. The top nibble
 * @c unkC_28 is unsigned: gosub_update_group_selection reads it with a plain @c srl, where a
 * signed field would sign-extend.
 *
 * The word at 0x1C is a flag set. Most builders write single bits through the
 * @c f bitfields, but gosub_build_roster_list tests bit 0 through the @c half alias and
 * gosub_build_packed_record_list rewrites bit 2 through @c word, so all three views are exposed
 * as a union; each spelling is the access width the game's code uses.
 *
 * @note Only the fields the builders touch are known; 0x10..0x1B is a block of
 *       u16s copied verbatim out of the source record.
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

extern GosubListEntry g_gosub_rows[];

/**
 * @brief Resolve one entry of a text archive block.
 *
 * D_8014F27C heads a 13-word table of block offsets, each relative to the
 * table itself. A block begins with a u16 per entry giving that entry's offset
 * from the same table base, so a lookup is base + block + block[idx].
 *
 * @param blk Block offset word, e.g. D_8014F280[0] or D_8014F27C[12].
 * @param idx Entry index within the block.
 * @return Pointer to the entry.
 * @note The term order matters: writing the base first is what produces the
 *       target's accumulate chain (see idioms.md [EXPAND-14]).
 */
#define ARCHIVE_ENTRY(blk, idx) ((u8*)D_8014F27C + (blk) + *(u16*)((u8*)D_8014F27C + (blk) + (idx) * 2))

/**
 * @brief Resolve a message-archive entry at byte offset @p off.
 *
 * The same lookup as ARCHIVE_ENTRY against block 8, but spelled through
 * D_8014F29C (which is that block's offset word) because that is the
 * relocation the game's code carries; do not fold it into ARCHIVE_ENTRY.
 *
 * @param off Byte offset of the u16 entry index within the resolved block.
 * @return Pointer to the entry.
 */
#define GOSUB_MSG_PTR(off) ((u8*)&D_8014F29C - 0x20 + D_8014F29C + *(u16*)((u8*)&D_8014F29C + D_8014F29C + (off)))

/**
 * @brief Resolve the message-archive entry at @p off and hand it to func_80145CEC.
 *
 * @param off Byte offset of the u16 entry index within the resolved block.
 */
#define GOSUB_MSG(off) func_80145CEC(GOSUB_MSG_PTR(off))

/**
 * @brief Open the gosub overlay for a sequence of screen ids.
 *
 * @param unused Unused loader argument.
 * @param screen_sequence Pointer to an s32 array terminated by
 *        @c GOSUB_SCREEN_SEQUENCE_END.
 *
 * decomp.me (100%) https://decomp.me/scratch/qM81L
 */
void gosub_open_screen_sequence(s32 unused, s32 screen_sequence)
{
    field_set_default_fade_target();
    g_gosub_frame_parity = 0;
    g_gosub_finished = 0;
    func_800AA02C();
    gosub_load_screen_sequence(screen_sequence);
}

/**
 * @brief Run one frame of the gosub overlay and return its completion state.
 *
 * decomp.me (100%) https://decomp.me/scratch/ykfW4
 */
s32 gosub_update_frame(s32 render_ctx)
{
    s32* frame_parity;
    s32 finished;
    field_text_reset_scratch();
    gosub_update_screen(render_ctx);
    func_80063194();
    frame_parity = &g_gosub_frame_parity;
    finished = g_gosub_finished;
    *frame_parity ^= 1;
    return finished;
}

/**
 * @brief Copy and enter a GOSUB_SCREEN_SEQUENCE_END-terminated screen sequence.
 *
 * decomp.me (100%) https://decomp.me/scratch/weBhP
 */
void gosub_load_screen_sequence(s32* screen_sequence)
{
    u8* sequence_cursor;
    s32 screen_count;
    u8 screen_id;
    s32 pad[2];

    func_801465BC();
    g_gosub_scroll_frames_remaining = 0;
    g_gosub_scroll_target_y = 0;
    g_gosub_scroll_y = 0;
    g_gosub_cursor_row = 0;
    func_80143BD0();
    D_8017098C = 0;
    g_gosub_screen_sequence_index = 0;
    g_gosub_result_count = 0;
    g_gosub_dialog_handler = (void*)&D_80145744;
    screen_count = 0;
    if (*screen_sequence != GOSUB_SCREEN_SEQUENCE_END)
    {
        u8* arr = g_gosub_screen_sequence;
        s32 sentinel = GOSUB_SCREEN_SEQUENCE_END;
        sequence_cursor = (u8*)screen_sequence;
        do
        {
            screen_id = *sequence_cursor;
            sequence_cursor += 4;
            *((u8*)(screen_count + (u32)arr)) = screen_id;
            screen_count++;
        } while (*(s32*)sequence_cursor != sentinel);
    }
    g_gosub_screen_sequence[screen_count] = ((u8*)screen_sequence)[screen_count * 4];
    D_8016B948 = 0;
    gosub_enter_screen(g_gosub_screen_sequence[g_gosub_screen_sequence_index], screen_count);
    func_80146DA8();
}

/**
 * @brief Enter one of the 20 gosub sub-screens: reset the shared state, install
 *        the screen's selection/completion callbacks, and queue its intro message.
 *
 * Every arm zeroes the common state block, runs the screen's own setup helper,
 * publishes a selection callback in g_gosub_select_handler and a completion
 * callback in g_gosub_finish_handler,
 * then calls the screen's enter routine. Unless that routine reported a failure
 * (g_gosub_row_count, or the save-slot count at g_pad_ctx[0x29D6] for arms 10 and 11)
 * the arm finishes by submitting its message through GOSUB_MSG.
 *
 * @param screen_id Screen id, 0..19; anything else returns without touching state.
 * @param unused Unused by this function; passed by gosub_load_screen_sequence.
 */
void gosub_enter_screen(screen_id, unused) s32 screen_id;
s32 unused;
{
    g_gosub_scroll_frames_remaining = 0;
    g_gosub_scroll_target_y = 0;
    g_gosub_scroll_y = 0;
    g_gosub_cursor_row = 0;
    D_8016B8E4 = 0;
    D_8017097C = 0;
    D_8016B8EC = 0;
    g_gosub_allow_duplicate_selection = 0;
    D_8016B900 = 0;

    switch (screen_id)
    {
    case 0:
        func_80143B64();
        gosub_build_screen_0_item_list();
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(8);
        }
        break;

    case 1:
        func_80143B64();
        gosub_build_screen_1_item_list();
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0xA);
        }
        break;

    case 2:
        func_80143B64();
        gosub_build_equipment_list(0);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(2);
        }
        break;

    case 3:
        func_80143B64();
        gosub_build_equipment_list(1);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(4);
        }
        break;

    case 4:
        func_80143B64();
        gosub_build_equipment_list(2);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(6);
        }
        break;

    case 5:
        func_80143B64();
        gosub_build_equipment_list(3);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0);
        }
        break;

    case 6:
    case 7:
    case 8:
        func_80143B64();
        gosub_build_grouped_option_list(screen_id - 6);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        break;

    case 9:
        func_80143B64();
        gosub_build_equipment_list(4);
        g_gosub_required_selection_count = 4;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_update_group_selection;
        g_gosub_finish_handler = (void*)gosub_publish_group_selection;
        gosub_build_screen_9_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(-2);
        }
        break;

    case 10:
        func_80143B64();
        gosub_build_equipment_list(3);
        D_8016B900 = 1;
        g_gosub_visible_row_count = 6;
        g_gosub_row_height = 0x10;
        g_gosub_window_width = 0xE8;
        g_gosub_window_height = 0x64;
        D_8016B8E4 = 0;
        D_8017097C = 0;
        D_8016B8EC = 0;
        g_gosub_required_selection_count = 2;
        D_8016B8FC = 2;
        g_gosub_select_handler = (void*)gosub_validate_pending_pair_selection;
        g_gosub_finish_handler = (void*)gosub_publish_two_row_selection;
        g_gosub_dialog_handler = (void*)gosub_handle_combination_dialog;
        gosub_build_screen_10_elements();
        if (g_pad_ctx[0x29D6] >= 0x28)
        {
            func_80143B64();
            GOSUB_MSG(-4);
        }
        break;

    case 11:
        func_80143B64();
        gosub_build_packed_record_list();
        g_gosub_required_selection_count = 2;
        D_8016B8FC = 2;
        g_gosub_select_handler = (void*)gosub_commit_row_reorder;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        g_gosub_allow_duplicate_selection = 1;
        gosub_build_screen_11_elements();
        if (g_pad_ctx[0x29D6] == 0)
        {
            func_80143B64();
            GOSUB_MSG(-6);
        }
        break;

    case 12:
        func_80143B64();
        gosub_build_roster_list(0);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row_with_validation;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x32);
        }
        break;

    case 13:
        func_80143B64();
        gosub_build_roster_list(1);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row_with_validation;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x34);
        }
        break;

    case 14:
        func_80143B64();
        gosub_build_roster_list(2);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x36);
        }
        break;

    case 15:
        func_80143B64();
        gosub_build_screen_15_item_list();
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x3E);
        }
        break;

    case 16:
        func_80143B64();
        gosub_build_screen_16_item_list();
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(0);
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x3C);
        }
        break;

    case 17:
        func_80143B64();
        gosub_build_roster_list(0);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x32);
        }
        break;

    case 18:
        func_80143B64();
        gosub_build_roster_list(1);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_compact_list_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x34);
        }
        break;

    case 19:
        func_80143B64();
        gosub_build_screen_19_item_list();
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)gosub_publish_selection;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x4C);
        }
        break;
    }
}

/**
 * @brief Build the three elements of the gosub screen entered by arm 9.
 *
 * Each element is allocated by func_80143C04, given its draw handler, and
 * positioned. The first is centred horizontally against the current window
 * width in g_gosub_window_width and stacked vertically by g_gosub_row_height * g_gosub_visible_row_count; the
 * other two sit at a fixed x with only their attr byte differing.
 */
void gosub_build_screen_9_elements(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->draw_handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    p->attr.f.unk0_16 = 0x38;
    p->unk4_0 = 0;
    p->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEM_CODE(p, 0xE8);
    g_gosub_selection_count = 0;

    p = func_80143C04();
    p->draw_handler = (void*)&func_80145EA4;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->draw_handler = (void*)&func_801460D0;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0xB0;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the three elements of the gosub screen entered by arm 10.
 *
 * Same layout as gosub_build_screen_9_elements - the first element is centred against the
 * window width in g_gosub_window_width and stacked by g_gosub_row_height * g_gosub_visible_row_count, the other
 * two sit at a fixed x - but with a different set of draw handlers and attr
 * bytes.
 */
void gosub_build_screen_10_elements(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->draw_handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    p->attr.f.unk0_16 = 0x48;
    p->unk4_0 = 0;
    p->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEM_CODE(p, 0xE8);
    g_gosub_selection_count = 0;

    p = func_80143C04();
    p->draw_handler = (void*)&func_80145DF8;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0xB0;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->draw_handler = (void*)&D_801452F0;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x20;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Configure reserved element 0 and reset the dialog choice.
 *
 * Unlike gosub_build_screen_9_elements / gosub_build_screen_10_elements this one does not allocate: it reuses a
 * single statically allocated element, so it needs no frame. Its attr top byte
 * is cleared rather than set to one of the other observed attribute codes.
 */
void gosub_initialize_fixed_element(void)
{
    GosubElement* p;

    p = &g_gosub_elements[0];
    p->draw_handler = (void*)&func_80145F80;
    g_gosub_dialog_choice = 0;
    p->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x20;
    p->attr.f.unk0_16 = 0x70;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 0);
}

/**
 * @brief Build the three elements of the gosub screens entered by arms 2 to 5.
 *
 * Same layout as gosub_build_screen_9_elements and gosub_build_screen_10_elements, with its own handlers and
 * attr bytes; the third element also sits higher up the screen than in the
 * other two variants.
 */
void gosub_build_category_screen_elements(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->draw_handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    p->attr.f.unk0_16 = 0x28;
    p->unk4_0 = 0;
    p->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEM_CODE(p, 0xE8);
    g_gosub_selection_count = 0;

    p = func_80143C04();
    p->draw_handler = (void*)&func_801460D0;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0xB0;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->draw_handler = (void*)&D_80146418;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the elements of the gosub screens entered by arms 0, 1, 6-8,
 *        15, 16 and 19.
 *
 * Same layout as gosub_build_category_screen_elements, except the middle element is optional: callers
 * pass zero to get just the header and footer elements.
 *
 * @param include_middle Non-zero to include the middle element, zero to skip it.
 */
void gosub_build_list_screen_elements(s32 include_middle)
{
    GosubElement* p;

    p = func_80143C04();
    p->draw_handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    p->attr.f.unk0_16 = 0x28;
    p->unk4_0 = 0;
    p->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEM_CODE(p, 0xE8);
    g_gosub_selection_count = 0;

    if (include_middle != 0)
    {
        p = func_80143C04();
        p->draw_handler = (void*)&func_801460D0;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x1C;
        p->attr.f.unk0_16 = 0xB0;
        p->unk4_0 = 1;
        p->y = 0x14;
        SET_ELEM_CODE(p, 8);
    }

    p = func_80143C04();
    p->draw_handler = (void*)&D_80146418;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the three elements of the gosub screen entered by arm 11.
 *
 * Same layout as gosub_build_category_screen_elements, but the first element has its unk4_0 flag set
 * rather than cleared and takes attr code 0x20 instead of 0xE8.
 */
void gosub_build_screen_11_elements(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->draw_handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    p->attr.f.unk0_16 = 0x28;
    p->unk4_0 = 1;
    p->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEM_CODE(p, 0x20);
    g_gosub_selection_count = 0;

    p = func_80143C04();
    p->draw_handler = (void*)&func_801460D0;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0xB0;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->draw_handler = (void*)&D_80146418;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the two elements of the gosub screens entered by arms 12-14 and
 *        17-18.
 *
 * The shortest member of this family: no middle element, so only a header
 * centred against g_gosub_window_width and a footer at a fixed x.
 */
void gosub_build_compact_list_elements(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->draw_handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - g_gosub_window_width / 2;
    p->attr.f.unk0_16 = 0x30;
    p->unk4_0 = 1;
    p->y = g_gosub_row_height * g_gosub_visible_row_count + 4;
    SET_ELEM_CODE(p, 0x18);
    g_gosub_selection_count = 0;

    p = func_80143C04();
    p->draw_handler = (void*)&D_80146418;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 15.
 *
 * Walks the player's item slots at g_pad_ctx[0x25E0 + 0x60 .. 0x84] and emits
 * one g_gosub_rows row per non-empty slot, resolving the item's two archive
 * strings, recording the slot's count and index, and tagging the row kind. The
 * number of rows lands in g_gosub_row_count, and the screen's title message pointer in
 * g_gosub_title_text.
 */
void gosub_build_screen_15_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x85; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &g_gosub_rows[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->desc = ARCHIVE_ENTRY(D_8014F27C[12], D_8016B5AC[i]);
            e->value = *(g_pad_ctx + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x3A);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 19.
 *
 * Same row layout as gosub_build_screen_15_item_list, over the adjacent slot range
 * g_pad_ctx[0x25E0 + 0x60 .. 0x8F]. Both archive strings for a row come from
 * blocks indexed directly by the slot number, so no D_8016B5AC indirection is
 * needed for the description. The row count lands in g_gosub_row_count and the
 * screen's title message pointer in g_gosub_title_text.
 */
void gosub_build_screen_19_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x90; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &g_gosub_rows[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->desc = ARCHIVE_ENTRY(D_8014F27C[2], i);
            e->value = *(g_pad_ctx + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x4A);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 16.
 *
 * Same row layout as gosub_build_screen_15_item_list over slots g_pad_ctx[0x25E0 + 0x40 .. 0x4F],
 * but these rows carry no description string: only the name, the slot's count,
 * its index, and the row kind are written. The row count lands in g_gosub_row_count
 * and the screen's title message pointer in g_gosub_title_text.
 */
void gosub_build_screen_16_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0x50; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &g_gosub_rows[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->value = *(g_pad_ctx + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x38);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 1.
 *
 * Identical row layout and archive blocks to gosub_build_screen_19_item_list, over the widest
 * slot range in the family: g_pad_ctx[0x25E0 + 0x40 .. 0xFE]. The row count
 * lands in g_gosub_row_count and the screen's title message pointer in g_gosub_title_text.
 */
void gosub_build_screen_1_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0xFF; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &g_gosub_rows[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->desc = ARCHIVE_ENTRY(D_8014F27C[2], i);
            e->value = *(g_pad_ctx + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x14);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 0.
 *
 * Identical row layout and archive blocks to gosub_build_screen_19_item_list, over the slot
 * range g_pad_ctx[0x25E0 + 0x00 .. 0x3F] -- the block immediately below the
 * one gosub_build_screen_1_item_list walks. The row count lands in g_gosub_row_count and the screen's
 * title message pointer in g_gosub_title_text.
 */
void gosub_build_screen_0_item_list(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0; i < 0x40; i++)
    {
        if (*(g_pad_ctx + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &g_gosub_rows[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->desc = ARCHIVE_ENTRY(D_8014F27C[2], i);
            e->value = *(g_pad_ctx + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    g_gosub_row_count = count;
    g_gosub_visible_row_count = 8;
    g_gosub_row_height = 0x10;
    g_gosub_window_width = 0xE8;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x12);
}

/**
 * @brief Build the packed-record list for the gosub screen entered by arm 11.
 *
 * Unlike the slot builders above, every record produces a row: the count comes
 * from g_pad_ctx[0x29D6] and each record is a packed word at
 * g_pad_ctx[0x29DC + i * 4]. Three fields are unpacked out of it into the
 * row's byte fields, and the row's name is composed into its own 0x50-byte
 * scratch buffer in g_gosub_text_buffers -- the archive string, then optionally a
 * separator and the decimal form of the unkC field appended after it.
 *
 * @note Bit 2 of the flag word is cleared only for records that are both unflagged at
 *       bit 16 and have both low bits set; every other record sets it.
 */
void gosub_build_packed_record_list(void)
{
    s32 i;
    u8* buf;
    u8 tmp[32];

    for (i = 0; i < *(g_pad_ctx + 0x29D6); i++)
    {
        g_gosub_rows[i].unkE = *(g_pad_ctx + (i << 2) + 0x29DC) >> 2;
        g_gosub_rows[i].unkC = (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 8) & 0xF;
        buf = g_gosub_text_buffers + i * 0x50;
        func_80146538(buf, ARCHIVE_ENTRY(D_8014F294[0], g_gosub_rows[i].unkE));
        if (g_gosub_rows[i].unkC != 0)
        {
            func_80146468(buf, D_800EC3DA - 0x16 + D_800EC3DA[0] + (D_800EC3DA[1] << 8));
            func_800A8B90(tmp, g_gosub_rows[i].unkC, 1);
            func_80146468(buf, tmp);
        }
        g_gosub_rows[i].name = buf;
        g_gosub_rows[i].desc = ARCHIVE_ENTRY(D_8014F27C[7], g_gosub_rows[i].unkE);
        g_gosub_rows[i].value = -2;
        g_gosub_rows[i].unkD = (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 12) & 0xF;
        if (((*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 16) & 1) != 0 || (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) & 3) != 3)
        {
            g_gosub_rows[i].flags.word |= 4;
        }
        else
        {
            g_gosub_rows[i].flags.word &= ~4;
        }
        g_gosub_rows[i].index = i;
        g_gosub_rows[i].kind = 4;
    }
    g_gosub_row_count = *(g_pad_ctx + 0x29D6);
    g_gosub_visible_row_count = 4;
    g_gosub_row_height = 0x20;
    g_gosub_window_width = 0x120;
    g_gosub_window_height = 0x84;
    g_gosub_title_text = GOSUB_MSG_PTR(0x18);
}

/**
 * @brief Build a roster list for gosub screens 12-14 and 17-18.
 *
 * Two record sources feed the same g_gosub_rows row array, and @p mode selects
 * which of them contribute: mode 1 suppresses the first block, mode 2
 * suppresses the second, and any other value emits both back to back.
 *
 * The first block walks three record-index bytes at g_pad_ctx[0x29D8]. A byte
 * below 3 selects a 332-byte record at g_pad_ctx[0x2B0C], whose name string,
 * two packed bytes and five u16s are copied into the row; the row is marked
 * current when the record index matches the selection byte at
 * g_pad_ctx[0x29D7]. In mode 0 the row's index keeps only bit 7 of the record
 * index instead of the index itself.
 *
 * The second block walks five 0x60-byte slots at g_pad_ctx[0x2EF4], skipping
 * any whose first byte (the name string's first character) is zero. Each row
 * takes its name straight from the slot and its unkC/unkD from the slot's byte
 * fields, unless bit 31 of the slot word at +0x44 is set: those rows bias unkC
 * by 0x48 and derive unkD from three ranges of the u16 at +0x42. The row is
 * marked current when the slot index matches the selection word at
 * g_pad_ctx[0x2EF0].
 *
 * @param mode Which blocks to emit: 1 = second only, 2 = first only, otherwise
 *             both. Also picks the screen's title message.
 * @note @c tmp is reserved but never written; the sibling builders use their
 *       scratch buffer to compose names, and this one has nothing to compose.
 */
void gosub_build_roster_list(s32 mode)
{
    s32 large_ref_index;
    s32 stat_index;
    s32 row_count;
    s32 record_index;
    s32 record_offset;
    s32 slot;
    s32 record_offset_reload;
    u8 unused_name_buf[32];

    row_count = 0;
    if (mode != 1)
    {
        for (large_ref_index = 0; large_ref_index < 3; large_ref_index++)
        {
            record_index = *(g_pad_ctx + large_ref_index + 0x29D8);
            if (record_index < 3)
            {
                if (mode == 0)
                {
                    g_gosub_rows[row_count].index = record_index & 0x80;
                }
                else
                {
                    g_gosub_rows[row_count].index = record_index;
                }
                g_gosub_rows[row_count].value = -3;
                g_gosub_rows[row_count].flags.f.flag2 = 0;
                if (*(s8*)(g_pad_ctx + 0x29D7) == record_index)
                {
                    g_gosub_rows[row_count].unkE = 1;
                }
                else
                {
                    g_gosub_rows[row_count].unkE = 0;
                }
                g_gosub_rows[row_count].flags.f.flag0 = 0;
                g_gosub_rows[row_count].flags.f.flag1 = 0;
                g_gosub_rows[row_count].kind = 4;
                record_offset = record_index * 332;
                g_gosub_rows[row_count].name = g_pad_ctx + 0x2B0C + record_offset;
                g_gosub_rows[row_count].unkC = *(g_pad_ctx + record_offset + 0x2B50) & 0xF;
                g_gosub_rows[row_count].unkD = *(g_pad_ctx + record_offset + 0x2B54);
                g_gosub_rows[row_count].unk10 = *(u16*)(g_pad_ctx + record_offset + 0x2B24);
                for (stat_index = 0; stat_index < 4; stat_index++)
                {
                    g_gosub_rows[row_count].unk12[stat_index] = *(u16*)(g_pad_ctx + record_offset + 0x2B26 + stat_index * 2);
                }
                record_offset_reload = record_index * 332;
                g_gosub_rows[row_count].unk1A = *(u16*)(g_pad_ctx + record_offset_reload + 0x2B22);
                row_count++;
            }
        }
    }
    if (mode != 2)
    {
        for (slot = 0; slot < 5; slot++)
        {
            record_offset = slot * 0x60;
            if (*(g_pad_ctx + record_offset + 0x2EF4) != 0)
            {
                g_gosub_rows[row_count].index = slot;
                g_gosub_rows[row_count].value = -3;
                g_gosub_rows[row_count].flags.f.flag2 = 1;
                if (*(s32*)(g_pad_ctx + 0x2EF0) == slot)
                {
                    g_gosub_rows[row_count].unkE = 1;
                }
                else
                {
                    g_gosub_rows[row_count].unkE = 0;
                }
                g_gosub_rows[row_count].kind = 4;
                g_gosub_rows[row_count].name = g_pad_ctx + 0x2EF4 + slot * 0x60;
                g_gosub_rows[row_count].unkC = *(g_pad_ctx + record_offset + 0x2F09);
                g_gosub_rows[row_count].unk10 = *(u16*)(g_pad_ctx + record_offset + 0x2F12);
                g_gosub_rows[row_count].flags.f.flag0 = *(u32*)(g_pad_ctx + record_offset + 0x2F38) >> 31;
                g_gosub_rows[row_count].flags.f.flag1 = (*(u32*)(g_pad_ctx + record_offset + 0x2F38) >> 30) & 1;
                g_gosub_rows[row_count].unkD = *(g_pad_ctx + record_offset + 0x2F0C);
                if (g_gosub_rows[row_count].flags.half & 1)
                {
                    g_gosub_rows[row_count].unkC = *(g_pad_ctx + record_offset + 0x2F0A) + 0x48;
                    if (*(u16*)(g_pad_ctx + record_offset + 0x2F36) < 6)
                    {
                        g_gosub_rows[row_count].unkD = 0;
                    }
                    else if (*(u16*)(g_pad_ctx + record_offset + 0x2F36) < 0x1F)
                    {
                        g_gosub_rows[row_count].unkD = 1;
                    }
                    else
                    {
                        g_gosub_rows[row_count].unkD = 2;
                    }
                }
                for (stat_index = 0; stat_index < 4; stat_index++)
                {
                    g_gosub_rows[row_count].unk12[stat_index] = *(u16*)(g_pad_ctx + record_offset + 0x2F14 + stat_index * 2);
                }
                record_offset_reload = slot * 0x60;
                g_gosub_rows[row_count].unk1A = *(u16*)(g_pad_ctx + record_offset_reload + 0x2F10);
                row_count++;
            }
        }
    }
    g_gosub_row_count = row_count;
    g_gosub_visible_row_count = 3;
    g_gosub_row_height = 0x30;
    g_gosub_window_width = 0x120;
    g_gosub_window_height = 0x94;
    g_gosub_title_text = GOSUB_MSG_PTR(mode * 2 + 0x2C);
}

/**
 * @brief Confirm the currently highlighted row of the gosub list.
 *
 * Rows carrying either of the two low flag bits cannot be picked: each shows
 * its own refusal message, clears g_gosub_selection_count to close the picker and raises
 * D_8016B95C, then reports failure. Otherwise, and only while g_gosub_selection_count is
 * still set, the row is appended to the running selection: its index goes to
 * g_gosub_result_values and the row number itself to g_gosub_result_rows, both keyed by the shared
 * write cursor g_gosub_result_count.
 *
 * @return 0 if the row was rejected by a flag, 1 otherwise. Note that 1 is also
 *         returned when g_gosub_selection_count is clear and nothing was appended.
 */
s32 gosub_select_row_with_validation(void)
{
    s32 row;
    GosubListEntry* list;
    GosubListEntry* e;

    list = g_gosub_rows;
    row = g_gosub_cursor_row;
    e = &list[row];
    if (e->flags.half & 1)
    {
        GOSUB_MSG(0x42);
        g_gosub_selection_count = 0;
        D_8016B95C = 1;
        return 0;
    }
    if (e->flags.f.flag1)
    {
        GOSUB_MSG(0x50);
        g_gosub_selection_count = 0;
        D_8016B95C = 1;
        return 0;
    }
    if (g_gosub_selection_count != 0)
    {
        g_gosub_result_values[g_gosub_result_count] = e->index;
        g_gosub_result_rows[g_gosub_result_count] = row;
        g_gosub_result_count++;
    }
    return 1;
}

/**
 * @brief Append the highlighted row to the selection, with no flag checks.
 *
 * The unconditional counterpart to gosub_select_row_with_validation: the same append -- row index
 * to g_gosub_result_values, row number to g_gosub_result_rows, both keyed by the shared write
 * cursor g_gosub_result_count -- but no refusal messages, so any row may be picked. It is
 * the handler g_gosub_select_handler is pointed at for the plain list screens.
 *
 * @return Always 1. Nothing is appended while g_gosub_selection_count is clear.
 */
s32 gosub_select_row(void)
{
    s32 n;

    if (g_gosub_selection_count != 0)
    {
        n = g_gosub_result_count;
        g_gosub_result_count = n + 1;
        g_gosub_result_values[n] = g_gosub_rows[g_gosub_cursor_row].index;
        g_gosub_result_rows[n] = g_gosub_cursor_row;
    }
    return 1;
}

/**
 * @brief Validate a pending two-row selection before publishing it.
 *
 * Runs only while at least one row is selected. If D_8017097C is set, the work
 * is delegated to gosub_publish_two_row_selection and its result is passed
 * straight back. Otherwise a just-completed pair is rolled back to one
 * selection and the caller is told nothing happened.
 *
 * @return gosub_publish_two_row_selection's result while D_8017097C is set, 0 on every other path.
 */
s32 gosub_validate_pending_pair_selection(void)
{
    if (g_gosub_selection_count != 0)
    {
        if (D_8017097C == 0)
        {
            if (g_gosub_selection_count == 2)
            {
                g_gosub_selection_count = 1;
            }
            return 0;
        }
        return gosub_publish_two_row_selection();
    }
    return 0;
}

/**
 * @brief Commit a pending row move by swapping the two marked rows.
 *
 * Runs only when two rows are selected. g_gosub_selected_rows holds the pair being reordered --
 * [0] is the row the move started on and [1] is where it was dropped. If they
 * differ, three things are exchanged: the rows' 4-byte records in the
 * g_pad_ctx[0x29DC] table (keyed by each row's index), the g_gosub_rows rows
 * themselves, and finally the index fields, which must stay with the slot
 * rather than travel with the row. The picker is then closed.
 *
 * Dropping a row onto itself is not a move: func_801458A4 is rung instead and
 * the picker falls back to state 1.
 *
 * @return Always 0.
 */
s32 gosub_commit_row_reorder(void)
{
    GosubListEntry entry_tmp;
    u32 rec_tmp;
    s32 tmp;

    if (g_gosub_selection_count == 0)
    {
        return 0;
    }
    if (g_gosub_selection_count != 2)
    {
        return 0;
    }
    if (g_gosub_selected_rows[0] != g_gosub_selected_rows[1])
    {
        func_80146AA8(&rec_tmp, g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[0]].index * 4 + 0x29DC));
        func_80146AA8(g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[0]].index * 4 + 0x29DC),
                      g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[1]].index * 4 + 0x29DC));
        func_80146AA8(g_pad_ctx + (g_gosub_rows[g_gosub_selected_rows[1]].index * 4 + 0x29DC), &rec_tmp);
        func_80146AD0(&entry_tmp, &g_gosub_rows[g_gosub_selected_rows[0]]);
        func_80146AD0(&g_gosub_rows[g_gosub_selected_rows[0]], &g_gosub_rows[g_gosub_selected_rows[1]]);
        func_80146AD0(&g_gosub_rows[g_gosub_selected_rows[1]], &entry_tmp);
        tmp = g_gosub_rows[g_gosub_selected_rows[0]].index;
        g_gosub_rows[g_gosub_selected_rows[0]].index = g_gosub_rows[g_gosub_selected_rows[1]].index;
        g_gosub_rows[g_gosub_selected_rows[1]].index = tmp;
        g_gosub_selection_count = 0;
    }
    else
    {
        func_801458A4();
        g_gosub_selection_count = 1;
    }
    return 0;
}

/**
 * @brief Recompute every row's kind for the current pick, and report when the
 *        selection is complete.
 *
 * Runs in four passes. The first resets all g_gosub_row_count rows to kind 4. The
 * second walks the g_gosub_selection_count entries already picked in g_gosub_selected_rows and tallies
 * them by their rows' top nibble: unmarked rows into @c open_count, marked rows
 * into @c marked_count. The remaining two passes promote candidate rows to kind
 * 5 -- unmarked ones once anything unmarked has been picked, marked ones once
 * exactly three marked rows have been -- with gosub_is_row_unselected deciding whether an
 * individual row qualifies.
 *
 * @return 1 when both conditions hold, in which case gosub_publish_selection is called to
 *         announce the completed selection; 0 otherwise.
 */
s32 gosub_update_group_selection(void)
{
    s32 i;
    s32 open_count;
    s32 marked_count;

    for (i = 0; i < g_gosub_row_count; i++)
    {
        g_gosub_rows[i].kind = 4;
    }
    open_count = 0;
    marked_count = 0;
    for (i = 0; i < g_gosub_selection_count; i++)
    {
        if (g_gosub_rows[g_gosub_selected_rows[i]].unkC_28 == 0)
        {
            open_count++;
        }
        else
        {
            marked_count++;
        }
    }
    if (open_count != 0)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].unkC_28 == 0 && gosub_is_row_unselected(i) != 0)
            {
                g_gosub_rows[i].kind = 5;
            }
        }
    }
    if (marked_count == 3)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].unkC_28 != 0 && gosub_is_row_unselected(i) != 0)
            {
                g_gosub_rows[i].kind = 5;
            }
        }
    }
    if (open_count != 0)
    {
        if (marked_count == 3)
        {
            gosub_publish_selection();
            return 1;
        }
        return 0;
    }
    return 0;
}

/**
 * @brief Publish the picked rows' indices as the screen's result.
 *
 * Only acts when two rows are selected. The number of entries picked so far
 * (g_gosub_selection_count) becomes the result count in g_gosub_result_count, and each entry in
 * g_gosub_selected_rows is resolved through g_gosub_rows so that g_gosub_result_values ends up holding
 * the rows' index fields rather than their row numbers.
 *
 * @return 1 if the result was published, 0 if the picker was not in state 2.
 */
s32 gosub_publish_two_row_selection(void)
{
    s32 i;

    if (g_gosub_selection_count == 2)
    {
        g_gosub_result_count = g_gosub_selection_count;
        for (i = 0; i < g_gosub_selection_count; i++)
        {
            g_gosub_result_values[i] = g_gosub_rows[g_gosub_selected_rows[i]].index;
        }
        return 1;
    }
    return 0;
}

void gosub_build_equipment_list(u32 item_kind); /* extern */
s32 gosub_handle_input(s32 unused);             /* extern */
void gosub_scroll_to_cursor(void);              /* extern */
s32 gosub_toggle_cursor_selection(void);        /* extern */
s32 gosub_advance_screen_sequence(void);        /* extern */
s32 gosub_are_elements_idle(void);              /* extern */
s32 func_80145F80();                            /* extern */
void func_80143B64();                           /* extern */
void func_80143BB0();                           /* extern */
void func_80143C58();                           /* extern */
void func_80145CEC();                           /* extern */
void func_80146468();                           /* extern */
void func_80146538();                           /* extern */

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
    volatile u16 unkE;
} StructS0;

/**
 * @brief Same layout as StructS0, but with a plain (non-volatile) @c unkE.
 *
 * func_801448EC writes each highlight tile's fields once and never reads them
 * back, so it does not need StructS0's volatile @c unkE (which exists for the
 * scrollbar thumb's read-modify-write clamp at func_80143C58). Sharing the
 * volatile field there costs 1.80% and one extra instruction: the volatile
 * store cannot be scheduled among the neighbouring field stores.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s16 unk8;
    s16 unkA;
    s16 unkC;
    u16 unkE;
} GosubTile;

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
#define SET_PRIM_ADDR(p, a) (((GosubTag*)(p))->addr = (u_long)(a))
/** @brief libgpu getaddr(): read a packet's 24-bit ordering-table link. */
#define GET_PRIM_ADDR(p) ((u_long)((GosubTag*)(p))->addr)
/** @brief libgpu addPrim(): splice packet @p p in at ordering-table tag @p ot. */
#define ADD_PRIM(ot, p) (SET_PRIM_ADDR(p, GET_PRIM_ADDR(ot)), SET_PRIM_ADDR(ot, p))
/**
 * @brief ADD_PRIM with the incoming link masked explicitly.
 * @note The mask is redundant against the 24-bit bitfield, but spelling it out
 *       changes register allocation. It is worth 7 rows in func_801448EC and
 *       costs 8 rows in func_801450D8, so the two spellings are both required
 *       to match; do not collapse them into one macro.
 */
#define SET_PRIM_ADDR_MASK(p, a) (((GosubTag*)(p))->addr = ((u_long)(a) & 0xFFFFFF))
#define ADD_PRIM_MASKED(ot, p) (SET_PRIM_ADDR_MASK(p, GET_PRIM_ADDR(ot)), SET_PRIM_ADDR(ot, p))

s32 func_8001A5D4(s32, void*);                /* extern */
s32 func_8001C56C(void*, s32, s32, s32, s32); /* extern */
void* func_801443E4();                        /* extern */
StructS0* func_80144544();                    /* extern */
GosubLine* func_80144764();                   /* extern */
StructS0* func_80146E30();                    /* extern */
GosubTile* func_801448EC();                   /* extern */

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
s32 func_80146178(s32 prim, s32* ot, s32 x_off, s32 y_off);                        /* extern */
s32 func_801466B4(s32 prim, s32* ot, s32 x, s32 y, s32 w, s32 h);                  /* extern */

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
                              ARCHIVE_ENTRY(D_8014F280[0], GOSUB_EQUIPMENT_AT_SHIFTED_INDEX(item_index)->attributes.half.material & 0x3F));
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
GosubElement* func_80143C04(void)
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
 * @see decomp.me (100%) https://decomp.me/scratch/t79hi
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
    u16 temp_a0;
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
                    {
                        u32 field;
                        u32 high;
                        field = (temp_a3 >> 7) & 0x1FF;
                        high = temp_a3 >> 24;
                        var_s0 = func_801443E4(var_a0, var_s4, (field + (((temp_t0 & 1) << 8) | high)) - 0x10, (*((u8*)var_s5 + 2)) + temp_t1, 0);
                    }
                }
                if (g_gosub_scroll_y != 0)
                {
                    {
                        u32 field;
                        u32 high;
                        temp_t0_2 = *var_s5;
                        field = (temp_t0_2 >> 7) & 0x1FF;
                        high = temp_t0_2 >> 24;
                        var_s0 = func_801443E4(var_s0, var_s4, (field + (((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | high)) - 0x10, (*((u8*)var_s5 + 2)), 1);
                    }
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
                    {
                        s32 clamp_h;
                        clamp_h = (s16)var_s0->unkE;
                        temp_v1_2 = *(u32*)((u8*)var_s5 + 4);
                        temp_a0 = ((u32)temp_v1_2 >> 1) & 0xFF;
                        if (clamp_h >= (s32)temp_a0 - 2)
                        {
                            var_s0->unkE = temp_a0;
                        }
                    }
                    var_s0->unk8 = 1;
                    var_s0->unkA = (s16)((s32)(((*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF) * (g_gosub_scroll_y / g_gosub_row_height)) / g_gosub_row_count);
                    var_s0->unk0 = (var_s0->unk0 & var_s7) | (var_s4->unk0 & var_s6);

                    temp_v1 = (s32)var_s0 & var_s6;
                    var_s0 = (StructS0*)((u8*)var_s0 + 0x10);
                    var_s4->unk0 = (s32)((var_s4->unk0 & var_s7) | temp_v1);
                }
                {
                    u32 field;
                    u32 high;
                    temp_t0_3 = *var_s5;
                    field = (temp_t0_3 >> 7) & 0x1FF;
                    high = temp_t0_3 >> 24;
                    var_s0 = func_80144544(var_s0, var_s4, field + (((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | high) + 3, (*((u8*)var_s5 + 2)), 0xA,
                                           (*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF, arg0->unk40B2);
                }
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

                var_s0 = (*(Unk6Func*)((u8*)var_s5 + 8))(var_s4, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_3 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s5;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 =
                        func_80144544(var_s0, var_s4, field + (s32)((((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                      (*((u8*)var_s5 + 2)) + ((s32)((*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF) - temp_s2) / 2, temp_s1, temp_s2, arg0->unk40B2);
                }
                temp_v0_3 = *var_s5;
                temp_a0_4 = temp_v0_3 & ~0x78;
                temp_a0_4 |= (((((temp_v0_3 >> 3) & 0xF) + 1) & 0xF) * 8);
                *var_s5 = temp_a0_4;
                if (((temp_a0_4 >> 3) & 0xF) == 8)
                {
                    *var_s5 = (temp_a0_4 & ~7) | 2;
                }
                break;

            case 2:
                var_s0 = (*(Unk6Func*)((u8*)var_s5 + 8))(var_s4, var_s0, 0, 0);
                {
                    u32 case_word;
                    u32 high;
                    case_word = *var_s5;
                    high = case_word >> 24;
                    var_s0 = func_80144544(var_s0, var_s4, (case_word >> 7) & 0x1FF, (*((u8*)var_s5 + 2)), ((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | high,
                                           (*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF, arg0->unk40B2);
                }
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

                var_s0 = (*(Unk6Func*)((u8*)var_s5 + 8))(var_s4, var_s0, (s32)(temp_a2 - temp_s1) / 2, temp_a3_6 / 2);
                {
                    u32 post_word;
                    u32 field;
                    u32 high;
                    post_word = *var_s5;
                    field = (post_word >> 7) & 0x1FF;
                    high = post_word >> 24;
                    var_s0 =
                        func_80144544(var_s0, var_s4, field + (s32)((((*(u32*)((u8*)var_s5 + 4) & 1) << 8) | high) - temp_s1) / 2,
                                      (*((u8*)var_s5 + 2)) + ((s32)((*(u32*)((u8*)var_s5 + 4) >> 1) & 0xFF) - temp_s2) / 2, temp_s1, temp_s2, arg0->unk40B2);
                }
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
 * @see decomp.me (100%)
 */
void *func_801443E4(GosubPrim *prim, s32 *ot, s32 x, s32 y, s32 flag)
{
    s32 color;
    s16 tmp_x;
    s32 tmp_y;
    u32 i;
    u32 addr_mask;
    u8 *p;
    GosubPrim *prim2;

    prim->len = 6;
    prim->code = 0x4C;
    prim->mask = 0x55555555;
    if (D_800F22AC & 0x10)
    {
        color = D_800F22AC & 0xF;
    }
    else
    {
        color = (~D_800F22AC) & 0xF;
    }
    tmp_y = color * 4;
    color = tmp_y + 0x70;
    prim->b = color;
    prim->g = color;
    prim->r = color;
    if (flag != 0)
    {
        do {
            prim->y3 = y - 8;
            prim->y0 = y - 8;
        } while (0);
        tmp_x = x - 6;
        tmp_y = y + 4;
    }
    else
    {
        do {
            prim->y3 = y + 8;
            prim->y0 = y + 8;
        } while (0);
        tmp_x = x - 6;
        tmp_y = y - 4;
    }
    do
    {
        prim->x1 = tmp_x;
        prim->x3 = x;
        prim->x0 = x;
        prim->y1 = tmp_y;
        prim->x2 = x + 6;
        prim->y2 = tmp_y;
    } while (0);

    addr_mask = 0xFFFFFF;
    p = (u8 *)prim;
    prim2 = (GosubPrim *)(p + 0x1C);
    prim = prim2;
    *(u32 *)p = (*(u32 *)p & 0xFF000000) | (*ot & addr_mask);
    i = 0;
    *ot = (*ot & 0xFF000000) | ((u32)p & addr_mask);
    do
    {
        i += 1;
        *(u8 *)prim = *p;
        p += 1;
        prim = (GosubPrim *)((u8 *)prim + 1);
    } while (i < 0x14U);

    prim2->len = 4;
    *(u32 *)&prim2->r = 0;
    prim2->code = 0x20;
    *(u32 *)prim2 = (*(u32 *)prim2 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((u32)prim2 & 0xFFFFFF);
    return (u8 *)prim2 + 0x14;
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
 * @note The `do { temp_a2 = (s32)var_s0; } while (0)` copy block is required to
 *       match: it keeps the tile cursor in its own live range so the packet
 *       cursor lands in a2 rather than being coalesced onto the call return.
 *
 * @see decomp.me (100%)
 */
StructS0* func_80144544(StructS0* prim, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 flag)
{
    StructS0* var_s0;
    StructS0* var_a0;
    StructS0* buf;
    s32 sp20[24];
    s32 temp_a2;

    buf = prim;
    if (flag != 0)
    {
        temp_a2 = y + 0xF2;
        func_8001C56C(sp20, x + 2, temp_a2, w - 4, h - 4);
    }
    else
    {
        temp_a2 = y + 0xA;
        func_8001C56C(sp20, x + 2, temp_a2, w - 4, h - 4);
    }
    func_8001A5D4((s32)buf, sp20);

    buf->unk0 = (buf->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((s32)buf & 0xFFFFFF);

    buf = (StructS0*)((u8*)buf + 0x40);
    var_s0 = func_80146E30(buf, ot, x, y, w, h);
    var_s0 = (StructS0*)func_80144764((GosubLine*)var_s0, ot, x, y, w, h, 0xFFFFFF);
    var_s0 = (StructS0*)func_80144764((GosubLine*)var_s0, ot, x + 1, y + 1, w - 2, h - 2, 0);
    var_s0 = (StructS0*)func_80144764((GosubLine*)var_s0, ot, x - 1, y - 1, w + 2, h + 2, 0);

    do { temp_a2 = (s32)var_s0; } while (0);
    ((StructS0*)temp_a2)->unk4 = 0xC0C0C0;
    ((u8*)temp_a2)[3] = 3;
    ((u8*)temp_a2)[7] = 0x62;
    ((StructS0*)temp_a2)->unk8 = x;
    ((StructS0*)temp_a2)->unkA = y;
    ((StructS0*)temp_a2)->unkC = w;
    ((StructS0*)temp_a2)->unkE = h;
    ((StructS0*)temp_a2)->unk0 = (((StructS0*)temp_a2)->unk0 & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (temp_a2 & 0xFFFFFF);

    var_a0 = (StructS0*)(temp_a2 + 0x10);
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
GosubLine* func_80144764(GosubLine* p, s32* ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    *(u32*)&p->r0 = color;
    ((GosubTag*)p)->len = 3;
    p->code = 0x40;
    p->x0 = x + 4;
    p->y0 = y;
    p->x1 = (x + w) - 4;
    p->y1 = y;
    ADD_PRIM(ot, p);
    p++;

    *(u32*)&p->r0 = color;
    ((GosubTag*)p)->len = 3;
    p->code = 0x40;
    p->x0 = x + w;
    p->y0 = y + 4;
    p->x1 = x + w;
    p->y1 = (y + h) - 4;
    ADD_PRIM(ot, p);
    p++;

    *(u32*)&p->r0 = color;
    ((GosubTag*)p)->len = 3;
    p->code = 0x40;
    p->x0 = (x + w) - 4;
    p->y0 = y + h;
    p->x1 = x + 4;
    p->y1 = y + h;
    ADD_PRIM(ot, p);
    p++;

    *(u32*)&p->r0 = color;
    ((GosubTag*)p)->len = 3;
    p->code = 0x40;
    p->x0 = x;
    p->y0 = y + 4;
    p->x1 = x;
    p->y1 = (y + h) - 4;
    ADD_PRIM(ot, p);
    return p + 1;
}

/** @brief Message archive header at &D_8014F29C + D_8014F29C: u16 h[] offsets, then u16 t[]. */
typedef struct
{
    u16 h[0x22];
    u16 t[16];
} MsgHdr;

/* The `- -` is required to match: it keeps gcc from folding the base and the
   offset into one %hi/%lo relocation at the MSG_HI call sites. */
#define MSG_HDR ((MsgHdr*)((u8*)&D_8014F29C - -D_8014F29C))
#define MSG_HI(off) ((void*)(D_8014F29C + (MSG_HDR->h[(off) >> 1] + base)))
#define MSG_LO(off) ((void*)(D_8014F29C + (*(u16*)(base + D_8014F29C + (off)) + base)))

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
 * @param ot       Ordering-table tag every packet is linked into.
 * @param arg_prim Packet cursor; copied into the local @c prim, which is what
 *                 the body advances.
 * @param x_off    Horizontal offset subtracted from every column position.
 * @param y_off    Vertical scroll offset subtracted from every row position.
 * @return Packet cursor just past the last highlight tile.
 * @see decomp.me (100%)
 */
GosubTile* func_801448EC(s32* ot, s32 arg_prim, s32 x_off, s32 y_off)
{
    s32 prim;
    s32 drawn_count;
    Vec2s* pos_p;
    s32 row_offset;
    Vec2s pos;
    s32 row;
    s32 y;
    s32 line_y;
    s32 y_top;
    s32 sel_mul;
    s32 msg_off;
    u8* msg_p;
    s32 y_top2;
    s32 y_top3;
    s32 line_y2;
    s32 line_y3;
    s32 label_x;
    s32 x_pad;
    s32 status_pad;
    s32* table;
    s32 base;
    s32 blk;
    s32 blk2;
    u8* d2ptr;
    GosubTile* tile;
    GosubTile* mark;
    s32* cursor_p;
    s32* height_p;
    s32* scroll_p;
    u32 cursor_color;
    u32 addr_mask;

    prim = arg_prim;
    row = 0;
    drawn_count = 0;
    if (g_gosub_row_count > 0)
    {
        label_x = 0x30 - x_off;
        pos_p = &pos;
        row_offset = 0;
        d2ptr = (u8*)&D_8014F2C4;
        do
        {
            table = &D_8014F29C;
            base = (s32)table - 0x20;
            if (g_gosub_rows[row].value == -3)
            {
                y = ((row * 0x30) - y_off) - g_gosub_scroll_y;
                if (y >= -0x2F && y < g_gosub_window_height)
                {
                    prim =
                        func_800A88A0(func_801450D8(prim, ot, row, -x_off, y, drawn_count), ot, g_gosub_rows[row].name, g_gosub_rows[row].kind, label_x, y, 0);
                    if (g_gosub_rows[row].flags.f.flag2)
                    {
                        if ((g_gosub_rows[row].flags.half & 1) == 0)
                        {
                            line_y = y + 0x10;
                            prim = func_800A88A0(prim, ot, MSG_HI(0x24), g_gosub_rows[row].kind, label_x, line_y, 0);
                            pos.x = 0x54 - x_off;
                            pos.y = line_y;
                            prim = func_800A8A78(ot, prim, g_gosub_rows[row].unkD, g_gosub_rows[row].kind, pos_p, 0);
                            blk2 = *(s32*)(base + 0x24);
                            prim = func_800A88A0(prim, ot, (void*)(blk2 + (*(u16*)((blk2 + g_gosub_rows[row].unkC * 2) + base) + base)), g_gosub_rows[row].kind,
                                                 0x84 - x_off, line_y, 0);
                        }
                        else
                        {
                            msg_off = *(u16*)((u8*)&D_8014F29C + D_8014F29C + g_gosub_rows[row].unkD * 2 + 0x44);
                            prim = func_800A88A0(prim, ot, (void*)(D_8014F29C + (msg_off + base)), g_gosub_rows[row].kind, label_x, y + 0x10, 0);
                        }
                    }
                    else
                    {
                        blk = D_8014F2A4;
                        prim = func_800A88A0(prim, ot, (void*)(blk + (*(u16*)((blk + g_gosub_rows[row].unkC * 2) + base) + base)), g_gosub_rows[row].kind,
                                             label_x, y + 0x10, 0);
                    }
                    line_y2 = y + 0x20;
                    prim = func_800A88A0(prim, ot, MSG_HI(0x26), g_gosub_rows[row].kind, label_x, line_y2, 0);
                    pos.x = 0x48 - x_off;
                    pos.y = line_y2;
                    prim = func_800A8A78(ot, prim, g_gosub_rows[row].unk1A, g_gosub_rows[row].kind, pos_p, 0);
                    msg_off = D_8014F29C - -(*(u16*)((s32)D_8014F29C - -(s32)d2ptr) + base);
                    prim = func_800A88A0(prim, ot, (void*)msg_off, g_gosub_rows[row].kind, 0x64 - x_off, line_y2, 0);
                    pos.x = 0xB0 - x_off;
                    pos.y = line_y2;
                    prim = func_800A8A78(ot, prim, g_gosub_rows[row].unk10, g_gosub_rows[row].kind, pos_p, 0);
                    if (g_gosub_rows[row].unkE != 0)
                    {
                        s32 pad1;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x4A), g_gosub_rows[row].kind, g_gosub_window_width - (pad1 = x_off, pad1 += 0xC), line_y2, 1);
                    }
                    else if (g_gosub_rows[row].flags.half & 1)
                    {
                        s32 pad2;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x60), g_gosub_rows[row].kind, g_gosub_window_width - (pad2 = x_off, pad2 += 0xC), line_y2, 1);
                    }
                    else if (g_gosub_rows[row].flags.f.flag1)
                    {
                        s32 pad3;
                        prim = func_800A88A0(prim, ot, MSG_LO(0x6E), g_gosub_rows[row].kind, g_gosub_window_width - (pad3 = x_off, pad3 += 0xC), line_y2, 1);
                    }
                    drawn_count += 1;
                }
            }
            else if (g_gosub_rows[row].value == -2)
            {
                y = (row_offset - y_off) - g_gosub_scroll_y;
                if (y >= -0x1F && y < g_gosub_window_height)
                {
                    line_y3 = y + 8;
                    prim = func_801466B4(prim, ot, 0xC - x_off, y, g_gosub_rows[row].unkE, g_gosub_rows[row].unkD);
                    prim = func_800A88A0(prim, ot, g_gosub_rows[row].name, g_gosub_rows[row].kind, 0x4C - x_off, line_y3, 0);
                    if (g_gosub_rows[row].flags.f.flag2)
                    {
                        prim = func_800A88A0(prim, ot, MSG_HI(0x20), g_gosub_rows[row].kind, 0x110 - x_off, line_y3, 1);
                    }
                }
            }
            else
            {
                status_pad = row * g_gosub_row_height;
                y_top = y_off - 2;
                y = (status_pad - y_top) - g_gosub_scroll_y;
                if (-g_gosub_row_height < y && y < g_gosub_window_height)
                {
                    prim = func_800A88A0(prim, ot, g_gosub_rows[row].name, g_gosub_rows[row].kind, 0xC - x_off, y, 0);
                    pos.y = y;
                    x_pad = x_off + 0xC;
                    pos.x = g_gosub_window_width - x_pad;
                    if (g_gosub_rows[row].value >= 0)
                    {
                        prim = func_800A8A78(ot, prim, g_gosub_rows[row].value, g_gosub_rows[row].kind, pos_p, 1);
                    }
                }
            }
            row_offset += 0x20;
            row += 1;
        } while (row < g_gosub_row_count);
    }

    tile = (GosubTile*)prim;
    cursor_color = 0xF080F0;
    addr_mask = 0xFFFFFF;
    for (;;)
    {
        cursor_p = &g_gosub_cursor_row;
        height_p = &g_gosub_row_height;
        scroll_p = &g_gosub_scroll_y;
        break;
    }
    mark = tile + 1;
    status_pad = *cursor_p * *height_p;
    y_top2 = y_off - 2;
    y = (status_pad - y_top2) - *scroll_p;
    ((u8*)tile)[3] = 3;
    tile->unk4 = cursor_color;
    ((u8*)tile)[7] = 0x62;
    row = 0;
    tile->unkC = g_gosub_window_width;
    tile->unkA = y - 2;
    tile->unk8 = 1;
    tile->unkE = g_gosub_row_height - 1;
    ((GosubTag*)tile)->addr = (GET_PRIM_ADDR(ot) & addr_mask);
    SET_PRIM_ADDR(ot, tile);
    while (row < g_gosub_selection_count)
    {
        {
            mark->unk8 = (row | 1) & 1;
            mark->unk4 = 0x808080;
            ((u8*)mark)[3] = 3;
            ((u8*)mark)[7] = 0x62;
            sel_mul = g_gosub_selected_rows[row] * g_gosub_row_height;
            y_top3 = y_off - 2;
            y = (sel_mul - y_top3) - g_gosub_scroll_y;
            mark->unkC = g_gosub_window_width;
            mark->unkA = y - 2;
            mark->unkE = g_gosub_row_height - 1;
            row += 1;
            ADD_PRIM_MASKED(ot, mark);
            mark += 1;
        }
    }
    return mark;
}

/** @brief Sprite primitive (0x14 bytes, code 0x64), SPRT layout. */
typedef struct
{
    u_long tag; /* 0x00 P_TAG */
    u8 r0;      /* 0x04 */
    u8 g0;      /* 0x05 */
    u8 b0;      /* 0x06 */
    u8 code;    /* 0x07 */
    s16 x0;     /* 0x08 */
    s16 y0;     /* 0x0A */
    u8 u0;      /* 0x0C */
    u8 v0;      /* 0x0D */
    u16 clut;   /* 0x0E */
    s16 w;      /* 0x10 */
    s16 h;      /* 0x12 */
} GosubSprt;    /* 0x14 */

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
 * @see decomp.me (100%)
 */
s32 func_801450D8(s32 prim, s32* ot, s32 row, s32 x, s32 y, s32 count)
{
    GosubSprt* sprt;
    GosubRect rect;
    s32 idx;
    s32 cell;
    s32 n;

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
    n = count;
    cell = n * 3;

    rect.x = cell * 4 + 0x140;
    rect.w = 0xC;
    rect.h = 0x30;
    rect.y = g_gosub_frame_parity * 0x30;
    LoadImage(&rect, (u8*)D_80152DF4 + D_80152DF4[idx] + 0x1C);

    rect.y = 0x1F2;
    rect.w = 0x10;
    rect.h = 1;
    n = n * 0x10;
    rect.x = n + g_gosub_frame_parity * 0x50;
    LoadImage(&rect, (u8*)D_80152DF4 + D_80152DF4[idx] - 4);

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
    sprt->clut = (((n + g_gosub_frame_parity * 0x50) >> 4) & 0x3F) | 0x7C80;
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

s32 func_80145A14(s32* ot, s32 prim, s32 x_off, s32 y_off);

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

s32 func_80145B28(s32* ot, s32 prim, s32 x_off, s32 y_off);

/**
 * @see decomp.me (100%)
 */
void func_8014595C(void)
{
    GosubElement* element;

    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&func_80145B28;
    g_gosub_dialog_handler = func_80145800;
    g_gosub_dialog_choice = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.unk0_3 = 1;
    element->attr.f.x = 0x80;
    element->attr.f.unk0_16 = 0x70;
    element->unk4_0 = 0;
    element->y = 0x34;
    SET_ELEM_CODE(element, 0x80);
    func_800AA02C();
}

/**
 * @brief Draw handler for element 0 of the wide confirmation dialog.
 *
 * Emits the two option labels through func_800A88A0, highlighting the one that
 * matches the current selection: the color toggles between 5 (highlighted) and
 * 4 (dim) with g_gosub_dialog_choice bit 0, inverted between the two rows. Both
 * labels sit at x 0x40 - x_off; the rows are at y 2 - y_off and 0x12 - y_off.
 * The label text is resolved out of the D_8014F29C archive block (offsets -0x12
 * and -0x10 of the entry table). The @c base and @c table locals reconstruct
 * the archive base the way the target's register roles do.
 *
 * @param ot    Ordering-table tag every packet links into.
 * @param prim  Packet cursor; threaded through both draws.
 * @param x_off Horizontal offset subtracted from the label column.
 * @param y_off Vertical offset subtracted from both row positions.
 * @return Packet cursor past the last emitted label.
 *
 * @see decomp.me (100%)
 */
s32 func_80145A14(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* glyph0;
    void* glyph1;
    s32 color;
    s32 color0;
    s32 p;
    s32 pad[14];

    p = prim;
    table = &D_8014F29C;
    base = (s32)table - 0x20;

    glyph0 = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0x12)));
    color0 = 5;
    if ((g_gosub_dialog_choice & 1) == 0)
    {
        color0 = 4;
    }
    p = func_800A88A0(p, ot, glyph0, color0, 0x40 - x_off, 2 - y_off, 2);

    glyph1 = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0x10)));
    color = 5;
    if ((g_gosub_dialog_choice & 1) != 0)
    {
        color = 4;
    }
    p = func_800A88A0(p, ot, glyph1, color, 0x40 - x_off, 0x12 - y_off, 2);

    return p;
}

/**
 * @brief Draw handler for element 0 of the three-option wide confirmation dialog.
 *
 * Emits the three option labels through func_800A88A0, dimming the one that
 * matches the current selection: the color is 5 (bright) unless the row index
 * equals g_gosub_dialog_choice % 3, in which case it is 4 (dim). All labels sit
 * at x 0x40 - x_off; the rows are at y 2 - y_off, 0x12 - y_off, and 0x22 - y_off.
 * The label text is resolved out of the D_8014F29C archive block (offsets -0xE,
 * -0xC, and -0xA of the entry table). The @c base and @c table locals reconstruct
 * the archive base the way the target's register roles do; this is the
 * three-option sibling of func_80145A14 and mirrors its proven operand shapes.
 *
 * @param ot    Ordering-table tag every packet links into.
 * @param prim  Packet cursor; threaded through all three draws.
 * @param x_off Horizontal offset subtracted from the label column.
 * @param y_off Vertical offset subtracted from every row position.
 * @return Packet cursor past the last emitted label.
 *
 * @see decomp.me (100%)
 */
s32 func_80145B28(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* glyph0;
    void* glyph1;
    void* glyph2;
    s32 color;
    s32 color0;
    s32 p;
    s32 rem0;
    s32 pad[14];

    p = prim;
    table = &D_8014F29C;
    base = (s32)table - 0x20;

    glyph0 = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0xE)));
    rem0 = g_gosub_dialog_choice;
    rem0 %= 3;
    color0 = 5;
    if (rem0 == 0)
    {
        color0 = 4;
    }
    p = func_800A88A0(p, ot, glyph0, color0, 0x40 - x_off, 2 - y_off, 2);

    glyph1 = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0xC)));
    color = 5;
    if (g_gosub_dialog_choice % 3 == 1)
    {
        color = 4;
    }
    p = func_800A88A0(p, ot, glyph1, color, 0x40 - x_off, 0x12 - y_off, 2);

    glyph2 = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0xA)));
    color = 5;
    if (g_gosub_dialog_choice % 3 == 2)
    {
        color = 4;
    }
    p = func_800A88A0(p, ot, glyph2, color, 0x40 - x_off, 0x22 - y_off, 2);

    return p;
}

/**
 * @see decomp.me (100%)
 */
void func_80145CEC(s32 arg0)
{
    GosubElement* element;

    D_8016B8F4 = arg0;
    element = &g_gosub_elements[0];
    element->draw_handler = (void*)&func_80145DA8;
    g_gosub_dialog_choice = 0;
    D_8016B948 = 1;
    D_8016B95C = 0;
    element->attr.f.state = GOSUB_ELEMENT_STATE_ENTERING;
    element->attr.f.unk0_3 = 1;
    element->attr.f.x = 0x20;
    element->attr.f.unk0_16 = 0x70;
    element->unk4_0 = 1;
    element->y = 0x14;
    SET_ELEM_CODE(element, 0);
    func_800AA02C();
    D_801228F0 = 0;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80145DA8(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32 pad[14];

    prim = func_800A88A0(prim, ot, (void*)D_8016B8F4, 4, 0x80 - x_off, 2 - y_off, 2);
    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80145DF8(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* glyph;
    s32 pad[14];

    table = &D_8014F29C;
    base = (s32)table - 0x20;

    glyph = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0x18)));
    prim = func_800A88A0(prim, ot, glyph, 4, 0x84 - x_off, 2 - y_off, 2);
    if (D_8016B900 != 0)
    {
        prim = func_80146178(prim, ot, x_off, y_off);
    }
    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80145EA4(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* glyph;
    s32 pad[14];

    table = &D_8014F29C;
    base = (s32)table - 0x20;

    glyph = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0x1C)));
    prim = func_800A88A0(prim, ot, glyph, 4, 0x84 - x_off, 2 - y_off, 2);

    glyph = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0x1A)));
    prim = func_800A88A0(prim, ot, glyph, 4, 0x84 - x_off, 0x12 - y_off, 2);

    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80145F80(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32* table;
    s32 base;
    void* glyph;
    s32 color;
    s32 pad[14];

    table = &D_8014F29C;
    base = (s32)table - 0x20;

    glyph = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0x14)));
    prim = func_800A88A0(prim, ot, glyph, 4, 0x80 - x_off, 2 - y_off, 2);

    glyph = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0x20)));
    color = 5;
    if ((g_gosub_dialog_choice & 1) == 0)
    {
        color = 4;
    }
    prim = func_800A88A0(prim, ot, glyph, color, 0x78 - x_off, 0x12 - y_off, 1);

    glyph = (void*)(D_8014F29C + (base + *(u16*)((u8*)&D_8014F29C + D_8014F29C - 0x1E)));
    color = 4;
    if ((g_gosub_dialog_choice & 1) == 0)
    {
        color = 5;
    }
    prim = func_800A88A0(prim, ot, glyph, color, 0x88 - x_off, 0x12 - y_off, 0);

    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 func_801460D0(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32 pad[12];

    prim = func_800A88A0(prim, ot, g_gosub_rows[g_gosub_cursor_row].desc, 4, 0x84 - x_off, 2 - y_off, 2);
    if (D_8016B900 != 0)
    {
        prim = func_80146178(prim, ot, x_off, y_off);
    }
    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80146178(s32 prim, s32* ot, s32 x_off, s32 y_off)
{
    s32 kind;
    Vec2s pos;
    u8* base;
    s32* table;
    s32 inner;

    kind = g_gosub_rows[g_gosub_cursor_row].unkC_28;

    switch (kind)
    {
    case 0:
        prim = func_800A88A0(prim, ot, (void*)((u8*)D_800EC3EE - 0x2A + D_800EC3EE[0] + (D_800EC3EE[1] << 8)), 4, 0x10 - x_off, 0x12 - y_off, 0);
        pos.x = 0x68 - x_off;
        pos.y = (s16)(0x12 - y_off);
        prim = func_800A8A78(ot, prim, g_gosub_rows[g_gosub_cursor_row].unk10, 4, &pos, 0);
        break;

    case 1:
        prim = func_800A88A0(prim, ot, (void*)((u8*)D_800EC3F0 - 0x2C + D_800EC3F0[0] + (D_800EC3F0[1] << 8)), 4, 0x10 - x_off, 0x12 - y_off, 0);
        pos.x = 0x60 - x_off;
        pos.y = (s16)(0x12 - y_off);
        prim = func_800A8A78(ot, prim,
                             g_gosub_rows[g_gosub_cursor_row].unk12[0] + g_gosub_rows[g_gosub_cursor_row].unk12[1] + g_gosub_rows[g_gosub_cursor_row].unk12[2] +
                                 g_gosub_rows[g_gosub_cursor_row].unk12[3],
                             4, &pos, 0);
        break;

    default:
        table = &D_8014F2A8;
        prim = func_800A88A0(prim, ot, (void*)((u8*)D_800EC3F2 - 0x2E + D_800EC3F2[0] + (D_800EC3F2[1] << 8)), 4, 0x10 - x_off, 0x12 - y_off, 0);
        pos.x = 0x38 - x_off;
        pos.y = (s16)(0x12 - y_off);
        prim = func_800A8A78(ot, prim, g_gosub_rows[g_gosub_cursor_row].unk10, 4, &pos, 0);
        base = (u8*)table;
        base -= 0x2C;
        inner = base + *(u16*)(g_gosub_rows[g_gosub_cursor_row].unk12[0] * 2 + D_8014F2A8 + base);
        prim = func_800A88A0(prim, ot, (void*)(D_8014F2A8 + inner), 4, 0x60 - x_off, 0x12 - y_off, 0);
        break;
    }
    return prim;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80146418(s32* ot, s32 prim, s32 x_off, s32 y_off)
{
    s32 pad[14];

    prim = func_800A88A0(prim, ot, g_gosub_title_text, 4, 0x84 - x_off, 2 - y_off, 2);
    return prim;
}

s32 func_801464EC(const u8*); /* extern */

/**
 * @see decomp.me (100%)
 */
void func_80146468(u8* dst, u8* src2)
{
    s32 len1;
    s32 len2;
    s32 i;

    len1 = func_801464EC(dst);
    len2 = func_801464EC(src2);

    for (i = 0; i < len2; i++)
    {
        dst[len1 + i] = src2[i];
    }

    dst[len1 + i] = 0;
}

#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))

/**
 * @see decomp.me (100%)
 */
s32 func_801464EC(const u8* name_buf)
{
    const u8* scan_cursor;
    s32 byte_count;

    scan_cursor = name_buf;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += 2;
            byte_count += 2;
        }
        else
        {
            scan_cursor += 1;
            byte_count += 1;
        }
    }

    return byte_count;
}

/**
 * @see decomp.me (100%)
 */
void func_80146538(u8* dst, u8* src)
{
    const u8* scan_cursor;
    s32 byte_count;
    s32 i;

    scan_cursor = src;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += 2;
            byte_count += 2;
        }
        else
        {
            scan_cursor += 1;
            byte_count += 1;
        }
    }

    for (i = 0; i < byte_count; i++)
    {
        dst[i] = src[i];
    }

    dst[i] = 0;
}

/** @brief VRAM upload position for an image and its CLUT (see func_801465FC). */
typedef struct
{
    u16 x;
    u16 y;
    u16 clut_x;
    u16 clut_y;
} GosubImageClutPos;

extern u8 D_80147058[];

void func_801465FC(GosubImageClutPos* pos, u8* archive); /* extern */

/**
 * @see decomp.me (100%)
 */
void func_801465BC(void)
{
    GosubImageClutPos pos;

    pos.x = 0x140;
    pos.y = 0;
    pos.clut_x = 0;
    pos.clut_y = 0x1F2;
    func_801465FC(&pos, D_80147058);
}

/**
 * @brief Upload an image (and, when the archive's flag bit 3 is set, its
 *        CLUT) into VRAM via LoadImage.
 * @param pos Destination VRAM position for the image and, if present, its
 *            CLUT (see GosubImageClutPos).
 * @param archive Image archive: word flags at +0x4 (bit 3 = has CLUT), word
 *                data offset at +0x8, dimensions/pixel data at +0x10 (or
 *                +off8+0x10 when a CLUT precedes them), CLUT pixel data at
 *                +0x14, image pixel data at +off8+0x14.
 * @see decomp.me (100%)
 */
void func_801465FC(GosubImageClutPos* pos, u8* archive)
{
    GosubRect rect;
    s32 flags;
    s32 off8;
    u16* dims;

    flags = *(s32*)(archive + 4);
    off8 = *(s32*)(archive + 8);

    if (flags & 8)
    {
        rect.x = pos->clut_x;
        rect.y = pos->clut_y;
        rect.w = 0x100;
        rect.h = 1;
        LoadImage(&rect, archive + 0x14);
        dims = (u16*)(off8 - (-(s32)archive) + 0x10);
    }
    else
    {
        dims = (u16*)(archive + 0x10);
    }

    rect.x = pos->x;
    rect.y = pos->y;
    rect.w = dims[0];
    rect.h = dims[1];
    LoadImage(&rect, off8 - (-(s32)archive) + 0x14);
}

extern s32 D_800F2180[];
extern u8 D_800F1CD0[];

/**
 * @brief Draw a table row from D_800F1CD0[row_idx] into two column buffers
 *        via the still-unmatched func_80146860, then finalize with
 *        func_8014680C.
 * @param arg_prim Initial prim cursor.
 * @param ot Ordering table, passed through unchanged to every call.
 * @param x First coordinate base (column offset added to it below).
 * @param y Second coordinate base (column offset added to it below).
 * @param table_idx Index into D_800F2180[]; also offsets the header call's
 *                   3rd arg (table_idx + 0x13) and supplies table_val, the
 *                   loop calls' constant last arg.
 * @param row_idx Row index into D_800F1CD0 (stride 88 bytes: count byte,
 *                 four header fields, then a repeating {s8, s8, s16} tuple
 *                 array of `count` entries).
 * @return Advanced prim cursor (func_8014680C's return).
 * @see decomp.me (100%)
 */
s32 func_801466B4(s32 arg_prim, s32* ot, s32 x, s32 y, s32 table_idx, s32 row_idx)
{
    u8* entry;
    s32 table_val;
    s16 field4;
    s16 field6;
    s8 field8;
    s8 field9;
    s32 col_x;
    s32 col_y;
    u8* item;
    s32 i;
    s32 prim;
    u8* base;
    table_val = D_800F2180[table_idx];
    base = D_800F1CD0;
    entry = (u8*)(row_idx * 11 * 8 + (s32)base);
    field4 = *(s16*)(entry + 4);
    field6 = *(s16*)(entry + 6);
    field8 = *(s8*)(entry + 8);
    field9 = *(s8*)(entry + 9);
    col_x = x + field4 * 8;
    col_y = y + field6 * 8;
    prim = func_80146860(arg_prim, ot, table_idx + 0x13, field8 * 8 + col_x, field9 * 8 + col_y, 9);
    field6 = 0;
    for (i = field6; i < entry[field6]; i++)
    {
        u8* loop_base = &D_800F1CD0[field6];
        item = (u8*)(row_idx * 11 * 8 + i * 4 + (s32)loop_base);
        prim = func_80146860(prim, ot, *(s16*)(item + 0xE), *(s8*)(item + 0xC) * 16 + col_x, *(s8*)(item + 0xD) * 16 + col_y, table_val);
    }
    return func_8014680C(prim, ot);
}

/** @brief Draw-mode packet (0x08 bytes, GPU code 0xE1). */
typedef struct
{
    u_long tag;  /* 0x00 P_TAG */
    u_long code; /* 0x04 GPU draw-mode word */
} GosubTPage;    /* 0x08 */

/** @brief Glyph cell descriptor in the 8-byte table at D_8016B634. */
typedef struct
{
    u8 u0;    /* 0x00 texture u */
    u8 unk1;  /* 0x01 */
    u8 v0;    /* 0x02 texture v */
    u8 unk3;  /* 0x03 */
    u16 w;    /* 0x04 */
    u16 h;    /* 0x06 */
} GosubGlyph; /* 0x08 */

/** @brief Glyph cell table indexed by the glyph id passed to func_80146860. */
extern GosubGlyph D_8016B634[];
/** @brief Texture archive holding the gosub font page (+0x00) and its CLUT (+0x2C). */
extern u8 D_8016B3DC[];

void func_80016E7C(); /* extern */
void func_80019788(); /* extern */

inline void func_80146AA8(void* dst, void* src);
inline void func_80146AD0(void* dst, void* src);
s32 func_80146CC4(s32 mode, s32 a, s32 b);

/**
 * @brief Append the texture-page packet that closes a glyph run.
 *
 * @param prim Packet cursor.
 * @param ot   Ordering-table tag to link the packet into.
 * @return Packet cursor past the 8-byte draw-mode packet.
 * @see decomp.me (100%)
 */
s32 func_8014680C(s32 prim, s32* ot)
{
    GosubTPage* tp;

    tp = (GosubTPage*)prim;
    ((GosubTag*)tp)->len = 1;
    tp->code = 0xE1000005;
    ADD_PRIM(ot, tp);
    return prim + 8;
}

/**
 * @brief Emit one glyph sprite described by the D_8016B634 cell table.
 *
 * @param prim  Packet cursor.
 * @param ot    Ordering-table tag to link the sprite into.
 * @param glyph Index into D_8016B634 supplying the cell u/v and size.
 * @param x     Sprite left edge.
 * @param y     Sprite top edge.
 * @param clut  CLUT selector; the low 6 bits index the 0x7C80 palette row.
 * @return Packet cursor past the 0x14-byte sprite.
 * @see decomp.me (100%)
 */
s32 func_80146860(s32 prim, s32* ot, s32 glyph, s32 x, s32 y, s32 clut)
{
    GosubSprt* sprt;

    sprt = (GosubSprt*)prim;
    *(u32*)&sprt->r0 = 0x808080;
    ((GosubTag*)sprt)->len = 4;
    sprt->code = 0x64;
    sprt->x0 = x;
    sprt->y0 = y;
    sprt->w = D_8016B634[glyph].w;
    sprt->h = D_8016B634[glyph].h;
    sprt->u0 = D_8016B634[glyph].u0;
    sprt->v0 = D_8016B634[glyph].v0;
    sprt->clut = (clut & 0x3F) | 0x7C80;
    ADD_PRIM(ot, sprt);
    return prim + 0x14;
}

/**
 * @brief Delete one 4-byte record from the table at g_pad_ctx[0x29DC].
 *
 * Every record above @p slot is shifted down one place and the record count at
 * g_pad_ctx[0x29D6] is decremented.
 *
 * @param slot Index of the record to remove.
 * @see decomp.me (100%)
 */
void func_80146908(s32 slot)
{
    s32 i;

    for (i = slot; i < g_pad_ctx[0x29D6] - 1; i++)
    {
        func_80146AA8(g_pad_ctx + (i * 4 + 0x29DC), g_pad_ctx + (i * 4 + 0x29E0));
    }
    g_pad_ctx[0x29D6]--;
}

/**
 * @brief Delete one row from g_gosub_rows and renumber the rows above it.
 *
 * Rows above @p row are shifted down one slot and g_gosub_row_count is
 * decremented; every surviving row whose index still points past @p row has
 * that index pulled down by one so it keeps naming the same record.
 *
 * @param row Index of the row to remove.
 * @see decomp.me (100%)
 */
void func_801469BC(s32 row)
{
    s32 i;

    for (i = row; i < g_gosub_row_count - 1; i++)
    {
        func_80146AD0(&g_gosub_rows[i], &g_gosub_rows[i + 1]);
    }
    g_gosub_row_count--;
    for (i = 0; i < g_gosub_row_count; i++)
    {
        if (row < g_gosub_rows[i].index)
        {
            g_gosub_rows[i].index--;
        }
    }
}

/**
 * @brief Copy one 4-byte record.
 *
 * @param dst Destination record.
 * @param src Source record.
 * @see decomp.me (100%)
 */
inline void func_80146AA8(void* dst, void* src)
{
    u8* d;
    u8* s;
    u32 i;

    d = (u8*)dst;
    s = (u8*)src;
    i = 0;
    do
    {
        i += 1;
        *d = *s;
        s += 1;
        d += 1;
    } while (i < 4U);
}

/**
 * @brief Copy one 0x20-byte GosubListEntry.
 *
 * @param dst Destination row.
 * @param src Source row.
 * @see decomp.me (100%)
 */
inline void func_80146AD0(void* dst, void* src)
{
    u8* d;
    u8* s;
    u32 i;

    d = (u8*)dst;
    s = (u8*)src;
    i = 0;
    do
    {
        i += 1;
        *d = *s;
        s += 1;
        d += 1;
    } while (i < 0x20U);
}

/**
 * @brief Sort the gosub row list, carrying each row's backing record with it.
 *
 * An insertion sort driven by func_80146CC4 builds a permutation in
 * @c order[0..0xFF] without moving anything. The live rows and their 4-byte
 * records are then snapshotted into the tail of that same scratch buffer
 * (records at +0x100, rows at +0x1A0) and written back through the
 * permutation. The packed record list is rebuilt afterwards so it agrees with
 * the new row order.
 *
 * @param mode Comparison selector handed to func_80146CC4: the low nibble picks
 *             the field and a non-zero high nibble reverses the order.
 *
 * @note The write-back loop calls func_80146AA8/func_80146AD0, which are
 *       declared @c inline and defined ABOVE this function, so GCC expands them
 *       here while func_80146908 and func_801469BC - both defined before those
 *       bodies - still emit real calls, exactly as the target does. Moving
 *       either definition or dropping @c inline breaks the match.
 *
 * @see decomp.me (100%)
 */
void func_80146AF8(s32 mode)
{
    u8 order[0x6A0];
    s32 i;
    s32 j;
    s32 k;

    for (i = 0; i < g_gosub_row_count; i++)
    {
        for (j = 0; j < i; j++)
        {
            if (func_80146CC4(mode, i, order[j]) == 0)
            {
                break;
            }
        }
        if (j != i)
        {
            for (k = i; k > j; k--)
            {
                order[k] = order[k - 1];
            }
        }
        order[j] = i;
    }

    func_80016E7C(g_pad_ctx + 0x29DC, &order[0x100], 0xA0);
    func_80016E7C(g_gosub_rows, &order[0x1A0], 0x500);

    i = 0;
    if (g_gosub_row_count > 0)
    {
        do
        {
            func_80146AA8(g_pad_ctx + i * 4 + 0x29DC, order + order[i] * 4 + 0x100);
            func_80146AD0(&g_gosub_rows[i], order + order[i] * 0x20 + 0x1A0);
            i++;
        } while (i < g_gosub_row_count);
    }

    gosub_build_packed_record_list();
}

/**
 * @brief Order two g_gosub_rows entries for func_80146AF8's insertion sort.
 *
 * @param mode Low nibble selects the compared field (0 unkE, 1 unkC, 2 unkD);
 *             a non-zero high nibble swaps @p a and @p b, reversing the order.
 * @param a    First row index.
 * @param b    Second row index.
 * @return 1 when @p a sorts before @p b, 0 otherwise (unknown modes included).
 * @see decomp.me (100%)
 */
s32 func_80146CC4(s32 mode, s32 a, s32 b)
{
    s32 tmp;

    if (mode & 0xF0)
    {
        tmp = a;
        a = b;
        b = tmp;
    }
    switch (mode & 0xF)
    {
    case 0:
        if (g_gosub_rows[a].unkE < g_gosub_rows[b].unkE)
        {
            return 1;
        }
        break;
    case 1:
        if (g_gosub_rows[a].unkC < g_gosub_rows[b].unkC)
        {
            return 1;
        }
        break;
    case 2:
        if (g_gosub_rows[a].unkD < g_gosub_rows[b].unkD)
        {
            return 1;
        }
        break;
    }
    return 0;
}

/**
 * @brief Upload the gosub font page and its CLUT to VRAM.
 * @see decomp.me (100%)
 */
void func_80146DA8(void)
{
    GosubRect rect;

    rect.x = 0x150;
    rect.y = 0xFF;
    rect.w = 0x10;
    rect.h = 1;
    LoadImage(&rect, D_8016B3DC);
    rect.x = 0x140;
    rect.y = 0xF0;
    rect.w = 0x10;
    rect.h = 0x10;
    LoadImage(&rect, D_8016B3DC + 0x2C);
    func_80019788(0);
}

/**
 * @brief Emit the four corner sprites of a gosub panel frame and close the run
 *        with a texture-page packet.
 *
 * The corners sit two pixels outside (@p x, @p y) and five pixels inside the
 * far edge; the top pair uses texture row 0xF0 and the bottom pair 0xF8.
 *
 * @param prim Packet cursor; four sprites and one draw-mode packet are written.
 * @param ot   Ordering-table tag every packet is linked into.
 * @param x    Panel left edge.
 * @param y    Panel top edge.
 * @param w    Panel width.
 * @param h    Panel height.
 * @return Packet cursor past the draw-mode packet.
 * @see decomp.me (100%)
 */
StructS0* func_80146E30(GosubSprt* prim, s32* ot, s32 x, s32 y, s32 w, s32 h)
{
    s32 x0;
    s32 y0;
    s32 x1;
    s32 y1;

    x0 = x - 2;
    y0 = y - 2;

    *(u32*)&prim->r0 = 0x808080;
    ((GosubTag*)prim)->len = 4;
    prim->code = 0x64;
    prim->x0 = x0;
    prim->y0 = y0;
    prim->u0 = 0;
    prim->v0 = 0xF0;
    prim->w = 8;
    prim->h = 8;
    prim->clut = 0x3FD5;
    ADD_PRIM(ot, prim);
    prim += 1;

    x1 = x + w - 5;
    y1 = y + h - 5;

    *(u32*)&prim->r0 = 0x808080;
    ((GosubTag*)prim)->len = 4;
    prim->code = 0x64;
    prim->x0 = x1;
    prim->y0 = y0;
    prim->u0 = 8;
    prim->v0 = 0xF0;
    prim->w = 8;
    prim->h = 8;
    prim->clut = 0x3FD5;
    ADD_PRIM(ot, prim);
    prim += 1;

    *(u32*)&prim->r0 = 0x808080;
    ((GosubTag*)prim)->len = 4;
    prim->code = 0x64;
    prim->x0 = x0;
    prim->y0 = y1;
    prim->u0 = 0;
    prim->v0 = 0xF8;
    prim->w = 8;
    prim->h = 8;
    prim->clut = 0x3FD5;
    ADD_PRIM(ot, prim);
    prim += 1;

    *(u32*)&prim->r0 = 0x808080;
    ((GosubTag*)prim)->len = 4;
    prim->code = 0x64;
    prim->x0 = x1;
    prim->y0 = y1;
    prim->u0 = 8;
    prim->v0 = 0xF8;
    prim->w = 8;
    prim->h = 8;
    prim->clut = 0x3FD5;
    ADD_PRIM(ot, prim);
    prim += 1;

    ((GosubTag*)prim)->len = 1;
    *(u32*)&prim->r0 = 0xE1000005;
    ADD_PRIM(ot, prim);
    return (StructS0*)((u8*)prim + 8);
}
