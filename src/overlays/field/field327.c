#include "common.h"

extern s32 D_8012269C;
extern s32 D_801229AC;

/**
 * @see decomp.me (100%) TODO
 */
void func_800AB170(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    if (D_8012269C == 0)
    {
        func_80084240();
        cdrom_stream(0x7, (void *)0x80140000);
        cdrom_wait_queue_empty();
        D_801229AC = 1;
        D_8012269C = 1;
        func_80140004((void *)0x80150000, 1, arg0, arg1, arg2, arg3);
    }
}
