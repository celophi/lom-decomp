#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013B208;
extern s32 D_8013B20C;
extern s32 D_801B29B0;
extern s32 D_801B29B4;
extern void func_8008BCC8(void);

/** @brief Set two flags, play sound 35, and register a callback before a four-tick delay. */
void func_8008B494(void)
{
    D_8013B20C = 1;
    D_8013B208 = 1;
    func_800652A8(0x23, 0x80);
    func_8006CAC0(&func_8008BCC8);
    D_801B29B4 = 4;
    D_801B29B0 += 1;
}
