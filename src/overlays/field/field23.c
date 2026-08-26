#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef union
{
    s32 w;
    struct
    {
        u32 low24 : 24;
        u32 mid7 : 7;
        u32 top1 : 1;
    } b;
} Word8;

typedef union
{
    s32 w;
    struct
    {
        u8 b0;
        s8 b1;
        u8 b2;
        u8 b3;
    } b;
} Word4C;

typedef union
{
    s32 w;
    struct
    {
        u32 low10 : 10;
        u32 rest6 : 6;
        u32 hi16 : 16;
    } b;
    struct
    {
        s16 lo;
        s16 hi;
    } h;
} Word174;

typedef union
{
    s32 w;
    struct
    {
        u32 b0 : 1;
        u32 p1 : 4;
        u32 b5 : 1;
        u32 b6 : 1;
        u32 rest : 25;
    } b;
} Word178;

typedef struct
{
    u8 pad0[4];
    Word8 unk4;
    Word8 unk8;
    s32 unkC;
    u8 pad10[4];
    s32 unk14;
    s16 unk18;
    u8 pad1A[0x48 - 0x1A];
    s16 unk48;
    s16 unk4A;
    Word4C unk4C;
    u8 pad50[0x174 - 0x50];
    Word174 unk174;
    Word178 unk178;
    u8 pad17C[0x18E - 0x17C];
    u8 unk18E;
    u8 pad18F[0x23C - 0x18F];
} Slot;

extern Slot D_80105AE0[];

void field_load_vram_resource(s32 id, s16 *rect, s32 arg2);

/**
 * @brief Reset all 13 field animation slots to their idle defaults and reload
 *        the shared VRAM resource block.
 * @note Each slot's animation cursor, timers and flag bits are cleared; the
 *       run-position field unk8 is reseeded from unk4's low 24 bits.
 */
void func_80084524(void)
{
    s16 rect[4];
    s32 i;

    i = 0;
    do
    {
        D_80105AE0[i].unk4A = 0;
        D_80105AE0[i].unk48 = 0;
        D_80105AE0[i].unk14 = i;
        D_80105AE0[i].unk18 = 0;
        D_80105AE0[i].unkC = 0;
        D_80105AE0[i].unk18E = 0;
        D_80105AE0[i].unk8.b.low24 = D_80105AE0[i].unk4.b.low24;
        D_80105AE0[i].unk8.b.mid7 = 0;
        D_80105AE0[i].unk8.b.top1 = 0;
        D_80105AE0[i].unk4C.w |= 1;
        D_80105AE0[i].unk174.b.low10 = 0x32;
        D_80105AE0[i].unk174.h.hi = 0;
        D_80105AE0[i].unk178.b.b0 = 0;
        D_80105AE0[i].unk4C.b.b1 = (s8)i;
        D_80105AE0[i].unk178.b.b5 = 0;
        D_80105AE0[i].unk178.b.b6 = 0;
        i++;
    } while (i < 13);

    rect[0] = 0x3C0;
    rect[1] = 0x100;
    rect[2] = 0x100;
    rect[3] = 0x1E0;
    field_load_vram_resource(0x5DA, rect, 0);
    DrawSync(0);
}
