#include "common.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"

typedef struct
{
    s32 unk0;
    u8 pad4[0xC - 4];
    s32 unkC;
    u8 pad10[0x18 - 0x10];
    s32 unk18;
} Struct_D80105880;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x244 - 0x25];
} FieldActorState;

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

extern Struct_D80105880 D_80105880[];
extern FieldActorState g_field_actor_slots[];
extern s32 D_8010CFD4;

void func_800A3B78(s32 arg0);
void func_8009A4A0(s32 arg0);

/**
 * @brief Tear down the actor bound to a track once its slot has gone idle.
 * @param arg0 Track index (clamped to 2 when >= 3 for the D_80105880 lookup).
 * @note Only acts when the track's stored value matches arg0 and the bound
 *       actor slot is free; then it releases the actor, clears the track and
 *       D_8010CFD4, and notifies func_8009A4A0.
 */
void func_80084424(s32 arg0)
{
    s32 value;
    s32 index;
    s32 call_index;
    s32 small;
    Struct_D80105880 *base;
    FieldActorState *actors;

    value = arg0;
    base = D_80105880;
    small = value < 3;
    index = value;
    if (small == 0)
    {
        index = 2;
    }

    if (base[index].unkC == value)
    {
        index = value;
        actors = g_field_actor_slots;
        if (value >= 3)
        {
            index = 2;
        }

        if (actors[base[index].unk18].unk24 == 0)
        {
            func_800A3B78(value);
            index = value;
            if (value >= 3)
            {
                index = 2;
            }

            call_index = value;
            base[index].unk0 = 0;
            D_8010CFD4 = 0;
            if (value >= 3)
            {
                call_index = 2;
            }
            func_8009A4A0(call_index);
        }
    }
}

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
