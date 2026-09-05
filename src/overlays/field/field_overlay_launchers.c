#include "common.h"

/*
 * Sub-overlay launchers. Each one streams an overlay from CD to 0x80140000,
 * waits for the transfer, then calls into the freshly loaded overlay with a
 * work-buffer address and the caller's arguments. func_80084240 brackets the
 * load on both sides where the overlay returns synchronously.
 */

extern u8 *g_pad_ctx;
extern s32 D_8011F41C;
extern s32 D_8012269C;
extern s32 D_801227F0;
extern s32 D_801229AC;
extern s32 g_gosub_result_count;

/**
 * @brief Load overlay 5 and run its entry point with five arguments.
 * @param arg0 Forwarded to the overlay entry point.
 * @param arg1 Forwarded to the overlay entry point.
 * @param arg2 Forwarded to the overlay entry point.
 * @param arg3 Forwarded to the overlay entry point.
 * @param arg4 Forwarded to the overlay entry point.
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

/**
 * @brief Load overlay 8 and run its entry point at 0x80140E00.
 * @param arg0 Forwarded to the overlay entry point.
 */
void func_800AAF9C(s32 arg0)
{
    func_80084240();
    cdrom_stream(8, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140E00((void *)0x80160000, arg0);
    func_80084240();
}

/**
 * @brief Load overlay 0xD and hand it a parameter block, unless a sub-overlay is already active.
 * @param arg0 Parameter block forwarded to the overlay entry point at 0x80140080.
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

/**
 * @brief Load overlay 7 in mode 0 when any record or counter is populated, else call func_800AB638.
 * @param arg0 Forwarded as the overlay entry point's last argument.
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

/**
 * @brief Load overlay 7 in mode 1 with four arguments, unless a sub-overlay is already active.
 * @param arg0 Forwarded to the overlay entry point.
 * @param arg1 Forwarded to the overlay entry point.
 * @param arg2 Forwarded to the overlay entry point.
 * @param arg3 Forwarded to the overlay entry point.
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
