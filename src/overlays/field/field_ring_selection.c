/**
 * @file field_ring_selection.c
 * @brief Ring menu (icons on a rotating ellipse), party script pages, golem palettes
 *        and the framebuffer thumbnail.
 */

#include "common.h"
#include "cdrom.h"
#include "display.h"
#include "main.h"
#include "menu.h"
#include "pad.h"
#include "field_actor_tables.h"
#include "field_calls.h"
#include "field_effect_render_state.h"
#include "field_records.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/memory.h"

/** @brief Ring menu states (g_field_ring_menu_state). */
enum
{
    FIELD_RING_CLOSED = 0,
    FIELD_RING_SELECTING = 1,
    FIELD_RING_OPENING = 2,
    FIELD_RING_CLOSING = 3
};

/** @brief Ring menu placement (position_mode of field_open_ring_menu). */
enum
{
    FIELD_RING_AT_SCREEN_CENTER = 0,
    FIELD_RING_AT_PLAYER = 1
};

/** @brief CD resource id of ring menu 0's icon sheet; ring menu n uses the id plus n. */
#define FIELD_RING_ICON_RESOURCE_BASE 0xBE8
/** @brief VRAM position of the icon sheet (a 4-bit texture page). */
#define FIELD_RING_IMAGE_X SCREEN_WIDTH
#define FIELD_RING_IMAGE_Y 0
/** @brief Width in texels of the 4-bit icon sheet page. */
#define FIELD_RING_PAGE_WIDTH 256
/** @brief Colors in a 4-bit CLUT; each ring icon has its own, in one VRAM row from x = 0. */
#define FIELD_CLUT_COLORS 16

/** @brief Screen center used by FIELD_RING_AT_SCREEN_CENTER and as the player offset. */
#define FIELD_RING_CENTER_X (SCREEN_WIDTH / 2)
#define FIELD_RING_CENTER_Y (VRAM_DRAW_HEIGHT / 2)

/** @brief Radius the ring opens from. */
#define FIELD_RING_START_RADIUS 256
/** @brief Frames the radius takes to ease to its target while opening. */
#define FIELD_RING_RADIUS_EASE_STEPS 32
/** @brief Frames the opening spin takes at full, half and quarter speed. */
#define FIELD_RING_SPIN_FAST_STEPS 8
#define FIELD_RING_SPIN_MEDIUM_STEPS 16
#define FIELD_RING_SPIN_STEPS 32
/** @brief Frames the ring takes to collapse when closing. */
#define FIELD_RING_CLOSE_STEPS 16
/** @brief Frames a one-entry rotation takes. */
#define FIELD_RING_ROTATE_STEPS 10
/** @brief Icon brightness once the ring is open (0x80 = unmodulated texture). */
#define FIELD_RING_FULL_BRIGHTNESS 0x80
/** @brief Brightness change per frame while opening and closing. */
#define FIELD_RING_OPEN_FADE_STEP 4
#define FIELD_RING_CLOSE_FADE_STEP 8

/** @brief cancel_index of a ring menu that cannot be cancelled. */
#define FIELD_RING_NO_CANCEL -1

/** @brief Ordering-table slot of the icon at the front of the ring; icons further back go deeper. */
#define FIELD_RING_OT_FRONT 16

/** @brief Buttons that confirm, cancel and rotate the ring. */
#define FIELD_RING_CONFIRM_BUTTONS (PAD_BTN_CROSS | PAD_BTN_L3)
#define FIELD_RING_CANCEL_BUTTONS PAD_BTN_CIRCLE
#define FIELD_RING_PREVIOUS_BUTTONS (PAD_BTN_LEFT | PAD_BTN_UP | PAD_BTN_R1)
#define FIELD_RING_NEXT_BUTTONS (PAD_BTN_DOWN | PAD_BTN_RIGHT | PAD_BTN_L1)

/**
 * @brief Scale @p value by the depth of ring angle @p angle.
 * @note Gives @p value at the front of the ring and half of it at the back.
 */
#define FIELD_RING_DEPTH_SCALE(value, angle) ((value) + ((((value) >> 1) * (rcos(angle) - ONE)) >> 13))
/** @brief Ordering-table depth of ring angle @p angle: 0 at the front, -128 at the back. */
#define FIELD_RING_DEPTH(angle) ((rcos(angle) - ONE) / 64)

/** @brief Palettes per golem palette bank; a palette index selects bank index / 16. */
#define FIELD_GOLEM_BANK_PALETTES 16
/** @brief VRAM position of the golem CLUTs: one row per golem. */
#define FIELD_GOLEM_CLUT_X 256
#define FIELD_GOLEM_CLUT_Y 496

/** @brief Thumbnail strips: one per 64-texel column of the 320-pixel framebuffer. */
#define FIELD_THUMBNAIL_STRIPS 5
/** @brief Width of one thumbnail strip on screen (a quarter of the 64-texel column). */
#define FIELD_THUMBNAIL_STRIP_WIDTH 16
/** @brief Texture pages of framebuffer rows 0-255 and 256-511 at x = 0 (15-bit direct); OR in the 64-texel page column. */
#define FIELD_THUMBNAIL_TPAGE_TOP getTPage(2, 1, 0, 0)
#define FIELD_THUMBNAIL_TPAGE_BOTTOM getTPage(2, 1, 0, 256)
/** @brief Unmodulated grey of the thumbnail quads (r = g = b = 0x80). */
#define FIELD_THUMBNAIL_GREY 0x808080

/** @brief POLY_FT4 with its color and command byte addressed as one word. */
typedef struct
{
    u_long tag;
    u_long rgbc;
    short x0, y0;
    u_char u0, v0;
    u_short clut;
    short x1, y1;
    u_char u1, v1;
    u_short tpage;
    short x2, y2;
    u_char u2, v2;
    u_short pad1;
    short x3, y3;
    u_char u3, v3;
    u_short pad2;
} FieldThumbnailQuad;

extern Vec2s g_field_screen_scroll;
extern u8* g_field_cd_buffer;
extern FieldActionRow g_field_resource_actions[];
/** @brief Golem sprite CLUTs, two banks of 16. */
extern u16 g_field_golem_palettes[2][FIELD_GOLEM_BANK_PALETTES * FIELD_CLUT_COLORS];
/** @brief Golem portrait palettes, two banks of 16. */
extern u16 g_field_golem_portrait_palettes[2][FIELD_GOLEM_BANK_PALETTES * FIELD_CLUT_COLORS];

extern u16 g_field_ring_excluded_mask;
extern s32 g_field_ring_menu_id;
extern s32 g_field_ring_radius_steps;
extern s32 g_field_ring_target_angle;
extern s32 g_field_ring_center_x;
extern s32 g_field_ring_center_y;
extern s32 g_field_ring_icon_width;
extern s32 g_field_ring_icon_height;
extern s32 g_field_ring_radius;
extern s32 g_field_ring_target_radius;
extern u8 g_field_ring_saved_selections[];
extern s32 g_field_ring_cursor;
extern s32 g_field_ring_cancel_index;
extern s32 D_8011F380;
extern u8 g_field_ring_entries[];
extern s32 g_field_ring_step;
extern s32 g_field_ring_menu_state;
extern s32 g_field_ring_start_angle;
extern s32 g_field_ring_semi_trans;
extern s32 g_field_ring_entry_count;
extern s32 g_field_ring_brightness;
extern s32 g_field_ring_angle;
extern s32 g_field_ring_icon_count;
extern s32 g_field_frame_thumbnail_enabled;

static void field_ring_menu_open_step(void);
static void field_ring_menu_close_step(void);
static void field_ring_menu_select_step(void);
static void field_draw_ring_menu(FieldRenderHalf* render_half);
static void field_draw_frame_thumbnail(FieldRenderHalf* render_half, s32 frame);

/**
 * @brief Open a ring menu: load its icon sheet and start the opening animation.
 * @param position_mode FIELD_RING_AT_SCREEN_CENTER or FIELD_RING_AT_PLAYER.
 * @param menu_id Ring menu id: selects the icon sheet and the saved selection.
 * @param excluded_mask One bit per icon of the sheet that is left out of the ring.
 * @param cancel_index Entry that cancel selects and that is saved as 0, or FIELD_RING_NO_CANCEL.
 * @note Does nothing while a ring menu is already open.
 */
void field_open_ring_menu(s32 position_mode, s32 menu_id, u16 excluded_mask, s32 cancel_index)
{
    RECT rect;
    DVECTOR center;
    s32 view_x;
    s32 view_y;
    s32 screen_x;
    s32 screen_y;
    s32 angle;
    s32 bit;
    s32 icon;
    u32 sheet_info;
    u8* saved_selection;
    u8 selection;

    if (g_field_ring_menu_state == FIELD_RING_CLOSED)
    {
        field_reset_input_repeat();
        g_field_ring_cancel_index = cancel_index;
        g_field_ring_excluded_mask = excluded_mask;
        setRECT(&rect, FIELD_RING_IMAGE_X, FIELD_RING_IMAGE_Y, 0, VRAM_CLUT_Y);
        /* The sheet describes itself in CLUT entries 240/241: icon size, icon count and ring radius. */
        sheet_info = field_load_vram_resource(menu_id + FIELD_RING_ICON_RESOURCE_BASE, &rect, 1);
        g_field_ring_icon_width = (sheet_info & 0x1F) * 8;
        g_field_ring_icon_height = (sheet_info >> 2) & 0xF8;
        g_field_ring_icon_count = (sheet_info >> 10) & 0x1F;
        g_field_ring_entry_count = 0;
        bit = 1;
        for (icon = 0; icon < g_field_ring_icon_count; icon++, bit <<= 1)
        {
            if (!(excluded_mask & bit))
            {
                g_field_ring_entries[g_field_ring_entry_count] = icon;
                g_field_ring_entry_count++;
            }
        }
        g_field_ring_radius = FIELD_RING_START_RADIUS;
        g_field_ring_target_radius = (sheet_info >> 13) & 0xF8;
        g_field_ring_radius_steps = FIELD_RING_RADIUS_EASE_STEPS;
        saved_selection = &g_field_ring_saved_selections[menu_id];
        selection = *saved_selection;
        if (selection >= g_field_ring_entry_count)
        {
            *saved_selection = 0;
            g_field_ring_cursor = 0;
        }
        else
        {
            g_field_ring_cursor = selection % g_field_ring_entry_count;
        }
        angle = -(ONE / g_field_ring_entry_count) * g_field_ring_cursor;
        g_field_ring_menu_id = menu_id;
        D_8011F380 = 0;
        g_field_ring_angle = angle;
        g_field_ring_target_angle = angle;
        switch (position_mode)
        {
        case FIELD_RING_AT_SCREEN_CENTER:
            g_field_ring_center_x = FIELD_RING_CENTER_X;
            g_field_ring_center_y = FIELD_RING_CENTER_Y;
            break;

        case FIELD_RING_AT_PLAYER:
            view_x = g_field_view_offset_x / 256;
            screen_x = g_field_actors[0].x / 256 + FIELD_RING_CENTER_X;
            center.vx = view_x + screen_x;
            view_y = g_field_view_offset_y / 256;
            screen_y = g_field_actors[0].y / 256 + FIELD_RING_CENTER_Y;
            center.vy = view_y + screen_y - g_field_actors[0].z / 512 - g_field_view_offset_z / 512;
            g_field_ring_center_x = center.vx;
            g_field_ring_center_y = center.vy;
            break;
        }
        g_field_ring_menu_state = FIELD_RING_OPENING;
        g_field_ring_brightness = 0;
        g_field_ring_semi_trans = 1;
        g_field_ring_step = 0;
        g_field_screen_scroll.y = 0;
        g_field_screen_scroll.x = 0;
    }
}

/**
 * @brief Return the entry chosen in the last ring menu.
 * @return The icon index under the cursor once the menu has closed, or -1 while it is open.
 */
s32 field_get_ring_result(void)
{
    if (g_field_ring_menu_state != FIELD_RING_CLOSED)
    {
        return -1;
    }
    return g_field_ring_entries[g_field_ring_cursor];
}

/**
 * @brief Return the icon index of the ring entry under the cursor.
 * @return The icon index of the cursor entry.
 */
u8 field_get_ring_cursor_entry(void)
{
    return g_field_ring_entries[g_field_ring_cursor];
}

/**
 * @brief Run one frame of the ring menu and draw it.
 * @param render_half Render half the icons are drawn into.
 * @return 1 while a ring menu is open, otherwise 0.
 */
s32 field_update_ring_menu(FieldRenderHalf* render_half)
{
    s32 state;

    state = g_field_ring_menu_state;
    if (state == FIELD_RING_CLOSED)
    {
        return 0;
    }

    switch (state)
    {
    case FIELD_RING_SELECTING:
        field_ring_menu_select_step();
        break;

    case FIELD_RING_OPENING:
        field_ring_menu_open_step();
        break;

    case FIELD_RING_CLOSING:
        field_ring_menu_close_step();
        break;
    }

    field_draw_ring_menu(render_half);
    return 1;
}

/**
 * @brief Opening animation: ease the radius in, spin two turns while slowing down and fade in.
 */
static void field_ring_menu_open_step(void)
{
    if (g_field_ring_radius_steps != 0)
    {
        g_field_ring_radius += (g_field_ring_target_radius - g_field_ring_radius) / g_field_ring_radius_steps;
        g_field_ring_radius_steps--;
    }
    else
    {
        g_field_ring_radius = g_field_ring_target_radius;
    }

    if (g_field_ring_step < FIELD_RING_SPIN_FAST_STEPS)
    {
        g_field_ring_angle += ONE / 8;
    }
    else if (g_field_ring_step < FIELD_RING_SPIN_MEDIUM_STEPS)
    {
        g_field_ring_angle += ONE / 16;
    }
    else if (g_field_ring_step < FIELD_RING_SPIN_STEPS)
    {
        g_field_ring_angle += ONE / 32;
    }
    else
    {
        g_field_ring_semi_trans = 0;
        g_field_ring_menu_state = FIELD_RING_SELECTING;
        g_field_ring_step = 0;
        g_field_ring_brightness = FIELD_RING_FULL_BRIGHTNESS;
        return;
    }
    g_field_ring_brightness += FIELD_RING_OPEN_FADE_STEP;
    g_field_ring_step++;
}

/**
 * @brief Closing animation: collapse the radius and fade out, then close the menu.
 */
static void field_ring_menu_close_step(void)
{
    g_field_ring_semi_trans = 1;
    if (g_field_ring_step < FIELD_RING_CLOSE_STEPS)
    {
        g_field_ring_radius -= g_field_ring_radius / (FIELD_RING_CLOSE_STEPS - g_field_ring_step);
    }
    else
    {
        g_field_ring_step = 0;
        g_field_ring_menu_state = FIELD_RING_CLOSED;
        return;
    }
    g_field_ring_brightness -= FIELD_RING_CLOSE_FADE_STEP;
    g_field_ring_step++;
}

/**
 * @brief Selection: finish a running rotation, otherwise handle confirm, cancel and rotate input.
 * @note Confirm saves the cursor as the menu's selection (the cancel entry saves 0) and starts closing.
 */
static void field_ring_menu_select_step(void)
{
    s32 cursor;

    if (g_field_ring_step != 0)
    {
        g_field_ring_angle = g_field_ring_start_angle + (g_field_ring_target_angle - g_field_ring_start_angle) * g_field_ring_step / FIELD_RING_ROTATE_STEPS;
        if (g_field_ring_step == FIELD_RING_ROTATE_STEPS)
        {
            g_field_ring_step = 0;
            g_field_ring_angle = -g_field_ring_cursor * ONE / g_field_ring_entry_count;
            return;
        }
        g_field_ring_step++;
        return;
    }
    if (g_pad_input & FIELD_RING_CONFIRM_BUTTONS)
    {
        g_field_ring_menu_state = FIELD_RING_CLOSING;
        if (g_field_ring_cancel_index != FIELD_RING_NO_CANCEL && g_field_ring_cursor == g_field_ring_cancel_index)
        {
            g_field_ring_saved_selections[g_field_ring_menu_id] = 0;
        }
        else
        {
            g_field_ring_saved_selections[g_field_ring_menu_id] = g_field_ring_cursor;
        }
    }
    if ((g_pad_input & FIELD_RING_CANCEL_BUTTONS) && g_field_ring_cancel_index != FIELD_RING_NO_CANCEL)
    {
        g_field_ring_cursor = g_field_ring_cancel_index;
        g_field_ring_step = 1;
        g_field_ring_start_angle = g_field_ring_angle;
        g_field_ring_target_angle = -g_field_ring_cancel_index * ONE / g_field_ring_entry_count;
    }
    if (g_pad_input & FIELD_RING_PREVIOUS_BUTTONS)
    {
        g_field_ring_step = 1;
        g_field_ring_start_angle = g_field_ring_angle;
        cursor = g_field_ring_cursor - 1;
        g_field_ring_cursor = cursor;
        g_field_ring_target_angle = g_field_ring_angle + ONE / g_field_ring_entry_count;
        if (cursor < 0)
        {
            g_field_ring_cursor = g_field_ring_entry_count - 1;
        }
    }
    if (g_pad_input & FIELD_RING_NEXT_BUTTONS)
    {
        g_field_ring_step = 1;
        g_field_ring_start_angle = g_field_ring_angle;
        g_field_ring_target_angle = g_field_ring_angle - ONE / g_field_ring_entry_count;
        g_field_ring_cursor = (g_field_ring_cursor + 1) % g_field_ring_entry_count;
    }
}

/**
 * @brief Draw the ring entries as icons on an ellipse around the ring center.
 * @param render_half Render half that receives the icon quads.
 * @note Icons further back are smaller, darker and sorted deeper; the cursor icon pulses
 *       every eighth frame.
 */
static void field_draw_ring_menu(FieldRenderHalf* render_half)
{
    POLY_FT4* prim;
    u_long* ot;
    s32 index;
    s32 columns;
    s32 icon;
    s32 u_or_angle; /* texture u, then the entry's ring angle: separate locals change the register allocation */
    s32 v_or_width; /* texture v, then the icon width, for the same reason */
    s32 height;

    ot = &render_half->ordering_table[FIELD_RING_OT_FRONT];
    prim = (POLY_FT4*)render_half->primitive_cursor;
    for (index = 0; index < g_field_ring_entry_count; index++)
    {
        setPolyFT4(prim);
        setSemiTrans(prim, g_field_ring_semi_trans);
        columns = FIELD_RING_PAGE_WIDTH / g_field_ring_icon_width;
        icon = g_field_ring_entries[index];
        u_or_angle = (icon % columns) * g_field_ring_icon_width;
        v_or_width = (icon / columns) * g_field_ring_icon_height;
        prim->u0 = prim->u2 = u_or_angle;
        prim->u1 = prim->u3 = u_or_angle + g_field_ring_icon_width - 1;
        prim->v0 = prim->v1 = v_or_width;
        prim->v2 = prim->v3 = v_or_width + g_field_ring_icon_height - 1;

        u_or_angle = (index * ONE) / g_field_ring_entry_count + g_field_ring_angle;
        prim->r0 = prim->g0 = prim->b0 = FIELD_RING_DEPTH_SCALE(g_field_ring_brightness, u_or_angle);
        v_or_width = FIELD_RING_DEPTH_SCALE(g_field_ring_icon_width, u_or_angle);
        height = FIELD_RING_DEPTH_SCALE(g_field_ring_icon_height, u_or_angle);
        if (index == g_field_ring_cursor && !(g_frame_counter & 7))
        {
            v_or_width = v_or_width * 9 / 8;
            height = height * 9 / 8;
        }
        /* The ellipse is a quarter as tall as it is wide. */
        prim->x0 = prim->x2 = g_field_ring_center_x + ((g_field_ring_radius * rsin(u_or_angle)) >> 12) - (v_or_width >> 1);
        prim->y1 = prim->y0 = g_field_ring_center_y + ((g_field_ring_radius * rcos(u_or_angle)) >> 14) - (height >> 1);
        prim->tpage = getTPage(0, 1, FIELD_RING_IMAGE_X, FIELD_RING_IMAGE_Y);
        prim->y2 = prim->y3 = prim->y0 + height;
        prim->x1 = prim->x3 = prim->x0 + v_or_width;
        prim->clut = getClut(g_field_ring_entries[index] * FIELD_CLUT_COLORS, VRAM_CLUT_Y);
        if (FIELD_RING_DEPTH(u_or_angle) <= 0)
        {
            addPrim(ot - FIELD_RING_DEPTH(u_or_angle), prim);
            /* Called with an int depth: the original passes it without narrowing to s16. */
            ((void (*)(const void*, s32))field_add_fade_prim)(prim, -FIELD_RING_DEPTH(u_or_angle) + FIELD_RING_OT_FRONT);
            prim++;
        }
    }
    render_half->primitive_cursor = (u8*)prim;
}

/**
 * @brief Load a party member's script package from CD into its script page.
 * @param party_slot Party slot (1 partner, 2 companion); the companion also gets its action slots.
 * @param resource_id CD resource id of the package (low 16 bits).
 * @note The package starts with the offsets of its sections: the action table (a count, then
 *       FieldActionSlot records), the script page, and the end of the script page.
 */
void field_load_party_script_page(s32 party_slot, s32 resource_id)
{
    s32* section;
    s32* action_table;
    FieldActionSlot* actions;
    FieldActionRow* action_rows;
    s32 action_count;

    section = (s32*)g_field_cd_buffer;
    cdrom_queue_read(resource_id & 0xFFFF, section);
    cdrom_wait_queue_empty();
    action_table = (s32*)(g_field_cd_buffer + section[0]);
    actions = (FieldActionSlot*)(action_table + 1);
    action_count = action_table[0];
    if (party_slot == 2)
    {
        /* Through a local: a constant &g_field_resource_actions[2] changes the address code. */
        action_rows = g_field_resource_actions;
        bcopy((u8*)actions, (u8*)&action_rows[2], action_count * sizeof(FieldActionSlot));
    }
    section++;
    bcopy(g_field_cd_buffer + section[0], g_field_party_script_pages[party_slot].bytes, section[1] - section[0]);
}

/**
 * @brief Draw a quarter-size thumbnail of a framebuffer in the top-left corner.
 * @param render_half Render half that receives the thumbnail quads.
 * @param frame 0 shows the frame drawn at y = SCREEN_HEIGHT, otherwise the frame drawn at y = VRAM_BACK_DRAW_Y.
 * @note Debug view: nothing calls it and g_field_frame_thumbnail_enabled is never set. With @p frame 0
 *       each strip leaves one unused quad in the packet buffer, otherwise every strip does.
 */
static void field_draw_frame_thumbnail(FieldRenderHalf* render_half, s32 frame)
{
    u_long* ot;
    s32 strip;
    FieldThumbnailQuad* quad;

    if (g_field_frame_thumbnail_enabled != 0)
    {
        quad = (FieldThumbnailQuad*)render_half->primitive_cursor;
        ot = render_half->ordering_table;
        if (frame == 0)
        {
            s16 left;
            s32 page;

            /* Rows 240-255 of the top page, then rows 0-207 of the bottom page. */
            strip = 0;
            do
            {
                left = strip * FIELD_THUMBNAIL_STRIP_WIDTH;
                page = strip & 0xF;
                quad->rgbc = FIELD_THUMBNAIL_GREY;
                setPolyFT4(quad);
                setXY4(quad, left, 0, strip * FIELD_THUMBNAIL_STRIP_WIDTH + FIELD_THUMBNAIL_STRIP_WIDTH, 0, left, 4,
                       strip * FIELD_THUMBNAIL_STRIP_WIDTH + FIELD_THUMBNAIL_STRIP_WIDTH, 4);
                setUV4(quad, 0, 240, 64, 240, 0, 255, 64, 255);
                quad->clut = 0;
                quad->tpage = FIELD_THUMBNAIL_TPAGE_TOP | page;
                addPrim(ot, quad);
                quad++;
                quad->rgbc = FIELD_THUMBNAIL_GREY;
                setPolyFT4(quad);
                setXY4(quad, left, 4, strip * FIELD_THUMBNAIL_STRIP_WIDTH + FIELD_THUMBNAIL_STRIP_WIDTH, 4, left, 56,
                       strip * FIELD_THUMBNAIL_STRIP_WIDTH + FIELD_THUMBNAIL_STRIP_WIDTH, 56);
                strip++;
                setUV4(quad, 0, 0, 64, 0, 0, 208, 64, 208);
                quad->clut = 0;
                quad->tpage = FIELD_THUMBNAIL_TPAGE_BOTTOM | page;
                addPrim(ot, quad);
                quad += 2;
            } while (strip < FIELD_THUMBNAIL_STRIPS);
            render_half->primitive_cursor = (u8*)quad;
            return;
        }
        /* Rows 8-231 of the top page. */
        {
            /* Constants, link masks and the reused value local: literals and addPrim change the loop's register use. */
            u32 address_mask;
            u32 tag_mask;
            s32 color;
            s32 length;
            s32 height;
            s32 v_top;
            s32 u_right;
            s32 v_bottom;
            s16 left;
            s16 right;
            s32 page;
            s32 value;

            strip = 0;
            color = FIELD_THUMBNAIL_GREY;
            length = 9;
            height = 56;
            v_top = 8;
            u_right = 64;
            v_bottom = 232;
            address_mask = 0xFFFFFF;
            tag_mask = 0xFF000000;
            right = FIELD_THUMBNAIL_STRIP_WIDTH;
            do
            {
                quad->x3 = quad->x1 = right;
                right += FIELD_THUMBNAIL_STRIP_WIDTH;
                left = strip * FIELD_THUMBNAIL_STRIP_WIDTH;
                page = strip & 0xF;
                strip++;
                quad->rgbc = color;
                setlen(quad, length);
                value = 0x2C;
                setcode(quad, value);
                quad->x0 = left;
                value = 0;
                quad->y0 = value;
                quad->y1 = 0;
                quad->x2 = left;
                quad->y2 = height;
                quad->y3 = height;
                setUV4(quad, 0, v_top, u_right, v_top, 0, v_bottom, u_right, v_bottom);
                quad->clut = 0;
                quad->tpage = FIELD_THUMBNAIL_TPAGE_TOP | page;
                quad->tag = (quad->tag & tag_mask) | (*ot & address_mask);
                *ot = (*ot & tag_mask) | ((u_long)quad & address_mask);
                quad += 2;
            } while (strip < FIELD_THUMBNAIL_STRIPS);
        }
        render_half->primitive_cursor = (u8*)quad;
    }
}

/**
 * @brief Upload the CLUT of each existing golem to its VRAM row.
 */
void field_upload_golem_palettes(void)
{
    RECT rect;
    s32 i;

    for (i = 0; i < LARGE_HISTORY_RECORD_COUNT; i++)
    {
        if (g_pad_ctx->large_history_records[i].name[0] != 0)
        {
            if ((u32)g_pad_ctx->large_history_records[i].unknown_0x48 < FIELD_GOLEM_BANK_PALETTES)
            {
                setRECT(&rect, FIELD_GOLEM_CLUT_X, FIELD_GOLEM_CLUT_Y + i, FIELD_CLUT_COLORS, 1);
                LoadImage(&rect, (u_long*)&g_field_golem_palettes[0][g_pad_ctx->large_history_records[i].unknown_0x48 * FIELD_CLUT_COLORS]);
            }
            else
            {
                setRECT(&rect, FIELD_GOLEM_CLUT_X, FIELD_GOLEM_CLUT_Y + i, FIELD_CLUT_COLORS, 1);
                LoadImage(
                    &rect,
                    (u_long*)&g_field_golem_palettes[1][(g_pad_ctx->large_history_records[i].unknown_0x48 - FIELD_GOLEM_BANK_PALETTES) * FIELD_CLUT_COLORS]);
            }
        }
    }
}

/**
 * @brief Copy a golem portrait palette into a portrait image.
 * @param destination Palette strip at the start of the portrait image.
 * @param palette Golem palette index (0-31).
 */
void field_copy_golem_portrait_palette(u8* destination, s32 palette)
{
    if (palette < FIELD_GOLEM_BANK_PALETTES)
    {
        bcopy((u8*)&g_field_golem_portrait_palettes[0][palette * FIELD_CLUT_COLORS], destination, FIELD_CLUT_COLORS * sizeof(u16));
    }
    else
    {
        bcopy((u8*)&g_field_golem_portrait_palettes[1][(palette - FIELD_GOLEM_BANK_PALETTES) * FIELD_CLUT_COLORS], destination,
              FIELD_CLUT_COLORS * sizeof(u16));
    }
}
