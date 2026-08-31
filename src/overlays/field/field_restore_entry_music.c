#include "common.h"
#include "cd_resources.h"
#include "cdrom.h"
#include "akao.h"
#include "game_state.h"
#include "sdk/memory.h"

#define EFFECT_BLOB_BASE 0x80180000
#define EFFECT_BLOB_OFFSETS 0x80180004

extern AkaoHeader *D_8011F304;

/** @brief Restore the shared effect bank when entering FIELD from a state that did not preserve it. */
/** @see decomp.me (100.00%) */
void field_restore_entry_music(void)
{
    u8 *base;
    u32 *off;

    if (((u32)(g_previousGameState - 2) >= 2U) && (g_previousGameState != 0) &&
        (g_previousGameState != 6) && (g_previousGameState != 7) && (g_previousGameState != 5))
    {
        D_8011F304 = (AkaoHeader *)0x8013C000;
        cdrom_queue_read(CD_RES_SOUND_EFFECT_SET, (void *)EFFECT_BLOB_BASE);
        cdrom_wait_queue_empty();

        base = (u8 *)EFFECT_BLOB_BASE;
        off = (u32 *)EFFECT_BLOB_OFFSETS;
        bcopy(base + off[0], (u8 *)D_8011F304, (s32)(off[1] - off[0]));
        akao_register_bank(D_8011F304);
        akao_upload_bank_blocking((AkaoBankHeader *)(base + off[1]), 1);
    }
}
