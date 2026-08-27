#include "common.h"

/**
 * @brief Field state header; only its leading byte is referenced here.
 */
typedef struct StateBCC74
{
    u8 unk0;
} StateBCC74;

extern StateBCC74 *D_80123FB8;
void func_800B0710(s32, s32, s32, s32);

/**
 * @brief Forwards four values to func_800B0710, resolving 0xFF sentinels.
 *
 * A first argument of 0xFF is replaced by the active state's leading byte; a
 * 0xFF in any of the remaining three arguments becomes -1. All four resolved
 * values are then passed to func_800B0710.
 *
 * WIP: 89.26%. Structure matches; the sole residual is that the target keeps
 * the 0xFF sentinel constant in v0 (shared with the pointer load, forcing a
 * rematerialization) whereas gcc here parks it in v1. The register-naming
 * cascade downstream all stems from that single allocation choice; the
 * permuter also stalls at the same wall.
 */
void func_800BCC74(s32 a0, s32 a1, s32 a2, s32 a3)
{
    s32 t1 = a1;
    s32 t2 = a2;
    s32 t3 = a3;

    if (a0 == 0xFF)
    {
        a0 = D_80123FB8->unk0;
    }
    a1 = (t1 == 0xFF) ? -1 : t1;
    a2 = (t2 == 0xFF) ? -1 : t2;
    a3 = (t3 == 0xFF) ? -1 : t3;
    func_800B0710(a0, a1, a2, a3);
}
