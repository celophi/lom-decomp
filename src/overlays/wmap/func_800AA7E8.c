#include "common.h"
#include "cdrom.h"

extern void func_800A8AA8(s32);
extern void func_800A8AF0(s32);
extern void func_800A8B38(s32);
extern u8 D_800DCF18[];
extern u8 D_80121538[];
extern u8 D_80123538[];
extern u8 D_80125538[];
extern u8 D_80127538[];

/** @brief Queue the resource set used by this world-map sequence. */
void func_800AA7E8(void)
{
    func_800A8AA8(0x123E);
    func_800A8AF0(0x123F);
    func_800A8B38(0x1240);
    cdrom_queue_read(0x1241, &D_80125538);
    cdrom_queue_read(0x1242, &D_80127538);
    cdrom_queue_read(0x1243, &D_80123538);
    cdrom_queue_read(0x1244, &D_80121538);
    cdrom_queue_read(0x1245, &D_800DCF18);
}
