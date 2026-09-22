/* Partial WMAP decompilation: 85.538460% (gcc280_g0). */
#include "common.h"

/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;


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

extern WmapConfigA D_800D9268[];
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_80121538[];
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_801B2AC0;
extern s32 D_801B2AC4;
extern void func_8008FA40(void);

/** @brief Initialize twenty-four actors and advance to their update step. */
void func_8008F970(void)
{
    s32 i;
    WmapConfigA *actor;
    WmapSlot8 *resource;
    WmapSlot14 *slot;

    i = 60;
    slot = &D_801AFBD0[60];
    resource = &D_80139988[60];
    actor = &D_800D9268[60];
    D_80139234 = 0;
    D_8013923C = 0xFFFF;
next_actor:
    {
        resource->field_04 = D_80121538;
        actor->field_06 = 15;
        actor->field_10 = -1;
        actor->field_26 = 4;
        actor->field_22 = 1;
        actor->field_02 = 0;
        actor->field_0E = 0;
        actor->field_24 = 0x81;
        slot->field_00 = 0;
        slot++;
        resource++;
        i++;
        actor++;
    }
    if (i < 84)
    {
        goto next_actor;
    }
    D_801B2AC4 = D_8013923C;
    D_801B2AC0++;
    func_8008FA40();
}
