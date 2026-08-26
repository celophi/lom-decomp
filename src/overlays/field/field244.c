#include "common.h"

void func_800AAF9C(s32 arg0)
{
    func_80084240();
    cdrom_stream(8, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140E00((void *)0x80160000, arg0);
    func_80084240();
}
