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

/* SAVED_CHARACTER_SIZE / 4 */
#define HERO_TEMPLATE_WORDS 0x94U

/* SAVED_GAME_DATA_SIZE / 4 */
#define SAVED_GAME_TEMPLATE_WORDS 0xC9AU

#define SAVE_LAYOUT_TEX_COUNT 11
#define SAVE_LAYOUT_ENTRIES 0x1B

#define GLYPH_CHUNK_WIDTH 0x80

/* SaveLayoutEntry::type; any other nonzero value is a glyph strip. */
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

extern SlotUvRect g_save_slot_panel_uv_table[];
extern SlotUvRect g_save_slot_sprite_uv_table[];

void reset_save_slot_panel(void);
void handle_save_slot_input(void);
void animate_save_slot_panel(void);
static void scroll_slots_right(void);
static void scroll_slots_left(void);
void load_hero_template(s32 hero_type);
void upload_save_layout_textures(void);
void* render_save_layout_prims(u8* ptr, u_long* ot);

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
    g_slot_highlight_x = 0;
    g_slot_highlight_target_x = 0;
    g_slot_highlight_frames = 0;
    upload_save_layout_textures();
}

/**
 * @brief Render the save-slot picker and process its input for one frame.
 * @param context Active title display and primitive buffers.
 * @see decomp.me (100%) https://decomp.me/scratch/so5cY
 */
void render_save_slot_menu(TitleMenuContext* context)
{
    context->screen.prim_cursor = (u_long*)render_save_layout_prims((u8*)context->screen.prim_cursor, context->screen.ot);
    handle_save_slot_input();
}

/**
 * @brief Handle one frame of new-game picker input.
 *
 * @details On the home panel, left/right toggles between the two heroes
 * (layout entries 18 and 19) and confirm slides to the chosen hero's weapon
 * list. On a weapon list, up/down picks a weapon and confirm starts the game
 * with that hero and weapon; the other weapon categories lose their techniques.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Xl8gF
 */
void handle_save_slot_input(void)
{
    s32 slide_x_step;
    s32* slide_x_lerped_ptr;
    s32 prev_index;
    s32 next_index;
    s32 slide_y_step;

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
            SaveLayoutEntry* entry;
            play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
            entry = g_save_layout_table;

            if (entry[18].type != 0)
            {
                entry[18].type = 0;
                entry[19].type = 1;
                return;
            }

            entry[18].type = 1;
            entry[19].type = 0;
            return;
        }

        if (g_debounced_input & (PAD_BTN_START | PAD_CONFIRM_BUTTONS))
        {
            play_sfx(SFX_CONFIRM, AKAO_PAN_CENTER);

            if (g_save_layout_table[18].type != 0)
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
                s32 rng_lo;
                int rng_hi;

                load_hero_template(0);
                g_saved_game.layout.characters[0].info.word &= ~FIELD_CHARACTER_TYPE_MASK;
                rng_lo = rand();
                rng_hi = rand();
                rng_lo |= rng_hi << TITLE_RNG_HIGH_SHIFT;
                g_saved_game.layout.identity.ids.game_id = rng_lo;
            }
            else
            {
                s32 rng_lo;
                int rng_hi;

                load_hero_template(1);
                g_saved_game.layout.characters[0].info.word = (g_saved_game.layout.characters[0].info.word & ~FIELD_CHARACTER_TYPE_MASK) | 1;
                rng_lo = rand();
                rng_hi = rand();
                rng_lo |= rng_hi << TITLE_RNG_HIGH_SHIFT;
                g_saved_game.layout.identity.ids.game_id = rng_lo;
            }

            {
                s32 i;
                u8* src;
                u8* dst;

                dst = (u8*)&g_saved_game.layout.characters[0].equipment[FIELD_WEAPON_SLOT];
                src = (u8*)&g_starting_weapon_records[g_slot_selected_index];
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
            }

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
        else if (g_slot_highlight_frames == 0)
        {
            if ((g_debounced_input & PAD_BTN_UP) != 0U)
            {
                play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
                prev_index = g_slot_selected_index - 1;
                g_slot_selected_index = prev_index;

                if (prev_index < 0)
                {
                    g_slot_selected_index = 0xA;
                }
            }

            if (g_debounced_input & PAD_BTN_DOWN)
            {
                play_sfx(SFX_CURSOR, AKAO_PAN_CENTER);
                next_index = g_slot_selected_index + 1;
                g_slot_selected_index = next_index;

                if (next_index >= 0xB)
                {
                    g_slot_selected_index = 0;
                }
            }
        }

        animate_save_slot_panel();
    }
}

/**
 * @brief Scroll the weapon list to keep the selection visible and move the highlight.
 * @note The list shows seven 16-pixel rows at a time.
 * @see decomp.me (100%) https://decomp.me/scratch/d3s3Q
 */
void animate_save_slot_panel(void)
{
    SaveLayoutEntry* layout;
    s16 scroll_width;
    s32 target_adjusted;
    s32 scroll_offset;
    s32 highlight_position;
    SaveLayoutEntry* ptr;

    if (g_slot_highlight_frames != 0)
    {
        g_slot_highlight_x += (g_slot_highlight_target_x - g_slot_highlight_x) / g_slot_highlight_frames;
        g_slot_highlight_frames--;
    }
    else
    {
        g_slot_highlight_x = g_slot_highlight_target_x;
    }

    highlight_position = g_slot_highlight_target_x;
    target_adjusted = highlight_position;

    if (highlight_position < 0)
    {
        target_adjusted = highlight_position + 0xF;
    }

    target_adjusted >>= 4;
    scroll_offset = g_slot_selected_index;

    if (g_slot_selected_index < target_adjusted)
    {
        g_slot_highlight_target_x = scroll_offset * 0x10;
        g_slot_highlight_frames = 4;
    }
    else if ((target_adjusted + 6) < g_slot_selected_index)
    {
        g_slot_highlight_target_x = (g_slot_selected_index - 6) * 0x10;
        g_slot_highlight_frames = 4;
    }

    ptr = g_save_layout_table;
    ptr[2].v0 = g_slot_highlight_x;
    ptr[3].v0 = (g_slot_highlight_x) + SAVE_HIGHLIGHT_SPAN;
    ptr[9].v0 = g_slot_highlight_x;
    ptr[10].v0 = (g_slot_highlight_x) + SAVE_HIGHLIGHT_SPAN;

    if (g_slot_highlight_x != 0)
    {
        g_save_layout_table[7].type = 1;
        g_save_layout_table[8].type = 1;
        g_save_layout_table[14].type = 1;
        g_save_layout_table[15].type = 1;
    }
    else
    {
        g_save_layout_table[7].type = 0;
        g_save_layout_table[8].type = 0;
        g_save_layout_table[14].type = 0;
        g_save_layout_table[15].type = 0;
    }

    if (g_slot_highlight_x != 0x40)
    {
        g_save_layout_table[4].type = 1;
        g_save_layout_table[5].type = 1;
        g_save_layout_table[11].type = 1;
        g_save_layout_table[12].type = 1;
    }
    else
    {
        g_save_layout_table[4].type = 0;
        g_save_layout_table[5].type = 0;
        g_save_layout_table[11].type = 0;
        g_save_layout_table[12].type = 0;
    }

    scroll_offset = (g_slot_selected_index * 0x10) - (highlight_position = g_slot_highlight_x);

    if (scroll_offset < 0)
    {
        scroll_offset = 0;
    }

    if (scroll_offset > 0x60)
    {
        scroll_offset = 0x60;
    }

    layout = g_save_layout_table;
    scroll_width = scroll_offset + SAVE_SCROLL_WIDTH_HOME;
    layout[6].y = scroll_width;
    layout[6].tile_y = scroll_width;
    layout[13].y = scroll_width;
    layout[13].tile_y = scroll_width;
}

/**
 * @brief Reset the weapon-list selection and scroll after a panel slide.
 * @see decomp.me (100%) https://decomp.me/scratch/0YgmZ
 */
void reset_save_slot_panel(void)
{
    s16 highlight_bottom_v;

    if (g_slot_slide_x != 0)
    {
        SaveLayoutEntry* entry = g_save_layout_table;
        entry[0].v0 = SAVE_SLOT_HOME_V;
        g_slot_selected_index = 0;
        g_slot_highlight_x = 0;
        g_slot_highlight_target_x = 0;
        g_slot_highlight_frames = 0;
        entry[7].type = 0;
        entry[8].type = 0;
        entry[14].type = 0;
        entry[15].type = 0;
        highlight_bottom_v = (g_slot_highlight_x) + SAVE_HIGHLIGHT_SPAN;
        entry[6].y = SAVE_SCROLL_WIDTH_HOME;
        entry[6].tile_y = SAVE_SCROLL_WIDTH_HOME;
        entry[13].y = SAVE_SCROLL_WIDTH_HOME;
        entry[13].tile_y = SAVE_SCROLL_WIDTH_HOME;
        entry[0].u0 = 0;
        entry[4].type = 1;
        entry[5].type = 1;
        entry[11].type = 1;
        entry[12].type = 1;

        entry[2].v0 = g_slot_highlight_x;
        entry[3].v0 = highlight_bottom_v;
        entry[9].v0 = g_slot_highlight_x;
        entry[10].v0 = highlight_bottom_v;
        return;
    }

    {
        SaveLayoutEntry* entry = g_save_layout_table;
        entry[0].u0 = SAVE_SLOT_HOME_V;
        entry[0].v0 = 0;
    }
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
 * @param flags Blend mode in bits 2 and up.
 * @param x VRAM x of the page.
 * @return The texture page value.
 */
static inline u32 get_save_layout_tpage(SaveLayoutTex* tex, u32 flags, s32 x)
{
    return getTPage(tex->control.mode & 3, flags >> 2, x, *(u16*)&tex->tex_y);
}

/**
 * @brief Texture page of a save-layout texture at its own VRAM position.
 * @param tex Texture whose mode and position are used.
 * @param flags Blend mode in bits 2 and up.
 * @return The texture page value.
 */
static inline u32 get_save_layout_base_tpage(SaveLayoutTex* tex, u32 flags)
{
    return getTPage(tex->control.mode & 3, flags >> 2, *(u16*)&tex->tex_x, *(u16*)&tex->tex_y);
}

/**
 * @brief Draw every entry of g_save_layout_table.
 * @param ptr Next free byte in the primitive buffer.
 * @param ot OT entry to link the primitives into.
 * @return Pointer just past the last primitive.
 */
void* render_save_layout_prims(u8* ptr, u_long* ot)
{
    SaveLayoutEntry* entry = g_save_layout_table;
    s32 entry_index = 0;
    s32 idx;
    u32 tint;
    SlotUvRect* uv;

    while (entry_index < SAVE_LAYOUT_ENTRIES)
    {
        s32 type = entry->type;

        if (type == SAVE_LAYOUT_PRIM_POLY_FT4)
        {
            /* Drawn flipped: the left corners take the texture's right edge. */
            POLY_FT4* poly;
            u16 vx;
            s32 offx;
            u16 vy;
            s32 offy;
            SaveLayoutTex* tex;
            SaveLayoutTex* tex2;
            u32 tpw;

            if (g_slot_slide_x > 0)
            {
                idx = g_slot_selected_index + 1;
            }
            else
            {
                idx = 0;
            }

            uv = &g_save_slot_panel_uv_table[idx];

            poly = (POLY_FT4*)ptr;
            tint = GPU_TINT_NEUTRAL;
            SET_BGR0_PACKED(poly, tint);
            setPolyFT4(poly);

            setSemiTrans(poly, *(u32*)entry & 2);

            vx = entry->x + g_slot_slide_x_lerped;
            offx = uv->ox * 8 - 0x20;
            poly->x2 = poly->x0 = vx - offx;
            vy = entry->y + g_slot_slide_y_lerped;
            offy = uv->oy * 8 - 0x28;
            poly->y1 = poly->y0 = vy - offy;

            poly->x1 = poly->x3 = (poly->x0 + (uv->w * 8)) - 1;
            poly->y2 = poly->y3 = (poly->y0 + (uv->h * 8)) - 1;

            poly->u3 = poly->u1 = uv->u * 8;
            poly->v1 = poly->v0 = uv->v * 8;

            poly->u0 = poly->u2 = (poly->u1 + (uv->w * 8)) - 1;
            poly->v2 = poly->v3 = (poly->v0 + (uv->h * 8)) - 1;

            ptr = (u8*)poly + sizeof(POLY_FT4);

            tex = &(g_save_layout_tex_table)[entry->tex_slot];
            setClut(poly, *(u16*)&tex->clut_x, *(u16*)&tex->clut_y);

            tex2 = &(g_save_layout_tex_table)[entry->tex_slot];
            tpw =getTPage(*(u8*)&tex2->control & 3, *(u32*)entry >> 2, *(u16*)&tex2->tex_x, *(u16*)&tex2->tex_y);
            poly->tpage = tpw;

            addPrim(ot, poly);
        }
        else
        {
            if (type == SAVE_LAYOUT_PRIM_SPRT)
            {
                u16 vx;
                s32 offx;
                u16 vy;
                s32 offy;
                SaveLayoutTex* tex;
                DR_TPAGE* tp;

                if (g_slot_slide_x < 0)
                {
                    idx = g_slot_selected_index + 1;
                }
                else
                {
                    idx = 0;
                }

                tint = GPU_TINT_NEUTRAL;
                SET_BGR0_PACKED((SPRT*)ptr, tint);
                setSprt((SPRT*)ptr);

                uv = &g_save_slot_sprite_uv_table[idx];
                setSemiTrans((SPRT*)ptr, *(u32*)entry & 2);

                vx = entry->x + g_slot_slide_x_lerped;
                offx = uv->ox * 8 - 0x20;
                ((SPRT*)ptr)->x0 = vx - offx;
                vy = entry->y + g_slot_slide_y_lerped;
                offy = uv->oy * 8 - 0x28;
                ((SPRT*)ptr)->y0 = vy - offy;
                setUV0((SPRT*)ptr, uv->u * 8, uv->v * 8);
                setWH((SPRT*)ptr, uv->w * 8, uv->h * 8);

                tex = &(g_save_layout_tex_table)[entry->tex_slot];
                setClut((SPRT*)ptr, *(u16*)&tex->clut_x, *(u16*)&tex->clut_y);

                addPrim(ot, ptr);
                ptr += sizeof(SPRT);

                tp = (DR_TPAGE*)ptr;
                setDrawTPage(tp, 0, 0, get_save_layout_base_tpage(&(g_save_layout_tex_table)[entry->tex_slot], *(u32*)entry));

                addPrim(ot, tp);
                ptr += sizeof(DR_TPAGE);
            }
            else
            {
                if (type == SAVE_LAYOUT_PRIM_TILE)
                {
                    TILE* tile = (TILE*)ptr;
                    DR_TPAGE* tp;

                    SET_BGR0_PACKED(tile, GPU_COLOR_WORD(0x40, 0, 0));
                    setTile(tile);
                    setSemiTrans(tile, 1);

                    setXY0(tile, entry->tile_x + g_slot_slide_x_lerped, entry->tile_y + g_slot_slide_y_lerped);
                    setWH(tile, entry->width, entry->height);

                    addPrim(ot, tile);

                    tp = (DR_TPAGE*)(ptr + sizeof(TILE));
                    setDrawTPage(tp, 0, 0, getTPage(GPU_TEXTURE_4BIT, GPU_BLEND_ADD, 0x140, 0));
                    addPrim(ot, tp);

                    ptr += sizeof(TILE) + sizeof(DR_TPAGE);
                }
                else if (type != SAVE_LAYOUT_PRIM_NONE)
                {
                    /* A sprite cannot sample across a texture page, so the
                       strip is drawn in 128-pixel chunks. */
                    s32 remaining = entry->width;
                    u16 u0 = entry->u0;
                    SaveLayoutTex* tex;
                    SaveLayoutTex* tex0 = &(g_save_layout_tex_table)[entry->tex_slot];
                    s32 x;
                    s32 y;
                    s32 chunk;

                    idx = *(u16*)tex0;

                    if (*(u32*)entry & 1)
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
                        SPRT* sprt = (SPRT*)ptr;
                        DR_TPAGE* tp;

                        SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
                        setSprt(sprt);

                        setSemiTrans(sprt, *(u32*)entry & 2);

                        setXY0(sprt, x, y);
                        setUV0(sprt, u0, entry->v0);
                        setWH(sprt, chunk, entry->height);

                        remaining -= chunk;

                        tex = &(g_save_layout_tex_table)[entry->tex_slot];
                        setClut(sprt, *(u16*)&tex->clut_x, *(u16*)&tex->clut_y);

                        addPrim(ot, ptr);
                        ptr += sizeof(SPRT);

                        tp = (DR_TPAGE*)ptr;
                        setDrawTPage(tp, 0, 0, get_save_layout_tpage(&(g_save_layout_tex_table)[entry->tex_slot], *(u32*)entry, idx));

                        addPrim(ot, ptr);
                        ptr += sizeof(DR_TPAGE);

                        if (remaining == 0)
                        {
                            break;
                        }

                        /* A 4-bit page holds two chunks; in other modes each
                           chunk starts a new page. */
                        u0 ^= 0x80;
                        tex = &(g_save_layout_tex_table)[entry->tex_slot];

                        if (tex->control.mode == 0)
                        {
                            idx += 0x20;
                        }
                        else
                        {
                            idx += 0x40;
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
        }

        entry_index++;
        entry++;
    }

    return ptr;
}

/**
 * @brief Upload the picker TIMs and record each one's mode and size.
 * @see decomp.me (100%) https://decomp.me/scratch/lzJHa
 */
void upload_save_layout_textures(void)
{
    SaveLayoutTex* tex;
    u8* tim;
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
        tim = tex->src;
        tex->control.mode = tim[4];
        clut_width = ((TimPrefix*)tim)->clut_block.dimensions.width;
        clut_height = ((TimPrefix*)tim)->clut_block.dimensions.height;
        clut_size = ((TimPrefix*)tim)->clut_block.bnum;
        setRECT(&rect, tex->clut_x, tex->clut_y, clut_width * clut_height, 1);
        clut_block = &((TimPrefix*)tim)->clut_block;
        LoadImage(&rect, (u_long*)(clut_block + 1));
        tim = (u8*)clut_block + clut_size;
        pixel_block = (TimBlock*)tim;
        tex->control.width = pixel_block->dimensions.width;
        tex->control.height = pixel_block->dimensions.height;
        setRECT(&rect, tex->tex_x, tex->tex_y, tex->control.width, tex->control.height);
        LoadImage(&rect, (u_long*)(pixel_block + 1));
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
        src = (s32*)&g_new_game_template;
        g_field_entry_scene.id = 0xD;
        g_current_land = 0;
        g_field_music_id = 0;
    }
    else
    {
        src = (s32*)&g_field_start_template;
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
 * @brief Copy the chosen hero's starting record into the first party slot.
 * @param hero_type Character type of the hero (0 or 1).
 * @note Hero 1 also sets bit 0 of the saved game's mode flags.
 * @see decomp.me (100%) https://decomp.me/scratch/CU7Ml
 */
void load_hero_template(s32 hero_type)
{
    s32* src;
    s32* dst;
    u32 i;

    if (hero_type != 0)
    {
        src = g_hero_template_type_1;
        g_saved_game.layout.words[SAVED_GAME_WORD_MODE_FLAGS] |= 1;
    }
    else
    {
        src = g_hero_template_type_0;
    }

    dst = (s32*)&g_saved_game.layout.characters[0];

    for (i = 0; i < HERO_TEMPLATE_WORDS; i++)
    {
        dst[i] = src[i];
    }
}
