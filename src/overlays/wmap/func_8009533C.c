#include "common.h"

/** @brief Partially identified world-map resource fields. */
typedef struct
{
    u8 unknown_0[0x22];
    s16 value_22;
    u8 unknown_24[2];
    s16 value_26;
} WmapResourceFields;

extern WmapResourceFields D_800D9370;
extern s32 D_801B2B68;
extern s32 D_801B2B6C;
extern void func_8009538C(void);

/** @brief Initialize resource fields, begin a 16-tick delay, and run the next step. */
void func_8009533C(void)
{
    D_800D9370.value_22 = 2;
    D_800D9370.value_26 = 8;
    D_801B2B6C = 16;
    D_801B2B68 += 1;
    func_8009538C();
}
