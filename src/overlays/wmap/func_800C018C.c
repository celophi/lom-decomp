#include "wmap_sequence_runtime.h"
/* Partial WMAP decompilation: 96.000000% (gcc280_g0). */
#include "common.h"

typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

extern s32 D_80054A18[];
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182DD8;
extern s32 D_801B3220;
extern void func_800C0254(void);

void func_800C018C(void)
{
    func_8006D0F0(D_80054A18[D_80182DD8 - 4], &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    D_801B3220++;
    func_800C0254();
}
