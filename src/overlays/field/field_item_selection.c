/**
 * @file field_item_selection.c
 * @brief The item-drop menu: pick an owned field item and place it next to the player.
 *
 * The menu lists every owned item from FIELD_DROP_ITEM_FIRST up to
 * FIELD_DROP_ITEM_LIMIT. Confirming spawns an item actor next to the
 * leader and takes one item out of the inventory; picking the actor up again
 * (field_pick_up_item_actor) puts it back.
 */

#include "main.h"
#include "common.h"
#include "field_scene_transition.h"
#include "field_actor_runtime.h"
#include "field_calls.h"
#include "sdk/libgpu.h"
#include "field_actor_tables.h"
#include "field_menu_element.h"

/** @brief First item id that can be dropped on the field; its actor animation is 0. */
#define FIELD_DROP_ITEM_FIRST 0x60
/** @brief One past the last item id that can be dropped on the field. */
#define FIELD_DROP_ITEM_LIMIT 0x90

/** @brief Buttons that drop the selected item. */
#define FIELD_ITEM_MENU_CONFIRM (PADi | PADRright)
/** @brief Button that closes the menu without dropping anything. */
#define FIELD_ITEM_MENU_CANCEL PADRdown
/** @brief Buttons that move the cursor (directions, L1 and R1). */
#define FIELD_ITEM_MENU_MOVE (PADLup | PADLdown | PADLleft | PADLright | PADL1 | PADR1)
/** @brief Buttons that move the cursor to the next row. */
#define FIELD_ITEM_MENU_NEXT (PADLdown | PADLright)
/** @brief Buttons that move the cursor to the previous row. */
#define FIELD_ITEM_MENU_PREVIOUS (PADLup | PADLleft)
/** @brief Rows skipped by one L1/R1 page step. */
#define FIELD_ITEM_MENU_PAGE_ROWS 10

/** @brief Window geometry of the item list, in pixels. */
#define FIELD_ITEM_WINDOW_X 44
#define FIELD_ITEM_WINDOW_Y 32
#define FIELD_ITEM_WINDOW_WIDTH 232
#define FIELD_ITEM_WINDOW_HEIGHT 160
/** @brief Height of one list row, in pixels. */
#define FIELD_ITEM_ROW_HEIGHT 16
/** @brief Top edge of list row @p row inside the window's content. */
#define FIELD_ITEM_ROW_Y(row) ((row) * FIELD_ITEM_ROW_HEIGHT)
/** @brief Left edge of the owned-count column, relative to the window. */
#define FIELD_ITEM_COUNT_X 202
/** @brief Frames the window takes to scroll to a new cursor row. */
#define FIELD_ITEM_SCROLL_TICKS 4

/** @brief Text color of the list rows. */
#define FIELD_ITEM_TEXT_COLOR 4
/** @brief Text alignment of the owned-count column. */
#define FIELD_ITEM_COUNT_ALIGN 1
/** @brief Color of the translucent cursor-row highlight. */
#define FIELD_ITEM_HIGHLIGHT_COLOR 0xF080F0
/** @brief Height of the cursor-row highlight, in pixels. */
#define FIELD_ITEM_HIGHLIGHT_HEIGHT 14

/** @brief Sounds of the item-drop menu. */
#define FIELD_SOUND_ACTION_REFUSED 0x78
#define FIELD_SOUND_CURSOR 0x7D
#define FIELD_SOUND_SELECT 0x7E
#define FIELD_SOUND_CANCEL 0x7F
#define FIELD_SOUND_WINDOW_OPEN 0xB9
#define FIELD_SOUND_PAN_CENTRE 0x80

/** @brief Initial held-button delay set when the menu flushes the pads. */
#define FIELD_PAD_REPEAT_DELAY 15

/** @brief Resource entry an item actor is initialized from. */
#define FIELD_ITEM_ACTOR_RESOURCE 4
/** @brief Object keys of item actors start here (key = actor index + base). */
#define FIELD_ITEM_ACTOR_KEY_BASE 0x14
/** @brief Initial FieldActor::removal_delay of a dropped item actor. */
#define FIELD_ITEM_ACTOR_REMOVAL_DELAY 2
/** @brief Control mode of a dropped item actor. */
#define FIELD_ITEM_ACTOR_CONTROL_MODE 2
/** @brief Built-in animation played when an item is dropped. */
#define FIELD_ITEM_DROP_ANIMATION 0xAF
/** @brief Item value that makes field_spawn_item_record release the record. */
#define FIELD_ITEM_RELEASE 0xFF

/**
 * @brief Text of an item name in g_field_item_names.
 * @param table Offset table (one halfword per item id) followed by the names.
 * @param item Item id.
 * @note Written as an integer sum so the offset stays the first addu operand.
 */
#define FIELD_ITEM_NAME(table, item) ((u8 *)((table)[item] + (u32)(table)))

/**
 * @brief Facing offset of one of the eight facings.
 * @param table Facing offset table.
 * @param facing Facing index, 0 to 7.
 * @note Written as an integer sum so the index stays the first addu operand.
 */
#define FIELD_FACING_OFFSET(table, facing) ((FieldFacingOffset *)((facing) * sizeof(FieldFacingOffset) + (u32)(table)))

/** @brief One row of the item-drop list. */
typedef struct FieldItemListEntry
{
    u8 item_id;
    u8 count;
} FieldItemListEntry;

/** @brief Entry of g_field_direction_offsets: the (dx, dz) offset of one of the eight facings. */
typedef struct FieldFacingOffset
{
    s16 dx;
    s16 dz;
} FieldFacingOffset;

/** @brief TILE whose color and code bytes are written as one word. */
typedef struct FieldPackedTile
{
    u32 tag;
    u32 color;
    s16 x, y, w, h;
} FieldPackedTile;

extern FieldFacingOffset g_field_direction_offsets[];
extern u16 g_field_item_names[];
extern FieldItemListEntry g_field_item_list[];
extern s32 g_field_item_list_count;
extern s32 g_field_item_list_cursor;
extern s32 D_80122714;
extern s32 g_field_primary_held_buttons, g_field_primary_repeat_delay;
extern s32 g_field_secondary_held_buttons, g_field_secondary_repeat_delay, g_field_buffered_input;

void field_restart_actor_animation(FieldActor *actor);
s32 field_read_controller_buttons(s32 controller);
void *field_draw_text(void *cursor, u32 *ot, u8 *text, s32 color, s32 x, s32 y, s32 flags);
void field_format_number(u8 *text, s32 value, s32 wide);

static void field_play_action_refused_sound(void);
static u8 *field_draw_item_drop_list(u32 *ot, u8 *cursor, s32 scroll_x, s32 scroll_y, s32 unused, void *element);
static s32 field_update_item_drop_menu(FieldMenuElement *window);

/**
 * @brief Clear delivered input and restart both pads' held-button delays.
 * @note Same as field_reset_input_repeat, expanded inline.
 */
static inline void flush_input(void)
{
    g_pad_input = 0;
    g_field_primary_held_buttons = field_read_controller_buttons(0);
    g_field_primary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    g_pad_input_inject = 0;
    g_field_secondary_held_buttons = field_read_controller_buttons(1);
    g_field_secondary_repeat_delay = FIELD_PAD_REPEAT_DELAY;
    g_field_buffered_input = 0;
}

/**
 * @brief Close every menu element at once.
 * @note Same as field_reset_menu_elements, expanded inline.
 */
static inline void reset_menu_elements(void)
{
    FieldMenuElement *element;
    s32 i;

    g_menu_element_counter = 0;
    element = g_field_menu_elements;
    for (i = 0; i < FIELD_MENU_ELEMENT_COUNT; i++)
    {
        element->attr.word &= ~FIELD_MENU_STATE_MASK;
        element++;
    }
}

/**
 * @brief Claim the first idle menu element and put it in the opening state.
 * @return The claimed element, or the first element when none is idle.
 * @note Same as field_claim_menu_element, expanded inline.
 */
static inline FieldMenuElement *claim_menu_element(void)
{
    FieldMenuElement *element;
    s32 i;

    element = g_field_menu_elements;
    for (i = 0; i < FIELD_MENU_ELEMENT_COUNT; i++, element++)
    {
        if ((element->attr.word & FIELD_MENU_STATE_MASK) == FIELD_MENU_STATE_IDLE)
        {
            element->attr.bits.state = FIELD_MENU_STATE_OPENING;
            element->scroll = 0;
            element->scroll_target = 0;
            element->size.word &= ~FIELD_MENU_SIZE_BLINK;
            element->size.word &= ~FIELD_MENU_SIZE_SCROLL_MODE;
            element->size.fields.content_height = 0;
            element->scroll_ticks = 0;
            return element;
        }
    }
    return g_field_menu_elements;
}

/**
 * @brief Format a value and draw it with field_draw_text.
 * @param cursor Primitive buffer cursor.
 * @param ot Ordering table.
 * @param value Value to format.
 * @param position Screen position (x, y).
 * @param color Text color selector.
 * @param flags Text alignment flags.
 * @return Cursor after the text primitives.
 * @note Same as field_draw_number, expanded inline.
 */
static inline u8 *draw_number(u8 *cursor, u32 *ot, s32 value, s16 *position, s32 color, s32 flags)
{
    u8 text[64];

    field_format_number(text, value, 0);
    return field_draw_text(cursor, ot, text, color, position[0], position[1], flags);
}

/** @brief Build the list of droppable items the player owns and open the item-drop menu. */
void field_open_item_drop_menu(void)
{
    FieldMenuElement *window;
    u32 attr;
    s32 item_count;
    s32 item_id;
    s32 cursor_y;

    reset_menu_elements();
    item_count = 0;
    for (item_id = FIELD_DROP_ITEM_FIRST; item_id < FIELD_DROP_ITEM_LIMIT; item_id++)
    {
        if (g_pad_ctx->item_counts[item_id] != 0)
        {
            g_field_item_list[item_count].item_id = item_id;
            g_field_item_list[item_count].count = g_pad_ctx->item_counts[item_id];
            item_count++;
        }
    }
    g_field_item_list_count = item_count;
    if (item_count == 0)
    {
        field_play_action_refused_sound();
        return;
    }

    D_80122714 = 1;
    field_play_sound(FIELD_SOUND_WINDOW_OPEN, FIELD_SOUND_PAN_CENTRE);
    window = claim_menu_element();
    window->draw = (FieldMenuDrawFn)field_draw_item_drop_list;
    window->scroll = 0;
    window->attr.bits.step = 1;
    window->attr.bits.x = FIELD_ITEM_WINDOW_X;
    window->attr.bits.y = FIELD_ITEM_WINDOW_Y;
    window->size.bits.width_high = 0;
    window->size.bits.height = FIELD_ITEM_WINDOW_HEIGHT;
    attr = window->attr.word;
    attr &= ~FIELD_MENU_ATTR_WIDTH_LOW;
    attr |= FIELD_ITEM_WINDOW_WIDTH << FIELD_MENU_ATTR_WIDTH_LOW_SHIFT;
    window->attr.word = attr;
    window->size.bits.scroll_mode = FIELD_MENU_SCROLL_BY_OWNER;
    window->size.fields.content_height = g_field_item_list_count * FIELD_ITEM_ROW_HEIGHT;
    if (g_field_item_list_cursor >= g_field_item_list_count)
    {
        g_field_item_list_cursor = g_field_item_list_count - 1;
    }
    cursor_y = FIELD_ITEM_ROW_Y(g_field_item_list_cursor);
    window->scroll_target = 0;
    if (cursor_y < 0)
    {
        window->scroll_target = cursor_y;
    }
    else if ((window->size.bits.height - FIELD_ITEM_ROW_HEIGHT) < cursor_y)
    {
        window->scroll_target = cursor_y - (window->size.bits.height - FIELD_ITEM_ROW_HEIGHT);
    }
    window->scroll_ticks = 0;
    flush_input();
}

/** @brief Play the sound for an action that cannot be taken. */
static void field_play_action_refused_sound(void)
{
    field_play_sound(FIELD_SOUND_ACTION_REFUSED, FIELD_SOUND_PAN_CENTRE);
}

/**
 * @brief Draw callback of the item-drop menu: item rows, owned counts and the cursor highlight.
 * @param ot Ordering table.
 * @param cursor Primitive buffer cursor.
 * @param scroll_x Horizontal scroll offset.
 * @param scroll_y Vertical scroll offset.
 * @param unused Visible height passed by the menu code; not used.
 * @param element Menu element of the item list.
 * @return Cursor after the list, the highlight tile and its draw-mode packet.
 */
static u8 *field_draw_item_drop_list(u32 *ot, u8 *cursor, s32 scroll_x, s32 scroll_y, s32 unused, void *element)
{
    s16 position[2];
    FieldItemListEntry *entry;
    s32 count_x;
    s32 y;
    s32 i;
    FieldPackedTile *tile;
    DR_TPAGE *mode;
    u16 *names;
    FieldMenuElement *window;

    window = element;
    field_update_item_drop_menu(window);
    if (D_80122714 == 0)
    {
        return cursor;
    }

    for (i = 0; i < g_field_item_list_count; i++)
    {
        /* Read inside the loop; a table pointer set before the loop allocates differently. */
        names = g_field_item_names;
        count_x = FIELD_ITEM_COUNT_X - scroll_x;
        entry = &g_field_item_list[i];
        y = FIELD_ITEM_ROW_Y(i) - scroll_y;
        if (y > -FIELD_ITEM_ROW_HEIGHT && y < (s32)window->size.bits.height)
        {
            cursor = field_draw_text(cursor, ot, FIELD_ITEM_NAME(names, entry->item_id), FIELD_ITEM_TEXT_COLOR, -scroll_x, y, 0);
            position[1] = y;
            position[0] = count_x;
            cursor = draw_number(cursor, ot, entry->count, position, FIELD_ITEM_TEXT_COLOR, FIELD_ITEM_COUNT_ALIGN);
        }
    }
    tile = (FieldPackedTile *)cursor;
    tile->color = FIELD_ITEM_HIGHLIGHT_COLOR;
    setTile(tile);
    setSemiTrans(tile, 1);
    tile->w = FIELD_ITEM_WINDOW_WIDTH;
    mode = (DR_TPAGE *)(tile + 1);
    tile->x = 0;
    tile->h = FIELD_ITEM_HIGHLIGHT_HEIGHT;
    tile->y = FIELD_ITEM_ROW_Y(g_field_item_list_cursor) - scroll_y;
    addPrim(ot, tile);
    setDrawTPage(mode, 0, 0, getTPage(0, 0, 320, 0));
    addPrim(ot, mode);
    return (u8 *)(mode + 1);
}

/**
 * @brief Drop the selected item: spawn its actor next to the leader and play the drop animation.
 */
static inline void drop_selected_item(void)
{
    FieldActor *actor;
    FieldObjectState *state;
    FieldFacingOffset *offsets;
    FieldFacingOffset *facing;
    s32 offset[3];
    s32 actor_index;
    s32 control;
    s32 item_id;
    s32 leader_animation;
    s32 animation_slot;

    /* Initialized in this order outside the for statement; folding them into it reorders the setup. */
    actor_index = FIELD_ACTOR_COUNT - 1;
    offsets = g_field_direction_offsets;
    for (; actor_index >= FIELD_PARTY_COUNT; actor_index--)
    {
        actor = &g_field_actors[actor_index];
        if (actor->presence == FIELD_ACTOR_UNUSED)
        {
            field_initialize_actor_record(actor_index, FIELD_ITEM_ACTOR_RESOURCE);
            actor->x = g_field_actors[0].x;
            actor->y = g_field_actors[0].y;
            actor->z = g_field_actors[0].z;
            facing = FIELD_FACING_OFFSET(offsets, g_field_actors[0].direction >> 5);
            offset[0] = -facing->dx;
            offset[1] = 0;
            offset[2] = -facing->dz;
            field_move_actor_position(actor, offset);
            control = actor->control.word;
            state = &g_field_object_states[actor_index];
            actor->presence = FIELD_ACTOR_HIDDEN;
            item_id = g_field_item_list[g_field_item_list_cursor].item_id;
            leader_animation = g_field_actors[0].animation;
            control &= ~FIELD_CONTROL_MODE_MASK;
            control |= FIELD_ITEM_ACTOR_CONTROL_MODE;
            actor->animation_frame = 0;
            actor->animation_active = 1;
            actor->command = FIELD_ACTOR_COMMAND_BA;
            actor->removal_delay = FIELD_ITEM_ACTOR_REMOVAL_DELAY;
            actor->animation_state = 0xFE;
            actor->control.word = control;
            actor->script_index = 0;
            actor->unk10 = 1;
            actor->animation = (item_id - FIELD_DROP_ITEM_FIRST) | (leader_animation & FIELD_ANIMATION_FACING);
            state->key = actor_index + FIELD_ITEM_ACTOR_KEY_BASE;
            state->interaction_kind = FIELD_INTERACTION_ITEM;
            state->unk18 = 0;
            field_restart_actor_animation(actor);
            field_spawn_item_record(state->key, g_field_item_list[g_field_item_list_cursor].item_id);
            animation_slot = field_find_free_actor_slot(actor_index, 0);
            if ((animation_slot != -1) && (field_start_builtin_animation(actor_index, animation_slot, FIELD_ITEM_DROP_ANIMATION) != 0))
            {
                field_start_actor_animation(animation_slot, 0, NULL);
            }
            break;
        }
    }
}

/**
 * @brief Handle the item-drop menu's input: drop the selected item, cancel, or move the cursor.
 * @param window Menu element of the item list.
 * @return Undefined; the function is int-typed but never sets a value and callers ignore it.
 */
static s32 field_update_item_drop_menu(FieldMenuElement *window)
{
    s32 scroll_target;
    u16 window_height;
    s32 steps;

    if (((window->attr.word & FIELD_MENU_STATE_MASK) != FIELD_MENU_STATE_OPEN) || (window->scroll_ticks != 0))
    {
        return;
    }

    if (g_field_buffered_input & FIELD_ITEM_MENU_CONFIRM)
    {
        flush_input();
        field_play_sound(FIELD_SOUND_SELECT, FIELD_SOUND_PAN_CENTRE);
        reset_menu_elements();
        D_80122714 = 0;
        g_pad_ctx->item_counts[g_field_item_list[g_field_item_list_cursor].item_id]--;
        drop_selected_item();
        return;
    }
    if (g_field_buffered_input & FIELD_ITEM_MENU_CANCEL)
    {
        flush_input();
        reset_menu_elements();
        field_play_sound(FIELD_SOUND_CANCEL, FIELD_SOUND_PAN_CENTRE);
        D_80122714 = 0;
        return;
    }
    if (!(g_field_buffered_input & FIELD_ITEM_MENU_MOVE))
    {
        return;
    }

    field_play_sound(FIELD_SOUND_CURSOR, FIELD_SOUND_PAN_CENTRE);
    steps = 1;
    if (g_field_buffered_input & PADR1)
    {
        steps = FIELD_ITEM_MENU_PAGE_ROWS;
        g_field_buffered_input = PADLdown;
    }
    if (g_field_buffered_input & PADL1)
    {
        steps = FIELD_ITEM_MENU_PAGE_ROWS;
        g_field_buffered_input = PADLup;
    }
    while (steps != 0)
    {
        if (g_field_buffered_input & FIELD_ITEM_MENU_NEXT)
        {
            g_field_item_list_cursor++;
            if (g_field_item_list_cursor >= g_field_item_list_count)
            {
                g_field_item_list_cursor = 0;
            }
        }
        if (g_field_buffered_input & FIELD_ITEM_MENU_PREVIOUS)
        {
            g_field_item_list_cursor--;
            if (g_field_item_list_cursor < 0)
            {
                g_field_item_list_cursor = g_field_item_list_count - 1;
            }
        }
        /* A page step stops at either end of the list. */
        if ((g_field_item_list_cursor == g_field_item_list_count - 1) || (g_field_item_list_cursor == 0))
        {
            steps = 1;
        }
        steps--;
    }

    scroll_target = window->scroll_target;
    if (FIELD_ITEM_ROW_Y(g_field_item_list_cursor) < scroll_target)
    {
        window->scroll_target = FIELD_ITEM_ROW_Y(g_field_item_list_cursor);
        window->scroll_ticks = FIELD_ITEM_SCROLL_TICKS;
        return;
    }
    window_height = window->size.bits.height;
    if ((window_height - FIELD_ITEM_ROW_HEIGHT) < (FIELD_ITEM_ROW_Y(g_field_item_list_cursor) - scroll_target))
    {
        window->scroll_target = FIELD_ITEM_ROW_Y(g_field_item_list_cursor) - (window_height - FIELD_ITEM_ROW_HEIGHT);
        window->scroll_ticks = FIELD_ITEM_SCROLL_TICKS;
    }
}

/**
 * @brief Pick an item actor up again: return its item to the inventory and free the actor.
 * @param actor_index Field actor and object index of the item actor.
 */
void field_pick_up_item_actor(s32 actor_index)
{
    FieldActor *actor;
    FieldObjectState *state;
    PadContext *save = g_pad_ctx;
    FieldActor *actors = g_field_actors;

    actor = &actors[actor_index];
    save->item_counts[FIELD_DROP_ITEM_FIRST + (actor->animation & FIELD_ANIMATION_INDEX_MASK)]++;
    state = &g_field_object_states[actor_index];
    field_spawn_item_record(state->key, FIELD_ITEM_RELEASE);
    actor->presence = FIELD_ACTOR_UNUSED;
    state->interaction_kind = 0;
}
