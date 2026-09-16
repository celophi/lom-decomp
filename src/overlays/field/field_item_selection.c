#include "common.h"
extern u8 D_800EB254[];
extern u8 D_800FDF58[];
extern u8 D_80105AE0[];
extern u8 D_80122738[];
extern u8 *g_pad_ctx;
extern s32 D_80122714, D_80122734, D_801227BC, D_801227C0;
extern s32 D_801227D8, D_801227E4, D_80122828, D_801229F8, D_80122A00;
extern s32 g_menu_element_counter, g_pad_input, g_pad_input_inject;
void field_start_actor_animation(s32, s32, s32);
void func_8006B4D0(s32, s32);
void func_8006C3FC(u8 *);
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
extern void func_800AF350(u8 *);
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
    cursor[3] = 3;
    cursor[7] = 0x62;
    tile->w = 0xE8;
    mode = cursor + 16;
    tile->x = 0;
    tile->h = 14;
    tile->y = D_80122A00 * 16 - scroll_y;
    tile->tag = (tile->tag & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((u32)cursor & 0xFFFFFF);
    mode[3] = 1;
    *(u32 *)(mode + 4) = 0xE1000005;
    *(u32 *)(cursor + 16) = (*(u32 *)(cursor + 16) & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | ((u32)mode & 0xFFFFFF);
    return cursor + 24;
}

/**
 * @brief Process item selection, cancellation, scrolling, and actor creation.
 * @param arg0 Window state with packed flags, clipping height, and scroll state.
 */
void func_800AF350(u8 *arg0)
{
    s32 offset[3];
    s16 temp_a0;
    s16 temp_v1_2;
    s32 *var_a0_2;
    s32 *var_v1;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 temp_a2;
    s32 temp_a2_2;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 temp_v0_4;
    s32 var_a0;
    s32 var_a0_3;
    s32 var_a1;
    s32 var_s1;
    s32 var_s4;
    u8 *temp_s0;
    u8 *temp_v0;
    u8 *temp_v1;
    u8 *var_s0;

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
            var_v1 = &D_80122828;
            var_a0 = 0;
            g_menu_element_counter = 0;
            do
            {
                var_a0 += 1;
                *var_v1 &= ~7;
                var_v1 += 5;
            } while (var_a0 < 8);
            var_s1 = 0xC;
            var_s4 = 0x1AD0;
            D_80122714 = 0;
            temp_v1 = g_pad_ctx + D_80122738[D_80122A00 * 2];
            var_s0 = D_800FDF58 + 0x3F0;
            (*(u8 *)(temp_v1 + 0x25E0)) = (u8) ((*(u8 *)(temp_v1 + 0x25E0)) - 1);
loop_6:
            if ((*(u8 *)(var_s0 + 0x25)) == 0xFF)
            {
                func_8006B4D0(var_s1, 4);
                (*(s32 *)(var_s0 + 0x0)) = (s32) (*(s32 *)(D_800FDF58 + 0x0));
                (*(s32 *)(var_s0 + 0x4)) = (s32) (*(s32 *)(D_800FDF58 + 0x4));
                (*(s32 *)(var_s0 + 0x8)) = (s32) (*(s32 *)(D_800FDF58 + 0x8));
                temp_v0 = (((u8) (*(u8 *)(D_800FDF58 + 0x1B)) >> 5) * 4) + D_800EB254;
                offset[1] = 0;
                offset[0] = (s32) -(*(s16 *)(temp_v0 + 0x0));
                offset[2] = (s32) -(*(s16 *)(temp_v0 + 0x2));
                func_8009C2E0(var_s0, offset);
                temp_s0 = var_s4 + D_80105AE0;
                (*(u8 *)(var_s0 + 0x25)) = 0xFEU;
                temp_a1 = ((*(s32 *)(var_s0 + 0x1C)) & ~0x1FF) | 2;
                (*(s16 *)(var_s0 + 0x2A)) = 0xBA;
                (*(u8 *)(var_s0 + 0x3D)) = 2;
                temp_a2 = (*(u8 *)(D_800FDF58 + 0x21)) & 0x80;
                (*(u16 *)(var_s0 + 0x2E)) = 0xFE;
                (*(u8 *)(var_s0 + 0x27)) = 0;
                (*(u8 *)(var_s0 + 0x24)) = 1;
                (*(s32 *)(var_s0 + 0x1C)) = temp_a1;
                (*(u8 *)(var_s0 + 0x28)) = 0;
                (*(s16 *)(var_s0 + 0x10)) = 1;
                (*(u8 *)(var_s0 + 0x21)) = (s8) ((D_80122738[D_80122A00 * 2] - 0x60) | temp_a2);
                (*(s32 *)(temp_s0 + 0x14)) = (s32) (var_s1 + 0x14);
                (*(u8 *)(temp_s0 + 0x18E)) = 1;
                (*(s16 *)(temp_s0 + 0x18)) = 0;
                func_8006C3FC(var_s0);
                func_800C2640((*(s32 *)(temp_s0 + 0x14)), D_80122738[D_80122A00 * 2]);
                temp_v0_2 = func_800839F8(var_s1, 0);
                if ((temp_v0_2 != -1) && (func_80083EEC(var_s1, temp_v0_2, 0xAF) != 0))
                {
                    field_start_actor_animation(temp_v0_2, 0, 0);
                }
            }
            else
            {
                var_s4 -= 0x23C;
                var_s1 -= 1;
                var_s0 -= 0x54;
                if (var_s1 < 3)
                {
                    return;
                }
                goto loop_6;
            }
        }
        else
        {
            if (D_801229F8 & 0x40)
            {
                g_pad_input = 0;
                D_801227BC = func_800A9D70(0);
                D_801227C0 = 0xF;
                g_pad_input_inject = 0;
                var_a0_2 = &D_80122828;
                var_a1 = 0;
                D_801227D8 = func_800A9D70(1);
                D_801227E4 = 0xF;
                D_801229F8 = 0;
                g_menu_element_counter = 0;
                do
                {
                    var_a1 += 1;
                    *var_a0_2 &= ~7;
                    var_a0_2 += 5;
                } while (var_a1 < 8);
                func_800A3938(0x7F, 0x80);
                D_80122714 = 0;
                return;
            }
            if (D_801229F8 & 0xF00C)
            {
                func_800A3938(0x7D, 0x80);
                var_a0_3 = 1;
                if (D_801229F8 & 8)
                {
                    var_a0_3 = 0xA;
                    D_801229F8 = 0x4000;
                }
                if (D_801229F8 & 4)
                {
                    var_a0_3 = 0xA;
                    D_801229F8 = 0x1000;
                }
                if (var_a0_3 != 0)
                {
                    temp_a2_2 = D_80122734 - 1;
                    do
                    {
                        if (D_801229F8 & 0x6000)
                        {
                            temp_v0_3 = D_80122A00 + 1;
                            D_80122A00 = temp_v0_3;
                            if (temp_v0_3 >= D_80122734)
                            {
                                D_80122A00 = 0;
                            }
                        }
                        if (D_801229F8 & 0x9000)
                        {
                            temp_v0_4 = D_80122A00 - 1;
                            D_80122A00 = temp_v0_4;
                            if (temp_v0_4 < 0)
                            {
                                D_80122A00 = temp_a2_2;
                            }
                        }
                        if ((D_80122A00 == temp_a2_2) || (var_a0_3 -= 1, (D_80122A00 == 0)))
                        {
                            var_a0_3 = 0;
                        }
                    } while (var_a0_3 != 0);
                }
                temp_v1_2 = (*(s16 *)(arg0 + 0xA));
                temp_a0 = D_80122A00 * 0x10;
                if (temp_a0 < temp_v1_2)
                {
                    (*(s16 *)(arg0 + 0xA)) = temp_a0;
                    goto block_37;
                }
                temp_a1_2 = ((u32) (*(s32 *)(arg0 + 0x4)) >> 1) & 0xFF;
                if ((temp_a1_2 - 0x10) < (temp_a0 - temp_v1_2))
                {
                    (*(s16 *)(arg0 + 0xA)) = (s16) (temp_a0 - (temp_a1_2 - 0x10));
block_37:
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
