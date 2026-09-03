#include "common.h"

/**
 * @brief Partial FIELD state used by the frame/update dispatcher.
 */
typedef struct
{
    u8 pad0[0xBC];
    s32 unkBC;
    u8 padC0[0x400 - 0xC0];
    s32 unk400;
    u8 pad404[0x418 - 0x404];
    s32 unk418;
} FieldStateB19FC;

extern FieldStateB19FC *D_80122B78;

/**
 * @brief Dispatch the current FIELD update path and advance its frame counter.
 *
 * A negative state value selects the reset path. Otherwise bit 30 optionally
 * runs an auxiliary update before the normal update chain. Bit 16 of unk400
 * gates an additional handler, and the per-state frame counter is incremented.
 *
 * @note 100% match with the FIELD GCC 2.8.0 G0 toolchain.
 */
void func_800B19FC(void)
{
    s32 temp_v0;

    temp_v0 = D_80122B78->unk418;
    if (temp_v0 < 0)
    {
        func_800B1AA8();
        return;
    }
    if (((u32)temp_v0 >> 30) & 1)
    {
        func_800B1BBC();
    }
    func_800B1D10();
    func_800B1F10();
    func_800B20B4();
    if (D_80122B78->unk400 & 0x10000)
    {
        func_800B49C0();
    }
    D_80122B78->unkBC++;
}
