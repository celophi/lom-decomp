#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800E1F18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 D_80123538[];

/** @brief Queue the world-map effect resource set. */
void func_800A8B80(void)
{
    D_8011CF1C = D_800E1F18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x11B0);
    func_800A8AF0(0x11B1);
    func_800A8B38(0x11B2);
    cdrom_queue_read(0x11B3, D_8011F538);
    cdrom_queue_read(0x11B4, D_8011D538);
    cdrom_queue_read(0x11B5, D_80121538);
    cdrom_queue_read(0x11B6, D_80123538);
    cdrom_queue_read(0x11B7, D_8011CF24);
    cdrom_queue_read(0x11B8, D_8011CF1C);
    cdrom_queue_read(0x11B9, D_800DCF18);
}
