#include "wmap_sequence_runtime.h"
#include "common.h"
#include "cdrom.h"

/** @brief Four-word world-map transform state. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
    s32 pad;
    } WmapTransform;

    extern WmapTransform D_800DCEC8;
    extern WmapTransform D_80139950;
    extern s32 D_800DCEF8;
    extern s32 D_800DCF00;
    extern s32 D_801398D0;
    extern s32 D_8013B208;
    extern s32 D_80182D68;
    extern s32 D_80182D78;
    extern s32 D_801B2E48;
    extern void func_800A7C78(void);
    extern u8 D_800DCA98[];
    extern void func_80064F64(s32);
    extern void func_80058FF4(s32, s32, s32);

    /** @brief Load resources and set the effect's map-relative position. */
    void func_800A6A20(void)
    {
        D_800DCEC8 = D_80139950;
        D_8013B208 = 1;
        func_8006D0F0(12, &D_800DCEF8, &D_800DCF00);
        cdrom_queue_read(0x10E2, D_800DCA98);
        func_80064F64(0x10E3);
        func_80058FF4(3, D_800DCEF8, D_800DCF00);
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 48) - D_80139950.x;
        D_80182D78 = ((D_800DCF00 - 1) * 48) - D_80139950.y;
        D_801B2E48++;
        func_800A7C78();
    }
