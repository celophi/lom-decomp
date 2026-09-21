#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32 resource_index);
extern void func_800A8AF0(s32 resource_index);
extern void func_800A8B38(s32 resource_index);
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 D_800ECF18;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern u8 D_800DCF18;

/** @brief World-map step handler: kick off the batch of resource reads for this map. */
void func_800A93D4(void)
{
    D_8011CF1C = &D_800ECF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x115A);
    func_800A8AF0(0x115B);
    func_800A8B38(0x115C);
    cdrom_queue_read(0x115D, &D_8011D538);
    cdrom_queue_read(0x115E, &D_8011F538);
    cdrom_queue_read(0x115F, &D_80121538);
    cdrom_queue_read(0x1160, &D_800DCF18);
    cdrom_queue_read(0x1161, D_8011CF24);
    cdrom_queue_read(0x1162, D_8011CF1C);
}
