#include "common.h"

extern s32 D_8012269C;
extern s32 D_8012299C;
extern s32 D_801227C4;
extern s32 D_801227F0;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values[];

/**
 * @see decomp.me (100%) TODO
 */
void func_800AD030(s32 arg0)
{
    if (D_8012269C == 0)
    {
        func_80084240();
        cdrom_stream(0xC, (void *)0x80140000);
        cdrom_wait_queue_empty();
        D_8012299C = arg0 + 1;
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        D_8012269C = 3;
        D_801227C4 = g_gosub_result_values[0];
        func_8014024C((void *)0x80170000, arg0);
    }
}
