#include "decomp4.h"

/**
 * @brief Submits an AKAO sequence to the audio driver and spins until accepted.
 *
 * Clears bit 0 of the driver status word, then repeatedly calls akao_submit
 * until it stops returning the "busy" sentinel. Used to hand a freshly-loaded
 * AKAO sequence (BGM or SFX program) to the driver and block until the SPU
 * transfer window opens and the data is consumed.
 *
 * @param sequenceData       Pointer to an AKAO-tagged sequence buffer in main RAM.
 * @param waitForCompletion  When non-zero, akao_submit further blocks inside the
 *                           driver until the SPU DMA completes.
 *
 * @see decomp.me: (100%) https://decomp.me/scratch/Mz7yX
 */
void akao_play_sequence_blocking(AkaoSeqHeader* sequenceData, s32 waitForCompletion)
{
    D_8004F750 &= ~1;
    while (akao_submit(sequenceData, waitForCompletion) == 1);
}