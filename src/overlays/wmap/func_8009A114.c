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
extern s32 D_8011CF50;
extern s32 D_800DCEC0;
extern s32 D_801B2C48;
extern u8 D_8011D538[];
extern void func_8005FF88(s32);
extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern s32 D_801398D0;
extern s32 D_8013B208;
extern s32 D_8013B288;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_801B2C54;
extern s32 func_8006D0F0(s32, s32 *, s32 *);
extern void func_8009ABB0(void);

/** @brief Save the projection state and set the next effect's map-relative position. */
void func_8009A114(void)
{
    D_8013B288 = 0;
    D_8011CF50 = 1;
    D_801B2C48 = 0;
    D_8013B208 = 1;
    func_8005FF88(-1);
    D_800DCEC0 = 0;
    D_800DCEC8 = D_80139950;
    cdrom_queue_read(0x1145, D_8011D538);
    cdrom_queue_read(0x1146, D_8011D538 + 0x2000);
    func_800A8AA8(0x1147);
    func_800A8AF0(0x1148);
    func_8006D0F0(24, &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 48) - D_80139950.x;
    D_80182D78 = ((D_800DCF00 - 1) * 48) - D_80139950.y;
    D_801B2C54++;
    func_8009ABB0();
}
