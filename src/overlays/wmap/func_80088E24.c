#include "common.h"

/** @brief Actor storage consumed by the animation and drawing helpers. */
typedef struct
{
    u8 pad_00[0x22];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapActor;
/** @brief Animation resource slot. */
typedef struct { s32 word; void *data; } WmapResource;
/** @brief Drawing position with the original 0x14-byte stride. */
typedef struct { s32 position; u8 pad[0x10]; } WmapPosition;

extern WmapActor D_800D9268[];
extern WmapResource D_80139988[];
extern WmapPosition D_801AFBE0[];
extern s32 D_80139234;
extern s32 D_801B2988;
extern s32 D_801B298C;
extern s32 func_8006CC4C(WmapActor *, WmapResource *);
extern void func_80066F9C(WmapActor *, s32, s32, s32, s32);

/** @brief Animate and draw the active actor range, then advance its countdown. */
void func_80088E24(void)
{
    s32 i;
    s32 remaining;

    for (i = 61; i < D_80139234 + 60; i++)
    {
        D_800D9268[i].field_22 = 0;
        D_800D9268[i].field_26 = 2;
        func_8006CC4C(&D_800D9268[i], &D_80139988[i]);
        func_80066F9C(&D_800D9268[i], D_801AFBE0[i].position, 8, 10, 0);
    }
    remaining = D_801B298C - 1;
    D_801B298C = remaining;
    if (remaining == 0)
    {
        D_801B2988++;
    }
}
