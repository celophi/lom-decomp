#include "common.h"

extern s32 D_80122B74;
extern s32 D_80122B78;

typedef struct
{
    u8 pad0[0x28];
    u32 unk28;
} StructC1B60;

StructC1B60 *func_800C1B60(s32 arg0);

/**
 * @brief Decode a packed slot value and emit its two sub-fields.
 *
 * Extracts the 16-bit field @c (arg1>>16), splits its low 12 bits into
 * @c v>>5 (written to @p arg2) and @c v&0x1F (written to @p arg3), then
 * selects a return record based on bits 12-14 of the field: for values below
 * 3 it returns @c D_80122B74+0xE4; otherwise @c D_80122B78, additionally
 * folding a per-slot adjustment into @p arg2 when bit 15 is set.
 *
 * @param arg0 Slot handle passed through to func_800C1B60.
 * @param arg1 Packed value; the slot descriptor is its high 16 bits.
 * @param arg2 Out: primary sub-field (updated again on the bit-15 path).
 * @param arg3 Out: secondary sub-field (low 5 bits).
 * @return Selected record pointer/value, or 0 when bit 14 of the field is set.
 * @note 84.21% match (gcc280_g0). Residue is a coupled sched1 emit-order tie:
 *       `arg1>>16` will not schedule ahead of the arg2->s1 parameter-save copy
 *       (a LUID tie-break). Same mechanism as sibling func_800BD3B0.
 */
s32 func_800BD318(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3)
{
    s32 a;
    u32 v;
    s32 result;

    a = arg1 >> 16;
    v = ((unsigned short)a) & 0xFFF;
    *arg2 = v >> 5;
    *arg3 = v & 0x1F;
    if ((((u32)a >> 12) & 7) < 3)
    {
        result = D_80122B74 + 0xE4;
    }
    else
    {
        result = D_80122B78;
        if (a & 0x8000)
        {
            *arg2 += (func_800C1B60(arg0)->unk28 >> 9) & 0x7F;
        }
    }
    return result;
}
