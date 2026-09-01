#include "common.h"

extern u8 D_800FE3A0[];
extern u8 D_801226E0[];
extern u8 D_801227D0;
extern u8 D_801228D0[];
extern u8 D_801228E0[];
extern void akao_stop_sfx_by_id(s32 id);

/**
 * @brief Restores per-part bytes saved for each active field actor.
 *
 * Stops sound effect 0x7E, then walks the active actor-index table and copies
 * the saved bytes back to offsets 0x2E and 0x33 of each 0x48-byte part record.
 *
 * @note gcc272_cdk, 100% match.
 */
void func_800A90F8(void)
{
    s32 i;
    u8 *rec;

    akao_stop_sfx_by_id(0x7E);
    for (i = 0; i < D_801227D0; i++) {
        rec = D_800FE3A0 + D_801226E0[i] * 0x48;
        rec[0x2E] = D_801228D0[i];
        rec[0x33] = D_801228E0[i];
    }
}
