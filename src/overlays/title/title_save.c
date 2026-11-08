#include "common/saved_game.h"
#include "internal/title_internal.h"
#include "internal/title_save.h"
#include "overlays/field/field_sound.h"
#include <rand.h>
#include "common/tim.h"
#include "common/gpu_packet.h"
#include "main/audio/akao.h"

#define SLOT_PANEL_WIDTH 160
#define SLOT_SLIDE_FRAMES 8

#define HERO_TEMPLATE_WORDS (SAVED_CHARACTER_SIZE / sizeof(s32))
#define SAVED_GAME_TEMPLATE_WORDS (SAVED_GAME_DATA_SIZE / sizeof(s32))

#define SAVE_LAYOUT_TEX_COUNT 11

#define GLYPH_CHUNK_WIDTH 0x80

/* Where a hero figure's pose origin lands inside its layout entry. */
#define FIGURE_ANCHOR_X 0x20
#define FIGURE_ANCHOR_Y 0x28

#define CURSOR_BAR_COLOR GPU_COLOR_WORD(0x40, 0, 0)
/* The bar is untextured; this page is only set for its additive blend mode. */
#define CURSOR_BAR_TPAGE_X 0x140

/* SaveLayoutEntry::type. Any other nonzero type is a glyph strip. */
#define SAVE_LAYOUT_PRIM_NONE 0
#define SAVE_LAYOUT_PRIM_TILE 2
#define SAVE_LAYOUT_PRIM_POLY_FT4 3
#define SAVE_LAYOUT_PRIM_SPRT 4

/** @brief Texture rectangle and origin of a picker sprite, all in 8-pixel units. */
typedef struct
{
    u8 u;
    u8 v;
    u8 w;
    u8 h;
    u8 ox;
    u8 oy;
} SlotUvRect;

/** @brief A TILE followed by the DR_TPAGE that sets its blend mode. */
typedef struct
{
    TILE tile;
    DR_TPAGE tpage;
} TileTPage;

/** @brief Primitive buffer cursor, viewed as the primitive being written next. */
typedef union
{
    u8* bytes;
    POLY_FT4* poly;
    SPRT* sprt;
    DR_TPAGE* tpage;
    TileTPage* tile_tpage;
} PrimCursor;

extern SlotUvRect g_save_slot_panel_uv_table[];
extern SlotUvRect g_save_slot_sprite_uv_table[];

void reset_save_slot_panel(void);
void handle_save_slot_input(void);
void animate_save_slot_panel(void);
static void scroll_slots_right(void);
static void scroll_slots_left(void);
void load_hero_template(s32 hero_type);
void upload_save_layout_textures(void);
void* render_save_layout_prims(PrimCursor cursor, u_long* ot);

#include "../../common/pad_repeat/sample_pad_buttons.inc.c"

#include "../../common/pad_repeat/init_pad_repeat.inc.c"

/**
 * @brief Initialize the save-slot picker state and upload its sprite atlases.
 * @see decomp.me (100%) https://decomp.me/scratch/t2lHt
 */
void init_save_slot_menu(void)
{
    init_pad_repeat();
    g_slot_slide_frames = 0;
    g_slot_slide_y_lerped = 0;
    g_slot_slide_y = 0;
    g_slot_slide_x_lerped = 0;
    g_slot_slide_x = 0;
    g_slot_selected_index = 0;
    g_weapon_list_scroll = 0;
    g_weapon_list_scroll_target = 0;
    g_weapon_list_scroll_frames = 0;
    upload_save_layout_textures();
}

/**
 * @brief Render the save-slot picker and process its input for one frame.
 * @param context Active title display and primitive buffers.
 * @see decomp.me (100%) https://decomp.me/scratch/so5cY
 */
void render_save_slot_menu(TitleMenuContext* context)
{
    PrimCursor cursor;

    cursor.bytes = context->screen.prim_cursor;
    context->screen.prim_cursor = render_save_layout_prims(cursor, context->screen.ot);
    handle_save_slot_input();
}

/**
 * @brief Give the new saved game a random 30-bit game id.
 */
static inline void roll_new_game_id(void)
{
    s32 random_low;
    s32 random_high;

    random_low = rand();
    random_high = rand();
    g_saved_game.layout.identity.ids.game_id = random_low | (random_high << TITLE_RNG_HIGH_SHIFT);
}

/**
 * @brief Handle one frame of new-game picker input.
 *
 * @details On the home panel, left/right toggles between the two heroes
 * (SAVE_LAYOUT_HERO0_SELECTED / SAVE_LAYOUT_HERO1_SELECTED) and confirm
 * slides to the chosen hero's weapon list. On a weapon list, up/down picks a weapon and confirm starts the game
 * with that hero and weapon; the other weapon categories lose their techniques.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Xl8gF
 */
void handle_save_slot_input(void)
{
    s32 slide_x_step;
    s32* slide_x_lerped_ptr;
    s32 slide_y_step;
    s32 i;
    u8* src;
    u8* dst;

    if (g_slot_slide_frames != 0)
    {
        slide_x_lerped_ptr = &g_slot_slide_x_lerped;
        slide_x_step = (g_slot_slide_x - *slide_x_lerped_ptr) / g_slot_slide_frames;
        slide_y_step = (g_slot_slide_y - g_slot_slide_y_lerped) / g_slot_slide_frames;
        g_slot_slide_frames--;
        g_slot_slide_x_lerped += slide_x_step;
        g_slot_slide_y_lerped += slide_y_step;
        return;
    }

    g_slot_slide_x_lerped = g_slot_slide_x;
    g_slot_slide_y_lerped = g_slot_slide_y;
    update_menu_input();

    if (g_slot_slide_x == 0)
    {
        if (g_debounced_input & (PAD_BTN_LEFT | PAD_BTN_RIGHT))
        {
            play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);

            if (g_save_layout_table[SAVE_LAYOUT_HERO0_SELECTED].type != 0)
            {
                g_save_layout_table[SAVE_LAYOUT_HERO0_SELECTED].type = 0;
                g_save_layout_table[SAVE_LAYOUT_HERO1_SELECTED].type = 1;
                return;
            }

            g_save_layout_table[SAVE_LAYOUT_HERO0_SELECTED].type = 1;
            g_save_layout_table[SAVE_LAYOUT_HERO1_SELECTED].type = 0;
            return;
        }

        if (g_debounced_input & (PAD_BTN_START | PAD_CONFIRM_BUTTONS))
        {
            play_sfx(SFX_CONFIRM, AKAO_PAN_CENTER);

            if (g_save_layout_table[SAVE_LAYOUT_HERO0_SELECTED].type != 0)
            {
                scroll_slots_right();
                reset_save_slot_panel();
                return;
            }

            scroll_slots_left();
            reset_save_slot_panel();
            return;
        }

        if (g_debounced_input & PAD_BTN_CIRCLE)
        {
            play_sfx(SFX_CANCEL, AKAO_PAN_CENTER);
            g_title_menu_exit_state = 2;
        }
    }
    else
    {
        if (g_debounced_input & (PAD_BTN_START | PAD_CONFIRM_BUTTONS))
        {
            if (g_slot_slide_x > 0)
            {
                load_hero_template(0);
                g_saved_game.layout.characters[0].info.word &= ~FIELD_CHARACTER_TYPE_MASK;
                roll_new_game_id();
            }
            else
            {
                load_hero_template(1);
                g_saved_game.layout.characters[0].info.word = (g_saved_game.layout.characters[0].info.word & ~FIELD_CHARACTER_TYPE_MASK) | 1;
                roll_new_game_id();
            }

            /* Copies the whole record, not just the name. */
            dst = g_saved_game.layout.characters[0].equipment[FIELD_WEAPON_SLOT].name;
            src = g_starting_weapon_records[g_slot_selected_index].name;
            i = 0;

            while (i < sizeof(FieldItemRecord))
            {
                i++;
                *dst++ = *src++;
            }

            for (i = 0; i < FIELD_WEAPON_CATEGORY_COUNT; i++)
            {
                if (g_slot_selected_index != i)
                {
                    g_saved_game.layout.technique_bits[i] = 0;
                }
            }

            play_sfx(SFX_CONFIRM, AKAO_PAN_CENTER);
            g_title_menu_exit_state = 1;
        }
        else if (g_debounced_input & PAD_BTN_CIRCLE)
        {
            play_sfx(SFX_CANCEL, AKAO_PAN_CENTER);

            if (g_slot_slide_x > 0)
            {
                scroll_slots_left();
                reset_save_slot_panel();
            }
            else
            {
                scroll_slots_right();
                reset_save_slot_panel();
            }
        }
        else if (g_weapon_list_scroll_frames == 0)
        {
            if (g_debounced_input & PAD_BTN_UP)
            {
                play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
                if (--g_slot_selected_index < 0)
                {
                    g_slot_selected_index = FIELD_WEAPON_CATEGORY_COUNT - 1;
                }
            }

            if (g_debounced_input & PAD_BTN_DOWN)
            {
                play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
                if (++g_slot_selected_index >= FIELD_WEAPON_CATEGORY_COUNT)
                {
                    g_slot_selected_index = 0;
                }
            }
        }

        animate_save_slot_panel();
    }
}

/**
 * @brief Scroll the weapon list to keep the selection visible and place the cursor bar.
 * @note The list shows SAVE_WEAPON_VISIBLE_ROWS rows of SAVE_WEAPON_ROW_HEIGHT pixels.
 * @see decomp.me (100%) https://decomp.me/scratch/d3s3Q
 */
void animate_save_slot_panel(void)
{
    SaveLayoutEntry* layout;
    s16 cursor_y;
    s32 top_row;
    s32 cursor;
    s32 scroll;
    s32 row_y;

    if (g_weapon_list_scroll_frames != 0)
    {
        g_weapon_list_scroll += (g_weapon_list_scroll_target - g_weapon_list_scroll) / g_weapon_list_scroll_frames;
        g_weapon_list_scroll_frames--;
    }
    else
    {
        g_weapon_list_scroll = g_weapon_list_scroll_target;
    }

    /* top_row = scroll / SAVE_WEAPON_ROW_HEIGHT */
    scroll = g_weapon_list_scroll_target;
    top_row = scroll;

    if (scroll < 0)
    {
        top_row = scroll + SAVE_WEAPON_ROW_HEIGHT - 1;
    }

    top_row >>= SAVE_WEAPON_ROW_SHIFT;
    cursor = g_slot_selected_index;

    if (g_slot_selected_index < top_row)
    {
        g_weapon_list_scroll_target = cursor * SAVE_WEAPON_ROW_HEIGHT;
        g_weapon_list_scroll_frames = SAVE_WEAPON_SCROLL_FRAMES;
    }
    else if (top_row + SAVE_WEAPON_VISIBLE_ROWS - 1 < g_slot_selected_index)
    {
        g_weapon_list_scroll_target = (g_slot_selected_index - (SAVE_WEAPON_VISIBLE_ROWS - 1)) * SAVE_WEAPON_ROW_HEIGHT;
        g_weapon_list_scroll_frames = SAVE_WEAPON_SCROLL_FRAMES;
    }

    g_save_layout_table[SAVE_LAYOUT_HERO0_ICONS].v0 = g_weapon_list_scroll;
    g_save_layout_table[SAVE_LAYOUT_HERO0_NAMES].v0 = g_weapon_list_scroll + SAVE_WEAPON_NAMES_V;
    g_save_layout_table[SAVE_LAYOUT_HERO1_ICONS].v0 = g_weapon_list_scroll;
    g_save_layout_table[SAVE_LAYOUT_HERO1_NAMES].v0 = g_weapon_list_scroll + SAVE_WEAPON_NAMES_V;

    if (g_weapon_list_scroll != 0)
    {
        g_save_layout_table[SAVE_LAYOUT_HERO0_UP_L].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO0_UP_R].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO1_UP_L].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO1_UP_R].type = 1;
    }
    else
    {
        g_save_layout_table[SAVE_LAYOUT_HERO0_UP_L].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO0_UP_R].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO1_UP_L].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO1_UP_R].type = 0;
    }

    if (g_weapon_list_scroll != SAVE_WEAPON_SCROLL_MAX)
    {
        g_save_layout_table[SAVE_LAYOUT_HERO0_DOWN_L].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO0_DOWN_R].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO1_DOWN_L].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO1_DOWN_R].type = 1;
    }
    else
    {
        g_save_layout_table[SAVE_LAYOUT_HERO0_DOWN_L].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO0_DOWN_R].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO1_DOWN_L].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO1_DOWN_R].type = 0;
    }

    /* Keep the cursor bar on the selected row, inside the window. */
    row_y = g_slot_selected_index * SAVE_WEAPON_ROW_HEIGHT;
    scroll = g_weapon_list_scroll;
    cursor = row_y - scroll;

    if (cursor < 0)
    {
        cursor = 0;
    }

    if (cursor > SAVE_WEAPON_CURSOR_MAX)
    {
        cursor = SAVE_WEAPON_CURSOR_MAX;
    }

    layout = g_save_layout_table;
    cursor_y = cursor + SAVE_WEAPON_LIST_Y;
    layout[SAVE_LAYOUT_HERO0_CURSOR].y = cursor_y;
    layout[SAVE_LAYOUT_HERO0_CURSOR].tile_y = cursor_y;
    layout[SAVE_LAYOUT_HERO1_CURSOR].y = cursor_y;
    layout[SAVE_LAYOUT_HERO1_CURSOR].tile_y = cursor_y;
}


/**
 * @brief Reset the weapon-list selection and scroll after a panel slide.
 * @see decomp.me (100%) https://decomp.me/scratch/0YgmZ
 */
void reset_save_slot_panel(void)
{
    s16 names_v;

    if (g_slot_slide_x != 0)
    {
        g_save_layout_table[SAVE_LAYOUT_CAPTION].v0 = SAVE_CAPTION_UV_STEP;
        g_slot_selected_index = 0;
        g_weapon_list_scroll = 0;
        g_weapon_list_scroll_target = 0;
        g_weapon_list_scroll_frames = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO0_UP_L].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO0_UP_R].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO1_UP_L].type = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO1_UP_R].type = 0;
        names_v = g_weapon_list_scroll + SAVE_WEAPON_NAMES_V;
        g_save_layout_table[SAVE_LAYOUT_HERO0_CURSOR].y = SAVE_WEAPON_LIST_Y;
        g_save_layout_table[SAVE_LAYOUT_HERO0_CURSOR].tile_y = SAVE_WEAPON_LIST_Y;
        g_save_layout_table[SAVE_LAYOUT_HERO1_CURSOR].y = SAVE_WEAPON_LIST_Y;
        g_save_layout_table[SAVE_LAYOUT_HERO1_CURSOR].tile_y = SAVE_WEAPON_LIST_Y;
        g_save_layout_table[SAVE_LAYOUT_CAPTION].u0 = 0;
        g_save_layout_table[SAVE_LAYOUT_HERO0_DOWN_L].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO0_DOWN_R].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO1_DOWN_L].type = 1;
        g_save_layout_table[SAVE_LAYOUT_HERO1_DOWN_R].type = 1;

        g_save_layout_table[SAVE_LAYOUT_HERO0_ICONS].v0 = g_weapon_list_scroll;
        g_save_layout_table[SAVE_LAYOUT_HERO0_NAMES].v0 = names_v;
        g_save_layout_table[SAVE_LAYOUT_HERO1_ICONS].v0 = g_weapon_list_scroll;
        g_save_layout_table[SAVE_LAYOUT_HERO1_NAMES].v0 = names_v;
        return;
    }

    g_save_layout_table[SAVE_LAYOUT_CAPTION].u0 = SAVE_CAPTION_UV_STEP;
    g_save_layout_table[SAVE_LAYOUT_CAPTION].v0 = 0;
}


/**
 * @brief Start sliding the picker one panel right, unless already at the right panel.
 * @see decomp.me (100%) https://decomp.me/scratch/SRP9z
 */
static void scroll_slots_right(void)
{
    if (g_slot_slide_x_lerped != SLOT_PANEL_WIDTH)
    {
        g_slot_slide_x += SLOT_PANEL_WIDTH;
        g_slot_slide_frames = SLOT_SLIDE_FRAMES;
    }
}

/**
 * @brief Start sliding the picker one panel left, unless already at the left panel.
 * @see decomp.me (100%) https://decomp.me/scratch/W1iA5
 */
static void scroll_slots_left(void)
{
    if (g_slot_slide_x_lerped != -SLOT_PANEL_WIDTH)
    {
        g_slot_slide_x -= SLOT_PANEL_WIDTH;
        g_slot_slide_frames = SLOT_SLIDE_FRAMES;
    }
}

/**
 * @brief Texture page of a save-layout texture at a given VRAM x.
 * @param tex Texture whose mode and y position are used.
 * @param flags SaveLayoutEntry::flags of the entry.
 * @param x VRAM x of the page.
 * @return The texture page value.
 */
static inline u32 get_save_layout_tpage(SaveLayoutTex* tex, u32 flags, s32 x)
{
    return getTPage(tex->control.mode & 3, flags >> SAVE_LAYOUT_FLAG_BLEND_SHIFT, x, tex->tex_y);
}

/**
 * @brief Texture page of a save-layout texture at its own VRAM position.
 * @param tex Texture whose mode and position are used.
 * @param flags SaveLayoutEntry::flags of the entry.
 * @return The texture page value.
 */
static inline u32 get_save_layout_base_tpage(SaveLayoutTex* tex, u32 flags)
{
    return getTPage(tex->control.mode & 3, flags >> SAVE_LAYOUT_FLAG_BLEND_SHIFT, tex->tex_x, tex->tex_y);
}

/**
 * @brief Draw every entry of g_save_layout_table.
 * @param cursor Next free byte in the primitive buffer.
 * @param ot OT entry to link the primitives into.
 * @return Pointer just past the last primitive.
 */
void* render_save_layout_prims(PrimCursor cursor, u_long* ot)
{
    SaveLayoutEntry* entry;
    s32 entry_index;
    s32 type;
    s32 idx;
    SlotUvRect* uv;
    POLY_FT4* poly;
    u16 panel_x;
    s32 panel_origin_x;
    u16 panel_y;
    s32 panel_origin_y;
    SaveLayoutTex* tex;
    SaveLayoutTex* tpage_tex;
    u16 sprite_x;
    s32 sprite_origin_x;
    u16 sprite_y;
    s32 sprite_origin_y;
    DR_TPAGE* tp;
    TILE* tile;
    s32 remaining;
    u16 u0;
    SaveLayoutTex* strip_tex;
    s32 x;
    s32 y;
    s32 chunk;
    SPRT* sprt;

    entry = g_save_layout_table;

    for (entry_index = 0; entry_index < SAVE_LAYOUT_ENTRY_COUNT; entry_index++, entry++)
    {
        type = entry->type;

        if (type == SAVE_LAYOUT_PRIM_POLY_FT4)
        {
            /* Drawn flipped: the left corners take the texture's right edge. */
            if (g_slot_slide_x > 0)
            {
                idx = g_slot_selected_index + 1;
            }
            else
            {
                idx = 0;
            }

            uv = &g_save_slot_panel_uv_table[idx];

            poly = cursor.poly;
            SET_BGR0_PACKED(poly, GPU_TINT_NEUTRAL);
            setPolyFT4(poly);

            setSemiTrans(poly, entry->flags & SAVE_LAYOUT_FLAG_SEMI_TRANS);

            panel_x = entry->x + g_slot_slide_x_lerped;
            panel_origin_x = uv->ox * 8 - FIGURE_ANCHOR_X;
            poly->x2 = poly->x0 = panel_x - panel_origin_x;
            panel_y = entry->y + g_slot_slide_y_lerped;
            panel_origin_y = uv->oy * 8 - FIGURE_ANCHOR_Y;
            poly->y1 = poly->y0 = panel_y - panel_origin_y;

            poly->x1 = poly->x3 = poly->x0 + uv->w * 8 - 1;
            poly->y2 = poly->y3 = poly->y0 + uv->h * 8 - 1;

            poly->u3 = poly->u1 = uv->u * 8;
            poly->v1 = poly->v0 = uv->v * 8;

            poly->u0 = poly->u2 = poly->u1 + uv->w * 8 - 1;
            poly->v2 = poly->v3 = poly->v0 + uv->h * 8 - 1;

            cursor.poly = poly + 1;

            tex = &g_save_layout_tex_table[entry->tex_slot];
            setClut(poly, tex->clut_x, tex->clut_y);

            tpage_tex = &g_save_layout_tex_table[entry->tex_slot];
            poly->tpage = getTPage(tpage_tex->control.mode & 3, entry->flags >> SAVE_LAYOUT_FLAG_BLEND_SHIFT, tpage_tex->tex_x, tpage_tex->tex_y);

            addPrim(ot, poly);
        }
        else if (type == SAVE_LAYOUT_PRIM_SPRT)
        {
            if (g_slot_slide_x < 0)
            {
                idx = g_slot_selected_index + 1;
            }
            else
            {
                idx = 0;
            }

            SET_BGR0_PACKED(cursor.sprt, GPU_TINT_NEUTRAL);
            setSprt(cursor.sprt);

            uv = &g_save_slot_sprite_uv_table[idx];
            setSemiTrans(cursor.sprt, entry->flags & SAVE_LAYOUT_FLAG_SEMI_TRANS);

            sprite_x = entry->x + g_slot_slide_x_lerped;
            sprite_origin_x = uv->ox * 8 - FIGURE_ANCHOR_X;
            cursor.sprt->x0 = sprite_x - sprite_origin_x;
            sprite_y = entry->y + g_slot_slide_y_lerped;
            sprite_origin_y = uv->oy * 8 - FIGURE_ANCHOR_Y;
            cursor.sprt->y0 = sprite_y - sprite_origin_y;
            setUV0(cursor.sprt, uv->u * 8, uv->v * 8);
            setWH(cursor.sprt, uv->w * 8, uv->h * 8);

            tex = &g_save_layout_tex_table[entry->tex_slot];
            setClut(cursor.sprt, tex->clut_x, tex->clut_y);

            addPrim(ot, cursor.sprt);
            cursor.sprt++;

            tp = cursor.tpage;
            setDrawTPage(tp, 0, 0, get_save_layout_base_tpage(&g_save_layout_tex_table[entry->tex_slot], entry->flags));

            addPrim(ot, tp);
            cursor.tpage++;
        }
        else if (type == SAVE_LAYOUT_PRIM_TILE)
        {
            tile = &cursor.tile_tpage->tile;

            SET_BGR0_PACKED(tile, CURSOR_BAR_COLOR);
            setTile(tile);
            setSemiTrans(tile, 1);

            setXY0(tile, entry->tile_x + g_slot_slide_x_lerped, entry->tile_y + g_slot_slide_y_lerped);
            setWH(tile, entry->width, entry->height);

            addPrim(ot, tile);

            tp = &cursor.tile_tpage->tpage;
            setDrawTPage(tp, 0, 0, getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_ADD, CURSOR_BAR_TPAGE_X, 0));
            addPrim(ot, tp);

            cursor.tile_tpage++;
        }
        else if (type != SAVE_LAYOUT_PRIM_NONE)
        {
            /* A sprite cannot sample across a texture page, so the strip is
               drawn in 128-pixel chunks. */
            remaining = entry->width;
            u0 = entry->u0;
            strip_tex = &g_save_layout_tex_table[entry->tex_slot];
            idx = strip_tex->tex_x;

            if (entry->flags & SAVE_LAYOUT_FLAG_FOLLOWS_SLIDE)
            {
                x = entry->x + g_slot_slide_x_lerped;
                y = entry->y + g_slot_slide_y_lerped;
            }
            else
            {
                x = entry->x;
                y = entry->y;
            }

            chunk = GLYPH_CHUNK_WIDTH;

            if (remaining <= GLYPH_CHUNK_WIDTH)
            {
                chunk = remaining;
            }

            while (1)
            {
                sprt = cursor.sprt;

                SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
                setSprt(sprt);

                setSemiTrans(sprt, entry->flags & SAVE_LAYOUT_FLAG_SEMI_TRANS);

                setXY0(sprt, x, y);
                setUV0(sprt, u0, entry->v0);
                setWH(sprt, chunk, entry->height);

                remaining -= chunk;

                tex = &g_save_layout_tex_table[entry->tex_slot];
                setClut(sprt, tex->clut_x, tex->clut_y);

                addPrim(ot, cursor.sprt);
                cursor.sprt++;

                tp = cursor.tpage;
                setDrawTPage(tp, 0, 0, get_save_layout_tpage(&g_save_layout_tex_table[entry->tex_slot], entry->flags, idx));

                addPrim(ot, cursor.tpage);
                cursor.tpage++;

                if (remaining == 0)
                {
                    break;
                }

                /* A 4-bit page holds two chunks; in other modes each chunk
                   starts a new page. */
                u0 ^= GLYPH_CHUNK_WIDTH;
                tex = &g_save_layout_tex_table[entry->tex_slot];

                if (tex->control.mode == GPU_TEXTURE_4BIT)
                {
                    idx += GLYPH_CHUNK_WIDTH / 4;
                }
                else
                {
                    idx += GLYPH_CHUNK_WIDTH / 2;
                    u0 = 0;
                }

                chunk = GLYPH_CHUNK_WIDTH;

                if (remaining <= GLYPH_CHUNK_WIDTH)
                {
                    chunk = remaining;
                }

                x += GLYPH_CHUNK_WIDTH;
            }
        }

    }

    return cursor.bytes;
}

/**
 * @brief Upload the picker TIMs and record each one's mode and size.
 * @see decomp.me (100%) https://decomp.me/scratch/lzJHa
 */
void upload_save_layout_textures(void)
{
    SaveLayoutTex* tex;
    void* tim;
    TimPrefix* header;
    TimBlock* clut_block;
    TimBlock* pixel_block;
    u32 clut_size;
    RECT rect;
    s32 i;
    u16 clut_width;
    u16 clut_height;

    tex = g_save_layout_tex_table;

    for (i = 0; i < SAVE_LAYOUT_TEX_COUNT; i++)
    {
        /* tim walks the file: header, then past the CLUT to the pixels. */
        tim = tex->src;
        header = tim;
        tex->control.mode = TIM_PIXEL_MODE(header);
        clut_width = header->clut_block.dimensions.width;
        clut_height = header->clut_block.dimensions.height;
        clut_size = header->clut_block.bnum;
        setRECT(&rect, tex->clut_x, tex->clut_y, clut_width * clut_height, 1);
        clut_block = &header->clut_block;
        LoadImage(&rect, TIM_BLOCK_DATA(clut_block));
        tim = TIM_SKIP_BLOCK(clut_block, clut_size);
        pixel_block = tim;
        tex->control.width = pixel_block->dimensions.width;
        tex->control.height = pixel_block->dimensions.height;
        setRECT(&rect, tex->tex_x, tex->tex_y, tex->control.width, tex->control.height);
        LoadImage(&rect, TIM_BLOCK_DATA(pixel_block));
        tex++;
    }
}


/**
 * @brief Replace the saved game with the new-game or the field-start template.
 * @param field_start Zero for a new game (scene 0xD), nonzero to start directly in FIELD (scene 0).
 * @see decomp.me (100%) https://decomp.me/scratch/aPcbW
 */
void load_saved_game_template(s32 field_start)
{
    s32* src;
    s32* dst;
    u32 i;

    if (field_start == 0)
    {
        src = g_new_game_template;
        g_field_entry_scene.id = 0xD;
        g_current_land = 0;
        g_field_music_id = 0;
    }
    else
    {
        src = g_field_start_template;
        g_field_entry_scene.id = 0;
        g_current_land = 0;
        g_field_music_id = 0;
    }

    i = 0;
    dst = g_saved_game.words;

    while (i < SAVED_GAME_TEMPLATE_WORDS)
    {
        *dst++ = *src++;
        i++;
    }
}

/**
 * @brief Copy a block of whole words.
 * @param dst Destination block.
 * @param src Source block.
 * @param count Number of 32-bit words to copy.
 */
static inline void copy_words(void* dst, void* src, u32 count)
{
    s32* to = dst;
    s32* from = src;
    u32 i;

    for (i = 0; i < count; i++)
    {
        to[i] = from[i];
    }
}

/**
 * @brief Copy the chosen hero's starting record into the first party slot.
 * @param hero_type Character type of the hero (0 or 1).
 * @note Hero 1 also sets bit 0 of the saved game's mode flags.
 * @see decomp.me (100%) https://decomp.me/scratch/CU7Ml
 */
void load_hero_template(s32 hero_type)
{
    s32* src;

    if (hero_type != 0)
    {
        src = g_hero_template_type_1;
        g_saved_game.layout.words[SAVED_GAME_WORD_MODE_FLAGS] |= 1;
    }
    else
    {
        src = g_hero_template_type_0;
    }

    copy_words(&g_saved_game.layout.characters[0], src, HERO_TEMPLATE_WORDS);
}
