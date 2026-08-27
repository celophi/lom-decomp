#include "common.h"

typedef struct FieldState800B66F0
{
    u8 pad0[0x1C];
    s32 unk1C;
    s32 unk20;
    s32 unk24;
} FieldState800B66F0;

extern FieldState800B66F0 *D_80123FB0;
extern s32 func_800B2A9C(void);
extern s32 func_800B6334(s32 value);
extern void func_800B65CC(s32 value);

/**
 * @brief Refreshes three field-state values and processes the resulting
 *        nonzero handle.
 */
void func_800B66F0(void)
{
    s32 value;
    s32 result;

    value = func_800B2A9C();
    D_80123FB0->unk20 = value;
    D_80123FB0->unk24 = value;
    D_80123FB0->unk1C = 0;
    if (value != 0)
    {
        result = func_800B6334(value);
        if (result != 0)
        {
            func_800B65CC(result);
        }
    }
}
