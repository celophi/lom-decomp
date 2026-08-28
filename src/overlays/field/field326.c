#include "common.h"

extern s32 D_8011F41C;
extern s32 D_8012269C;
extern s32 D_801227F0;
extern s32 g_gosub_result_count;

/**
 * @see decomp.me (100%) TODO
 */
void func_800AAFEC(void *arg0)
{
    if (D_8012269C == 0)
    {
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        func_80084240();
        cdrom_stream(0xD, (void *)0x80140000);
        cdrom_wait_queue_empty();
        D_8012269C = 2;
        D_8011F41C = 2;
        func_80140080((void *)0x80175000, arg0);
    }
}
