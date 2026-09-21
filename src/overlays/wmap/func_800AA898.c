#include "common.h"

#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern u8 D_800DCF18[];
extern u8 D_8011D538[];
extern u8 D_8011F538[];

/** @brief Queue the resource set used by this world-map sequence. */
void func_800AA898(void)
{
    func_800A8AA8(0x1246);
    func_800A8AF0(0x1247);
    cdrom_queue_read(0x1248, &D_8011D538);
    cdrom_queue_read(0x1249, &D_8011F538);
    cdrom_queue_read(0x124A, &D_800DCF18);
}
