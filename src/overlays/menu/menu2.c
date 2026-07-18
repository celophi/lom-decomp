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
