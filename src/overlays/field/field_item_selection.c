#include "common.h"
#include "sdk/libgpu.h"
extern u8 D_800EB254[];
extern u8 D_800FDF58[];
extern u8 D_80105AE0[];
extern u8 D_80122738[];
extern u8 *g_pad_ctx;
extern s32 D_80122714, D_80122734, D_801227BC, D_801227C0;
extern s32 D_801227D8, D_801227E4, D_80122828, D_801229F8, D_80122A00;
extern s32 g_menu_element_counter, g_pad_input, g_pad_input_inject;
void field_start_actor_animation(s32, s32, s32);
void field_initialize_actor_record(s32, s32);
void field_restart_actor_animation(u8 *);
s32 func_800839F8(s32, s32);
s32 func_80083EEC(s32, s32, s32);
void func_8009C2E0(u8 *, s32 *);
void func_800A3938(s32, s32);
s32 func_800A9D70(s32);
void func_800C2640(s32, s32);

struct Window;
u8 *func_800AF0E8(u32 *, u8 *, s32, s32, s32, struct Window *);
void func_800AF0C4(void);


/** @brief Packed item-window flags controlling state, type, priority, and value. */
typedef union
{
    u32 word;
    struct
    {
        u32 low : 3;
        u32 type : 4;
        u32 priority : 9;
        u32 value : 8;
        u32 high : 8;
    } bits;
} ItemWindowFlags;

/** @brief Packed item-window state controlling visibility, height, and display mode. */
typedef union
{
    u32 word;
    struct
    {
        u32 enabled : 1;
        u32 value : 8;
        u32 bit9 : 1;
        u32 mode : 2;
        u32 high : 20;
    } bits;
    s16 half[2];
} ItemWindowState;

/** @brief Item-selection window slot and its draw callback. */
typedef struct
{
    ItemWindowFlags flags;
    ItemWindowState state;
    s16 scroll;
    s16 scroll_target;
    s16 scroll_ticks;
    u16 padding;
    u8 *(*draw_callback)(u32 *, u8 *, s32, s32, s32, struct Window *);
} ItemWindowSlot;

/** @brief Build the available item list and open its selection window. */
void func_800AEE28(void)
{
    u8 *item_entry;
    s32 scroll_offset;
    ItemWindowSlot *slot_cursor;
    ItemWindowSlot *slot;
    s32 *reset_flags;
    s32 packed_flags;
    s32 slot_flags;
    s32 reset_index;
    s32 item_count;
    s32 slot_index;
    s32 item_id;
    u32 claimed_flags;

    g_menu_element_counter = 0;
    reset_flags = &D_80122828;
    reset_index = 0;
    do
    {
        reset_index += 1;
        *reset_flags &= ~7;
        reset_flags += 5;
    } while (reset_index < 8);
    item_count = 0;
    item_id = 0x60;
    do
    {
        if (*(item_id + g_pad_ctx + 0x25E0) != 0)
        {
            item_entry = &D_80122738[item_count * 2];
            item_entry[0] = item_id;
            item_entry[1] = *(item_id + g_pad_ctx + 0x25E0);
            item_count += 1;
        }
        item_id += 1;
    } while (item_id < 0x90);
    D_80122734 = item_count;
    if (item_count == 0)
    {
        func_800AF0C4();
        return;
    }

    goto start_window;

initialize_slot:
    claimed_flags = (slot_flags & ~7) | 1;
    do
    {
        slot = slot_cursor;
    } while (0);
    slot->flags.word = claimed_flags;
    slot->scroll = 0;
    slot->scroll_target = 0;
    slot->scroll_ticks = 0;
    slot->state.word &= ~0x200;
    slot->state.word &= ~0xC00;
    slot->state.half[1] = 0;
    goto setup_slot;

start_window:
    D_80122714 = 1;
    func_800A3938(0xB9, 0x80);
    slot_cursor = (ItemWindowSlot *)&D_80122828;
    slot_index = 0;
scan_slot:
    do { slot_flags = slot_cursor->flags.word; } while (0);
    slot_index += 1;
    if ((slot_flags & 7) == 0)
    {
        goto initialize_slot;
    }
    if (slot_index < 8)
    {
        slot_cursor += 1;
        goto scan_slot;
    }
    slot = (ItemWindowSlot *)&D_80122828;

setup_slot:
    slot->draw_callback = func_800AF0E8;
    slot->scroll = 0;
    slot->flags.bits.type = 1;
    slot->flags.bits.priority = 0x2C;
    slot->flags.bits.value = 0x20;
    slot->state.bits.enabled = 0;
    slot->state.bits.value = 0xA0;
    packed_flags = slot->flags.word;
    packed_flags &= 0xFFFFFF;
    packed_flags |= 0xE8000000;
    slot->flags.word = packed_flags;
    slot->state.bits.mode = 2;
    slot->state.half[1] = D_80122734 * 0x10;
    if (D_80122A00 >= D_80122734)
    {
        D_80122A00 = D_80122734 - 1;
    }
    scroll_offset = D_80122A00 * 0x10;
    slot->scroll_target = 0;
    if (scroll_offset < 0)
    {
        slot->scroll_target = scroll_offset;
    }
    else
    {
        if ((slot->state.bits.value - 0x10) < scroll_offset)
        {
            slot->scroll_target = scroll_offset - (slot->state.bits.value - 0x10);
        }
    }
    slot->scroll_ticks = 0;
    g_pad_input = 0;
    D_801227BC = func_800A9D70(0);
    D_801227C0 = 0xF;
    g_pad_input_inject = 0;
    D_801227D8 = func_800A9D70(1);
    D_801227E4 = 0xF;
    D_801229F8 = 0;
}



void func_800A3938(s32, s32);

/**
 * @brief Thin stack-frame wrapper around func_800A3938 with fixed args.
 */
void func_800AF0C4(void)
{
    func_800A3938(0x78, 0x80);
}

/** Window header containing the packed clipping height at offset four. */
typedef struct Window
{
    u32 unused;
    u32 size;
} Window;
/** Sixteen-byte GPU tile packet, including its ordering-table tag. */
typedef struct Tile
{
    u32 tag;
    u32 color;
    s16 x, y, w, h;
} Tile;
extern u8 D_800EE72C[];
extern s32 D_80122714, D_80122734, D_80122A00;
extern u8 D_80122738[];
extern u8 *func_800A88A0(u8 *, u32 *, u8 *, s32, s32, s32, s32);
extern void func_800A8B90(u8 *, s32, s32);
extern s32 func_800AF350(u8 *);
/**
 * @brief Format and draw one numeric value using a temporary text buffer.
 * @param cursor Primitive buffer cursor.
 * @param ot Ordering table.
 * @param number Value to format.
 * @param position Signed coordinates stored as halfwords.
 * @param color Text color selector.
 * @param flags Text rendering flags.
 * @return Cursor after the generated text primitives.
 */
static __inline__ u8 *draw_number(u8 *cursor, u32 *ot, s32 number, u16 *position, s32 color,
                                  s32 flags)
{
    u8 text[64];
    func_800A8B90(text, number, 0);
    return func_800A88A0(cursor, ot, text, color, (s16)position[0], (s16)position[1], flags);
}
/**
 * @brief Draw the visible item rows and append the highlighted-row tile.
 * @param ot Ordering table.
 * @param cursor Primitive buffer cursor.
 * @param scroll_x Horizontal scroll offset.
 * @param scroll_y Vertical scroll offset.
 * @param unused Unused callback argument retained for the six-argument ABI.
 * @param window Window used to prepare the list and determine its clipping height.
 * @return Cursor after the list, highlight tile, and draw-mode packet.
 */
u8 *func_800AF0E8(u32 *ot, u8 *cursor, s32 scroll_x, s32 scroll_y, s32 unused, Window *window)
{
    u16 point[4];
    Window *draw_window;
    s32 number_x;
    u8 *entry;
    u8 *names;
    s32 y;
    s32 index;
    u8 *mode;
    Tile *tile;

    draw_window = window;
    func_800AF350((u8 *)draw_window);
    if (D_80122714 == 0)
    {
        return cursor;
    }

    for (index = 0; index < D_80122734; index++)
    {
        names = D_800EE72C;
        number_x = 0xCA - scroll_x;
        entry = D_80122738 + index * 2;
        y = index * 16 - scroll_y;
        if (y >= -15 && y < (s32)((draw_window->size >> 1) & 255))
        {
            cursor = func_800A88A0(cursor, ot, (u8 *)(((u16 *)names)[entry[0]] + (u32)names), 4,
                                    -scroll_x, y, 0);
            point[1] = y;
            point[0] = number_x;
            cursor = draw_number(cursor, ot, entry[1], point, 4, 1);
        }
    }
    tile = (Tile *)cursor;
    tile->color = 0xF080F0;
    setlen(cursor, 3);
    setcode(cursor, 0x62);
    tile->w = 0xE8;
    mode = cursor + 16;
    tile->x = 0;
    tile->h = 14;
    tile->y = D_80122A00 * 16 - scroll_y;
    addPrim(ot, tile);
    mode[3] = 1;
    *(u32 *)(mode + 4) = 0xE1000005;
    addPrims(ot, mode, cursor + 16);
    return cursor + 24;
}

/**
 * @brief Process item selection, cancellation, scrolling, and actor creation.
 * @param arg0 Window state with packed flags, clipping height, and scroll state.
 * @see decomp.me (100%)
 */
s32 func_800AF350(u8 *arg0)
{
    s32 offset[3];
    s32 scroll_target;
    s32 *cancel_flags;
    s32 *window_flags;
    s32 actor_flags;
    u16 window_height;
    s32 actor_direction;
    s32 selected_item;
    s32 animation_actor;
    s32 next_selection;
    s32 previous_selection;
    s32 window_index;
    s32 scroll_step;
    s32 cancel_index;
    s32 actor_index;
    s32 slot_offset;
    u8 *direction_entry;
    u8 *entry;
    u8 *record;
    u8 *direction_table;
    u8 *items;

    if ((((*(s32 *)(arg0 + 0x0)) & 7) == 2) && ((*(s16 *)(arg0 + 0xC)) == 0))
    {
        if (D_801229F8 & 0x220)
        {
            g_pad_input = 0;
            D_801227BC = func_800A9D70(0);
            D_801227C0 = 0xF;
            g_pad_input_inject = 0;
            D_801227D8 = func_800A9D70(1);
            D_801227E4 = 0xF;
            D_801229F8 = 0;
            func_800A3938(0x7E, 0x80);
            window_flags = &D_80122828;
            window_index = 0;
            g_menu_element_counter = 0;
            do
            {
                window_index += 1;
                *window_flags &= ~7;
                window_flags += 5;
            } while (window_index < 8);
            actor_index = 0xC;
            direction_table = D_800EB254;
            D_80122714 = 0;
            entry = g_pad_ctx + D_80122738[D_80122A00 * 2];
            (*(u8 *)(entry + 0x25E0)) = (u8) ((*(u8 *)(entry + 0x25E0)) - 1);
            do
            {
                entry = D_800FDF58 + actor_index * 0x54;
                record = entry;
                items = D_80122738;
                if ((*(u8 *)(record + 0x25)) == 0xFF)
                {
                    field_initialize_actor_record(actor_index, 4);
                    (*(s32 *)(record + 0x0)) = (s32) (*(s32 *)(D_800FDF58 + 0x0));
                    (*(s32 *)(record + 0x4)) = (s32) (*(s32 *)(D_800FDF58 + 0x4));
                    (*(s32 *)(record + 0x8)) = (s32) (*(s32 *)(D_800FDF58 + 0x8));
                    direction_entry = (u8 *)(s32)(((u8) (*(u8 *)(D_800FDF58 + 0x1B)) >> 5) * 4);
                    direction_entry = (s32)direction_entry + direction_table;
                    offset[0] = (s32) -(*(s16 *)(direction_entry + 0x0));
                    offset[1] = 0;
                    offset[2] = (s32) -(*(s16 *)(direction_entry + 0x2));
                    func_8009C2E0(record, offset);
                    entry = record;
                    actor_flags = *(s32 *)(entry + 0x1C);
                    slot_offset = actor_index * 0x23C;
                    record = D_80105AE0 + slot_offset;
                    entry[0x25] = 0xFE;
                    selected_item = items[D_80122A00 * 2];
                    actor_direction = D_800FDF58[0x21];
                    actor_flags &= ~0x1FF;
                    actor_flags |= 2;
                    entry[0x27] = 0;
                    entry[0x24] = 1;
                    *(s16 *)(entry + 0x2A) = 0xBA;
                    entry[0x3D] = 2;
                    *(u16 *)(entry + 0x2E) = 0xFE;
                    *(s32 *)(entry + 0x1C) = actor_flags;
                    entry[0x28] = 0;
                    *(s16 *)(entry + 0x10) = 1;
                    entry[0x21] = (selected_item - 0x60) | (actor_direction & 0x80);
                    *(s32 *)(record + 0x14) = actor_index + 0x14;
                    record[0x18E] = 1;
                    *(s16 *)(record + 0x18) = 0;
                    field_restart_actor_animation(entry);
                    func_800C2640((*(s32 *)(record + 0x14)), items[D_80122A00 * 2]);
                    animation_actor = func_800839F8(actor_index, 0);
                    if ((animation_actor != -1) && (func_80083EEC(actor_index, animation_actor, 0xAF) != 0))
                    {
                        field_start_actor_animation(animation_actor, 0, 0);
                    }
                    break;
                }
                actor_index -= 1;
            } while (actor_index >= 3);
        }
        else
        {
            if (D_801229F8 & 0x40)
            {
                g_pad_input = 0;
                D_801227BC = func_800A9D70(0);
                D_801227C0 = 0xF;
                g_pad_input_inject = 0;
                D_801227D8 = func_800A9D70(1);
                cancel_flags = &D_80122828;
                cancel_index = 0;
                D_801227E4 = 0xF;
                D_801229F8 = 0;
                g_menu_element_counter = 0;
                do
                {
                    cancel_index += 1;
                    *cancel_flags &= ~7;
                    cancel_flags += 5;
                } while (cancel_index < 8);
                func_800A3938(0x7F, 0x80);
                D_80122714 = 0;
                return;
            }
            if (D_801229F8 & 0xF00C)
            {
                func_800A3938(0x7D, 0x80);
                scroll_step = 1;
                if (D_801229F8 & 8)
                {
                    scroll_step = 0xA;
                    D_801229F8 = 0x4000;
                }
                if (D_801229F8 & 4)
                {
                    scroll_step = 0xA;
                    D_801229F8 = 0x1000;
                }
                if (scroll_step != 0)
                {
                    do
                    {
                        if (D_801229F8 & 0x6000)
                        {
                            next_selection = D_80122A00 + 1;
                            D_80122A00 = next_selection;
                            if (next_selection >= D_80122734)
                            {
                                D_80122A00 = 0;
                            }
                        }
                        if (D_801229F8 & 0x9000)
                        {
                            previous_selection = D_80122A00 - 1;
                            D_80122A00 = previous_selection;
                            if (previous_selection < 0)
                            {
                                D_80122A00 = D_80122734 - 1;
                            }
                        }
                        if ((D_80122A00 == D_80122734 - 1) || (D_80122A00 == 0))
                        {
                            scroll_step = 1;
                        }
                        scroll_step--;
                    } while (scroll_step != 0);
                }
                scroll_target = (*(s16 *)(arg0 + 0xA));
                scroll_step = D_80122A00 * 0x10;
                if (scroll_step < scroll_target)
                {
                    (*(s16 *)(arg0 + 0xA)) = scroll_step;
                    (*(s16 *)(arg0 + 0xC)) = 4;
                    return;
                }
                window_height = ((u32) (*(s32 *)(arg0 + 0x4)) >> 1) & 0xFF;
                if ((window_height - 0x10) < (scroll_step - scroll_target))
                {
                    (*(s16 *)(arg0 + 0xA)) = (s16) (scroll_step - (window_height - 0x10));
                    (*(s16 *)(arg0 + 0xC)) = 4;
                }
            }
        }
    }
}

/**
 * @brief Bump a pad-slot counter and clear an actor slot's animation bits.
 *
 * Uses the D_800FDF58 entry for @p arg0 (stride 0x54) to index into the pad
 * context and increment a per-controller counter, dispatches func_800C2640 for
 * the actor slot's @c unk14 handle, marks the pad entry served (0xFF), and
 * clears the actor slot's @c unk18E byte.
 *
 * @param arg0 Actor/pad slot index.
 * @see decomp.me (100%) TODO
 */
void func_800AF824(s32 arg0)
{
    u8 *s1;
    u8 *v1;
    u8 *s0;
    u8 *pc = g_pad_ctx;
    u8 *base = D_800FDF58;

    s1 = base + arg0 * 0x54;
    v1 = pc + (s1[0x21] & 0x7F);
    v1[0x2640] = v1[0x2640] + 1;
    s0 = D_80105AE0 + arg0 * 0x23C;
    func_800C2640(*(s32 *)(s0 + 0x14), 0xFF);
    s1[0x25] = 0xFF;
    s0[0x18E] = 0;
}
