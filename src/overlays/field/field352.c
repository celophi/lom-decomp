#include "common.h"

extern s32 D_80117EE0;
extern s32 D_80117EE4;
extern s32 D_80117EE8;
extern s32 D_80117EEC;
extern s32 D_80117EF0;
extern s32 D_80119EF8;
extern s32 D_8011F300;
extern s32 D_8011F308;
extern s32 D_8011F320;
extern s32 D_8011F324;
extern s32 D_8011F328;

void *func_800A4348(s32 arg0, void *arg1);

/**
 * @brief Kick off a guarded CD streaming read when the channel is idle.
 *
 * When the busy flag @c D_80117EE4 is clear, resets the streaming state block
 * and issues a queued CD read for resource index @p arg0 + 0x17, latching the
 * completion callback func_800A4348 and its queue handle in @c D_80117EF0.
 *
 * @param arg0 Base resource index; the read uses arg0 + 0x17.
 *
 * @see decomp.me (100%) TODO
 */
void func_800A3F18(s32 arg0)
{
    if (D_80117EE4 == 0)
    {
        D_80117EE0 = 0;
        D_80117EEC = 0;
        D_8011F324 = 0;
        D_8011F328 = 0;
        D_8011F308 = 0;
        D_80117EF0 = 0;
        D_80119EF8 = 0;
        D_80117EE8 = 0;
        D_8011F320 = 0;
        D_8011F300 = 0;
        D_80117EE4 = arg0 + 0x17;
        D_80117EF0 = cdrom_queue_read_with_callback((u16)D_80117EE4, &func_800A4348);
    }
}
