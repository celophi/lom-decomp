#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "common.h"

extern s32 D_8013924C;
extern s32 D_8013B208;
extern s32 D_801ADAF4;
extern s32 D_80139234;
extern s32 D_801B3220;
extern s32 func_800C0474(s32);
extern s32 func_800C064C(s32);
extern void func_800C0150(void);
extern void func_800C018C(void);

void func_800C00C4(void)
{
    D_8013B208 = 1;
    func_800652A8(0x3B, 0x80);
    func_8006683C(0x703040);
    D_801ADAF4 = 0xA;
    D_80139234 = 1;
    D_8013924C = 1;
    func_8006CAC0(func_800C0474);
    func_8006CAC0(func_800C064C);
    D_801B3220++;
    func_800C0150();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800C0150(void)
{
    if (D_80139234 == 0)
    {
        D_801B3220 += 1;
        func_800C018C();
    }
}
