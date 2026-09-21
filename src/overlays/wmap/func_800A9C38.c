#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32 resource_index);
extern void func_800A8AF0(s32 resource_index);
extern void func_800A8B38(s32 resource_index);
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 D_800DEF18;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern u8 D_800DCF18;

/** @brief World-map step handler: kick off the batch of resource reads for this map. */
void func_800A9C38(void)
{
    D_8011CF1C = &D_800DEF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x11BA);
    func_800A8AF0(0x11BB);
    func_800A8B38(0x11BC);
    cdrom_queue_read(0x11BD, &D_8011D538);
    cdrom_queue_read(0x11BE, &D_8011F538);
    cdrom_queue_read(0x11BF, &D_80121538);
    cdrom_queue_read(0x11C0, &D_800DCF18);
    cdrom_queue_read(0x11C1, D_8011CF1C);
    cdrom_queue_read(0x11C2, D_8011CF24);
}
