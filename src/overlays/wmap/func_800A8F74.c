#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];

/** @brief Queue the effect's animation and texture resources. */
void func_800A8F74(void)
{
    func_800A8AA8(0x1125);
    func_800A8AF0(0x1126);
    func_800A8B38(0x1127);
    cdrom_queue_read(0x1129, D_8011D538);
    cdrom_queue_read(0x1128, D_8011F538);
    cdrom_queue_read(0x112A, D_80121538);
    cdrom_queue_read(0x112B, D_800DCF18);
}
