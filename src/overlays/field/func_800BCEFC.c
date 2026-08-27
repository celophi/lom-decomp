#include "common.h"

/**
 * @brief Field state header; only its leading byte is referenced here.
 */
typedef struct StateBCEFC
{
    u8 unk0;
} StateBCEFC;

extern StateBCEFC *D_80123FB8;
void func_800B28E0(s32, s32, s32);
void func_800B286C(s32, s32, s32);

/**
 * @brief Dispatches to one of two field handlers based on a selector.
 *
 * If @p p1 is the 0xFF sentinel it is replaced by the active state's leading
 * byte. Selector 0 forwards to func_800B28E0 and selector 1 to func_800B286C,
 * each with the resolved value and the low bytes of @p p2 and @p p3.
 *
 * WIP: 95.56%. The function body is byte-identical (25/27 rows); the only
 * residual is the epilogue delay-slot fill - the target parks the `addiu sp`
 * stack restore in the `jr` delay slot (two leading nops) while this build
 * emits it ahead of the `jr`. The permuter cannot beat this either, so it is a
 * dbr/maspsx scheduling artifact rather than a source-reachable difference.
 */
void func_800BCEFC(s32 p0, s32 p1, s32 p2, s32 p3)
{
    if (p1 == 0xFF)
    {
        p1 = D_80123FB8->unk0;
    }
    switch (p0)
    {
    case 0:
        func_800B28E0(p1, p2 & 0xFF, p3 & 0xFF);
        break;
    case 1:
        func_800B286C(p1, p2 & 0xFF, p3 & 0xFF);
        break;
    }
}
