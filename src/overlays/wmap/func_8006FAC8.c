#include "common.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

extern u8 D_800D9344[];
extern u8 D_801399B0[];
extern s32 D_801B2440;
extern s32 D_801B2444;
extern void func_8006CDDC(void);
extern void func_8006CC4C(void *, void *);
extern void func_80066F9C(void *, s32, s32, s32, s32);

/** @brief Draw the projected effect and advance after its countdown. */
void func_8006FAC8(void)
{
    s32 screen_position;
    s32 remaining;
    u8 *actor = D_800D9344;

    func_8006CDDC();
    gte_stsxy(&screen_position);
    func_8006CC4C(actor, D_801399B0);
    func_80066F9C(actor, screen_position, 8, 0x2E, 0);
    remaining = D_801B2444 - 1;
    D_801B2444 = remaining;
    if (remaining == 0)
    {
        D_801B2440++;
    }
}
