#include "common.h"
#include "display.h"
#include "gpu_packet.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

extern u8 *D_8014C168;
extern s32 D_8014C16C;
extern s32 D_8014C170;
extern u8 D_80042FD8[];
extern u8 D_800F1CD0[];
extern s32 D_80122C00;
extern s32 D_8014C180;
extern s32 D_8014C184;
extern s32 D_8014AAA4;
extern u8 D_800EC3DA[];
extern s32 D_8014C188;
extern s32 D_8014C18C;
extern u8 D_8014C198[];
extern s32 D_8014C238;
extern s32 D_8014C23C;
extern s32 D_8014C248;
extern s32 D_8014C24C;
extern s32 D_8014C254;
extern s32 D_8014C258;
extern s32 D_8014C25C;
extern s32 D_8014C260;
extern s32 D_8014C264;
extern s32 D_8014C268;
extern s32 D_8014C26C;
extern s32 D_8014C270;
extern s32 D_8014C274;
extern s32 D_8014C278;
extern s32 D_8014C27C;
extern s32 D_8014C280;
extern s32 D_8014C284;
extern s32 D_8014C288;
extern s32 D_80122988;

/** @brief Cell-grid view of D_80042FD8: 64 u32 cells at offset 0x29DC. */
typedef struct { u8 pad[0x29DC]; u32 cells[64]; } D80042FD8Type;
/** @brief 4-byte entry view of D_8014C198: a u16 flag/value word. */
typedef struct { u16 value; u16 pad; } C198Entry;
/** @brief UI panel bitfield block holding three animated corner words. */
typedef struct { u32 unk00; u8 pad04[0x10]; u32 unk14; u8 pad18[0x24]; u32 unk3C; } B560;
/** @brief 88-byte row of the D_800F1CD0 icon table; only bytes 2 and 3 are read here. */
typedef struct { u8 pad0[2]; u8 unk2; u8 unk3; u8 pad4[84]; } D800F1CD0Entry;
extern B560 D_8014B560;

/**
 * @brief One 0x14-byte record of the D_8014B560 panel table (0x9A entries).
 * @note The @ref B560 typedef above is a three-entry view of the same storage
 *       (its unk00/unk14/unk3C are entries 0, 1 and 3); do not merge them.
 */
typedef struct {
    u8 pad0[0xA];
    u16 unkA;
    u16 unkC;
    u16 unkE;
    u16 unk10;
    u16 pad12;
} B560Entry;

void func_800A3938();
void func_800CB918();
s32 func_800CBA9C();
s32 func_800CBC0C();
void func_800CBE64();
void func_80140CBC();

/** @brief Access grid cell @p i (u32) within D_80042FD8. */
#define CELL(i) (((D80042FD8Type *)&D_80042FD8)->cells[(i)])
/** @brief Clear the panel-animation bits [10:7] and set the "active" pair [8:7]. */
#define SET_UI_BITS(x) (((x) & ~0x780) | 0x180)

typedef struct {
    u16 x;
    u16 y;
    u16 clut_x;
    u16 clut_y;
} GolemImageClutPos;

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} GolemRect;

s32 func_80019A34();

extern u8 D_8014287C[];
void func_80140438(GolemImageClutPos *, u8 *);

extern s32 D_800F22AC;

void func_801405A0(void);
void func_80141228();

void func_80140024(u8 *arg0, s32 arg1)
{
    u8 *draw_buffer;
    u8 *next_buffer;
    u8 *other_buffer;
    u8 *draw_env_buffers;

    D_8014C168 = arg0;
    arg0 += 0x8180;

    *(s16 *)(D_8014C168 + 0x4044) = 0;
    *(s16 *)(D_8014C168 + 0x4046) = 8;
    *(s16 *)(D_8014C168 + 0x4048) = 0x140;
    *(s16 *)(D_8014C168 + 0x404A) = 0xE0;
    *(s16 *)(D_8014C168 + 0x8104) = 0;
    *(s16 *)(D_8014C168 + 0x8106) = 0xF0;
    *(s16 *)(D_8014C168 + 0x8108) = 0x140;
    *(s16 *)(D_8014C168 + 0x810A) = 0xE0;

    func_8001C62C(D_8014C168 + 0x4050, 0, 0, 0x140, 0xF0);
    func_8001C62C(D_8014C168 + 0x8110, 0, 0xE8, 0x140, 0xF0);
    func_8001C56C(D_8014C168 + 0x4064, 0, 0xF0, 0x140, 0xE0);
    func_8001C56C(D_8014C168 + 0x8124, 0, 8, 0x140, 0xE0);

    draw_env_buffers = D_8014C168;
    draw_env_buffers[0x813A] = 0;
    draw_env_buffers[0x407A] = 0;
    D_8014C16C = 0;
    *(s32 *)(D_8014C168 + 0x404C) = 0;
    *(s32 *)(D_8014C168 + 0x810C) = 1;

    D_8014C170 = (func_80140280(arg0, arg1) + 3) & ~3;

    next_buffer = D_8014C168;
    func_80019C74(next_buffer, 0x10);
    func_80019C74(D_8014C168 + 0x40C0, 0x10);
    func_8002054C(0);
    func_80019FB8(next_buffer + 0x4050);
    func_800157DC();

    for (;;) {
        draw_buffer = next_buffer;
        func_80019C74(draw_buffer, 0x10);
        *(u8 **)(draw_buffer + 0x4040) = draw_buffer + 0x40;
        field_text_reset_scratch();
        func_800A9E78();
        func_801404F0(draw_buffer);
        func_80063194();
        func_80019788(0);
        func_800157B0(2);
        func_8002054C(2);

        if (D_8014C16C != 0) {
            break;
        }

        func_8001990C(draw_buffer + 0x4044, 0, 0, 0);
        other_buffer = D_8014C168;
        if (draw_buffer == D_8014C168) {
            other_buffer = draw_buffer + 0x40C0;
        }
        next_buffer = other_buffer;
        func_80019FB8(other_buffer + 0x4050);
        func_80019DEC(next_buffer + 0x4064);
        func_80019D7C(draw_buffer + 0x3C);
        draw_buffer = other_buffer;
        func_800157DC();
        func_800122C0();
    }

    func_800AA02C();
    field_text_reset_windows();
}


u8 *func_80140280(u8 *arg0, s32 arg1)
{
    s32 i;
    s32 temp;
    s32 match;
    u8 *base;
    u8 *base2;
    s32 stack_padding[2];

    D_8014C288 = arg1;
    if (arg1 != 0) {
        i = 0;
        base = D_80042FD8;
        match = *(s8 *)(base + 0x29D7);
        D_8014C270 = D_80122C00;
        D_80122C00 = 0;
        do {
            if (base[i + 0x29D8] == match) {
                D_80122C00 = i;
            }
            i++;
        } while (i < 3);
    }

    D_8014C188 = func_800CB758() - 4;
    base2 = D_80042FD8;
    temp = base2[D_80122C00 + 0x29D8];
    D_8014C27C = 0;
    D_8014C24C = 0;
    D_8014C248 = 0;
    D_8014C238 = 0;
    D_8014C264 = 0xB8;
    D_8014C258 = 0xB8;
    D_8014C268 = 0x50;
    D_8014C25C = 0x50;
    D_8014C260 = temp;
    func_80140FF8(temp);
    D_8014C184 = func_800CBD70(D_8014C198);
    D_8014C180 = 0;
    D_8014C18C = 0;
    D_8014C23C = 0;
    D_8014C284 = 0;
    func_801403F8();
    func_800AA02C();
    func_80142418(0x100, 0x100, 0x100, 6);
    D_8014C26C = 0;
    D_8014C280 = 0;
    D_8014C278 = 0;
    D_8014C274 = 0;
    return arg0;
}

void func_801403F8(void)
{
    RECT rect;

    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0x1F2;
    func_80140438(&rect, D_8014287C);
}

void func_80140438(GolemImageClutPos* pos, u8* archive)
{
    GolemRect rect;
    s32 flags;
    s32 off8;
    u16* dims;

    flags = *(s32*)(archive + 4);
    off8 = *(s32*)(archive + 8);

    if (flags & 8)
    {
        rect.x = pos->clut_x;
        rect.y = pos->clut_y;
        rect.w = 0x100;
        rect.h = 1;
        func_80019A34(&rect, archive + 0x14);
        dims = (u16*)(off8 - (-(s32)archive) + 0x10);
    }
    else
    {
        dims = (u16*)(archive + 0x10);
    }

    rect.x = pos->x;
    rect.y = pos->y;
    rect.w = dims[0];
    rect.h = dims[1];
    func_80019A34(&rect, off8 - (-(s32)archive) + 0x14);
}


void func_801404F0(void)
{
    func_80141228();
    D_800F22AC += 1;
    func_801405A0();
    if (D_8014C274 != 0)
    {
        D_8014C278 += (D_8014C280 - D_8014C278) / D_8014C274;
        D_8014C274 -= 1;
        return;
    }
    D_8014C278 = D_8014C280;
}

/**
 * @brief Per-frame update for the golem grid cursor: scrolls the view, handles
 *        placement/rotation/confirm input, and animates the selection.
 * @note WIP - 99.88% (446/455 exact, gcc272_cdk). Residue is a v0/v1 register
 *       swap between D_8014C284 and D_8014C184 in the backward-search loop.
 * @see decomp.me (99.88%)
 */
void func_801405A0(void)
{
    s32 old_x, old_y, i, index, count, input, value;
    if ((D_80122988 & 0x800) && (D_8014C238 == 0)) goto cancel;
    count = D_8014C180;
    if (count != 0) {
        D_8014C180 = count - 1;
        D_8014C18C += (D_8014C23C - D_8014C18C) / count;
        if (D_8014C180 != 0) return;
        D_8014C284 = D_8014C18C / 40;
    } else {
        D_8014C18C = D_8014C23C;
        D_8014C284 = D_8014C23C / 40;
    }
    if (D_8014C180 != 0) return;
    if (D_8014C238 != 0) {
        input = D_80122988;
        if (input & 0xF000) {
            old_x = D_8014C248;
            old_y = D_8014C24C;
            if (input & 0x1000) D_8014C24C = old_y - 1;
            else if (input & 0x4000) D_8014C24C = old_y + 1;
            else if (input & 0x8000) D_8014C248 = old_x - 1;
            else if (input & 0x2000) D_8014C248 = old_x + 1;
            if (func_800CBC0C(D_8014C284, D_8014C27C, D_8014C248, D_8014C24C) == 0) {
                D_8014C248 = old_x; D_8014C24C = old_y; func_800A3938(0x78, 0x80);
            } else func_800A3938(0x7D, 0x80);
            return;
        }
        if (input & 0x90) {
            if (input & 0x10) { if (D_8014C27C >= 3) D_8014C27C = 0; else D_8014C27C++; }
            else { if (D_8014C27C == 0) D_8014C27C = 3; else D_8014C27C--; }
            if (func_800CBC0C(D_8014C284, D_8014C27C, D_8014C248, D_8014C24C) == 0) func_80140CBC();
            func_800A3938(0x7D, 0x80);
            D_8014B560.unk3C = SET_UI_BITS(D_8014B560.unk3C);
            return;
        }
        if (input & 0xA20) {
            if (func_800CBA9C(D_8014C284, D_8014C27C, D_8014C248, D_8014C24C) != 0) {
                D_8014C238 = 0; func_80140FF8(); func_800A3938(0x120, 0x80); func_800CB918(D_8014C284, D_8014C27C, D_8014C248, D_8014C24C);
            } else func_800A3938(0x78, 0x80);
            return;
        }
        if (input & 0x40) { D_8014C238 = 0; func_80140FF8(); func_800A3938(0x78, 0x80); CELL(D_8014C284) |= 3; }
        return;
    }
    if (D_8014C184 != 0) {
        i = 1; input = D_80122988;
        if (input & 4) { D_80122988 = 0x1000; i = 3; }
        else if (input & 8) { D_80122988 = 0x4000; i = 3; }
        input = D_80122988;
        if (input & 0x5000) {
            func_800A3938(0x7D, 0x80);
            while (i != 0) {
                if (D_80122988 & 0x1000) {
                    if (D_8014C23C != 0) { D_8014C180 = 4; D_8014C23C -= 40; D_8014B560.unk14 = SET_UI_BITS(D_8014B560.unk14); }
                } else if ((D_80122988 & 0x4000) && ((D_8014C23C / 40) != (D_8014C184 - 1))) {
                    D_8014C180 = 4; D_8014C23C += 40; D_8014B560.unk00 = SET_UI_BITS(D_8014B560.unk00);
                }
                i--;
            }
            return;
        }
        if (input & 2) {
            s32 limit, match_type; D80042FD8Type *base;
            index = D_8014C284; i = 0;
            if (D_8014C184 > 0) {
                limit = D_8014C184;
                base = (D80042FD8Type *)&D_80042FD8;
                match_type = D_8014C260;
                index++;
forward_loop:
                if (index == limit) index = 0;
                i++;
                if ((base->cells[index] & 3) == match_type) goto forward_done;
                index++;
                if (i < limit) goto forward_loop;
                index--;
            }
forward_done:
            D_8014C23C = index * 40; D_8014C180 = 4; D_8014B560.unk00 = SET_UI_BITS(D_8014B560.unk00); func_800A3938(0x7D, 0x80); return;
        }
        if (input & 1) {
            s32 limit, match_type; D80042FD8Type *base;
            index = D_8014C284; i = 0;
            if (D_8014C184 > 0) {
                limit = D_8014C184;
                base = (D80042FD8Type *)&D_80042FD8;
                match_type = D_8014C260;
                index--;
backward_loop:
                if (index < 0) index = limit - 1;
                if ((base->cells[index] & 3) == match_type) goto backward_done;
                input = D_8014C184;
                i++;
                index--;
                if (i < input) goto backward_loop;
                index++;
            }
backward_done:
            D_8014C23C = index * 40; D_8014C180 = 4; D_8014B560.unk14 = SET_UI_BITS(D_8014B560.unk14); func_800A3938(0x7D, 0x80); return;
        }
        if (input & 0x220) {
            if (((C198Entry *)D_8014C198)[D_8014C284].value == 0) {
                D_8014C238 = 1;
                if ((CELL(D_8014C284) & 3) == D_8014C260) {
                    D_8014C248 = (s32)(CELL(D_8014C284) << 8) >> 27;
                    D_8014C24C = (s32)(CELL(D_8014C284) << 3) >> 27;
                    D_8014C27C = (CELL(D_8014C284) >> 17) & 3;
                } else { func_80140CBC(); D_8014C27C = 0; }
                func_800CBE64(D_8014C284); func_800A3938(0x7E, 0x80);
            }
            return;
        }
        value = input & 0x40;
    } else {
        value = D_80122988 & 0x40;
    }
    if (value != 0) {
cancel:
        func_800A3938(0x7D, 0x80); D_8014C16C = 1; if (D_8014C288 != 0) D_80122C00 = D_8014C270;
    }
}
/**
 * @see decomp.me (100%)
 */
void func_80140CBC(void)
{
    if (D_8014C188 == 0)
    {
        D800F1CD0Entry *tbl;
        D_8014C248 = (tbl = (D800F1CD0Entry *)D_800F1CD0)[(CELL(D_8014C284) >> 12) & 0xF].unk2 - 1;
        D_8014C24C = tbl[(CELL(D_8014C284) >> 12) & 0xF].unk3 - 1;
        return;
    }
    if (D_8014C188 >= 0)
    {
        if (D_8014C188 < 3)
        {
            D800F1CD0Entry *tbl = (D800F1CD0Entry *)D_800F1CD0;
            D_8014C248 = tbl[(CELL(D_8014C284) >> 12) & 0xF].unk2;
            D_8014C24C = tbl[(CELL(D_8014C284) >> 12) & 0xF].unk3;
        }
    }
}

/**
 * @brief Read the 60-cell grid state and emit each occupied cell to the
 *        renderer, threading an accumulator through the calls.
 * @param arg0 Initial accumulator (render/prim cursor advanced by func_80140F68).
 * @param arg1 Renderer context passed through unchanged.
 * @note WIP, not byte-matched. Best local match 83.81% (45/97 exact rows);
 *       frame and control flow match. Residual is a register-numbering shift:
 *       gcc keeps @c acc in a0 (per-iteration caller-save) instead of s3, which
 *       frees a saved reg to hoist the 0x50 constant. The explicit @c yy
 *       snapshot is required to reproduce the giv+snapshot that spills the grid
 *       base to sp+0x108 (frame 0x138). Target/toolchain confirmed gcc272_cdk
 *       (gcc280_g0/g4 both score 80.28%). See working/func_80140DEC/.
 */
s32 func_80140DEC(s32 arg0, s32 arg1)
{
    s32 grid[60];
    s32 idx;
    s32 col;
    s32 x;
    s32 y;
    s32 val;
    s32 mode;
    s32 acc;
    s32 *base;
    s32 yy;

    func_800CBEC4(grid);

    acc = arg0;
    idx = 0;
    base = grid;
    y = 4;
    for (arg0 = 0; arg0 < 6; arg0++)
    {
        yy = y;
        x = 0xC;
        for (col = 0; col < 5; col++)
        {
            val = base[idx];
            if (val != 0)
            {
                if (val == 0x50)
                {
                    mode = 2;
                }
                else
                {
                    mode = 3;
                }
                acc = func_80140F68(acc, arg1, x, yy, mode);
            }
            x += 0x10;
            idx++;
        }
        y += 0x10;
    }

    for (arg0 = 0; arg0 < 5; arg0++)
    {
        x = 4;
        for (col = 0; col < 6; col++)
        {
            val = grid[idx];
            if (val != 0)
            {
                acc = func_80140F68(acc, arg1, x, (arg0 << 4) + 0xC, val != 0x4E);
            }
            x += 0x10;
            idx++;
        }
    }

    return func_80141020(acc, arg1);
}

/**
 * @see decomp.me (100%)
 */
s32 func_80140F68(s32 prim, s32 ot, s32 x, s32 y, s32 tile)
{
    SPRT *sprt = (SPRT *)prim;

    SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
    setSprt(sprt);
    setWH(sprt, 8, 8);
    setUV0(sprt, tile * 8 - 0x70, 0x58);
    setXY0(sprt, x, y);
    sprt->clut = 0x7C87;
    addPrim(ot, sprt);
    return prim + 0x14;
}

/**
 * @see decomp.me (100%)
 */
void func_80140FF8(void)
{
    D_8014C264 = 0xB8;
    D_8014C268 = 0x50;
    D_8014C254 = 8;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80141020(s32 prim, s32 ot)
{
    DR_TPAGE *draw_mode = (DR_TPAGE *)prim;

    setDrawTPage(draw_mode, 0, 0, getTPage(0, 1, 320, 0));
    addPrim(ot, draw_mode);
    return prim + 8;
}

/**
 * @see decomp.me (100%)
 */
s32 func_80141074(s32 prim, s32 arg1)
{
    SPRT *sprt = (SPRT *)prim;
    DR_TPAGE *draw_mode;
    s32 ot;
    u16 x;
    u16 y;
    s32 dx;
    s32 dy;

    SET_BGR0_PACKED(sprt, GPU_TINT_NEUTRAL);
    setSprt(sprt);
    x = D_8014C258;
    y = D_8014C25C;
    setWH(sprt, 0x10, 0x10);
    setUV0(sprt, 0xB0, 0xF0);
    sprt->clut = 0x7C87;
    setXY0(sprt, x + 8, y);
    addPrim(arg1 + 0x2C, sprt);

    prim += 0x14;
    ot = arg1 + 0x2C;

    if (D_8014C254 != 0)
    {
        dx = (D_8014C264 - D_8014C258) / D_8014C254;
        dy = (D_8014C268 - D_8014C25C) / D_8014C254;
        D_8014C254--;
        D_8014C258 += dx;
        D_8014C25C += dy;
    }
    else
    {
        D_8014C258 = D_8014C264;
        D_8014C25C = D_8014C268;
    }

    draw_mode = (DR_TPAGE *)prim;
    setDrawTPage(draw_mode, 0, 0, getTPage(0, 1, 320, 0));
    addPrim(ot, draw_mode);
    return prim + 8;
}

/**
 * @brief Emit the golem panel grid, the cursor, and the selected item's name
 *        and detail text into the render context's packet buffer.
 * @param arg0 Render context; packet cursor at +0x4040, panel OT at +0x3C,
 *             text OT at +0x28.
 * @note WIP, NOT byte-matched. Best local match 94.986% (122/148 exact rows).
 *       Instruction count, frame (-0x250), sp slots and control flow are all
 *       exact; the residue is a single coupled saved-register rotation.
 *       @c archive wins gcc's allocation priority race (3076) against @c arg0
 *       (1106) and takes s3, which pushes @c text_anchor out to s7 and rotates
 *       four registers. Matching needs archive's priority to land between 862
 *       and 1106. Nine probe classes and two permuter seeds are already
 *       retired - read working/func_80141228/STATUS.md before touching this,
 *       and do not re-derive them.
 * @see working/func_80141228/STATUS.md
 */
void func_80141228(s32 arg0)
{
    s32 stack_pad[2];
    u8 name_buf[0x100];
    u8 number_buf[0x100];
    B560Entry *entry;
    u8 *archive;
    u8 *offsets;
    u8 *text_base;
    s32 prim;
    s32 panel;
    s32 panel_ot;
    s32 text_anchor;
    s32 detail;

    panel_ot = arg0 + 0x3C;
    prim = *(s32 *)(arg0 + 0x4040);
    panel = 0;
    do
    {
        entry = (B560Entry *)&D_8014B560 + panel;
        prim = func_80141AD0(prim, panel_ot, panel, entry->unkA, entry->unkC, entry->unkE, entry->unk10);
        panel++;
    } while (panel < 0x9A);

    prim = func_80141478(prim, arg0);
    prim = func_801416C8(prim, arg0);
    prim = func_80141074(prim, arg0);
    text_anchor = arg0 + 0x28;

    if (D_8014C184 != 0)
    {
        archive = (u8 *)&D_8014AAA4;
        archive -= 4;
        func_801427F8(name_buf, D_8014AAA4 + (*(u16 *)(((u8)CELL(D_8014C284) >> 2) * 2 + D_8014AAA4 + archive) + archive));
        if ((CELL(D_8014C284) >> 8) & 0xF)
        {
            offsets = D_800EC3DA;
            text_base = offsets - 0x16;
            func_80142728(name_buf, offsets[0] + ((offsets[1] << 8) + text_base));
            func_800A8B90(number_buf, (CELL(D_8014C284) >> 8) & 0xF, 1);
            func_80142728(name_buf, number_buf);
        }
        prim = func_800A88A0(prim, text_anchor, name_buf, 0, 0xA0, 0xA0, 2);
        detail = *(s32 *)(archive + 8);
        prim = func_800A88A0(prim, text_anchor,
                             detail + (*(u16 *)(((u8)CELL(D_8014C284) >> 2) * 2 + detail + archive) + archive),
                             0, 0xA0, 0xB0, 2);
    }

    *(s32 *)(arg0 + 0x4040) = func_80142434(prim, arg0 + 0x24);
}

/**
 * @brief Link a base primitive into the render context OT, emit a highlight
 *        primitive for each on-screen panel cell, then link a final frame prim.
 * @param prim Running primitive pointer; advanced by 0x40 per emitted prim.
 * @param ctx  Render context base; panel OT tag at +0x38, mode flag at +0x404C.
 * @return The advanced primitive pointer (@p prim + 0x40 past the last prim).
 * @note WIP, NOT byte-matched. Best local match 98.28% (141/148 exact rows);
 *       insn count, frame (-0xA8) and sp slots all match. Residue is three
 *       ordering rows: the two entry param-copies (s6/s0 save order), the
 *       preheader idx-init placement (0xAC vs 0xBC), and the walker++/idx+=
 *       delay-slot pick at the loop bottom. Established shapes worth keeping:
 *       the nested `if (temp_v1 >= -0x27) { if (temp_v1 < 0x78)` split defeats
 *       the two-sided range fold into `sltiu` (+3 rows); the ternary is INLINED
 *       into the call arg so it materializes late in v0, not hoisted (+15);
 *       @c t18c splits `D_8014C18C - 0x28` to stop reassociation; `idx += 0x28`
 *       before `i++` fills the D_8014C184 load-delay slot. Handoff in
 *       working/func_80141478/code.c.
 */
s32 func_80141478(s32 prim, s32 ctx)
{
    u8 sp28[0x60];
    s32 tag;
    s32 next;
    s32 temp_v1;
    s32 idx;
    s32 i;
    s32 mode;
    s32 t18c;

    i = 0;
    tag = ctx + 0x38;
    func_8001A5D4(prim, D_8014C168 + (*(s32 *)(ctx + 0x404C) ^ 1) * 0x40C0 + 0x4064);
    next = prim + 0x40;
    addPrim(tag, prim);

    if (D_8014C184 > 0)
    {
        idx = 0;
        do
        {
            t18c = D_8014C18C - 0x28;
            temp_v1 = idx - t18c;
            if (temp_v1 >= -0x27)
            {
                if (temp_v1 < 0x78)
                {
                    if (D_8014C284 == i && ((C198Entry *)D_8014C198)[i].value == 0)
                    {
                        next = func_80141EB4(next, tag, i, 0, 6, temp_v1 + 4, ((C198Entry *)D_8014C198)[i].pad, 1, D_8014C238 ? 0x80 : 2);
                    }
                    else
                    {
                        next = func_80141EB4(next, tag, i, 0, 6, idx - D_8014C18C + 0x2C, ((C198Entry *)D_8014C198)[i].pad, 1, 0);
                    }
                }
            }
            idx += 0x28;
            i++;
        } while (i < D_8014C184);
    }

    prim = next;
    mode = 0x1E;
    if (*(s32 *)(ctx + 0x404C) != 0)
    {
        mode = 0x106;
    }
    func_8001C56C(sp28, 0xCA, mode, 0x4C, 0x74);
    func_8001A5D4(prim, sp28);
    addPrim(tag, prim);
    return prim + 0x40;
}

/** @brief 0x14-byte icon sub-entry within a D_800F1CD0 row; only bytes 8 and 9
 *         (signed x/y offsets) are read here. Distinct from @ref D800F1CD0Entry,
 *         which is the enclosing 0x58-byte row. */
typedef struct { u8 pad0[8]; s8 unk8; s8 unk9; u8 pad10[10]; } IconPos;

/**
 * @brief Link a base primitive into the render context OT, emit a highlight prim
 *        for the selected cursor cell and for each active grid cell, then draw a
 *        framing box sized by the current panel mode.
 * @param prim Running primitive pointer; advanced by 0x40 per emitted prim.
 * @param ctx  Render context base; OT tag at +0x34, mode flag at +0x404C.
 * @return The advanced primitive pointer (@p ret_acc + 0x40 past the last prim).
 * @note WIP, NOT byte-matched. Best local match 89.15% (209 exact rows); insn
 *       count, frame (-0xA8), control flow and sp slots all match. Residue is
 *       coloring/scheduling noise, not structure: the prologue param-copy order
 *       (s0/s5), the icon-block scratch-register numbering and a hoisted
 *       D_8014C248 load, and the switch-tail v0val (target holds it in v0 and
 *       copies to a3 at the merged call). Established shapes worth keeping: the
 *       mode ternary is INLINED as the last call arg so it materializes late
 *       (+12%); @c ret_acc reuses the dead prim/i register for the tail so next
 *       lives in a temp (t0) not a saved reg, keeping the saved-reg count at 6
 *       (+33 exact); rowidx is INLINED into the icon expression so D_8014C27C*0x14
 *       emits before the CELL read. The permuter's higher-scoring 89.69% basin
 *       had FEWER exact rows (204). Handoff in working/func_801416C8/code.c.
 */
s32 func_801416C8(s32 prim, s32 ctx)
{
    u8 sp28[0x60];
    s32 tag;
    s32 next;
    s32 i;
    u32 cell;
    IconPos *icon;
    s32 pos_x;
    s32 pos_y;
    s32 a1val;
    s32 a2val;
    s32 v0val;
    s32 ret_acc;
    C198Entry *c198;
    u8 *cell_ptr;

    tag = ctx + 0x34;
    func_8001A5D4(prim, D_8014C168 + (*(s32 *)(ctx + 0x404C) ^ 1) * 0x40C0 + 0x4064);
    addPrim(tag, prim);
    next = prim + 0x40;

    if (D_8014C238 != 0)
    {
        next = func_80141EB4(next, tag, D_8014C284, D_8014C27C, D_8014C248 * 0x10, D_8014C24C * 0x10, ((C198Entry *)D_8014C198)[D_8014C284].pad, 0, 3);
        icon = (IconPos *)(&D_800F1CD0[D_8014C27C * 0x14 + ((CELL(D_8014C284) >> 12) & 0xF) * 0x58]);
        pos_x = icon->unk8 * 8 + D_8014C248 * 0x10 - D_8014C188 * 8 + 0x3C;
        pos_y = icon->unk9 * 8 + D_8014C24C * 0x10 - D_8014C188 * 8 + 0x3C;
        if ((pos_x != D_8014C258 || pos_y != D_8014C25C) && D_8014C254 == 0)
        {
            D_8014C264 = pos_x;
            D_8014C268 = pos_y;
            D_8014C254 = 4;
        }
    }

    next = func_80140DEC(next, tag);

    if (D_8014C184 > 0)
    {
        i = 0;
        c198 = (C198Entry *)&D_8014C198;
        cell_ptr = &D_80042FD8[0];
        do
        {
            cell = *(u32 *)(cell_ptr + 0x29DC);
            if ((cell & 3) == D_8014C260)
            {
                next = func_80141EB4(next, tag, i, (cell >> 0x11) & 3, ((s32)(cell << 8) >> 27) << 4, ((s32)(cell << 3) >> 27) << 4, c198->pad, 0, i == D_8014C284 ? (D_8014C238 ? 0x80 : 2) : 0);
            }
            c198++;
            cell_ptr += 4;
            i++;
        } while (i < D_8014C184);
    }

    ret_acc = next;

    switch (D_8014C188)
    {
    case 0:
        a2val = 0x38;
        if (*(s32 *)(ctx + 0x404C) != 0)
        {
            a2val = 0x120;
        }
        v0val = 0x40;
        a1val = 0x50;
        goto block_call;
    case 1:
        a2val = 0x30;
        if (*(s32 *)(ctx + 0x404C) != 0)
        {
            a2val = 0x118;
        }
        v0val = 0x50;
        a1val = 0x48;
        goto block_call;
    case 2:
        a2val = 0x28;
        if (*(s32 *)(ctx + 0x404C) != 0)
        {
            a2val = 0x110;
        }
        v0val = 0x60;
        a1val = 0x40;
block_call:
        func_8001C56C(sp28, a1val, a2val, v0val, v0val);
        break;
    }

    func_8001A5D4(ret_acc, sp28);
    addPrim(tag, ret_acc);
    return ret_acc + 0x40;
}

/** @brief Bit-packed panel-cell view of one 0x14-byte D_8014B560 record.
 *  @note Third view of the D_8014B560 storage next to @ref B560 and
 *        @ref B560Entry; do not merge them. */
typedef struct {
    u32 w0;      /* 0x00 flags: abr[1:0], semi[2], state[6:3], anim[10:7], u0[18:11] */
    u32 w1;      /* 0x04 v0[10:3], clut[16:11], cell_w[25:17], cell_h_lo[31:26] */
    u32 w2;      /* 0x08 cell_h_hi[2:0] (overlaps the u16 x/y/w/h view) */
    u8 pad[0x8];
} PanelCell;

/** @brief Access panel record @p i of D_8014B560 as a bit-packed cell. */
#define PANEL(i) (((PanelCell *)&D_8014B560)[i])

/**
 * @brief Draw one UI panel record as a grid of textured sprites plus a
 *        trailing draw-mode packet, gated by the record's state field.
 * @param prim Running primitive pointer; advanced 0x14 per sprite, 8 for the tail.
 * @param ot   Ordering-table tag the primitives are linked into.
 * @param idx  Index of the record within D_8014B560.
 * @param x    Screen x of the panel origin.
 * @param y    Screen y of the panel origin.
 * @param w    Total panel width in pixels.
 * @param h    Total panel height in pixels.
 * @return The advanced primitive pointer.
 * @note WIP, NOT byte-matched. Best local match 96.69% (187/249 exact rows,
 *       gcc272_cdk). Residue is a frame-shape mismatch: target frame -0x20
 *       with saved regs at sp+0x10..0x1C, ours -0x10 with three extra local
 *       spills. Best source preserved in working/func_80141AD0_golem/.
 */
s32 func_80141AD0(s32 prim, s32 ot, s32 idx, s32 x, s32 y, s32 w, s32 h)
{
    SPRT *sprt;
    s32 anim;
    s32 color;
    s32 y_done;
    s32 x_done;
    s32 row_h;
    s32 cell_h;
    s32 seg_w;
    s32 avail;
    s32 remaining;
    s32 code_val;

    color = 0x808080;

    switch ((PANEL(idx).w0 >> 3) & 0xF) {
    case 0:
    case 7:
        break;
    case 1:
    case 2:
    case 3:
        if ((((PANEL(idx).w0 >> 3) & 0xF) - 1) != D_8014C188)
        {
            return prim;
        }
        break;
    case 4:
        if (D_8014C18C == 0)
        {
            return prim;
        }
        anim = (PANEL(idx).w0 >> 7) & 0xF;
        if (anim != 0)
        {
            color = 0xC0;
            PANEL(idx).w0 = (PANEL(idx).w0 & ~0x780) | (((anim - 1) & 0xF) << 7);
        }
        break;
    case 5:
        if (!((D_8014C18C / 40) < (D_8014C184 - 1)))
        {
            return prim;
        }
        anim = (PANEL(idx).w0 >> 7) & 0xF;
        if (anim != 0)
        {
            color = 0xC0;
            PANEL(idx).w0 = (PANEL(idx).w0 & ~0x780) | (((anim - 1) & 0xF) << 7);
        }
        break;
    case 6:
        anim = (PANEL(idx).w0 >> 7) & 0xF;
        if (anim != 0)
        {
            color = 0xC0C0C0;
            PANEL(idx).w0 = (PANEL(idx).w0 & ~0x780) | (((anim - 1) & 0xF) << 7);
        }
        break;
    }

    y_done = 0;
    if (y_done < h)
    {
        code_val = 0x64;
        remaining = h - y_done;
        do
        {
            row_h = remaining;
            cell_h = (PANEL(idx).w1 >> 26) | ((PANEL(idx).w2 & 7) << 6);
            x_done = 0;
            if (cell_h < row_h)
            {
                row_h = cell_h;
            }
            while (x_done < w)
            {
            sprt = (SPRT *)prim;
            avail = w - x_done;
            seg_w = (PANEL(idx).w1 >> 17) & 0x1FF;
            if (seg_w >= avail)
            {
                seg_w = avail;
            }
            SET_BGR0_PACKED(sprt, color);
            setlen(sprt, 4);
            setcode(sprt, code_val);
            if ((PANEL(idx).w0 >> 2) & 1)
            {
                setcode(sprt, 0x66);
            }
                sprt->x0 = x + (x_done + 8);
                sprt->y0 = y + y_done;
                sprt->w = seg_w;
                sprt->h = row_h;
                sprt->u0 = PANEL(idx).w0 >> 11;
                sprt->v0 = PANEL(idx).w1 >> 3;
                sprt->clut = ((PANEL(idx).w1 >> 11) & 0x3F) | 0x7C80;
                addPrim(ot, sprt);
                x_done += (PANEL(idx).w1 >> 17) & 0x1FF;
                prim += 0x14;
            }
            y_done += (PANEL(idx).w1 >> 26) | ((PANEL(idx).w2 & 7) << 6);
            remaining = h - y_done;
        } while (y_done < h);
    }

    setlen(prim, 1);
    ((u_long *)prim)[1] = ((PANEL(idx).w0 & 3) << 5) | 0xE1000005;
    addPrim(ot, prim);
    return prim + 8;
}

/** @brief Header view of an 88-byte D_800F1CD0 icon layout row. */
typedef struct
{
    u8 part_count;  /* 0x00 */
    u8 pad1[3];
    s16 origin_x;   /* 0x04 */
    s16 origin_y;   /* 0x06 */
    s8 base_x;      /* 0x08 */
    s8 base_y;      /* 0x09 */
} GolemIconLayout;

/** @brief One positioned part, viewed at layout + sub*0x14 + i*4. */
typedef struct
{
    u8 pad[0xC];
    s8 x;           /* 0x0C */
    s8 y;           /* 0x0D */
    s16 glyph_id;   /* 0x0E */
} GolemIconPart;

s32 func_801420CC();

/**
 * @brief Draw one grid cell's icon: the base glyph plus each positioned part
 *        from its D_800F1CD0 layout row, then splice a draw-mode packet.
 * @param packet     Running primitive pointer; advanced per emitted packet.
 * @param ot         Ordering-table tag the primitives are linked into.
 * @param cell_index Grid cell index; selects the layout row via CELL bits 12-15.
 * @param sub_index  Layout sub-entry (rotation) index; 0x14-byte stride.
 * @param x          Screen x of the cell.
 * @param y          Screen y of the cell.
 * @param clut       CLUT selector passed to the part glyphs.
 * @param use_origin When 1, offset x/y by the layout row's origin fields.
 * @param arg8       Flag word forwarded to func_801420CC for the part glyphs.
 * @return The advanced primitive pointer (past the trailing 8-byte packet).
 * @note WIP, NOT byte-matched. Best local match 94.33% (108/134 exact rows,
 *       gcc272_cdk); frame and sp slots match, residue is 23 argdiff rows of
 *       register-coloring noise plus one 3-row ordering run. Best source
 *       preserved in working/func_80141EB4_golem/.
 */
s32 func_80141EB4(s32 packet, s32 *ot, s32 cell_index, s32 sub_index, s32 x, s32 y, s32 clut, s32 use_origin, s32 arg8)
{
    u32 cell;
    s32 row;
    s32 sub_off;
    s32 row_off;
    u8 *table;
    u8 *layout;
    s32 i;

    cell = CELL(cell_index);
    row = cell >> 0xC;
    row = row & 0xF;
    if (use_origin == 1)
    {
        u8 *base = D_800F1CD0;
        GolemIconLayout *l = (GolemIconLayout *)(base + row * 0x58);
        x += l->origin_x * 8;
        y += l->origin_y * 8;
    }
    i = 0;
    table = D_800F1CD0;
    sub_off = sub_index * 0x14;
    row_off = row * 0x58;
    {
        GolemIconLayout *entry = (GolemIconLayout *)(sub_off + row_off + table);
        packet = func_801420CC(packet, ot, ((cell >> 2) & 0x3F) + 0x13,
                               (entry->base_x * 8) + x, (entry->base_y * 8) + y, 9, 0);
    }
    layout = row_off + table;
    if (*layout != 0)
    {
        u8 *tbl = table;
        s32 roff = row_off;
        u8 *lay = layout;
        s32 iv = sub_off;
        do
        {
            GolemIconPart *part = (GolemIconPart *)(iv + roff + tbl);
            iv += 4;
            i += 1;
            packet = func_801420CC(packet, ot, part->glyph_id,
                                   (part->x * 0x10) + x, (part->y * 0x10) + y, clut, arg8);
        } while (i < *lay);
    }
    *(u8 *)(packet + 3) = 1;
    *(u32 *)(packet + 4) = 0xE1000025;
    *(u32 *)(packet + 0) = (*(u32 *)(packet + 0) & 0xFF000000) | (*ot & 0xFFFFFF);
    *ot = (*ot & 0xFF000000) | (packet & 0xFFFFFF);
    return packet + 8;
}

/** @brief 8-byte texture entry: u/v bytes plus 16-bit width/height. */
typedef struct
{
    u8 u0;
    u8 pad1;
    u8 v0;
    u8 pad3;
    u16 w;
    u16 h;
} GolemTexEntry;

extern GolemTexEntry D_8014B2D0[];

/**
 * @brief Emit one glyph: an optional colored backing TILE (chosen by the low
 *        flag bits) followed by a textured SPRT from the D_8014B2D0 table.
 * @param prim  Running primitive pointer; advanced 0x10 for the TILE, 0x14 for the SPRT.
 * @param ot    Ordering-table tag the primitives are linked into.
 * @param index Entry index into the D_8014B2D0 UV/size table.
 * @param x     Screen x of the glyph.
 * @param y     Screen y of the glyph.
 * @param arg5  CLUT selector; 0xF and 9 also gate the brightness overrides.
 * @param flags Bit 7 dims the sprite; low bits 1-3 pick the backing tile color.
 * @return The advanced primitive pointer.
 */
s32 func_801420CC(s32 prim, s32 ot, s32 index, s32 x, s32 y, s32 arg5, s32 flags)
{
    TILE *tile;
    SPRT *sprt;
    GolemTexEntry *entry;
    GolemTexEntry *entry2;
    u32 color;
    GolemTexEntry *base;
    GolemTexEntry *base2;

    if ((flags & 0x7F) != 0)
    {
        tile = (TILE *)prim;
        switch (flags & 0x7F)
        {
            case 1:
                color = 0x80;
                break;
            case 2:
                color = 0x800080;
                break;
            case 3:
                color = 0x8000;
                break;
            default:
                setlen(tile, 3);
                goto skip_color;
        }
        SET_BGR0_PACKED(prim, color);
        setlen(tile, 3);
skip_color:
        setcode(tile, 0x62);
        base = D_8014B2D0;
        entry = base + index;
        setXY0(tile, x, y);
        setWH(tile, entry->w, entry->h);
        addPrim(ot, tile);
        prim = (s32)tile + 0x10;
    }

    sprt = (SPRT *)prim;
    SET_BGR0_PACKED(sprt, 0x606060);
    setSprt(sprt);
    if (flags & 0x80)
    {
        setRGB0(sprt, 0x38, 0x38, 0x38);
        sprt->code |= 2;
    }
    if (arg5 == 0xF)
    {
        setRGB0(sprt, 0x40, 0x40, 0x40);
        sprt->code |= 2;
    }
    if (arg5 != 9)
    {
        sprt->code |= 2;
    }
    base2 = D_8014B2D0;
    entry2 = base2 + index;
    setXY0(sprt, x, y);
    setWH(sprt, entry2->w, entry2->h);
    sprt->u0 = entry2->u0;
    sprt->v0 = entry2->v0;
    sprt->clut = (arg5 & 0x3F) | 0x7C80;
    addPrim(ot, sprt);
    return prim + 0x14;
}

#define GOLEM_GPU_ADDR_MASK 0xFFFFFF
#define GOLEM_GPU_TAG_HIGH_MASK 0xFF000000

/** @brief Generic GPU packet prefix used while advancing the primitive buffer. */
typedef struct
{
    /* 0x0 */ s32 tag;
    /* 0x4 */ s32 word4;
    /* 0x8 */ s16 x0;
    /* 0xA */ s16 y0;
    /* 0xC */ s16 unkC;
    /* 0xE */ u16 unkE;
} GolemGpuPacket;

/**
 * @brief Emit a rectangle outline as four LINE_F2 packets (top, right,
 *        bottom, left) linked into the ordering table.
 * @param p     Running packet cursor; one 0x10-byte packet per edge.
 * @param ot    Ordering-table tag the packets are linked into.
 * @param x     Left edge x.
 * @param y     Top edge y.
 * @param w     Rectangle width.
 * @param h     Rectangle height.
 * @param color Packed BGR line color.
 * @return The advanced packet cursor (@p p + 4 packets).
 * @note The first packet's OT splice is spelled manually through @c tmp, and
 *       @c tmp is reused for y + h below; both are required to match.
 */
GolemGpuPacket *func_801422BC(GolemGpuPacket *p, s32 *ot, s32 x, s32 y, s32 w, s32 h, s32 color)
{
    s32 tmp;

    p->word4 = color;
    setlen(p, 3);
    setcode(p, 0x40);
    p->x0 = x;
    p->y0 = y;
    p->unkC = x + w;
    p->unkE = y;
    tmp = GOLEM_GPU_TAG_HIGH_MASK;
    p->tag = (p->tag & GOLEM_GPU_TAG_HIGH_MASK) | (*ot & GOLEM_GPU_ADDR_MASK);
    *ot = (*ot & tmp) | ((s32)p & GOLEM_GPU_ADDR_MASK);
    p++;

    p->word4 = color;
    setlen(p, 3);
    setcode(p, 0x40);
    p->x0 = x + w;
    p->y0 = y;
    p->unkC = x + w;
    p->unkE = y + h;
    addPrim(ot, p);
    p++;

    p->word4 = color;
    setlen(p, 3);
    setcode(p, 0x40);
    p->x0 = x + w;
    tmp = y + h;
    p->y0 = tmp;
    p->unkC = x;
    p->unkE = y + h;
    addPrim(ot, p);
    p++;

    p->word4 = color;
    setlen(p, 3);
    setcode(p, 0x40);
    p->x0 = x;
    p->y0 = y;
    p->unkC = x;
    p->unkE = y + h;
    addPrim(ot, p);
    return p + 1;
}

#define GOLEM_FADE_NEUTRAL 0x100
#define GOLEM_FADE_ADDITIVE_THRESHOLD (GOLEM_FADE_NEUTRAL + 1)
#define GOLEM_FADE_ADDITIVE_DRAW_MODE 0x25
#define GOLEM_FADE_SUBTRACTIVE_DRAW_MODE 0x45

/** @brief Packet view for a fade TILE or draw-mode command. */
typedef union
{
    TILE tile;
    DR_TPAGE draw_mode;
} GolemFadePrimitive;

/** Advance a fade packet cursor by the concrete packet just emitted. */
#define GOLEM_NEXT_FADE_PRIMITIVE(primitive, type) \
    ((GolemFadePrimitive*)((u8*)(primitive) + sizeof(type)))

/** @brief Fade colour triple plus its remaining step count. */
typedef struct
{
    s16 r;                  // 0x00
    s16 g;                  // 0x02
    s16 b;                  // 0x04
    s16 steps;              // 0x06
} GolemFade;

extern GolemFade D_8014C178;
extern GolemFade D_8014C240;

/**
 * @brief Set the screen-fade target color and step count.
 * @param arg0 Target red component (0x100 = neutral).
 * @param arg1 Target green component.
 * @param arg2 Target blue component.
 * @param arg3 Number of frames to reach the target.
 */
void func_80142418(s16 arg0, s16 arg1, s16 arg2, s16 arg3)
{
    D_8014C178.r = arg0;
    D_8014C178.g = arg1;
    D_8014C178.b = arg2;
    D_8014C178.steps = arg3;
}

/**
 * @brief Step the current fade color toward the target, then emit a
 *        full-screen semi-transparent TILE plus its draw-mode packet unless
 *        the fade sits at neutral (0x100/0x100/0x100).
 * @param primitive           Running fade packet cursor.
 * @param ordering_table_tag  Ordering-table tag the packets are linked into.
 * @return The advanced packet cursor.
 */
GolemFadePrimitive* func_80142434(GolemFadePrimitive* primitive, u_long* ordering_table_tag)
{
    s32 dr;
    s32 dg;
    s32 db;
    s32 draw_mode;

    if (D_8014C178.steps != 0)
    {
        dr = (D_8014C178.r - D_8014C240.r) / D_8014C178.steps;
        dg = (D_8014C178.g - D_8014C240.g) / D_8014C178.steps;
        db = (D_8014C178.b - D_8014C240.b) / D_8014C178.steps;
        D_8014C178.steps = D_8014C178.steps - 1;
        D_8014C240.r = D_8014C240.r + dr;
        D_8014C240.g = D_8014C240.g + dg;
        D_8014C240.b = D_8014C240.b + db;
    }
    else
    {
        D_8014C240.r = D_8014C178.r;
        D_8014C240.g = D_8014C178.g;
        D_8014C240.b = D_8014C178.b;
    }
    if ((D_8014C240.r != GOLEM_FADE_NEUTRAL) ||
        (D_8014C240.g != D_8014C240.r) ||
        (D_8014C240.b != D_8014C240.g))
    {
        if (D_8014C240.r >= GOLEM_FADE_ADDITIVE_THRESHOLD)
        {
            primitive->tile.r0 = D_8014C240.r - 1;
            primitive->tile.g0 = D_8014C240.g - 1;
            primitive->tile.b0 = D_8014C240.b - 1;
        }
        else
        {
            if (D_8014C240.r == GOLEM_FADE_NEUTRAL)
            {
                primitive->tile.r0 = 0;
            }
            else
            {
                primitive->tile.r0 = ~D_8014C240.r;
            }
            if (D_8014C240.g == GOLEM_FADE_NEUTRAL)
            {
                primitive->tile.g0 = 0;
            }
            else
            {
                primitive->tile.g0 = ~D_8014C240.g;
            }
            if (D_8014C240.b == GOLEM_FADE_NEUTRAL)
            {
                primitive->tile.b0 = 0;
            }
            else
            {
                primitive->tile.b0 = ~D_8014C240.b;
            }
        }

        setTile(&primitive->tile);
        setSemiTrans(&primitive->tile, 1);
        primitive->tile.w = SCREEN_WIDTH;
        draw_mode = GOLEM_FADE_ADDITIVE_DRAW_MODE;
        SET_YX0(&primitive->tile, 0, 0);
        primitive->tile.h = SCREEN_HEIGHT;
        addPrim(ordering_table_tag, &primitive->tile);

        primitive = GOLEM_NEXT_FADE_PRIMITIVE(primitive, TILE);
        if (D_8014C240.r < GOLEM_FADE_ADDITIVE_THRESHOLD)
        {
            draw_mode = GOLEM_FADE_SUBTRACTIVE_DRAW_MODE;
        }
        setDrawTPage(&primitive->draw_mode, 0, 0, draw_mode);
        addPrim(ordering_table_tag, &primitive->draw_mode);

        primitive = GOLEM_NEXT_FADE_PRIMITIVE(primitive, DR_TPAGE);
    }
    return primitive;
}

/**
 * @brief Append the encoded string @p src to the end of @p dest.
 * @param dest Destination encoded-text buffer (null-terminated).
 * @param src  Source encoded-text buffer (null-terminated).
 * @note func_801427AC is intentionally left without a prototype here; the
 *       implicit declaration is required to match.
 */
void func_80142728(u8 *dest, u8 *src)
{
    s32 dst_len;
    s32 src_len;
    s32 i;

    dst_len = func_801427AC(dest);
    src_len = func_801427AC(src);
    for (i = 0; i < src_len; i++)
    {
        dest[dst_len + i] = src[i];
    }
    dest[dst_len + i] = 0;
}

/**
 * @brief Measure an encoded string's length in bytes: lead bytes 0x19..0x1F
 *        start a two-byte character, everything else is one byte.
 * @param text Null-terminated encoded-text buffer.
 * @return Length in bytes, excluding the terminator.
 */
s32 func_801427AC(u8 *text)
{
    u8 *p;
    u8 c;
    s32 len;

    p = text;
    c = *p;
    len = 0;
    while (c != 0)
    {
        if ((u32)(c - 0x19) < 7)
        {
            p += 2;
            len += 2;
        }
        else
        {
            p += 1;
            len += 1;
        }
        c = *p;
    }
    return len;
}

/** A lead byte in the range 0x19..0x1F starts a two-byte encoded character. */
#define IS_DBCS_LEAD_BYTE(byte) (((byte) >= 0x19) && ((byte) <= 0x1F))

/**
 * @brief Copy the encoded string @p src to @p dst, honoring two-byte
 *        characters, and null-terminate the destination.
 * @param dst Destination buffer.
 * @param src Source encoded-text buffer (null-terminated).
 */
void func_801427F8(u8* dst, u8* src)
{
    const u8* scan_cursor;
    s32 byte_count;
    s32 i;

    scan_cursor = src;
    byte_count = 0;

    while (*scan_cursor)
    {
        if (IS_DBCS_LEAD_BYTE(*scan_cursor))
        {
            scan_cursor += 2;
            byte_count += 2;
        }
        else
        {
            scan_cursor += 1;
            byte_count += 1;
        }
    }

    for (i = 0; i < byte_count; i++)
    {
        dst[i] = src[i];
    }

    dst[i] = 0;
}
