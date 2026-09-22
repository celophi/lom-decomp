#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern u8 D_800DCF18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];

/** @brief Queue the effect's animation and texture resources. */
void func_800A9358(void)
{
    func_800A8AA8(0x1155);
    func_800A8AF0(0x1156);
    cdrom_queue_read(0x1158, D_8011D538);
    cdrom_queue_read(0x1159, D_8011F538);
    cdrom_queue_read(0x1157, D_800DCF18);
}
