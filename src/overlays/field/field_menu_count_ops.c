#include "common.h"

/**
 * @brief Object record at D_80122C0C shared by the wrappers below.
 */
typedef struct
{
    s16 unk0; /* 0x00 */
    s16 unk2; /* 0x02 */
    u8 pad4[2];
    s16 unk6; /* 0x06 */
} UnkStruct80122C0C;

void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);
void func_80087F44(s16 arg0, s32 *arg1);
void func_80087D8C(s16 arg0, s32 arg1, s32 arg2, s32 arg3);

extern u8 D_800459AE;
extern s32 D_80122C00;
extern UnkStruct80122C0C D_80122C0C;
extern s16 D_80122C10;
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Set D_80122C10 to 1 when D_800459AE is at least 0x28, else 0.
 */
void func_800C61D8(void)
{
    if (D_800459AE >= 0x28)
    {
        D_80122C10 = 1;
    }
    else
    {
        D_80122C10 = 0;
    }
}

/**
 * @brief Thin wrapper around func_800AD0C8.
 */
void func_800C6208(void)
{
    func_800AD0C8();
}

/**
 * @brief Dispatch each populated row of the selected menu entry and count them.
 */
void func_800C6228(void)
{
    s32 count;
    s32 i;
    s32 entry_offset;
    s32 row_offset;
    u8 *menu;
    u8 *base;

    count = 0;
    i = 0;
    menu = g_menuLayoutBuffer;
    base = menu + 0x2B58;
loop:
    row_offset = i << 6;
    entry_offset = menu[D_80122C00 + 0x29D8] * 0x14C;
    if (menu[row_offset + entry_offset + 0x2B58] != 0)
    {
        entry_offset += (s32)base;
        func_800B2844(count, (u8 *)(entry_offset + row_offset), 0xFF);
        count += 1;
    }
    i += 1;
    if (i < 4)
    {
        goto loop;
    }
    D_80122C10 = count;
}

/**
 * @brief Count the empty 0x40-byte slots and subtract them from D_80122C10, clamping at 0.
 */
void func_800C62E8(void)
{
    s32 i;
    s32 count;
    u8 *p;
    u16 v0;
    s16 *ptr;

    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0xCE0] == 0)
        {
            count++;
        }
    }
    ptr = &D_80122C10;
    if (count >= *ptr)
    {
        v0 = 0;
    }
    else
    {
        v0 = (u16) *ptr - count;
    }
    *ptr = v0;
}

/**
 * @brief Thin stack-frame wrapper around func_800C3A00 with a fixed arg.
 */
void func_800C6344(void)
{
    func_800C3A00(0x92BC);
}

/**
 * @brief Forward the D_80122C0C object id and position pair to func_80087680.
 */
void func_800C6364(void)
{
    func_80087680(D_80122C0C.unk0, D_80122C0C.unk2, D_80122C0C.unk2, 0, 0, 0);
}

/**
 * @brief Fetch a 3D position for the current object, scale it down and re-emit it.
 *
 * Queries func_80087F44 for the object named by @c D_80122C0C.unk0 into a local
 * vector, divides each component by 256 (rounding toward zero via the +0xFF
 * negative fix-up), subtracts @c D_80122C0C.unk6 from the Y component, and passes
 * the result to func_80087D8C.
 */
void func_800C63A0(void)
{
    s32 vec[3];
    s32 x;
    s32 y;
    s32 z;
    s32 yshift;
    s32 y2;

    func_80087F44(D_80122C0C.unk0, vec);
    x = vec[0];
    if (x < 0)
    {
        x += 0xFF;
    }
    y = vec[1];
    x >>= 8;
    vec[0] = x;
    if (y < 0)
    {
        y += 0xFF;
    }
    z = vec[2];
    yshift = y >> 8;
    vec[1] = yshift;
    if (z < 0)
    {
        z += 0xFF;
    }
    z >>= 8;
    vec[2] = z;
    y2 = yshift - D_80122C0C.unk6;
    vec[1] = y2;
    func_80087D8C(D_80122C0C.unk0, x, y2, z);
}
