/** @file field_actor_animation_resume.c
 * @brief Detect released animation slots and resume the actor record animation.
 */

/* func_80094FDC */
#include "common.h"

typedef struct
{
    u8 pad0[0x18];
    s32 unk18;
} Struct_D80105880;

typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 pad25[0x244 - 0x25];
} FieldActorState;

typedef struct
{
    u8 pad0[0x25];
    u8 unk25;
    u8 pad26[0x2A - 0x26];
    s16 unk2A;
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;
} Struct_D800FDF58;

extern Struct_D80105880 D_80105880[];
extern FieldActorState g_field_actor_slots[];
extern void func_80095074(Struct_D800FDF58 *rec);

/**
 * @brief Resets a field record when its selected actor slot is free.
 *
 * The record's selector at 0x3A chooses one of three D_80105880 entries, with
 * values >= 2 clamped to the third entry. If that entry's actor slot is free,
 * clears the record timer, writes the 0xFF sentinel, and calls func_80095074.
 *
 * @note The s32 return type, despite the lack of an explicit return statement,
 *       is required to preserve the target v0 lifetime. gcc272_cdk, 100%.
 */
s32 func_80094FDC(Struct_D800FDF58 *rec)
{
    FieldActorState *actors;
    u8 *base;
    s32 offset;
    s32 idx;
    FieldActorState *actor;

    actors = g_field_actor_slots;
    base = (u8 *)D_80105880;
    if (rec->unk3A < 2)
        offset = rec->unk3A * 0x1C;
    else
        offset = 0x38;
    idx = *(s32 *)(base + offset + 0x18);
    actor = actors + idx;
    if (actor->unk24 == 0)
    {
        rec->unk2A = 0;
        rec->unk25 = 0xFF;
        func_80095074(rec);
    }
}


/* func_80095074 */
#include "common.h"



typedef struct
{
    u8 pad0[0x60];
    u8 unk60[4];   /* 0x60 */
    u8 pad64[0x16C - 0x64];
    u8 unk16C;     /* 0x16C */
    u8 pad16D[0x23C - 0x16D];
} Struct_D80105AE0;

extern Struct_D80105AE0 D_80105AE0[];
extern s32 func_800839F8(s32 arg0, s32 arg1);
extern s32 func_80083EEC(s32 arg0, s32 arg1, s32 arg2);
extern void field_start_actor_animation(s32 slot_index, int target_count, u8 *targets);

/**
 * @see decomp.me (100%) TODO
 */
void func_80095074(Struct_D800FDF58 *rec)
{
    s32 i;
    s32 anim;
    s32 anim_id;

    if (D_80105AE0[rec->unk3A].unk16C == 0xFF)
    {
        return;
    }
    if (D_80105AE0[rec->unk3A].unk16C == 0x1F)
    {
        for (i = 0; i < 4; i++)
        {
            if (D_80105AE0[rec->unk3A].unk60[i] != 0)
            {
                anim_id = D_80105AE0[rec->unk3A].unk16C;
                anim = func_800839F8(rec->unk3A, 0);
                if (anim != -1)
                {
                    if (func_80083EEC(rec->unk3A, anim, anim_id))
                    {
                        field_start_actor_animation(anim, 0, 0);
                    }
                }
                return;
            }
        }
        return;
    }
    anim_id = D_80105AE0[rec->unk3A].unk16C;
    anim = func_800839F8(rec->unk3A, 0);
    if (anim != -1)
    {
        if (func_80083EEC(rec->unk3A, anim, anim_id))
        {
            field_start_actor_animation(anim, 0, 0);
        }
    }
}


/* func_80095168 */
#include "common.h"





extern FieldActorState g_field_actor_slots[];
extern void func_80095074(Struct_D800FDF58 *rec);

/**
 * @brief Resets an actor record when its target field-actor slot is free.
 *
 * When the slot at @c g_field_actor_slots[rec->unk3A + 0x40] has a zero
 * @c unk24 (unclaimed), clears the record's @c unk2A, sets @c unk25 to the
 * sentinel 0xFF, and hands the record to func_80095074.
 *
 * @param rec Actor record to reset.
 * @return Unused status value (the return register is left live by the
 *         original, but no caller consumes it).
 */
s32 func_80095168(Struct_D800FDF58 *rec)
{
    if (g_field_actor_slots[rec->unk3A + 0x40].unk24 == 0)
    {
        rec->unk2A = 0;
        rec->unk25 = 0xFF;
        func_80095074(rec);
    }
}
