#include "common.h"

/**
 * @see decomp.me (100%) TODO
 */
void func_800AAF00(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4)
{
    func_80084240();
    cdrom_stream(0x5, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140004((void *)0x80160000, arg0, arg1, arg2, arg3, arg4, 0);
    field_text_reset_windows();
    func_80084240();
}
