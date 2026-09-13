#include "title_internal.h"

/* Width in pixels of a single save-slot panel; one horizontal slide moves the
 * stage by exactly this much. */
#define SLOT_PANEL_WIDTH 160

/* Number of frames the slide-lerper takes to animate a full panel scroll. */
#define SLOT_SLIDE_FRAMES 8

/* Length, in 32-bit words, of each sub-menu layout table copied by
 * load_sub_menu_layout (0x94 words == 0x250 bytes). */
#define SUB_MENU_LAYOUT_WORDS 0x94U

/* Length, in 32-bit words, of a full menu-layout template copied by
 * load_menu_layout (0xC9A words). */
#define MENU_LAYOUT_WORDS 0xC9AU

static void scroll_slots_right(void);
static void scroll_slots_left(void);

/**
 * Counterpart of CHECKPS update_controller_input.
 *
 * decomp.me (100%) https://decomp.me/scratch/1dQbp
 */
static void read_pad_input(void)
{
    SCDRegs* base = SCD_REGS;
    s32 state;
    u32 buttons;
    s16 axis;

    g_debouncedInput = 0;
    if (D_801ED600[0] >= 254)
    {
        state = 0;
    }
    else
    {
        buttons = ((base->held_buttons >> 8) & 0xFF) | (base->held_buttons << 8);
        buttons = PAD_REMAP_FACE_BITS(buttons);
        if (base->device_type != 0)
        {
            axis = base->axis_x.signed_value;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_LEFT;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_RIGHT;
            }
            axis = base->axis_y.signed_value;
            if (axis < (-1))
            {
                buttons |= PAD_BTN_UP;
            }
            else if (axis >= 2)
            {
                buttons |= PAD_BTN_DOWN;
            }
        }
        state = buttons;
    }
    g_lastInputState = state;
    g_inputRepeatTimer = 15;
}

/**
 * Initialises the save-slot sub-menu state and uploads its sprite atlases.
 *
 * decomp.me (100%) https://decomp.me/scratch/t2lHt
 */
void InitSaveSlotMenu(void)
{
    read_pad_input();
    g_slotSlideFrames = 0;
    g_slotSlideYLerped = 0;
    g_slotSlideY = 0;
    g_slotSlideXLerped = 0;
    g_slotSlideX = 0;
    g_slotSelectedIndex = 0;
    g_slotHighlightX = 0;
    g_slotHighlightTargetX = 0;
    g_slotHighlightFrames = 0;
    upload_save_layout_textures();
}

/**
 * decomp.me (100%) https://decomp.me/scratch/so5cY
 */
void RenderSaveSlotMenu(MenuContext* arg0)
{
    arg0->next_prim_ptr = (u_long*)RenderSaveLayoutPrims(arg0->next_prim_ptr, (u_long*)((char*)arg0 + 0x40));
    handle_save_slot_input();
}

/**
 * @brief Per-frame input dispatcher for the save-slot sub-menu.
 *
 * @details While a slide is in flight (g_slotSlideFrames != 0) it only steps
 * the X/Y slide lerpers and returns. Once settled it snaps the lerpers to
 * their targets, reads input, and branches on whether the stage is at its
 * home column (g_slotSlideX == 0) or scrolled to a side panel:
 *  - Home column: confirm toggles the new-game expand entries (18/19), a
 *    left/right press scrolls to a side panel, and cancel quits the sub-menu
 *    (g_titleMenuExitState = 2).
 *  - Side panel: confirm loads the matching sub-menu layout, seeds its RNG,
 *    copies the selected starting-weapon record into D_80043618, clears the per-slot
 *    field of every other menu-layout slot, and confirms (exit state 1);
 *    cancel scrolls back home; up/down move the slot cursor (wrapping over
 *    the 11 slots). Always re-runs the highlight-panel animation.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/Xl8gF
 */
void handle_save_slot_input(void)
{
    s32 slide_x_step;
    u8* unused_ptr;
    s32* slide_x_lerped_ptr;
    s32 prev_index;
    s32 next_index;
    s32 slide_y_step;
    s32 new_flags;
    s32 flag_mask;
    if (g_slotSlideFrames != 0)
    {
        slide_x_lerped_ptr = &g_slotSlideXLerped;
        slide_x_step = ((s32)(g_slotSlideX - (*slide_x_lerped_ptr))) / ((s32)g_slotSlideFrames);
        slide_y_step = ((s32)(g_slotSlideY - g_slotSlideYLerped)) / ((s32)g_slotSlideFrames);
        g_slotSlideFrames -= 1;
        g_slotSlideXLerped += slide_x_step;
        g_slotSlideYLerped += slide_y_step;
        return;
    }
    g_slotSlideXLerped = g_slotSlideX;
    g_slotSlideYLerped = g_slotSlideY;
    update_menu_input();
    if (g_slotSlideX == 0)
    {
        if (g_debouncedInput & (PAD_BTN_LEFT | PAD_BTN_RIGHT))
        {
            SaveLayoutEntry* entry;
            play_title_sfx(0x7D, 0x80);
            entry = ((SaveLayoutEntry*)g_saveLayoutTable);
            if (entry[18].type != 0)
            {
                entry[18].type = 0;
                entry[19].type = 1;
                return;
            }
            entry[18].type = 1;
            entry[19].type = g_slotSlideYLerped * 0;
            return;
        }
        if (g_debouncedInput & (PAD_BTN_START | PAD_BTN_L3 | PAD_BTN_CROSS))
        {
            play_title_sfx(0x7E, 0x80);
            if (D_800F9AED != 0)
            {
                scroll_slots_right();
                reset_save_slot_panel();
                return;
            }
            scroll_slots_left();
            reset_save_slot_panel();
            return;
        }
        if (g_debouncedInput & PAD_BTN_CIRCLE)
        {
            play_title_sfx(0x7F, 0x80);
            g_titleMenuExitState = 2;
        }
    }
    else
    {
        if (g_debouncedInput & (PAD_BTN_START | PAD_BTN_L3 | PAD_BTN_CROSS))
        {
            if (g_slotSlideX > 0)
            {
                s32 rng_lo;
                int rng_hi;
                u8* layout;

                load_sub_menu_layout(0);
                flag_mask = ~0x7F;
                layout = g_menuLayoutBuffer;
                new_flags = *(s32*)(layout + 0x608) & flag_mask;
                *(s32*)(layout + 0x608) = new_flags;
                rng_lo = rand();
                rng_hi = rand();
                rng_lo |= rng_hi << 0xF;
                *(s16*)(layout + 0xD4) = (s16)rng_lo;
            }
            else
            {
                s32 rng_lo;
                int rng_hi;
                u8* layout;

                load_sub_menu_layout(1);
                flag_mask = ~0x7F;
                layout = g_menuLayoutBuffer;
                new_flags = (*(s32*)(layout + 0x608) & flag_mask) | 1;
                *(s32*)(layout + 0x608) = new_flags;
                rng_lo = rand();
                rng_hi = rand();
                rng_lo |= rng_hi << 0xF;
                *(s16*)(layout + 0xD4) = (s16)rng_lo;
            }
            {
                s32 selected_slot;
                s32 slot_idx;
                u32 copy_count;
                u8 byte;
                u8* src_ptr;
                u8* dest_ptr;

                dest_ptr = D_80043618;
                src_ptr = g_startingWeaponRecords + (g_slotSelectedIndex << 6);
                copy_count = 0;
                while (copy_count < 0x40U)
                {
                    copy_count += 1;
                    byte = *src_ptr;
                    src_ptr += 1;
                    *dest_ptr = byte;
                    dest_ptr += 1;
                }

                slot_idx = 0;
                selected_slot = g_slotSelectedIndex;
                src_ptr = g_menuLayoutBuffer;
                slot_idx = 0;
                do
                {
                    if (selected_slot != slot_idx)
                    {
                        *((s32*)(src_ptr + 0x34)) = 0;
                    }
                    slot_idx += 1;
                    src_ptr += 4;
                } while (slot_idx < 0xB);
                play_title_sfx(0x7E, 0x80);
            }
            g_titleMenuExitState = 1;
        }
        else if (g_debouncedInput & PAD_BTN_CIRCLE)
        {
            play_title_sfx(0x7F, 0x80);
            if (g_slotSlideX > 0)
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
        else if (g_slotHighlightFrames == 0)
        {
            if ((g_debouncedInput & PAD_BTN_UP) != 0U)
            {
                play_title_sfx(0x7D, 0x80);
                prev_index = g_slotSelectedIndex - 1;
                g_slotSelectedIndex = prev_index;
                if (prev_index < 0)
                {
                    g_slotSelectedIndex = 0xA;
                }
            }
            if (g_debouncedInput & PAD_BTN_DOWN)
            {
                play_title_sfx(0x7D, 0x80);
                next_index = g_slotSelectedIndex + 1;
                g_slotSelectedIndex = next_index;
                if (next_index >= 0xB)
                {
                    g_slotSelectedIndex = 0;
                }
            }
        }
        AnimateSaveSlotPanel();
    }
}

/**
 * Lerps g_slotHighlightX toward g_slotHighlightTargetX over
 * g_slotHighlightFrames frames, pans the scroll window so the selected
 * slot is always visible, then writes the updated V-coordinate and
 * visibility flags for the highlight-bar layout entries in g_saveLayoutTable.
 *
 * decomp.me (100%) https://decomp.me/scratch/d3s3Q
 */
void AnimateSaveSlotPanel(void)
{
    u8* layout;
    s16 scroll_width;
    s32* new_var2;
    s32 target_adjusted;
    s32 scroll_offset;
    s32 new_var;
    SaveLayoutEntry* ptr;
    if (g_slotHighlightFrames != 0)
    {
        g_slotHighlightX += (g_slotHighlightTargetX - g_slotHighlightX) / g_slotHighlightFrames;
        g_slotHighlightFrames -= 1;
    }
    else
    {
        g_slotHighlightX = g_slotHighlightTargetX;
    }
    new_var = g_slotHighlightTargetX;
    target_adjusted = new_var;
    if (new_var < 0)
    {
        target_adjusted = new_var + 0xF;
    }
    target_adjusted >>= 4;
    scroll_offset = *(new_var2 = &g_slotSelectedIndex);
    if (g_slotSelectedIndex < target_adjusted)
    {
        g_slotHighlightTargetX = scroll_offset * 0x10;
        g_slotHighlightFrames = 4;
    }
    else if ((target_adjusted + 6) < (*new_var2))
    {
        g_slotHighlightTargetX = (g_slotSelectedIndex - 6) * 0x10;
        g_slotHighlightFrames = 4;
    }
    ptr = (SaveLayoutEntry*)g_saveLayoutTable;
    (ptr + 2)->v0 = (u16)g_slotHighlightX;
    ptr[3].v0 = ((u16)g_slotHighlightX) + 0x20;
    ptr[9].v0 = (u16)g_slotHighlightX;
    ptr[10].v0 = ((u16)g_slotHighlightX) + 0x20;
    if (g_slotHighlightX != 0)
    {
        SaveLayoutEntry* ptr4 = (SaveLayoutEntry*)g_saveLayoutTable;
        ptr4[7].type = 1;
        ptr4[8].type = 1;
        ptr4[14].type = 1;
        ptr4[15].type = 1;
    }
    else
    {
        SaveLayoutEntry* ptr5 = (SaveLayoutEntry*)g_saveLayoutTable;
        ptr5[7].type = 0;
        ptr5[8].type = 0;
        ptr5[14].type = 0;
        ptr5[15].type = 0;
    }
    if (g_slotHighlightX != 0x40)
    {
        SaveLayoutEntry* ptr3 = (SaveLayoutEntry*)g_saveLayoutTable;
        ptr3[4].type = 1;
        ptr3[5].type = 1;
        ptr3[11].type = 1;
        ptr3[12].type = 1;
    }
    else
    {
        SaveLayoutEntry* ptr2 = (SaveLayoutEntry*)g_saveLayoutTable;
        ptr2[4].type = 0;
        ptr2[5].type = 0;
        ptr2[11].type = 0;
        ptr2[12].type = 0;
    }
    scroll_offset = (g_slotSelectedIndex * 0x10) - (new_var = g_slotHighlightX);
    if (scroll_offset < 0)
    {
        scroll_offset = 0;
    }
    if (scroll_offset > 0x60)
    {
        scroll_offset = 0x60;
    }
    layout = g_saveLayoutTable;
    scroll_width = scroll_offset + 0x40;
    *((u16*)(layout + 0x96)) = scroll_width;
    *((u16*)(layout + 0x9A)) = scroll_width;
    *((u16*)(layout + 0x13E)) = scroll_width;
    *((u16*)(layout + 0x142)) = scroll_width;
}

/**
 * @brief Snap the save-slot panel back to its home position and clear its
 *        highlight/selection state.
 *
 * @details When a horizontal slide is in progress (g_slotSlideX != 0) this
 * rebuilds the panel's layout entries in g_saveLayoutTable: it re-homes the
 * scroll window (entries 6 and 13 reset to SAVE_SCROLL_WIDTH_HOME), shows the
 * right highlight halves (entries 4/5/11/12) while hiding the left halves
 * (entries 7/8/14/15), and rewrites the highlight-bar V coordinates (entries
 * 2/3/9/10) from the now-zeroed g_slotHighlightX. The selection/highlight
 * globals are all cleared. When no slide is active it only resets entry 0's
 * U/V to their home values.
 *
 * @note When a slide is active the entries are reached through a
 *       @ref SaveLayoutEntry pointer, matching AnimateSaveSlotPanel. The
 *       inactive-slide branch indexes the table by g_slotSlideX (always 0
 *       here) added to its base, so it stays raw pointer arithmetic.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/0YgmZ
 */
void reset_save_slot_panel(void)
{
    s16 highlight_bottom_v;
    if (g_slotSlideX != 0)
    {
        SaveLayoutEntry* entry = (SaveLayoutEntry*)g_saveLayoutTable;
        entry[0].v0 = SAVE_SLOT_HOME_V;
        g_slotSelectedIndex = 0;
        g_slotHighlightX = 0;
        g_slotHighlightTargetX = 0;
        g_slotHighlightFrames = 0;
        entry[7].type = 0;
        entry[8].type = 0;
        entry[14].type = 0;
        entry[15].type = 0;
        highlight_bottom_v = ((u16)g_slotHighlightX) + SAVE_HIGHLIGHT_SPAN;
        entry[6].y = SAVE_SCROLL_WIDTH_HOME;
        entry[6].tile_y = SAVE_SCROLL_WIDTH_HOME;
        entry[13].y = SAVE_SCROLL_WIDTH_HOME;
        entry[13].tile_y = SAVE_SCROLL_WIDTH_HOME;
        entry[0].u0 = 0;
        entry[4].type = 1;
        entry[5].type = 1;
        entry[11].type = 1;
        entry[12].type = 1;

        entry[2].v0 = (u16)g_slotHighlightX;
        entry[3].v0 = highlight_bottom_v;
        entry[9].v0 = (u16)g_slotHighlightX;
        entry[10].v0 = highlight_bottom_v;
        return;
    }
    {
        u32 low_addr = (u32)(&g_saveLayoutTable);
        u32 ptr = g_slotSlideX + low_addr;
        *((u16*)(ptr + 0xC)) = SAVE_SLOT_HOME_V;
        *((u16*)(ptr + 0xE)) = 0;
    }
}

/**
 * @brief Begins a slide of the save-slot stage one panel to the right.
 *
 * @details Sets the slide target to +SLOT_PANEL_WIDTH and seeds the lerper
 * with SLOT_SLIDE_FRAMES frames of remaining travel. If the lerper is
 * already showing the right-hand panel (g_slotSlideXLerped == SLOT_PANEL_WIDTH),
 * the call is a no-op so we don't accumulate further offset off the edge.
 *
 * @param void No parameters.
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/SRP9z
 */
static void scroll_slots_right(void)
{
    if (g_slotSlideXLerped != SLOT_PANEL_WIDTH)
    {
        g_slotSlideX += SLOT_PANEL_WIDTH;
        g_slotSlideFrames = SLOT_SLIDE_FRAMES;
    }
}

/**
 * @brief Begins a slide of the save-slot stage one panel to the left.
 *
 * @details Mirror of scroll_slots_right: nudges the slide target by
 * -SLOT_PANEL_WIDTH and re-arms the lerper with SLOT_SLIDE_FRAMES of
 * travel. No-ops when the lerper is already at the left-hand limit so
 * the offset cannot run away off-stage.
 *
 * @param void No parameters.
 * @return void No return value.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/W1iA5
 */
static void scroll_slots_left(void)
{
    if (g_slotSlideXLerped != -SLOT_PANEL_WIDTH)
    {
        g_slotSlideX -= SLOT_PANEL_WIDTH;
        g_slotSlideFrames = SLOT_SLIDE_FRAMES;
    }
}

/**
 * @brief One UV/size descriptor in the save-slot panel and sprite UV tables.
 *
 * @note Every field is in 8-pixel units; the renderer multiplies by 8 on use.
 */
typedef struct
{
    u8 u;     /**< +0x00: source U, in 8-pixel units */
    u8 v;     /**< +0x01: source V, in 8-pixel units */
    u8 w;     /**< +0x02: width, in 8-pixel units */
    u8 h;     /**< +0x03: height, in 8-pixel units */
    u8 ox;    /**< +0x04: X origin offset, in 8-pixel units */
    u8 oy;    /**< +0x05: Y origin offset, in 8-pixel units */
} SlotUvRect; /* sizeof == 6 */

/** Number of entries in g_saveLayoutTable. */
#define SAVE_LAYOUT_ENTRIES 0x1B

/** A glyph strip wider than this is split into chunks of this many pixels. */
#define GLYPH_CHUNK_WIDTH 0x80

/** Primitive selectors stored in SaveLayoutEntry::type. */
#define SAVE_LAYOUT_PRIM_NONE 0
#define SAVE_LAYOUT_PRIM_TILE 2
#define SAVE_LAYOUT_PRIM_POLY_FT4 3
#define SAVE_LAYOUT_PRIM_SPRT 4

static inline u32 get_save_layout_tpage(SaveLayoutTex* tex, u32 flags, s32 x)
{
    return getTPage(tex->control & 3, flags >> 2, x, *(u16*)&tex->tex_y);
}

static inline u32 get_save_layout_base_tpage(SaveLayoutTex* tex, u32 flags)
{
    return getTPage(tex->control & 3, flags >> 2, *(u16*)&tex->tex_x, *(u16*)&tex->tex_y);
}

/**
 * @brief Build the GPU primitive stream for the save-slot layout.
 *
 * @param ptr Pointer to the next free byte in the primitive buffer.
 * @param ot Pointer to the ordering-table entry receiving each primitive.
 * @return Pointer to the byte just past the last primitive emitted.
 *
 * @see decomp.me (100%)
 */
void* RenderSaveLayoutPrims(u8* ptr, u_long* ot)
{
    SaveLayoutEntry* entry = (SaveLayoutEntry*)g_saveLayoutTable;
    s32 i = 0;
    s32 tile_len = SAVE_LAYOUT_PRIM_POLY_FT4;
    s32 idx;
    u32 tint;
    SlotUvRect* uv;

    do
    {
        s32 type = entry->type;

        if (type == tile_len)
        {
            /* Slot panel background: a single textured quad. */
            POLY_FT4* poly;
            u16 vx;
            s32 offx;
            u16 vy;
            s32 offy;
            SaveLayoutTex* tex;
            SaveLayoutTex* tex2;
            u32 tpw;

            if (g_slotSlideX > 0)
            {
                idx = g_slotSelectedIndex + 1;
            }
            else
            {
                idx = 0;
            }

            uv = (SlotUvRect*)((idx * 6) + (u32)g_saveSlotPanelUvTable);

            poly = (POLY_FT4*)ptr;
            tint = GPU_TINT_NEUTRAL;
            SET_BGR0_PACKED(poly, tint);
            setPolyFT4(poly);

            setSemiTrans(poly, *(u32*)entry & 2);

            /* Share each truncated base coordinate across its two corners. */
            vx = *(u16*)&entry->x + g_slotSlideXLerped;
            offx = uv->ox * 8 - 0x20;
            poly->x2 = poly->x0 = vx - offx;
            vy = *(u16*)&entry->y + g_slotSlideYLerped;
            offy = uv->oy * 8 - 0x28;
            poly->y1 = poly->y0 = vy - offy;

            poly->x1 = poly->x3 = (poly->x0 + (uv->w * 8)) - 1;
            poly->y2 = poly->y3 = (poly->y0 + (uv->h * 8)) - 1;

            poly->u3 = poly->u1 = uv->u * 8;
            poly->v1 = poly->v0 = uv->v * 8;

            poly->u0 = poly->u2 = (poly->u1 + (uv->w * 8)) - 1;
            poly->v2 = poly->v3 = (poly->v0 + (uv->h * 8)) - 1;

            ptr = (u8*)poly + sizeof(POLY_FT4);

            tex = &((SaveLayoutTex*)g_saveLayoutTexTable)[entry->tex_slot];
            setClut(poly, *(u16*)&tex->clut_x, *(u16*)&tex->clut_y);

            tex2 = &((SaveLayoutTex*)g_saveLayoutTexTable)[entry->tex_slot];
            /* Form the texture-page value from the complete layout flags word. */
            tpw = getTPage(*(u8*)&tex2->control & 3, *(u32*)entry >> 2, *(u16*)&tex2->tex_x, *(u16*)&tex2->tex_y);
            poly->tpage = tpw;

            addPrim(ot, poly);
        }
        else
        {
            if (type == SAVE_LAYOUT_PRIM_SPRT)
            {
                /* Slot cursor / decoration: one free-size sprite. */
                u16 vx;
                s32 offx;
                u16 vy;
                s32 offy;
                SaveLayoutTex* tex;
                DR_TPAGE* tp;

                if (g_slotSlideX < 0)
                {
                    idx = g_slotSelectedIndex + 1;
                }
                else
                {
                    idx = 0;
                }

                tint = GPU_TINT_NEUTRAL;
                SET_BGR0_PACKED((SPRT*)ptr, tint);
                setSprt((SPRT*)ptr);

                uv = (SlotUvRect*)((idx * 6) + (u32)g_saveSlotSpriteUvTable);
                setSemiTrans((SPRT*)ptr, *(u32*)entry & 2);

                vx = *(u16*)&entry->x + g_slotSlideXLerped;
                offx = uv->ox * 8 - 0x20;
                ((SPRT*)ptr)->x0 = vx - offx;
                vy = *(u16*)&entry->y + g_slotSlideYLerped;
                offy = uv->oy * 8 - 0x28;
                ((SPRT*)ptr)->y0 = vy - offy;
                setUV0((SPRT*)ptr, uv->u * 8, uv->v * 8);
                setWH((SPRT*)ptr, uv->w * 8, uv->h * 8);

                tex = &((SaveLayoutTex*)g_saveLayoutTexTable)[entry->tex_slot];
                setClut((SPRT*)ptr, *(u16*)&tex->clut_x, *(u16*)&tex->clut_y);

                addPrim(ot, ptr);
                ptr += sizeof(SPRT);

                tp = (DR_TPAGE*)ptr;
                setDrawTPage(tp, 0, 0, get_save_layout_base_tpage(&((SaveLayoutTex*)g_saveLayoutTexTable)[entry->tex_slot], *(u32*)entry));

                addPrim(ot, tp);
                ptr += sizeof(DR_TPAGE);
            }
            else
            {
                if (type == SAVE_LAYOUT_PRIM_TILE)
                {
                    /* Dimmed backdrop behind the slot list: one solid tile. */
                    TILE* tile = (TILE*)ptr;
                    DR_TPAGE* tp;

                    *(u32*)(ptr + 4) = 0x40; /* solid dark-blue fill; code byte set below */
                    setlen(tile, tile_len);
                    setcode(tile, 0x62);

                    setXY0(tile, *(u16*)&entry->tile_x + g_slotSlideXLerped, *(u16*)&entry->tile_y + g_slotSlideYLerped);
                    setWH(tile, entry->width, entry->height);

                    addPrim(ot, tile);

                    tp = (DR_TPAGE*)(ptr + sizeof(TILE));
                    setDrawTPage(tp, 0, 0, 0x25);
                    addPrim(ot, tp);

                    ptr += sizeof(TILE) + sizeof(DR_TPAGE);
                }
                else if (type != SAVE_LAYOUT_PRIM_NONE)
                {
                    /* Glyph strip: one SPRT + DR_TPAGE per GLYPH_CHUNK_WIDTH pixels. */
                    s32 remaining = entry->width;
                    u16 u0 = entry->u0;
                    SaveLayoutTex* tex;
                    SaveLayoutTex* tex0 = &((SaveLayoutTex*)g_saveLayoutTexTable)[entry->tex_slot];
                    s32 x;
                    s32 y;
                    s32 chunk;

                    idx = *(u16*)tex0;

                    if (*(u32*)entry & 1)
                    {
                        x = entry->x + g_slotSlideXLerped;
                        y = entry->y + g_slotSlideYLerped;
                    }
                    else
                    {
                        x = entry->x;
                        y = entry->y;
                    }

                    chunk = GLYPH_CHUNK_WIDTH;
                    if (remaining < GLYPH_CHUNK_WIDTH + 1)
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
                        setUV0(sprt, u0, *(u8*)&entry->v0);
                        setWH(sprt, chunk, entry->height);

                        remaining -= chunk;

                        tex = &((SaveLayoutTex*)g_saveLayoutTexTable)[entry->tex_slot];
                        setClut(sprt, *(u16*)&tex->clut_x, *(u16*)&tex->clut_y);

                        addPrim(ot, ptr);
                        ptr += sizeof(SPRT);

                        tp = (DR_TPAGE*)ptr;
                        setDrawTPage(tp, 0, 0, get_save_layout_tpage(&((SaveLayoutTex*)g_saveLayoutTexTable)[entry->tex_slot], *(u32*)entry, idx));

                        addPrim(ot, ptr);
                        ptr += sizeof(DR_TPAGE);

                        if (remaining == 0)
                        {
                            break;
                        }

                        /* Next chunk: flip to the other half of the texture page, or
                           step the page origin on when the mode bits say the strip
                           spans pages. */
                        u0 ^= 0x80;
                        tex = &((SaveLayoutTex*)g_saveLayoutTexTable)[entry->tex_slot];

                        if (!(tex->control & 7))
                        {
                            idx += 0x20;
                        }
                        else
                        {
                            idx += 0x40;
                            u0 = 0;
                        }

                        chunk = GLYPH_CHUNK_WIDTH;
                        if (remaining < GLYPH_CHUNK_WIDTH + 1)
                        {
                            chunk = remaining;
                        }

                        x += GLYPH_CHUNK_WIDTH;
                    }
                }
            }
        }

        i++;
        entry++;
    } while (i < SAVE_LAYOUT_ENTRIES);

    return ptr;
}

/**
 * @brief Upload every g_saveLayoutTexTable entry's CLUT and pixel data to VRAM.
 *
 * @details For each of the 11 entries in g_saveLayoutTexTable (stride 0x10),
 * reads an on-disk-TIM-style source blob via the entry's data pointer
 * (+0x8), uploads its CLUT block and then its pixel block with LoadImage,
 * and bit-packs the uploaded image's dimensions back into the entry's control
 * word (SaveLayoutTex::control). The destination entry is typed as
 * SaveLayoutTex; the source blob's internal TIM-style block layout is only
 * partially understood, so it is still walked with raw byte offsets.
 *
 * @return Not explicitly set on any path (matches original codegen); callers
 *         should not rely on the return value.
 *
 * @note @p data_ptr and @p block_ptr are kept as two distinct pointers that
 *       both start out holding the source blob: @p data_ptr is the working
 *       cursor (advanced past the header, used for the CLUT upload) while
 *       @p block_ptr is reused to point at the pixel block. Merging them into a
 *       single variable changes gcc 2.7's register allocation and drops the
 *       match, so the pair is required to match.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/lzJHa
 */
/* Bit layout of SaveLayoutTex::control: bits 0-2 = TIM pixel mode, bits 3-12 =
 * texture width, bits 13-22 = texture height (widths/heights are 10-bit). */
#define SAVE_TEX_MODE_MASK 0x7
#define SAVE_TEX_WIDTH_SHIFT 3
#define SAVE_TEX_HEIGHT_SHIFT 13
#define SAVE_TEX_DIM_MASK 0x3ff

unsigned short upload_save_layout_textures(void)
{
    SaveLayoutTex* tex_entry;
    u32 orig_control;
    SaveLayoutTex* entry_ctrl_ptr;
    u8* data_ptr;
    u32 pixel_block_offset;
    u8* block_ptr;
    u16 clut_w;
    u16 clut_h;
    int shift;
    u32 reload_control;
    u32 control;
    RECT rect;
    int counter;
    u8* clut_h_ptr;
    tex_entry = (SaveLayoutTex*)g_saveLayoutTexTable;

    for (counter = 0; counter < 11; counter++)
    {
        entry_ctrl_ptr = tex_entry;
        block_ptr = entry_ctrl_ptr->src;
        orig_control = entry_ctrl_ptr->control;
        control = orig_control;
        data_ptr = block_ptr;
        clut_h_ptr = data_ptr + 0x12;
        control = (control & ~SAVE_TEX_MODE_MASK) | (data_ptr[4] & SAVE_TEX_MODE_MASK);
        entry_ctrl_ptr->control = control;
        clut_w = *((u16*)(data_ptr + 0x10));
        clut_h = *((u16*)clut_h_ptr);
        pixel_block_offset = *((u32*)(data_ptr + 8));
        setRECT(&rect, tex_entry->clut_x, tex_entry->clut_y, clut_w * clut_h, 1);
        data_ptr += 8;
        LoadImage(&rect, (u_long*)(data_ptr + 0xc));
        block_ptr = data_ptr + pixel_block_offset;
        shift = SAVE_TEX_WIDTH_SHIFT;
        control = (reload_control = entry_ctrl_ptr->control);
        control = (control & ~(SAVE_TEX_DIM_MASK << SAVE_TEX_WIDTH_SHIFT)) | (((*((u16*)(block_ptr + 8))) & SAVE_TEX_DIM_MASK) << shift);
        tex_entry->control = control;
        control = control & ~(SAVE_TEX_DIM_MASK << SAVE_TEX_HEIGHT_SHIFT);
        control = control | (((*((u16*)(block_ptr + 0xa))) & SAVE_TEX_DIM_MASK) << SAVE_TEX_HEIGHT_SHIFT);
        tex_entry->control = control;
        setRECT(&rect, tex_entry->tex_x, tex_entry->tex_y, (tex_entry->control >> SAVE_TEX_WIDTH_SHIFT) & SAVE_TEX_DIM_MASK,
                (tex_entry->control >> SAVE_TEX_HEIGHT_SHIFT) & SAVE_TEX_DIM_MASK);
        LoadImage(&rect, (u_long*)(block_ptr + 0xc));
        tex_entry++;
        entry_ctrl_ptr++;
    }
}

/**
 * @brief Load one of the two full game-state templates into g_menuLayoutBuffer.
 *
 * Copies a MENU_LAYOUT_WORDS-word (~13 KB) game-state template over the working
 * g_menuLayoutBuffer and sets the companion mode field g_scene_mode.
 *
 * @param use_alt Zero selects the new-game template (g_newGameStateTemplate,
 *                g_scene_mode = 0xD); non-zero selects the alternate template
 *                (g_menuLayoutTemplateAlt, g_scene_mode = 0).
 *
 * @note The copy is an explicit word loop, not a struct assignment, so it
 *       reproduces the original codegen; MenuLayout is only partially mapped.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/aPcbW
 */
void load_menu_layout(s32 use_alt)
{
    s32* src;
    s32* dst;
    u32 i;
    if (use_alt == 0)
    {
        src = (s32*)&g_newGameStateTemplate;
        g_scene_mode = 0xD;
        g_music_track_index = 0;
        g_layout_flag = 0;
    }
    else
    {
        src = (s32*)&g_menuLayoutTemplateAlt;
        g_scene_mode = 0;
        g_music_track_index = 0;
        g_layout_flag = 0;
    }
    i = 0;
    dst = (s32*)g_menuLayoutBuffer;

    while (i < MENU_LAYOUT_WORDS)
    {
        *dst++ = *src++;
        i++;
    }
}

/**
 * @brief Load one of the two sub-menu layout tables for the save-slot screen.
 *
 * Copies a SUB_MENU_LAYOUT_WORDS-word (0x250-byte) layout table into the
 * game-data buffer at g_gameDataBasePtr. The default table is used when
 * starting a new game; the continue table is used when resuming a saved game,
 * in which case bit 0 of MenuLayout::mode_flags is also set to flag the slot
 * as "continue mode".
 *
 * @param is_continue Zero selects the default layout (g_subMenuLayoutDefault);
 *                     non-zero selects the continue layout
 *                     (g_subMenuLayoutContinue) and sets the continue-mode bit.
 *
 * @see decomp.me (100%) https://decomp.me/scratch/CU7Ml
 */
void load_sub_menu_layout(s32 is_continue)
{
    s32* src;
    s32* dst;
    u32 i;

    if (is_continue != 0)
    {
        src = g_subMenuLayoutContinue;
        ((MenuLayout*)g_menuLayoutBuffer)->mode_flags |= 1;
    }
    else
    {
        src = g_subMenuLayoutDefault;
    }

    dst = (s32*)&g_gameDataBasePtr;

    for (i = 0; i < SUB_MENU_LAYOUT_WORDS; i++)
    {
        dst[i] = src[i];
    }
}
