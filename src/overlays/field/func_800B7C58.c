#include "common.h"

/** @brief Per-index field record; only the u16 at +0x24 is used here. */
typedef struct
{
    u8 pad0[0x24];
    u16 unk24;
    u8 pad26[0x22A];
} RecB7C58;

/** @brief Field state block holding the record array at +0x5F0. */
typedef struct
{
    u8 pad0[0x5F0];
    RecB7C58 unk5F0[1];
} StructB7C58;

/** @brief Output record initialized by func_800B7C58. */
typedef struct
{
    s32 unk0;
    s32 unk4;
    u32 unk8_low : 24;
    u32 unk8_high : 8;
    s32 unkC;
} OutB7C58;

extern StructB7C58 *D_80122B74;

void func_800B7B98(RecB7C58 *rec);
OutB7C58 *func_80087F0C(s32 index);

/**
 * @brief Initialize an output record from the indexed field record.
 * @param index Field record index.
 */
void func_800B7C58(s32 index)
{
    OutB7C58 *out;

    func_800B7B98(&D_80122B74->unk5F0[index]);
    out = func_80087F0C(index);
    out->unk0 = D_80122B74->unk5F0[index].unk24;
    if (out->unk0 == 0)
    {
        out->unk0 = 1;
    }
    out->unk4 = D_80122B74->unk5F0[index].unk24;
    out->unk8_low = D_80122B74->unk5F0[index].unk24;
    out->unkC = 0;
}
