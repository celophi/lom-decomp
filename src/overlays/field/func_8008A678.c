#include "common.h"

/** @brief Field state prefix and queued interaction targets in a 0x23C-byte slot. */
typedef struct
{
    u8 pad0[4];
    s32 active;
    u8 pad8[0x14 - 8];
    s32 actor;
    u8 pad18[0x16F - 0x18];
    u8 action;
    u8 pad170[8];
    union
    {
        s32 flags;
        struct
        {
            u8 pad[3];
            u8 count;
        } bytes;
    } state;
    u8 pad17c[4];
    u8 targets[0x23C - 0x180];
} FieldState;

/** @brief Seven-word interaction request consumed by func_800B5534. */
typedef struct
{
    s32 actor;
    s32 action;
    s32 param;
    s32 target;
    s32 unk10;
    s32 unk14;
    s32 mode;
} Request;

extern FieldState D_80105AE0[];
extern s32 func_800B5534(Request *request);

/**
 * @brief Process queued actor interactions and clear the source queue count.
 * @param index Source field-state slot index.
 * @note Preserve the loop-carried stride and address casts for matching codegen.
 */
void func_8008A678(s32 index)
{
    Request request;
    FieldState *base;
    FieldState *loop_base;
    FieldState *source;
    FieldState *saved_source;
    FieldState *current;
    FieldState *target;
    FieldState *first_target;
    s32 i;
    s32 stride8;

    base = D_80105AE0;
    stride8 = index * 8;
    source = (FieldState *)((u8 *)base + ((stride8 + index) * 16 - index) * 4);
    i = 0;
    if (source->state.bytes.count != 0)
    {
        /* Keep separate copies for values that must survive the request call. */
        loop_base = base;
        saved_source = source;
        stride8 = index * 8;
        do
        {
            /* Integer address arithmetic preserves the target operand order. */
            current = (FieldState *)((((stride8 + index) * 16 - index) * 4) + (u32)loop_base);
            first_target = (FieldState *)(current->targets[i] * 0x23C + (u32)loop_base);
            first_target->state.flags &= ~0x80;
            target = (FieldState *)(current->targets[i] * 0x23C + (u32)loop_base);
            if (!(((u32)target->state.flags >> 5) & 1))
            {
                stride8 = index * 8;
                if (target->active != 0)
                {
                    request.actor = current->actor;
                    if ((u8)current->action < 0xB)
                    {
                        request.action = current->action;
                    }
                    else
                    {
                        request.action = 0xA;
                    }
                    request.target = loop_base[saved_source->targets[i]].actor;
                    request.param = 0;
                    request.unk10 = 0;
                    request.unk14 = 0;
                    request.mode = 1;
                    func_800B5534(&request);
                    goto stride_update;
                }
            }
            else
            {
stride_update:
                stride8 = index * 8;
            }
            i++;
        } while (i < ((FieldState *)((u8 *)loop_base + (((stride8 + index) * 16 - index) * 4)))->state.bytes.count);
    }
    D_80105AE0[index].state.bytes.count = 0;
}
