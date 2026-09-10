#include "common.h"

/** @brief Animation channel selectors and control flags used during teardown. */
typedef struct
{
    u8 unk0, unk1;
    u8 pad2[10];
    u16 unkC;
    u8 padE[10];
    u16 unk18;
} Animation;
/** @brief Teardown fields in a 0x244-byte FIELD animation actor slot. */
typedef struct
{
    u8 pad0[12];
    Animation *unkC;
    u8 pad10[0x24 - 0x10];
    u8 unk24;
    u8 pad25[5];
    u8 unk2A;
    u8 pad2B[0x224 - 0x2B];
    union
    {
        u32 word;
        u16 halves[2];
        u8 bytes[4];
    } status;
    u8 unk228;
    u8 unk229[9];
    u8 unk232;
    u8 pad233[7];
    u8 unk23A, unk23B;
    u8 pad23C[8];
} ActorState;
/** @brief FIELD object record with its animation marker and state ID. */
typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[4];
    s16 unk2A;
    u8 pad2C[0x54 - 0x2C];
} Object;
/** @brief Object flags used to release ownership of animation state. */
typedef struct
{
    u8 pad0[12];
    s32 unkC;
    u8 pad10[0x178 - 0x10];
    s32 unk178;
    u8 pad17C[0x23C - 0x17C];
} ObjectState;
extern Object D_800FDF58[];
extern ObjectState D_80105AE0[];
extern s32 D_800F2278, D_800F227C, D_800F2280;
void field_set_global_color_scale(s32, s32, s32);
void func_8006D21C(ActorState *);
void func_80084424(s32);
/**
 * @brief Stop an actor animation and release its object and render state.
 * @param record Associated FIELD object record, unused by this routine.
 * @param actor Animation actor slot to stop.
 * @param force Nonzero bypasses the normal animation status checks.
 */
void func_80083BC0(void *record, ActorState *actor, s32 force)
{
    s16 object_state;
    s32 target_index;
    u8 owner_index;
    u8 target_slot;
    ObjectState *owner_state;
    ObjectState *target_state;
    Animation *animation;
    u8 *target_ptr;
    u8 *render = (u8 *)0x801ED600;

    if (force == 0)
    {
        if (actor->status.bytes[1] == 0)
        {
            if (!(actor->status.word & 1))
            {
                if (actor->status.halves[1] != 0x21)
                {
                    goto stop_animation;
                }
            }
            else
            {
                goto clear_tracks;
            }
        }
    }
    else
    {
    stop_animation:
    clear_tracks:
        actor->unk23A = 0;
        actor->unk23B = 0;
        func_8006D21C(actor);
        if (actor->unk24 != 0)
        {
            if (actor->unkC->unkC & 0x1000)
            {
                D_800F2280 = 0;
                D_800F227C = 0;
                D_800F2278 = 0;
            }
            if (actor->unkC->unk18 & 2)
            {
                owner_index = actor->unk228;
                object_state = D_800FDF58[owner_index].unk2A;
                if (((object_state != 0x90) && (object_state != 0x94)) ||
                    (D_80105AE0[owner_index].unkC & 0x200))
                {
                    D_800FDF58[actor->unk228].unk25 = 0;
                }
                D_80105AE0[actor->unk228].unk178 &= ~1;
            }
            if (actor->unkC->unk18 & 4)
            {
                for (target_index = 0; target_index < actor->unk232; target_index++)
                {
                    /* Keep this initial read and the subsequent direct array accesses. */
                    target_slot = actor->unk229[target_index];
                    if (actor->unk229[target_index] != 0xFF)
                    {
                        D_800FDF58[actor->unk229[target_index]].unk25 = 0;
                        target_state = &D_80105AE0[actor->unk229[target_index]];
                        target_state->unk178 = (s32)(target_state->unk178 & ~1);
                    }
                }
            }
            animation = actor->unkC;
            if ((*(u8 *)&animation->unkC < 0x10U) && (((u16)animation->unkC >> 8) & 4))
            {
                field_set_global_color_scale(0x100, 0x100, 0x100);
            }
            if (actor->unkC->unk1 != 0xFF)
            {
                render[0x140] = 0;
                render[0x92] = 0;
            }
            if (actor->unkC->unk0 != 0xFF)
            {
                render[0x13F] = 0;
                render[0x91] = 0;
            }
            if (!(actor->unkC->unkC & 0x800))
            {
                actor->unk24 = 0U;
                if (actor->status.word & 1)
                {
                    func_80084424(actor->unk228);
                }
            }
            else
            {
                actor->unk24 = 0U;
                func_80084424(actor->unk228);
                actor->unk2A = 0;
            }
        }
    }
}
