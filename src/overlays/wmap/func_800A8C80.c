#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800DEF18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A8C80(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x1197);
    func_800A8AF0(0x1198);
    func_800A8B38(0x1199);
    cdrom_queue_read(0x119A, D_8011D538);
    cdrom_queue_read(0x119B, D_8011F538);
    cdrom_queue_read(0x119C, D_80121538);
    cdrom_queue_read(0x119D, D_800DCF18);
    cdrom_queue_read(0x119E, D_8011CF1C);
    cdrom_queue_read(0x119F, D_8011CF24);
}
