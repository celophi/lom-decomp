#include "common.h"
#include "cdrom.h"

extern void func_80064F64(s32);
extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800E4F18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A8D6C(void)
{
    func_80064F64(0x1111);
    func_800A8AA8(0x110A);
    func_800A8AF0(0x110B);
    func_800A8B38(0x110C);
    D_8011CF1C = D_800E4F18;
    D_8011CF24 = D_8011CF1C + 0xC000;
    cdrom_queue_read(0x1108, D_8011D538);
    cdrom_queue_read(0x1109, D_8011F538);
    cdrom_queue_read(0x110D, D_8011CF1C);
    cdrom_queue_read(0x110E, D_8011CF24);
    cdrom_queue_read(0x110F, D_8011CF24 + 0x4000);
    cdrom_queue_read(0x1110, D_8011CF24 + 0x5000);
    cdrom_queue_read(0x1112, D_8011CF24 + 0x6000);
}
