#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 D_80123538[];
extern u8 D_80125538[];
extern u8 D_80127538[];

/** @brief Queue the effect's animation and texture resources. */
void func_800A8E9C(void)
{
    func_800A8AA8(0x111B);
    func_800A8AF0(0x111C);
    func_800A8B38(0x111D);
    cdrom_queue_read(0x111E, D_8011D538);
    cdrom_queue_read(0x111F, D_8011F538);
    cdrom_queue_read(0x1120, D_80121538);
    cdrom_queue_read(0x1121, D_80123538);
    cdrom_queue_read(0x1123, D_80125538);
    cdrom_queue_read(0x1124, D_80127538);
    cdrom_queue_read(0x1122, D_800DCF18);
}
