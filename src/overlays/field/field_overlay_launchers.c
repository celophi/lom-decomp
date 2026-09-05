#include "common.h"
#include "cd_resources.h"

/*
 * Sub-overlay launchers.
 *
 * Each launcher streams one BIN overlay from CD to 0x80140000, waits for the
 * transfer, then calls the overlay's entry point with a work-buffer address
 * and the caller's arguments. The entry points are fixed addresses inside the
 * loaded overlay, so they are reached through address-named symbols here:
 * func_80140004 is gname_run when GNAME is resident and shop_init when SHOP
 * is, and func_80140080 is gosub_open_screen_sequence. func_80084240 brackets
 * the load on both sides where the overlay returns synchronously.
 *
 * D_8012269C is nonzero while a sub-overlay or modal transition owns the
 * screen; the value identifies which one (1 shop, 2 gosub, 4 resource 0x11,
 * 6 the fade started by func_800AB638).
 */

/** @brief PadContext, see main.h. Only the equipment records at 0xCE0 and the item counts at 0x25E0 are read here. */
extern u8 *g_pad_ctx;
extern s32 D_8011F41C;
extern s32 D_8012269C;
extern s32 D_801227F0;
extern s32 D_801229AC;
extern s32 g_gosub_result_count;

/**
 * @brief Load GNAME.BIN and run the name-entry screen.
 *
 * Argument order follows gname_run, which receives the 0x80160000 render
 * buffers first and allow_empty_cancel = 0 last.
 *
 * @param initial_name Name shown when the screen opens.
 * @param active_name Name buffer edited by the UI.
 * @param source_mode Random-name source selector.
 * @param history_index History-list entry selector.
 * @param custom_name Custom random-name source.
 * @see decomp.me (100%) TODO
 */
void field_run_name_entry(s32 initial_name, s32 active_name, s32 source_mode, s32 history_index, s32 custom_name)
{
    func_80084240();
    cdrom_stream(CD_RES_GNAME_BIN, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140004((void *)0x80160000, initial_name, active_name, source_mode, history_index, custom_name, 0);
    field_text_reset_windows();
    func_80084240();
}

/**
 * @brief Load ZUKAN.BIN and run its entry point at 0x80140E00.
 * @param arg0 Forwarded to the ZUKAN entry point; meaning not yet established.
 */
void field_run_zukan(s32 arg0)
{
    func_80084240();
    cdrom_stream(CD_RES_ZUKAN_BIN, (void *)0x80140000);
    cdrom_wait_queue_empty();
    func_80140E00((void *)0x80160000, arg0);
    func_80084240();
}

/**
 * @brief Load GOSUB.BIN and open a screen sequence, unless a sub-overlay is already active.
 *
 * Sets D_8012269C to 2 for the duration and clears the gosub result count.
 *
 * @param screen_sequence Terminated s32 array passed to gosub_open_screen_sequence.
 * @see decomp.me (100%) TODO
 */
void field_open_gosub_screen_sequence(void *screen_sequence)
{
    if (D_8012269C == 0)
    {
        D_801227F0 = 1;
        g_gosub_result_count = 0;
        func_80084240();
        cdrom_stream(CD_RES_GOSUB_BIN, (void *)0x80140000);
        cdrom_wait_queue_empty();
        D_8012269C = 2;
        D_8011F41C = 2;
        func_80140080((void *)0x80175000, screen_sequence);
    }
}

/**
 * @brief Load SHOP.BIN in mode 0 when the party holds anything, else start the func_800AB638 fade.
 *
 * Scans the equipment records at 0xCE0 and the item counts at 0x25E0 of the
 * pad context. With nothing held, hands 0 to func_800AB638 instead of
 * opening the shop.
 *
 * @param arg0 Forwarded as shop_init's last argument; meaning not yet established.
 * @see decomp.me (100%) TODO
 */
void field_open_shop_mode_0(s32 arg0)
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
            cdrom_stream(CD_RES_SHOP_BIN, (void *)0x80140000);
            cdrom_wait_queue_empty();
            D_801229AC = 1;
            D_8012269C = 1;
            func_80140004((void *)0x80150000, 0, 0, 0, 0, arg0);
        }
    }
}

/**
 * @brief Load SHOP.BIN in mode 1 with four arguments, unless a sub-overlay is already active.
 * @param arg0 Forwarded as shop_init's second argument; meaning not yet established.
 * @param arg1 Forwarded as shop_init's third argument.
 * @param arg2 Forwarded as shop_init's fourth argument.
 * @param arg3 Forwarded as shop_init's fifth argument.
 * @see decomp.me (100%) TODO
 */
void field_open_shop_mode_1(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    if (D_8012269C == 0)
    {
        func_80084240();
        cdrom_stream(CD_RES_SHOP_BIN, (void *)0x80140000);
        cdrom_wait_queue_empty();
        D_801229AC = 1;
        D_8012269C = 1;
        func_80140004((void *)0x80150000, 1, arg0, arg1, arg2, arg3);
    }
}
