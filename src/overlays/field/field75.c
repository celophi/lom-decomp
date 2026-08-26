#include "common.h"

void func_800AD0C8(void)
{
    func_80084240();
    cdrom_stream(9, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140024((void *)0x80150000, 0);
    func_800C3BB0();
    func_80084240();
}

void func_800AD118(void)
{
}
