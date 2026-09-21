#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_800ECF18[];
extern u8 *D_8011CF1C;
extern u8 *D_8011CF24;
extern u8 D_8011D538[];
extern u8 D_8011F538[];
extern u8 D_80121538[];
extern u8 D_80123538[];
extern u8 D_80125538[];
extern u8 D_80127538[];

/** @brief Queue the world-map effect resource set. */
void func_800A94C0(void)
{
    D_8011CF1C = D_800ECF18;
    D_8011CF24 = D_8011CF1C + 0x2000;
    func_800A8AA8(0x1163);
    func_800A8AF0(0x1164);
    func_800A8B38(0x1165);
    cdrom_queue_read(0x1168, D_8011D538);
    cdrom_queue_read(0x1169, D_8011F538);
    cdrom_queue_read(0x116A, D_80121538);
    cdrom_queue_read(0x116B, D_80123538);
    cdrom_queue_read(0x116C, D_80125538);
    cdrom_queue_read(0x116D, D_80127538);
    cdrom_queue_read(0x1166, D_800DCF18);
    cdrom_queue_read(0x1167, D_8011CF1C);
}
