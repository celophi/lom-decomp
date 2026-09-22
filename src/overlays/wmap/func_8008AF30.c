/* Partial WMAP decompilation: 88.989130% (gcc280_g0). */
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
extern WmapMotion D_801AFD60[];
extern WmapResource D_80139988[];
extern s32 *D_80139280;
extern u8 D_80121538[];
extern s32 D_801B0FD0;
extern s32 D_801B29F8;
extern s32 D_801B29FC;
extern s32 rand(void);
extern void func_8008C558(void);

/** @brief Initialize the effect descriptor and actors with randomized angles. */
void func_8008AF30(void)
{
    s32 i;
    s32 angle;
    WmapConfigA *actor;
    s32 actor_offset;
    s32 resource_offset;
    WmapResource *resource;
    WmapMotion *motion;

    actor_offset = 0;
    resource_offset = 20 * 8;
    D_801B0FD0 = 30;
    D_80139280[0x15] = 192;
    D_80139280[0x16] = 16;
    D_80139280[0x17] = 128;
    D_80139280[0x18] = 0;
    D_80139280[0x19] = -1;
    D_80139280[0x1A] = 4000;
    D_80139280[0x1B] = 20;
    D_80139280[0x1C] = 8;
    D_80139280[0x1D] = 2;
    D_80139280[0x1E] = 10;
    for (i = 0; i < 30; i++)
    {
        motion = &D_801AFD60[i];
        motion->state = 1;
        angle = rand();
        actor = (WmapConfigA *)((u8 *)D_800D95D8 + actor_offset);
        actor_offset += 44;
        resource = (WmapResource *)((u8 *)D_80139988 + resource_offset);
        resource_offset += 8;
        motion->angle = angle & 4095;
        motion->scale = 128;
        motion->z = 10;
        motion->x = 0;
        motion->field_0E = 0;
        resource->resource = D_80121538;
        actor->field_06 = 15;
        actor->field_0E = 2;
        actor->field_10 = -1;
        actor->field_26 = 4;
        actor->field_24 = 127;
        actor->field_02 = 0;
        actor->field_22 = 0;
    }
    D_801B29FC = 64;
    D_801B29F8++;
    func_8008C558();
}
