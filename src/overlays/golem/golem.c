#include "common.h"

extern u8 *D_8014C168;
extern s32 D_8014C16C;
extern s32 D_8014C170;
extern u8 D_80042FD8[];
extern s32 D_80122C00;
extern s32 D_8014C180;
extern s32 D_8014C184;
extern s32 D_8014C188;
extern s32 D_8014C18C;
extern u8 D_8014C198[];
extern s32 D_8014C238;
extern s32 D_8014C23C;
extern s32 D_8014C248;
extern s32 D_8014C24C;
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
extern B560 D_8014B560;

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
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

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
void func_80140438(RECT *, u8 *);

extern s32 D_800F22AC;

void func_801405A0(void);
void func_80141228(void);

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