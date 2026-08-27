#include "common.h"

extern u8 *D_80123FB8;

void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Retriggers the active field song and advances its record timer.
 *
 * Issues an akao_set_song_params command (0x8001) using the byte at
 * @c D_80123FB8[0], then re-reads @c D_80123FB8, selects the record at its
 * stored index (field 0x4, stride 12), and increments that record's 0x8 timer.
 */
void func_800BA368(void)
{
    u8 *rec = D_80123FB8;
    u8 *rec2;

    akao_set_song_params(0x8001, 1, rec[0], 0x24);

    rec2 = D_80123FB8;
    rec2 += *(s32 *)(rec2 + 4) * 12;
    *(s32 *)(rec2 + 8) += 1;
}
