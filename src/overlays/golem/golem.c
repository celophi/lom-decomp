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

typedef struct {
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} RECT;

extern u8 D_8014287C[];
void func_80140438(RECT *, u8 *);

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
