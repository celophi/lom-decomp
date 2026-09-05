#include "common.h"

extern u8 *g_pad_ctx;
extern s32 D_8012269C;
extern s32 D_801229AC;

/**
 * @see decomp.me (100%) TODO
 */
void func_800AB070(s32 arg0)
{
    s32 count;
    s32 i;

    if (D_8012269C == 0)
    {
        count = 0;
        for (i = 0; i < 0x64; i++)
        {
            if (g_pad_ctx[0xCE0] != 0)
            {
                count++;
                break;
            }
        }
        for (i = 0; i < 0x100; i++)
        {
            if ((g_pad_ctx + i)[0x25E0] != 0)
            {
                count++;
                break;
            }
        }
        if (count == 0)
        {
            func_800AB638(0);
        }
        else
        {
            func_80084240();
            cdrom_stream(0x7, (void *)0x80140000);
            cdrom_wait_queue_empty();
            D_801229AC = 1;
            D_8012269C = 1;
            func_80140004((void *)0x80150000, 0, 0, 0, 0, arg0);
        }
    }
}
