#include "menu.h"

typedef struct
{
    s16 x;
    s16 y;
} Vec2s;

typedef struct
{
    u8 unk0;        /* 0x00 - set to 3 to request a state change */
    u8 pad01;
    u8 unk2;        /* 0x02 - cleared when the page opens a sub-window */
    u8 pad03;
    u16 sel_idx;    /* 0x04 */
    u16 item_count; /* 0x06 */
    u16 base_x;     /* 0x08 */
    u16 base_y;     /* 0x0A */
    s16 viewport_w; /* 0x0C */
    s16 viewport_h; /* 0x0E */
    u16 scroll_x;   /* 0x10 */
    u16 scroll_y;   /* 0x12 */
    s16 target_x;   /* 0x14 */
    s16 target_y;   /* 0x16 */
    u8 lerp_steps;  /* 0x18 */
} ScrollListState;

extern s32 g_menu_pending_overlay;
extern s32 g_menu_char_slot;
extern s32 g_menu_active_subtype;
extern void* D_80168C70;
extern void* D_801690A8;
extern void* D_801693FC;
extern u8 D_800F0BE0[];
extern u8 D_800F0BEC[];

s32 scroll_list_draw(s32 prim_buf, s32* ot, ScrollListState* state, u32* entries, Vec2s* view_origin, int active);
void func_800A8F8C();
void func_800A8FB4();
s32 func_800A9060();
s32 func_800A88A0(s32 prim, s32* ot, void* glyph, s32 a3, s32 x, s32 y, s32 mode);
void func_8014F210(s32 sound_id, s32 volume);
void* func_8014F060(void);
void func_8014E3C4(u32 content_id);

extern s32 g_menu_draw_early_out;
extern s32 D_801690B8[];
extern s32 D_80169414;
extern s32 D_80169558;

void* menu_slot_alloc(s32 arg0, void* rect);
s32 func_8014551C(s32 arg0);
s32 func_8014E8B8(void);
s32 func_8014E9A0(void);
s32 func_8014EA78(void);
s32* func_8014A3A4();
s32* func_8014BA58();
s32* func_8014BD48();
s32* func_8014BF68();
s32* func_8014C820();
s32* func_8014C8C8();

/**
 * @brief Scroll-list content callback for the item/technique list page.
 *
 * Circle (0x40) closes the page (SE 0x7F, state->unk0 = 3). Otherwise scans the
 * 11-word x 24-bit availability bitmask at g_pad_ctx + 0x34 to find the absolute
 * bit index of the sel_idx'th set bit. On confirm (0x220), if that entry's row
 * matches the active category ((D_801693FC->unk14 >> 10) & 0x3F), the per-char
 * slot byte at g_pad_ctx + slot * 0x250 + subtype + 0x609 is read: 0xFF or
 * 0x80-clear assigns the entry there (SE 0x7E); 0x80-set tries the
 * func_800A9060 / func_800A8F8C swap path, clearing all four g_menu_slots and
 * loading content 6 on failure. A wrong category plays SE 0x78. The draw pass
 * renders the list chrome via scroll_list_draw, then draws each set bit's glyph
 * string (func_800A88A0, color 1 on the active category's row) and finally
 * points g_menu_pending_overlay at the found entry's description.
 *
 * @param ot          Ordering-table pointer, forwarded to the glyph renderer.
 * @param arg1        Scroll-list state for this page (aliased into @c state).
 * @param arg2        Primitive buffer write cursor (aliased into @c prim).
 * @param view_origin Viewport anchor; glyph origins are (0x10 - x, rel - y).
 * @param active      Non-zero to process input this frame; zero draws only.
 * @return Updated primitive buffer write cursor.
 * @note Shapes required to match: @c flag_ptr and @c ctx must be SEPARATE
 *       variables (the target colors the 0x609 read chain v1 and the write
 *       chain a2; one shared local forces one color). The func_800A8F8C call
 *       and the 0x640 store must sit inside a do { } while (0) block - its
 *       block notes keep the off-chain addu out of the g_pad_ctx load-delay
 *       slot the scheduler must fill with it. @c off is assigned inside the
 *       store expression (operand-order idiom, as in func_8014B7DC). In the
 *       draw block, @c glyph is computed BEFORE the color compare, the
 *       D_801693FC deref stays inline in the compare (a named local outranks
 *       @c base and steals v1), and @c base accumulates via the two-step
 *       assignment.
 * @see decomp.me (100%)
 */
s32 func_8014DEEC(s32* ot, ScrollListState* arg1, s32 arg2, Vec2s* view_origin, s32 active)
{
    ScrollListState* state = arg1;
    s32 prim = arg2;
    s32 found;
    s32 count;
    s32 i;
    s32 j;
    s32 mask;
    s32 word;
    s32* p;
    s32 rel;
    s32 scroll_y;
    u8 flag;
    s32 v0;

    if ((g_pad_input & 0x40) && (active != 0))
    {
        func_8014F210(0x7F, 0x80);
        state->unk0 = 3;
        return prim;
    }

    found = -1;
    count = 0;
    i = count;
    p = (s32*)((u8*)g_pad_ctx + 0x34);
    do
    {
        j = 0;
        mask = 1;
        word = *p;
        do
        {
            if (word & mask)
            {
                if (state->sel_idx == (count >> 4))
                {
                    found = j + (i * 0x18);
                }
                count += 0x10;
            }
            j += 1;
            mask = mask << 1;
        } while (j < 0x18);
        i += 1;
        p += 1;
    } while (i < 0xB);

    if ((g_pad_input & 0x220) && (active != 0))
    {
        if ((found / 24) == (s32)(((u32)(*(s32*)((u8*)D_801693FC + 0x14)) >> 0xA) & 0x3F))
        {
            u8* flag_ptr = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            u8* ctx;

            flag_ptr += g_menu_active_subtype;
            flag = *(flag_ptr + 0x609);
            if (flag != 0xFF)
            {
                if (flag & 0x80)
                {
                    v0 = func_800A9060();
                    if (v0 != 0)
                    {
                        s32 off;

                        do
                        {
                            func_800A8F8C(v0, (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) + 0x90));
                            *((u8*)g_pad_ctx + (off = ((g_menu_active_subtype + 1) << 6) + (g_menu_char_slot * 0x250)) + 0x640) = 0;
                        } while (0);
                        func_800A8FB4(off);
                    }
                    else
                    {
                        s32 n = 3;
                        MenuSlot* pool;
                        s8* slot;
                        u8* b;

                        b = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 8);
                        pool = g_menu_slots;
                        slot = (s8*)pool + 0x6C;
                        D_801690A8 = b + *(u16*)(b + 0xAC);
                        do
                        {
                            *slot = 0;
                            n -= 1;
                            slot -= 0x24;
                        } while (n >= 0);
                        func_8014E3C4(6);
                        return prim;
                    }
                }
            }
            ctx = (u8*)g_pad_ctx + (g_menu_char_slot * 0x250);
            ctx += g_menu_active_subtype;
            *(ctx + 0x609) = found % 24;
            func_8014F210(0x7E, 0x80);
            state->unk0 = 3;
        }
        else
        {
            func_8014F210(0x78, 0x80);
        }
    }

    prim = scroll_list_draw(prim, ot, state, (u32*)&D_80168C70, view_origin, active);

    found = -1;
    count = 0;
    i = count;
    p = (s32*)((u8*)g_pad_ctx + 0x34);
    scroll_y = state->scroll_y;
    do
    {
        j = 0;
        mask = 1;
        do
        {
            if (*p & mask)
            {
                rel = count - scroll_y;
                if (rel >= -0xF)
                {
                    if (rel < state->viewport_h - 0x10)
                    {
                        s32 color;
                        u8* base;
                        s32 offs;
                        u8* glyph;

                        color = 3;
                        base = g_menu_state_ptr;
                        base += *(s32*)(base + 0x20);
                        offs = *(u16*)((u8*)base + (j * 2) + (i * 0x30));
                        glyph = base + offs;
                        if (i == (s32)(((u32)(*(s32*)((u8*)D_801693FC + 0x14)) >> 0xA) & 0x3F))
                        {
                            color = 1;
                        }
                        prim = func_800A88A0(prim, ot, glyph, color, 0x10 - view_origin->x, rel - view_origin->y, 0);
                    }
                }
                if (state->sel_idx == (count >> 4))
                {
                    found = j + (i * 0x18);
                }
                count += 0x10;
            }
            j += 1;
            mask = mask << 1;
        } while (j < 0x18);
        i += 1;
        p += 1;
    } while (i < 0xB);

    if (found != -1)
    {
        u8* base = g_menu_state_ptr + *(s32*)(g_menu_state_ptr + 0x1C);

        g_menu_pending_overlay = (s32)(base + *(u16*)(base + (found * 2)));
    }
    return prim;
}

/**
 * @brief Open the menu content window for the given content page id.
 *
 * Sets D_80169558 (pending item row) to 0xFF, then for ids 0-7 allocates a
 * MenuSlot window and installs its content callback:
 * - 0/1/2: 0xF0 x 0x60 window at (0x40, 0x60), callback func_8014A3A4; sets
 *   D_80169414 (active category) to the id and packs func_8014551C(id) into
 *   flags bits 24:16.
 * - 3/4/5: 0xE8 x 0x90/0x80/0x90 window at (0x40, 0x2C), callbacks
 *   func_8014BA58 / func_8014BD48 / func_8014BF68; sets D_80169414 = 3 and
 *   packs func_8014E8B8 / func_8014E9A0 / func_8014EA78 into flags bits 24:16.
 * - 6/7: 0x120 x 0x20/0x30 window at (0x10, 0x60), callbacks func_8014C820 /
 *   func_8014C8C8, flags bits 24:16 = 1; rebuilds D_801690B8 as a 1-entry
 *   circular nav list and sets g_menu_draw_early_out.
 * Ids >= 8 only reset D_80169558.
 *
 * @param content_id Content page id (0-7); out of range is a no-op beyond the
 *        D_80169558 reset.
 * @note Shapes required to match: the rect must be a single u16 rect[4] array
 *       (separate locals get their stores dead-eliminated since only the first
 *       address escapes), and the raw callee result must be held in @c v0 with
 *       the (& 0x1FF) << 16 done inside the flags assignment so gcc 2.7.2
 *       cross-jumping merges the case 1-5 tails at one label.
 * @see decomp.me (100%)
 */
void func_8014E3C4(u32 arg0)
{
    u16 rect[4];
    MenuSlot* slot;
    s32 v0;
    s32 j;
    s32 prev;
    s32 next;
    s32 more;
    s32 link;
    s32 word_self;
    s32 word_prev;

    D_80169558 = 0xFF;
    switch (arg0)
    {
    case 0:
        rect[0] = 0x40;
        rect[1] = 0x60;
        rect[2] = 0xF0;
        rect[3] = 0x60;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014A3A4;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014551C(0);
        D_80169414 = 0;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 1:
        rect[0] = 0x40;
        rect[1] = 0x60;
        rect[2] = 0xF0;
        rect[3] = 0x60;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014A3A4;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014551C(1);
        D_80169414 = 1;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 2:
        rect[0] = 0x40;
        rect[1] = 0x60;
        rect[2] = 0xF0;
        rect[3] = 0x60;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014A3A4;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014551C(2);
        D_80169414 = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 3:
        rect[0] = 0x40;
        rect[1] = 0x2C;
        rect[2] = 0xE8;
        rect[3] = 0x90;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014BA58;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014E8B8();
        D_80169414 = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 4:
        rect[0] = 0x40;
        rect[1] = 0x2C;
        rect[2] = 0xE8;
        rect[3] = 0x80;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014BD48;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014E9A0();
        D_80169414 = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 5:
        rect[0] = 0x40;
        rect[1] = 0x2C;
        rect[2] = 0xE8;
        rect[3] = 0x90;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014BF68;
        slot->has_title = 1;
        slot->anim_frame = 5;
        slot->active = 2;
        v0 = func_8014EA78();
        D_80169414 = 3;
        slot->flags = (slot->flags & 0xFE00FFFF) | ((v0 & 0x1FF) << 16);
        break;

    case 6:
        rect[0] = 0x10;
        rect[1] = 0x60;
        rect[2] = 0x120;
        rect[3] = 0x20;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014C820;
        slot->anim_frame = 5;
        slot->active = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | 0x10000;

        j = 0;
        do
        {
            s32 cur = D_801690B8[j];

            prev = 0;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            D_801690B8[j] = word_self;
            if ((j - 1) >= 0)
            {
                prev = j - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            D_801690B8[j] = word_prev;
            next = j + 1;
            more = next < 1;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            D_801690B8[j] = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
        g_menu_draw_early_out = 1;
        break;

    case 7:
        rect[0] = 0x10;
        rect[1] = 0x60;
        rect[2] = 0x120;
        rect[3] = 0x30;
        slot = (MenuSlot*)menu_slot_alloc(3, rect);
        slot->content_cb = (s32 * (*)()) & func_8014C8C8;
        slot->anim_frame = 5;
        slot->active = 2;
        slot->flags = (slot->flags & 0xFE00FFFF) | 0x10000;

        j = 0;
        do
        {
            s32 cur = D_801690B8[j];

            prev = 0;
            link = cur & ~0x3FFF;
            link = link | ((j * 0x10) & 0x3FFF);
            word_self = link;
            D_801690B8[j] = word_self;
            if ((j - 1) >= 0)
            {
                prev = j - 1;
            }
            word_prev = word_self & 0xFF803FFF;
            word_prev = word_prev | ((prev & 0x1FF) << 14);
            D_801690B8[j] = word_prev;
            next = j + 1;
            more = next < 1;
            link = 0;
            if (more != 0)
            {
                link = next;
            }
            D_801690B8[j] = (word_prev & 0x7FFFFF) | (link << 23);
            j = next;
        } while (more != 0);
        g_menu_draw_early_out = 1;
        break;
    }
}

/**
 * @brief Count usable 4-bit entries in the pad-ctx table at +0x104 and rebuild
 *        the D_80168C70 circular nav list to that size.
 *
 * Scans 16 words (128 nibbles) at g_pad_ctx + 0x104; every nibble with value
 * >= 2 adds one entry. Clears D_80168C70[0], then packs each of the count
 * entries with the same three bit-fields as func_80145278 (bits 13:0 = i * 0x10,
 * bits 22:14 = circular prev, bits 30:23 = circular next).
 *
 * @return Number of entries counted (also the nav-list length).
 * @note Shapes required to match: the outer scan must index by @c i
 *       (*(u32*)((u8*)g_pad_ctx + i * 4 + 0x104)) so loop.c strength-reduces it
 *       to the a0 pointer while keeping i for the slti compare - a separate
 *       pointer walk lets check_dbra_loop reverse the loop into a countdown.
 *       The 0xF nibble mask must be a local so it stays in a register. The
 *       link loop must REUSE @c j (the nibble counter) as its induction
 *       variable and route @c prev through the @c word temp - both reuses merge
 *       pseudos so the allocator reproduces the target's a0/a1/a2 coloring.
 * @see decomp.me (100%)
 */
s32 func_8014E8B8(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u32 word;
    s32 mask;

    count = 0;
    i = count;
    mask = 0xF;
    do
    {
        word = *(u32*)((u8*)g_pad_ctx + (i * 4) + 0x104);
        j = 7;
        do
        {
            if ((word & mask) >= 2)
            {
                count += 1;
            }
            j -= 1;
            word = word >> 4;
        } while (j >= 0);
        i += 1;
    } while (i < 0x10);

    D_80168C70 = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
            temp_t0 = (j) + (s32*)&D_80168C70;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            word = var_a2;
            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((word & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            j += 1;
            temp_a3 = j < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = j;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
        } while (temp_a3 != 0);
    }
    return count;
}

/**
 * @brief Count active byte entries in the pad-ctx table at +0x25E0 and rebuild
 *        the D_80168C70 circular nav list to that size.
 *
 * Clears the byte at g_pad_ctx + 0x26DF, then scans the 0x100 bytes at
 * g_pad_ctx + 0x25E0 counting non-zero entries. Clears D_80168C70[0] and packs
 * each of the count entries with the same three bit-fields as func_80145278
 * (bits 13:0 = i * 0x10, bits 22:14 = circular prev, bits 30:23 = circular
 * next). Sibling of func_8014E8B8 (case 4 vs case 3 of func_8014E3C4).
 *
 * @return Number of entries counted (also the nav-list length).
 * @note Shapes required to match: unlike func_8014E8B8 this one keeps the
 *       func_80145278 next/copy pair (temp_a1 = j + 1; ...; j = temp_a1) and
 *       the byte scan is a plain pointer walk (gcc's countdown reversal IS the
 *       target here). The link loop must sit inside a do { } while (0) block -
 *       its block notes shorten the live ranges so j outranks the temp_t0
 *       address temp and takes a3, cascading t0/t5/t4/t3/t2 into place.
 * @see decomp.me (100%)
 */
s32 func_8014E9A0(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u8* p;

    count = 0;
    ((u8*)g_pad_ctx)[0x26DF] = 0;
    p = (u8*)g_pad_ctx + 0x25E0;
    i = 0xFF;
    do
    {
        if (*p != 0)
        {
            count += 1;
        }
        i -= 1;
        p += 1;
    } while (i >= 0);

    D_80168C70 = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
        do
        {
            temp_t0 = (j) + (s32*)&D_80168C70;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((var_a2 & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            temp_a1 = j + 1;
            temp_a3 = temp_a1 < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = temp_a1;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
            j = temp_a1;
        } while (temp_a3 != 0);
        } while (0);
    }
    return count;
}

/**
 * @brief Count entries with bit 0 set in the pad-ctx table at +0x2F0 and
 *        rebuild the D_80168C70 circular nav list to that size.
 *
 * Scans 0x40 records of stride 0xC at g_pad_ctx + 0x2F0, counting those whose
 * first byte has bit 0 set. Clears D_80168C70[0] and packs each of the count
 * entries with the same three bit-fields as func_80145278 (bits 13:0 = i * 0x10,
 * bits 22:14 = circular prev, bits 30:23 = circular next). Third sibling of
 * func_8014E8B8 / func_8014E9A0 (case 5 of func_8014E3C4).
 *
 * @return Number of entries counted (also the nav-list length).
 * @note Same shapes as func_8014E9A0: pointer-walk countdown scan, the
 *       func_80145278 next/copy pair, and the do { } while (0) wrapper around
 *       the link loop that recolors the j / temp_t0 allocation race.
 * @see decomp.me (100%)
 */
s32 func_8014EA78(void)
{
    s32 temp_a1;
    s32 temp_a3;
    s32 temp_v1;
    s32 var_a2;
    s32 var_v1;
    s32* temp_t0;

    s32 tmp;
    s32 tmp2;
    s32 tmp3;

    s32 count;
    s32 i;
    s32 j;
    u8* p;

    count = 0;
    p = (u8*)g_pad_ctx + 0x2F0;
    i = 0x3F;
    do
    {
        if (*p & 1)
        {
            count += 1;
        }
        i -= 1;
        p += 0xC;
    } while (i >= 0);

    D_80168C70 = 0;
    j = 0;
    if (count > 0)
    {
        do
        {
        do
        {
            temp_t0 = (j) + (s32*)&D_80168C70;

            tmp = *temp_t0;
            var_a2 = j - 1;

            temp_v1 = (tmp & ~0x3FFF);

            tmp2 = (j * 0x10);
            tmp2 = tmp2 & 0x3FFF;

            temp_v1 = temp_v1 | tmp2;
            *temp_t0 = temp_v1;

            if (var_a2 < 0)
            {
                var_a2 = count - 1;
            }

            tmp3 = (temp_v1 & 0xFF803FFF);

            tmp3 = tmp3 | ((var_a2 & 0x1FF) << 0xE);

            *temp_t0 = tmp3;
            temp_a1 = j + 1;
            temp_a3 = temp_a1 < count;
            var_v1 = 0;
            if (temp_a3 != 0)
            {
                var_v1 = temp_a1;
            }
            *temp_t0 = (tmp3 & 0x7FFFFF) | (var_v1 << 0x17);
            j = temp_a1;
        } while (temp_a3 != 0);
        } while (0);
    }
    return count;
}

typedef struct
{
    u32 unk0; /* Slot 0 data pointer. */
    u32 unk4; /* Slot 1 data pointer. */
    u32 unk8; /* Slot 2 data pointer. */
    u32 unkC; /* Slot 3 data pointer. */
} ItemSlotData;

typedef struct
{
    u8 slot0; /* Slot 0 occupied flag. */
    u8 slot1; /* Slot 1 occupied flag. */
    u8 slot2; /* Slot 2 occupied flag. */
    u8 slot3; /* Slot 3 occupied flag. */
} ItemSlotFlags;

extern ItemSlotData g_item_slot_data;
extern ItemSlotFlags g_item_slot_flags;
extern u8 D_800F0BF8[];

/**
 * @brief Try to commit the pending item into the active character's slot
 *        buffer at g_pad_ctx + char_slot * 0x250 + 0x640.
 *
 * func_8014ECA4 supplies the candidate 0x40-byte record (returns 0 on failure,
 * which aborts with return 0). If the slot buffer still equals the template at
 * D_800F0BF8 (byte-wise), the candidate is copied in, its first byte is
 * cleared, and g_item_slot_data.unk0 is zeroed. Otherwise the slot and the
 * candidate are exchanged through a stack buffer (three func_800A8F8C copies,
 * same pattern as func_8014DE5C) and g_item_slot_data.unk0 points at the
 * candidate. Either way g_item_slot_flags.slot0 is set.
 *
 * @return 1 if a record was committed/exchanged, 0 if func_8014ECA4 failed.
 * @note Shapes required to match: the equality scan must use the
 *       mismatch-goto shape (every break/flag rewrite loses rows). buf must be
 *       the FIRST local so its address is virtual-stack-vars + 0 and reload
 *       rematerializes it per call (any nonzero offset routes &buf through a
 *       pseudo that cse merges across calls into a saved register). The
 *       `if (0)` six-arg call is required for that: gcc sizes the outgoing arg
 *       area (0x18) at expand time and jump1 deletes the call afterwards,
 *       which is how the original (likely a compiled-out debug call) got
 *       buf to sp+0x18. Residual 3 rows: the equal arm's a1 copy and
 *       %hi(g_pad_ctx) are scheduled earlier in the target (see
 *       working/func_8014EB4C/status.md).
 * @see decomp.me (96.92%)
 */
s32 func_8014EB4C(void)
{
    u8 buf[0x40];
    u8* entry;
    u8* t;
    u8* p;
    u8* slot_buf;
    u32 i;
    s32 diff;

    if (0)
    {
        func_800A8F8C(0, 0, 0, 0, 0, 0);
    }
    entry = (u8*)func_8014ECA4();
    if (entry != 0)
    {
        t = D_800F0BF8;
        i = 0;
        p = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640;
        do
        {
            i += 1;
            if (*t != *p)
            {
                diff = 1;
                goto done;
            }
            t += 1;
            p += 1;
        } while (i < 0x40);
        diff = 0;
done:
        if (diff == 0)
        {
            func_800A8F8C((u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640, entry);
            *entry = 0;
            g_item_slot_data.unk0 = 0;
        }
        else
        {
            slot_buf = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + 0x640;
            func_800A8F8C(buf, slot_buf);
            func_800A8F8C(slot_buf, entry);
            func_800A8F8C(entry, buf);
            g_item_slot_data.unk0 = (u32)entry;
        }
        g_item_slot_flags.slot0 = 1;
        return 1;
    }
    return 0;
}

/**
 * @brief Select the highest-valued eligible item record from the pad-context item table.
 *
 * Builds an exclusion mask from the other three active character-slot records, then
 * scans the 100 records at g_pad_ctx + 0xCE0. Records using the alternate lookup table
 * or an excluded category are skipped; the eligible record with the greatest halfword
 * value at offset 0x24 is returned.
 *
 * @return Pointer to the selected 0x40-byte record, or NULL if no eligible record exists.
 * @note The remaining mismatch is a pure a1/a2/a3 allocation rotation; instruction
 *       count and control-flow structure match the target. Continue from
 *       working/func_8014ECA4/.
 * @see decomp.me (98.84%)
 */
void* func_8014ECA4(void)
{
    s32 var_a0;
    s32 var_a0_2;
    s32 var_a1;
    s32 temp_v1_3;
    s32 var_t1;
    u32 temp_v1;
    u32 temp_v1_2;
    u8* var_v0;
    u8* temp_v0;
    u8* var_a2;
    u8* var_a3;
    u8* var_t0;

    var_t1 = 0;
    temp_v0 = (u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0);
    var_t0 = temp_v0 + 0x50;
    if (*(u8*)(temp_v0 + 0x50) != 0)
    {
        var_t1 = *(u16*)(var_t0 + 0x24);
    }
    var_a3 = (u8*)g_pad_ctx + 0xCE0;
    var_t0 = 0;
    var_a1 = 0;
    var_a2 = (u8*)D_801693FC;
    var_a0 = 0;
    do
    {
        if ((var_a0 != 0) && (*var_a2 != 0))
        {
            temp_v1_2 = *(u32*)(var_a2 + 0x14);
            temp_v1 = temp_v1_2;
            if (temp_v1 & 0x300)
            {
                var_v0 = ((temp_v1 >> 0xA) & 0x3F) + D_800F0BEC;
            }
            else
            {
                var_v0 = ((temp_v1 >> 0xA) & 0x3F) + D_800F0BE0;
            }
            var_a1 |= *var_v0;
        }
        var_a0 += 1;
        var_a2 += 0x40;
    } while (var_a0 < 4);
    var_a0_2 = 0;
    do
    {
        if (*var_a3 != 0)
        {
            temp_v1_2 = *(u32*)(var_a3 + 0x14);
            if (!(temp_v1_2 & 0x300) && !(var_a1 & D_800F0BE0[(temp_v1_2 >> 0xA) & 0x3F]))
            {
                temp_v1_3 = *(u16*)(var_a3 + 0x24);
                if (var_t1 < temp_v1_3)
                {
                    var_t0 = var_a3;
                    var_t1 = temp_v1_3;
                }
            }
        }
        var_a0_2 += 1;
        var_a3 += 0x40;
    } while (var_a0_2 < 0x64);
    return var_t0;
}

/** @brief Slot-data pointer table indexed by menu subtype; @ref g_item_slot_data is its entry 7. */
extern u32 D_8016951C[];
/** @brief Slot-occupied flag table indexed by menu subtype; @ref g_item_slot_flags is its entry 7. */
extern u8 D_80168C15[];

/**
 * @brief Commit the pending item into the active character's slot buffer for the
 *        CURRENT menu subtype (@ref g_menu_active_subtype).
 *
 * The subtype-indexed sibling of @ref func_8014EB4C, which handles subtype 7
 * (slot 0) only. func_8014F060 supplies the candidate 0x40-byte record; a NULL
 * return aborts with 0. Any of comparison slots 1..3 that still points at this
 * subtype's slot buffer is re-pointed at it first.
 *
 * If the slot buffer is empty (its first byte is 0) the candidate is copied in,
 * its own first byte cleared, and the subtype's slot-data entry zeroed.
 * Otherwise the slot and the candidate are exchanged through a stack buffer
 * (three func_800A8F8C copies, same pattern as func_8014EB4C / func_8014DE5C)
 * and the subtype's slot-data entry points at the candidate. Either way the
 * subtype's slot-occupied flag is set.
 *
 * @return 1 if a record was committed or exchanged, 0 if func_8014F060 failed.
 *
 * @note D_8016951C and D_80168C15 are the bases of the subtype-indexed tables
 *       that @ref g_item_slot_data (0x80169538 = D_8016951C + 7*4) and
 *       @ref g_item_slot_flags (0x80168C1C = D_80168C15 + 7) sit inside. The
 *       relocation must name these bases, not `&g_item_slot_data - 0x1C`:
 *       the addresses are equal but the emitted symbol is not.
 *
 * @note Shapes required to match:
 *       - The two found-store blocks are OUT OF LINE. gcc 2.7.2 has no
 *         basic-block reorder pass, so the emitted order is the source order:
 *         arm A's store sits AFTER its common code and jumps backwards into it,
 *         and arm B's store sits BEFORE its scan loop, reached only by the
 *         loop's branch. The leading `goto scan_b;` is deleted by jump.c's
 *         jump-to-jump redirection but is what places found_b ahead of the
 *         loop. Writing either store inline with `break` costs ~27%: the store's
 *         address expression becomes loop-invariant, loop.c hoists it to the
 *         preheader, and CSE then shares it with the following call argument
 *         instead of recomputing it.
 *       - `slots[i]` indexing (not a `p = &g_item_slot_data.unk4` pointer walk)
 *         is what makes loop.c strength-reduce to the target's two-step
 *         `addiu v0, v0, %lo(g_item_slot_data)` + `addiu a1, v0, 0x4`.
 *       - `slots` must be assigned separately inside each arm; hoisting it
 *         above the `if` costs a fifth saved register (frame 0x68 -> 0x70).
 *       - `pad` aliases g_pad_ctx so its %hi becomes a long-lived allocno and
 *         its `lui` is emitted early, and `off` is assigned before it so the
 *         subtype `lui` comes first.
 *       - `ret` carries the 1 into the shared tail, producing the target's
 *         `li v0, 1` + `addu v1, v0, zero` pair. It only works assigned at the
 *         very end of the if/else; hoisting it anywhere earlier adds two insns.
 *       - `pad - (-(offset))` rather than `pad + offset`: gcc canonicalizes the
 *         operands of a commutative PLUS itself, so the sum coalesces into the
 *         char-slot chain's register. Routing it through MINUS preserves the
 *         operand order and gives the target's `addu a0, a0, v0`. Every
 *         plain-`+` spelling tried tops out at 99.87%.
 *       - The `if (0)` six-arg call sizes the outgoing-argument area to 0x18 so
 *         `buf` lands at sp+0x18; see func_8014EB4C. `&buf` must never be
 *         cached in a named local - it has to rematerialize at each call.
 * @see decomp.me (100%)
 */
s32 func_8014EDEC(void)
{
    u8 buf[0x40];
    u8* entry;
    u8* slot_buf;
    u32* slots;
    u8* pad;
    s32 ret;
    s32 off;
    s32 i;

    if (0)
    {
        func_800A8F8C(0, 0, 0, 0, 0, 0);
    }
    entry = (u8*)func_8014F060();
    if (entry != 0)
    {
        i = 1;
        off = g_menu_active_subtype - 7;
        pad = (u8*)g_pad_ctx;
        if (*(pad - (-((off << 6) + (g_menu_char_slot * 0x250))) + 0x640) == 0)
        {
            slots = &g_item_slot_data.unk0;
            do
            {
                if ((u32)entry == slots[i])
                {
                    goto found_a;
                }
                i += 1;
            } while (i < 4);
done_a:
            func_800A8F8C((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170), entry);
            *entry = 0;
            D_8016951C[g_menu_active_subtype] = 0;
            goto tail;
found_a:
            slots[i] = (u32)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170));
            goto done_a;
        }
        else
        {
            goto scan_b;
found_b:
            slots[i] = (u32)((u8*)g_pad_ctx + ((g_menu_char_slot * 0x250) + 0x5F0) + ((g_menu_active_subtype << 6) - 0x170));
            goto done_b;
scan_b:
            slots = &g_item_slot_data.unk0;
            do
            {
                if ((u32)entry == slots[i])
                {
                    goto found_b;
                }
                i += 1;
            } while (i < 4);
done_b:
            slot_buf = (u8*)((g_menu_char_slot * 0x250) + (s32)g_pad_ctx) + (g_menu_active_subtype << 6) + 0x480;
            func_800A8F8C(buf, slot_buf);
            func_800A8F8C(slot_buf, entry);
            func_800A8F8C(entry, buf);
            D_8016951C[g_menu_active_subtype] = (u32)entry;
        }
        ret = 1;
tail:
        D_80168C15[g_menu_active_subtype] = ret;
        return 1;
    }
    return 0;
}

/**
 * @brief Pick the highest-valued eligible item record for the CURRENT menu subtype.
 *
 * The subtype-generic sibling of @ref func_8014ECA4 (which handles subtype 7 only).
 * The active character's slot buffer for @ref g_menu_active_subtype supplies the
 * score to beat: the sum of its four halfwords at 0x24/0x26/0x28/0x2A, or 0 when
 * the slot is empty. An exclusion mask is then built from the other three
 * comparison slots at D_801693FC, and the 100 records at g_pad_ctx + 0xCE0 are
 * scanned for the best record that is not excluded.
 *
 * @return Pointer to the winning 0x40-byte record, or NULL if none qualifies.
 *
 * @note Shapes required to match:
 *       - The opening test is a real `if/else` (`if (empty) { total = 0; } else
 *         { total = sum; }`), NOT `total = 0;` followed by a bare `if`. The
 *         target re-reads g_pad_ctx AND g_menu_active_subtype after the test
 *         (0x94-0xA8). With the bare `if`, gcc 2.7.2's CSE takes the AROUND
 *         path (`-fcse-skip-blocks`, on at -O2): cse_end_of_basic_block follows
 *         the branch around the if-body straight into the join block with its
 *         value table intact, so both globals are still live in registers there
 *         and the reloads fold away. The else arm puts a BARRIER in the way,
 *         CSE stops at the join, and both globals are re-read. Worth 9.36%.
 *       - `slot_idx` is deliberately reused after loop A as the loop-B mask
 *         carrier, and `mask` is reused inside loop B as the staging temp for
 *         `best_total`. gcc 2.7 has no live-range splitting, so one C variable
 *         is one hard register for the whole function; these two reuses are
 *         what give the target's a3 and a2 their second lives. A fresh variable
 *         in either place is coalesced away and costs the move.
 *       - `rec_flag` and `rec` walk the same records but must be ONE variable
 *         each in the roles shown: `rec_flag` for the occupancy byte, `rec` for
 *         the fields. Splitting `rec_flag` into an extra copy raises its
 *         allocation priority (pri = floor_log2(refs)*refs/live_len) above the
 *         mask carrier's and swaps a3/t0.
 *       - `char_base` uses `- (-(offset))` rather than `+ offset`; see
 *         @ref func_8014EDEC for why the MINUS routing is kept.
 * @see decomp.me (100%)
 */
void* func_8014F060(void)
{
    s32 slot_idx;
    s32 rec_idx;
    s32 mask;
    s32 total;
    s32 best_total;
    u32 slot_flags;
    u32 rec_flags;
    u8* category;
    u8* char_base;
    u8* slot;
    u8* rec_flag;
    u8* rec;
    u8* best;

    char_base = (u8*)g_pad_ctx - (-((g_menu_char_slot * 0x250) + 0x5F0));
    best = char_base + ((g_menu_active_subtype << 6) - 0x170);
    if (*best == 0)
    {
        best_total = 0;
    }
    else
    {
        best_total = *(u16*)(best + 0x24) + *(u16*)(best + 0x26) + *(u16*)(best + 0x28) + *(u16*)(best + 0x2A);
    }
    best = 0;
    mask = 0;
    slot_idx = 0;
    slot = (u8*)D_801693FC;
    rec_flag = (u8*)g_pad_ctx + 0xCE0;
    do
    {
        if ((slot_idx != (g_menu_active_subtype - 7)) && (*slot != 0))
        {
            slot_flags = *(u32*)(slot + 0x14);
            if (slot_flags & 0x300)
            {
                category = ((slot_flags >> 0xA) & 0x3F) + D_800F0BEC;
            }
            else
            {
                category = ((slot_flags >> 0xA) & 0x3F) + D_800F0BE0;
            }
            mask |= *category;
        }
        slot_idx += 1;
        slot += 0x40;
    } while (slot_idx < 4);
    slot_idx = mask;
    rec_idx = 0;
    rec = rec_flag;
    do
    {
        if (*rec_flag != 0)
        {
            rec_flags = *(u32*)(rec + 0x14);
            if (((rec_flags & 0x300) == 0x100) && !(slot_idx & D_800F0BEC[(rec_flags >> 0xA) & 0x3F]))
            {
                total = *(u16*)(rec + 0x24) + *(u16*)(rec + 0x26) + *(u16*)(rec + 0x28) + *(u16*)(rec + 0x2A);
                if (best_total < total)
                {
                    best = rec;
                    mask = total;
                    best_total = mask;
                }
            }
        }
        rec_idx += 1;
        rec += 0x40;
        rec_flag += 0x40;
    } while (rec_idx < 0x64);
    return best;
}
