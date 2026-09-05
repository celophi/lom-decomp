#include "common.h"

typedef struct
{
    u8 pad0[0x3A];
    u8 unk3A; /* 0x3A */
    u8 pad3B[0x54 - 0x3B];
} Struct_D800FDF58;

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
