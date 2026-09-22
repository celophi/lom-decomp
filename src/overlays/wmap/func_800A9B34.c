#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800E0F18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 *D_8011CF28;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9B34(void)
{
    D_8011CF1C = D_800E0F18;
    D_8011CF24 = D_8011CF1C + 0x4000;
    D_8011CF28 = D_8011CF24 + 0xC800;
    func_800A8AA8(0x11A7);
    func_800A8AF0(0x11A8);
    func_800A8B38(0x11A9);
    cdrom_queue_read(0x11AA, D_8011D538);
    cdrom_queue_read(0x11AB, D_8011F538);
    cdrom_queue_read(0x11AC, D_80121538);
    cdrom_queue_read(0x11AD, D_800DCF18);
    cdrom_queue_read(0x11AE, D_8011CF1C);
    cdrom_queue_read(0x11AF, D_8011CF24);
}
