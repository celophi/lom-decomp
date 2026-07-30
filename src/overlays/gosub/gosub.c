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

void field_set_default_fade_target(); /* extern */
void func_800A8B90();    /* extern */
void func_800AA02C();    /* extern */
void gosub_load_screen_sequence(s32*); /* extern */
void gosub_build_screen_9_elements();    /* extern */
void gosub_build_screen_10_elements();    /* extern */
void gosub_build_category_screen_elements();    /* extern */
void gosub_build_list_screen_elements();    /* extern */
void gosub_build_screen_11_elements();    /* extern */
void gosub_build_compact_list_elements();    /* extern */
void gosub_build_screen_15_item_list();    /* extern */
void gosub_build_screen_19_item_list();    /* extern */
void gosub_build_screen_16_item_list();    /* extern */
void gosub_build_screen_1_item_list();    /* extern */
void gosub_build_screen_0_item_list();    /* extern */
void gosub_build_packed_record_list();    /* extern */
void gosub_build_roster_list();    /* extern */
s32 gosub_select_row_with_validation();    /* extern */
s32 gosub_select_row();    /* extern */
s32 gosub_validate_pending_pair_selection();    /* extern */
s32 gosub_commit_row_reorder();    /* extern */
s32 gosub_update_group_selection();    /* extern */
s32 gosub_publish_two_row_selection();    /* extern */
void func_8014289C();    /* extern */
void func_80142B98();    /* extern */
void func_80142C64();    /* extern */
void func_80143054();    /* extern */
void func_80143B64();    /* extern */
void func_80145CEC();    /* extern */
void func_80146468();    /* extern */
void func_80146538();    /* extern */
extern s32 g_gosub_frame_parity;
extern s32 g_gosub_finished;
extern s32 D_8016B8E0;
extern s32 D_80170990;
extern s32 D_80170988;
extern s32 g_gosub_cursor_row;
extern s32 D_8017098C;
extern u8 g_gosub_screen_sequence_index;
extern s32 g_gosub_result_count;
extern u8 D_80145744;
extern void* D_80174A58;
extern s32 D_8016B948;
extern u8 g_gosub_screen_sequence[20];
extern u8 D_80142B18;
extern u8* g_pad_ctx;
extern s32 D_8014F29C;
extern s32 g_gosub_row_count;
extern s32 g_gosub_visible_row_count;
extern s32 D_8016B8E4;
extern s32 D_8016B8EC;
extern s32 D_8016B8F0;
extern void* g_gosub_finish_handler;
extern u8 D_8016B8FC;
extern u8 g_gosub_required_selection_count;
extern s32 D_8016B900;
extern s32 g_gosub_window_height;
extern void* g_gosub_select_handler;
extern s32 g_gosub_window_width;
extern s32 D_8016B95C;
extern u8 g_gosub_selected_rows[4];
extern s32 g_gosub_result_rows[16];
extern s32 g_gosub_result_values[];
extern s32 D_8017097C;
extern s32 g_gosub_row_height;
extern u8 D_801448EC;
extern u8 D_801452F0;
extern u8 D_80145DF8;
extern u8 D_80145EA4;
extern u8 D_801460D0;
extern u8 D_80145F80;
extern u8 D_80146418;
extern u8 g_gosub_selection_count;
extern s32 D_8016B8E8;
extern u8 D_800EC3DA[];
extern u32 D_8014F27C[];
extern u32 D_8014F280[];
extern u32 D_8014F294[];
extern u8 D_8016B960[];
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
 * @note Field meanings beyond x/y are unconfirmed; the `_N` suffixes record the
 *       bit position each one starts at.
 */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 unk0_0 : 3;
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

extern GosubElement g_gosub_fixed_element;

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
#define ARCHIVE_ENTRY(blk, idx) \
    ((u8*)D_8014F27C + (blk) + *(u16*)((u8*)D_8014F27C + (blk) + (idx) * 2))

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
#define GOSUB_MSG_PTR(off) \
    ((u8*)&D_8014F29C - 0x20 + D_8014F29C + *(u16*)((u8*)&D_8014F29C + D_8014F29C + (off)))

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
 * @param screen_sequence Pointer to an s32 array terminated by 0xFE.
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
    func_8006441C();
    func_80143258(render_ctx);
    func_80063194();
    frame_parity = &g_gosub_frame_parity;
    finished = g_gosub_finished;
    *frame_parity ^= 1;
    return finished;
}

/**
 * @brief Copy and enter a 0xFE-terminated sequence of gosub screen ids.
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
    D_8016B8E0 = 0;
    D_80170990 = 0;
    D_80170988 = 0;
    g_gosub_cursor_row = 0;
    func_80143BD0();
    D_8017098C = 0;
    g_gosub_screen_sequence_index = 0;
    g_gosub_result_count = 0;
    D_80174A58 = (void*)&D_80145744;
    screen_count = 0;
    if (*screen_sequence != 0xFE)
    {
        u8* arr = g_gosub_screen_sequence;
        s32 sentinel = 0xFE;
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
void gosub_enter_screen(s32 screen_id, s32 unused)
{
    D_8016B8E0 = 0;
    D_80170990 = 0;
    D_80170988 = 0;
    g_gosub_cursor_row = 0;
    D_8016B8E4 = 0;
    D_8017097C = 0;
    D_8016B8EC = 0;
    D_8016B8F0 = 0;
    D_8016B900 = 0;

    switch (screen_id)
    {
    case 0:
        func_80143B64();
        gosub_build_screen_0_item_list();
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)func_80142B98;
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
        g_gosub_finish_handler = (void*)func_80142B98;
        gosub_build_list_screen_elements(1);
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(0xA);
        }
        break;

    case 2:
        func_80143B64();
        func_80142C64(0);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)func_80142B98;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(2);
        }
        break;

    case 3:
        func_80143B64();
        func_80142C64(1);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)func_80142B98;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(4);
        }
        break;

    case 4:
        func_80143B64();
        func_80142C64(2);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)func_80142B98;
        gosub_build_category_screen_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(6);
        }
        break;

    case 5:
        func_80143B64();
        func_80142C64(3);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)func_80142B98;
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
        func_80143054(screen_id - 6);
        g_gosub_required_selection_count = 1;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_select_row;
        g_gosub_finish_handler = (void*)func_80142B98;
        gosub_build_list_screen_elements(1);
        break;

    case 9:
        func_80143B64();
        func_80142C64(4);
        g_gosub_required_selection_count = 4;
        D_8016B8FC = 1;
        g_gosub_select_handler = (void*)gosub_update_group_selection;
        g_gosub_finish_handler = (void*)&D_80142B18;
        gosub_build_screen_9_elements();
        if (g_gosub_row_count == 0)
        {
            func_80143B64();
            GOSUB_MSG(-2);
        }
        break;

    case 10:
        func_80143B64();
        func_80142C64(3);
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
        D_80174A58 = (void*)func_8014289C;
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
        g_gosub_finish_handler = (void*)func_80142B98;
        D_8016B8F0 = 1;
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
        g_gosub_finish_handler = (void*)func_80142B98;
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
        g_gosub_finish_handler = (void*)func_80142B98;
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
        g_gosub_finish_handler = (void*)func_80142B98;
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
        g_gosub_finish_handler = (void*)func_80142B98;
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
        g_gosub_finish_handler = (void*)func_80142B98;
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
        g_gosub_finish_handler = (void*)func_80142B98;
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
        g_gosub_finish_handler = (void*)func_80142B98;
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
        g_gosub_finish_handler = (void*)func_80142B98;
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
    p->draw_handler = (void*)&D_80145EA4;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->draw_handler = (void*)&D_801460D0;
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
    p->draw_handler = (void*)&D_80145DF8;
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
 * @brief Configure the fixed element g_gosub_fixed_element and clear D_8016B8E8.
 *
 * Unlike gosub_build_screen_9_elements / gosub_build_screen_10_elements this one does not allocate: it reuses a
 * single statically allocated element, so it needs no frame. Its attr top byte
 * is cleared rather than set to one of the other observed attribute codes.
 */
void gosub_initialize_fixed_element(void)
{
    GosubElement* p;

    p = &g_gosub_fixed_element;
    p->draw_handler = (void*)&D_80145F80;
    D_8016B8E8 = 0;
    p->attr.f.unk0_0 = 1;
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
    p->draw_handler = (void*)&D_801460D0;
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
        p->draw_handler = (void*)&D_801460D0;
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
    p->draw_handler = (void*)&D_801460D0;
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
 * scratch buffer in D_8016B960 -- the archive string, then optionally a
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
        buf = D_8016B960 + i * 0x50;
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
        if (((*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) >> 16) & 1) != 0 ||
            (*(u32*)(g_pad_ctx + (i << 2) + 0x29DC) & 3) != 3)
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
                    g_gosub_rows[row_count].unk12[stat_index] =
                        *(u16*)(g_pad_ctx + record_offset + 0x2B26 + stat_index * 2);
                }
                record_offset_reload = record_index * 332;
                g_gosub_rows[row_count].unk1A =
                    *(u16*)(g_pad_ctx + record_offset_reload + 0x2B22);
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
                g_gosub_rows[row_count].flags.f.flag0 =
                    *(u32*)(g_pad_ctx + record_offset + 0x2F38) >> 31;
                g_gosub_rows[row_count].flags.f.flag1 =
                    (*(u32*)(g_pad_ctx + record_offset + 0x2F38) >> 30) & 1;
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
                    g_gosub_rows[row_count].unk12[stat_index] =
                        *(u16*)(g_pad_ctx + record_offset + 0x2F14 + stat_index * 2);
                }
                record_offset_reload = slot * 0x60;
                g_gosub_rows[row_count].unk1A =
                    *(u16*)(g_pad_ctx + record_offset_reload + 0x2F10);
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
 * exactly three marked rows have been -- with func_80142C18 deciding whether an
 * individual row qualifies.
 *
 * @return 1 when both conditions hold, in which case func_80142B98 is rung to
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
            if (g_gosub_rows[i].unkC_28 == 0 && func_80142C18(i) != 0)
            {
                g_gosub_rows[i].kind = 5;
            }
        }
    }
    if (marked_count == 3)
    {
        for (i = 0; i < g_gosub_row_count; i++)
        {
            if (g_gosub_rows[i].unkC_28 != 0 && func_80142C18(i) != 0)
            {
                g_gosub_rows[i].kind = 5;
            }
        }
    }
    if (open_count != 0)
    {
        if (marked_count == 3)
        {
            func_80142B98();
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
