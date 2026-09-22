#include "common.h"

/** @brief World-map actor configuration. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s16 field_10;
    s16 field_12;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern WmapResource D_80139988[];
extern u8 D_80121538[];
extern s32 D_80182DF4;
extern s32 D_801B2648;
extern s32 D_801B264C;
extern u32 rand(void);
extern void func_80078CB4(void);

/** @brief Initialize randomized motion for the effect actors. */
void func_8007724C(void)
{
    s32 i;
    WmapConfigA *actor;

    D_80182DF4 = 1;
    for (i = 124; i < 170; i++)
    {
        actor = &D_800D9268[i];
        D_801AFBD0[i].state = 1;
        D_801AFBD0[i].angle = (rand() * 155) >> 10;
        D_801AFBD0[i].field_0E = (rand() * 7) >> 6;
        D_801AFBD0[i].x = ((s32)(rand() << 6) >> 15) + 16;
        D_801AFBD0[i].scale = rand() >> 9;
        D_801AFBD0[i].field_10 = ((s32)(rand() << 6) >> 15) + 4;
        D_801AFBD0[i].field_12 = 0;
        D_80139988[i].resource = D_80121538;
        actor->field_06 = 15;
        actor->field_02 = 0;
        actor->field_10 = -1;
        actor->field_26 = 0;
        actor->field_22 = D_80182DF4;
        actor->field_24 = D_80182DF4;
        actor->field_0E = i % 3;
    }
    D_801B264C = 16;
    D_801B2648++;
    func_80078CB4();
}
