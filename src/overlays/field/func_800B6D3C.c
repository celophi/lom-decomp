#include "common.h"

/** @brief Packed field command record consumed by func_800B6D3C. */
typedef struct
{
    u8 pad0[3];
    u8 selector;
    u32 packed;
} FieldCommandB6D3C;

/** @brief Minimal view of the active FIELD state used by func_800B6D3C. */
typedef struct
{
    u8 pad0[0x1C];
    FieldCommandB6D3C *command;
    s32 unk20;
    u8 *target;
} FieldStateB6D3C;

extern FieldStateB6D3C *D_80123FB0;
extern void akao_set_song_params(s32 command, s32 arg1, s32 arg2, s32 arg3);
extern void func_800B70F4(s32 arg0, s32 *out);
extern void func_800B7164(s32 arg0, s32 *out);
extern void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
extern s32 func_800B742C(s32 arg0, s32 arg1);
extern void func_800B2B54(s32 arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
extern void func_800B78C0(void);

/**
 * @brief Dispatch the active packed field command through its selected processing path.
 * @return Result produced by func_800B742C.
 */
s32 func_800B6D3C(void)
{
    u32 packed;
    s32 first_value;
    s32 second_value;
    s32 arg4_value;
    s32 arg5_value;
    s32 selector;
    s32 packed_selector;
    s32 result;
    u32 packed_tail;

    packed = D_80123FB0->command->packed;
    selector = D_80123FB0->target[3];
    packed_selector = (packed >> 8) & 0xF;
    packed_tail = packed;

    if (selector == packed_selector)
    {
        akao_set_song_params(0x8003, selector, selector, 1);

        packed++;
        packed--;
        packed_tail--;
        packed_tail++;
        func_800B70F4(packed & 0xF, &first_value);
        if (((packed >> 12) & 0xF) == 1)
        {
            first_value <<= 1;
        }
        func_800B7164((packed >> 4) & 0xF, &second_value);
        func_800B729C(0, 0, &first_value, &second_value);
        result = func_800B742C(first_value, second_value);

        arg4_value = (((packed >> 16) & 0xF) + 1) << 4;
        arg5_value = (packed >> 24) << 4;
        func_800B2B54(D_80123FB0->unk20, D_80123FB0->target, 0, (packed >> 20) & 0xF, arg4_value, arg5_value);
    }
    else
    {
        packed_tail = D_80123FB0->command->packed;
        akao_set_song_params(0x8003, packed_selector, selector, 0);

        func_800B70F4(packed_tail & 0xF, &first_value);
        func_800B7164((packed_tail >> 4) & 0xF, &second_value);
        func_800B729C(0, 0, &first_value, &second_value);
        result = func_800B742C(first_value, second_value);
    }

    func_800B78C0();
    return result;
}
