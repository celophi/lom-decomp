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
extern u8 D_80123538[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;

/** @brief Set effect resource buffers and queue animation and texture reads. */
void func_800A9D24(void)
{
    D_8011CF1C = D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x11C3);
    func_800A8AF0(0x11C4);
    func_800A8B38(0x11C5);
    cdrom_queue_read(0x11C7, D_8011D538);
    cdrom_queue_read(0x11C6, D_8011F538);
    cdrom_queue_read(0x11C8, D_80121538);
    cdrom_queue_read(0x11C9, D_80123538);
    cdrom_queue_read(0x11CA, D_800DCF18);
    cdrom_queue_read(0x11CB, D_8011CF1C);
}
