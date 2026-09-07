#include "common.h"

/** @brief Field-script command payload consumed by func_800BE404. */
typedef struct
{
    u16 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
    s32 unk10;
    s32 unk14;
    s32 unk18;
} ArgB800BE404;

/** @brief Actor state returned by func_800C1B60; unk90 holds status flags. */
typedef struct
{
    u8 pad0[0x90];
    u32 unk90;
} CmdB800BE404;

/** @brief View of D_80122B78 exposing the byte consumed at offset 0x403. */
typedef struct
{
    u8 pad0[0x403];
    u8 unk403;
} StructB78B800BE404;

extern u8 *g_field_script;
extern StructB78B800BE404 *D_80122B78;

void func_80087614(s32 arg0, s32 arg1);
s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
s32 func_80087F0C(s32 arg0);
void func_80089D44(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void *func_800B2B08(void);
void func_800B3F1C(s32 arg0, s32 arg1, s32 arg2);
CmdB800BE404 *func_800C1B60(s32 arg0);
void func_800C1EC8(s32 arg0, void *arg1, s32 arg2);

/**
 * @brief Dispatch a field-script command when its resolved actor is active.
 * @param arg0 Unused leading argument preserved from the original call shape.
 * @param arg1 Command payload containing the actor id, coordinates, and three optional overrides.
 */
void func_800BE404(s32 arg0, ArgB800BE404 *arg1)
{
    s32 id;
    s32 handle;
    s32 s4;
    s32 s3;
    s32 s1;

    if (arg1->unk0 == 0xFF)
    {
        id = *g_field_script;
    }
    else
    {
        id = arg1->unk0;
    }
    if (id != 0 && ((func_800C1B60(id)->unk90 >> 0x1E) & 1))
    {
        handle = (s32) func_800B2B08();
        if (handle != 0)
        {
            func_80087D8C(id, arg1->unk4, arg1->unk8, arg1->unkC);
            s4 = -1;
            func_80087614(id, D_80122B78->unk403);
            if (arg1->unk10 != 0xFF)
            {
                s4 = arg1->unk10;
            }
            s3 = -1;
            if (arg1->unk14 != 0xFF)
            {
                s3 = arg1->unk14;
            }
            s1 = -1;
            if (arg1->unk18 != 0xFF)
            {
                s1 = arg1->unk18;
            }
            func_800C1EC8(0, (void *) handle, 0x68);
            func_800B3F1C(id, handle, func_80087F0C(id));
            func_80089D44(id, s4, s3, s1);
        }
    }
}
