#include "wmap_sequence_runtime.h"
#include "common.h"

typedef struct { s32 w[4]; } WmapBlk16;

extern void cdrom_queue_read(s32 sector, void* dst);
extern void func_80064F64(s32 id);
extern void func_80058FF4(s32 a0, s32 a1, s32 a2);
extern void func_800A8060(void);
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCEC8[];
extern s32 D_80139950[];
extern u8 D_800DCA98[];
extern s32 D_8013B208;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2E50;

/**
 * @brief World-map step handler: kick off the streamed cell load and seed the scroll
 *        target from the current cell, then advance the step.
 */
void func_800A6C24(void)
{
    *(WmapBlk16*)D_800DCEC8 = *(WmapBlk16*)D_80139950;
    D_8013B208 = 1;
    func_8006D0F0(1, &D_800DCEF8, &D_800DCF00);
    cdrom_queue_read(0x10E0, D_800DCA98);
    func_80064F64(0x10E1);
    func_80058FF4(3, D_800DCEF8, D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = (D_800DCEF8 - 1) * 0x30 - D_80139950[0];
    D_80182D78 = (D_800DCF00 - 1) * 0x30 - D_80139950[1];
    D_801B2E50 += 1;
    func_800A8060();
}
