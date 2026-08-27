#include "common.h"

extern s32 D_8010AE58;
extern s32 D_8010AE60;
extern s32 D_8010AE68;
extern s32 D_8010AE6C;
extern s32 D_8010AE70;
extern s32 D_8010AE74;
extern s32 D_8010AE7C;
extern s32 D_8010AE80;
extern s32 D_8010CFD8;
extern s32 D_8010CFDC;
extern s16 D_801ED400;

/**
 * @brief Clears field camera state and snapshots the current scratchpad value.
 *
 * @note 99.565% match. The only residue is the address register used to load
 *       `D_801ED400`; all 23 instructions and 92 bytes are otherwise exact.
 */
void func_80092394(void)
{
    s32 value;

    do
    {
        value = D_801ED400;
    } while (0);

    D_8010AE6C = 0;
    D_8010AE60 = 0;
    D_8010AE58 = 0;
    D_8010AE80 = 0;
    D_8010AE7C = 0;
    D_8010CFDC = 0;
    D_8010CFD8 = 0;
    D_8010AE74 = 0;
    D_8010AE68 = value;
    D_8010AE70 = value;
}
