#include "common.h"

/** @brief Resource record with the field cleared by this sequence step. */
typedef struct
{
    u8 unknown_0[0x22];
    s16 value;
    u8 unknown_24[8];
} WmapResourceValue;

extern WmapResourceValue D_800D9268[];
extern s32 D_801B2E40;
extern s32 D_801B2E44;
extern void func_800A7AA0(void);

/** @brief Clear four resource fields and start a 64-tick sequence step. */
void func_800A7A40(void)
{
    s32 index;
    for (index = 0; index < 4; index++)
    {
        D_800D9268[index + 20].value = 0;
    }
    D_801B2E44 = 64;
    D_801B2E40 += 1;
    func_800A7AA0();
}
