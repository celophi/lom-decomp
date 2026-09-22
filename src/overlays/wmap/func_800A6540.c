#include "wmap_resource_support.h"
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
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

extern WmapConfigA D_800D95D8[];
extern WmapMotion D_801AFBD0[];
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern u8 D_800DCEF4[4];
extern u8 D_80123538[];
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void akao_cmd_c2(s32, s32, s32, s32);
extern void func_800A7738(void);

/** @brief Initialize active directional actors and play the transition sound. */
void func_800A6540(void)
{
    s32 i;
    WmapConfigA *actor;

    for (i = 40; i < 120; i++)
    {
        D_801AFBD0[i].state = 0;
        D_80139988[i].resource = D_80123538;
    }
    for (i = 0; i < 4; i++)
    {
        actor = &D_800D95D8[i];
        if (D_800DCEF4[i] != 0)
        {
            D_80139988[i + 20].resource = D_80123538;
            actor->field_06 = 15;
            actor->field_0E = i + 1;
            actor->field_10 = -1;
            actor->field_22 = 128;
            actor->field_26 = 2;
            actor->field_02 = 0;
            actor->field_24 = 0;
            D_801AFD60[i].state = 1;
            D_801AFD60[i].angle = i << 10;
            D_801AFD60[i].z = 90000;
            D_801AFD60[i].x = 900;
            D_801AFD60[i].field_0E = 0;
        }
    }
    akao_cmd_c2(0, 30, 127, 48);
    if (D_800DCEF4[3] & (D_800DCEF4[2] & (D_800DCEF4[0] & D_800DCEF4[1])))
    {
        func_800652A8(49, 128);
    }
    else
    {
        func_800652A8(21, 128);
    }
    D_801B2E44 = 64;
    D_801B2E40++;
    func_800A7738();
}
