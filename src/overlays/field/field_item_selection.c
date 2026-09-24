#include "field_scene_transition.h"
#include "common.h"
#include "sdk/libgpu.h"
#include "field_actor_tables.h"
extern u8 g_field_direction_offsets[];
extern u8 D_80122738[];
extern u8 *g_pad_ctx;
extern s32 D_80122714, D_80122734, g_field_primary_held_buttons, g_field_primary_repeat_delay;
extern s32 g_field_secondary_held_buttons, g_field_secondary_repeat_delay, D_80122828, g_field_buffered_input, D_80122A00;
extern s32 g_menu_element_counter, g_pad_input, g_pad_input_inject;
void field_start_actor_animation(s32, s32, s32);
void field_initialize_actor_record(s32, s32);
void field_restart_actor_animation(FieldActor *);
s32 func_800839F8(s32, s32);
s32 func_80083EEC(s32, s32, s32);

void func_800A3938(s32, s32);
s32 field_read_controller_buttons(s32);
void func_800C2640(s32, s32);

struct ItemWindowSlot;
u8 *func_800AF0E8(u32 *, u8 *, s32, s32, s32, struct ItemWindowSlot *);
void func_800AF0C4(void);


/**
 * @brief Address of the (dx, dz) offset pair for one of the eight facings.
 * @param table Direction offset table, four bytes per facing.
 * @param facing Facing index, 0 to 7.
 * @note Written as an integer sum so the index stays the first addu operand.
 */
#define FIELD_DIRECTION_OFFSET(table, facing) ((s16 *)(((facing) * 4) + (s32)(table)))

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
typedef struct ItemWindowSlot
{
    ItemWindowFlags flags;
    ItemWindowState state;
    s16 scroll;
    s16 scroll_target;
    s16 scroll_ticks;
    u16 padding;
    u8 *(*draw_callback)(u32 *, u8 *, s32, s32, s32, struct ItemWindowSlot *);
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

    /* Open-coded func_800ADF84 allocator; as a loop or inline helper it is 98.59%. */
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
    g_field_primary_held_buttons = field_read_controller_buttons(0);
    g_field_primary_repeat_delay = 0xF;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = 0xF;
    g_field_buffered_input = 0;
}

/**
 * @brief Thin stack-frame wrapper around func_800A3938 with fixed args.
 */
void func_800AF0C4(void)
{
    func_800A3938(0x78, 0x80);
}

/** Sixteen-byte GPU tile packet, including its ordering-table tag. */
typedef struct Tile
{
    u32 tag;
    u32 color;
    s16 x, y, w, h;
} Tile;
extern u8 D_800EE72C[];
extern u8 *func_800A88A0(u8 *, u32 *, u8 *, s32, s32, s32, s32);
extern void func_800A8B90(u8 *, s32, s32);
extern s32 func_800AF350(ItemWindowSlot *);
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
u8 *func_800AF0E8(u32 *ot, u8 *cursor, s32 scroll_x, s32 scroll_y, s32 unused, ItemWindowSlot *window)
{
    u16 point[4];
    ItemWindowSlot *draw_window;
    s32 number_x;
    u8 *entry;
    u8 *names;
    s32 y;
    s32 index;
    u8 *mode;
    Tile *tile;

    draw_window = window;
    func_800AF350(draw_window);
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
        if (y >= -15 && y < (s32)draw_window->state.bits.value)
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
 * @param window Item-selection window slot being updated.
 * @return Nothing meaningful; the original declares an int return and never sets it.
 * @see decomp.me (100%)
 */
s32 func_800AF350(ItemWindowSlot *window)
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
    s16 *direction;
    u8 *entry;
    FieldActor *actor;
    FieldObjectState *state;
    u8 *direction_table;
    u8 *items;

    if (((window->flags.word & 7) == 2) && (window->scroll_ticks == 0))
    {
        if (g_field_buffered_input & 0x220)
        {
            g_pad_input = 0;
            g_field_primary_held_buttons = field_read_controller_buttons(0);
            g_field_primary_repeat_delay = 0xF;
            g_pad_input_inject = 0;
            g_field_secondary_held_buttons = field_read_controller_buttons(1);
            g_field_secondary_repeat_delay = 0xF;
            g_field_buffered_input = 0;
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
            direction_table = g_field_direction_offsets;
            D_80122714 = 0;
            entry = g_pad_ctx + D_80122738[D_80122A00 * 2];
            entry[0x25E0]--;
            do
            {
                actor = &g_field_actors[actor_index];
                items = D_80122738;
                if (actor->presence == 0xFF)
                {
                    field_initialize_actor_record(actor_index, 4);
                    actor->x = g_field_actors[0].x;
                    actor->y = g_field_actors[0].y;
                    actor->z = g_field_actors[0].z;
                    direction = FIELD_DIRECTION_OFFSET(direction_table, g_field_actors[0].unk1B >> 5);
                    offset[0] = -direction[0];
                    offset[1] = 0;
                    offset[2] = -direction[1];
                    field_move_actor_position(actor, offset);
                    actor_flags = actor->control.word;
                    state = &g_field_object_states[actor_index];
                    actor->presence = 0xFE;
                    selected_item = items[D_80122A00 * 2];
                    actor_direction = g_field_actors[0].animation;
                    actor_flags &= ~0x1FF;
                    actor_flags |= 2;
                    actor->unk27 = 0;
                    actor->unk24 = 1;
                    actor->command = 0xBA;
                    actor->unk3D = 2;
                    actor->unk2E = 0xFE;
                    actor->control.word = actor_flags;
                    actor->script_index = 0;
                    actor->unk10 = 1;
                    actor->animation = (selected_item - 0x60) | (actor_direction & 0x80);
                    state->key = actor_index + 0x14;
                    state->unk18E = 1;
                    state->unk18 = 0;
                    field_restart_actor_animation(actor);
                    func_800C2640(state->key, items[D_80122A00 * 2]);
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
            if (g_field_buffered_input & 0x40)
            {
                g_pad_input = 0;
                g_field_primary_held_buttons = field_read_controller_buttons(0);
                g_field_primary_repeat_delay = 0xF;
                g_pad_input_inject = 0;
                g_field_secondary_held_buttons = field_read_controller_buttons(1);
                cancel_flags = &D_80122828;
                cancel_index = 0;
                g_field_secondary_repeat_delay = 0xF;
                g_field_buffered_input = 0;
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
            if (g_field_buffered_input & 0xF00C)
            {
                func_800A3938(0x7D, 0x80);
                scroll_step = 1;
                if (g_field_buffered_input & 8)
                {
                    scroll_step = 0xA;
                    g_field_buffered_input = 0x4000;
                }
                if (g_field_buffered_input & 4)
                {
                    scroll_step = 0xA;
                    g_field_buffered_input = 0x1000;
                }
                if (scroll_step != 0)
                {
                    do
                    {
                        if (g_field_buffered_input & 0x6000)
                        {
                            next_selection = D_80122A00 + 1;
                            D_80122A00 = next_selection;
                            if (next_selection >= D_80122734)
                            {
                                D_80122A00 = 0;
                            }
                        }
                        if (g_field_buffered_input & 0x9000)
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
                scroll_target = window->scroll_target;
                scroll_step = D_80122A00 * 0x10;
                if (scroll_step < scroll_target)
                {
                    window->scroll_target = scroll_step;
                    window->scroll_ticks = 4;
                    return;
                }
                window_height = window->state.bits.value;
                if ((window_height - 0x10) < (scroll_step - scroll_target))
                {
                    window->scroll_target = (s16) (scroll_step - (window_height - 0x10));
                    window->scroll_ticks = 4;
                }
            }
        }
    }
}

/**
 * @brief Return an item actor's item to the inventory and free the actor.
 *
 * Increments the inventory count of the item the actor shows (its animation
 * index is the item id minus 0x60), passes 0xFF for the object's key to
 * func_800C2640 (func_800AF350 passes the item id there), marks the actor
 * absent (0xFF) and clears the object's unk18E byte.
 *
 * @param actor_index Field actor and object index.
 * @see decomp.me (100%) TODO
 */
void func_800AF824(s32 actor_index)
{
    FieldActor *actor;
    u8 *pad_entry;
    FieldObjectState *state;
    u8 *pad_ctx = g_pad_ctx;
    FieldActor *actors = g_field_actors;

    actor = &actors[actor_index];
    pad_entry = pad_ctx + (actor->animation & 0x7F);
    pad_entry[0x2640] = pad_entry[0x2640] + 1;
    state = &g_field_object_states[actor_index];
    func_800C2640(state->key, 0xFF);
    actor->presence = 0xFF;
    state->unk18E = 0;
}
