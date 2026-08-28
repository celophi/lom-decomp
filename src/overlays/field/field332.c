#include "common.h"

typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;   /* 0x2A */
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;    /* 0x3A */
    u8 unk3B;    /* 0x3B */
    u8 pad3C[0x178 - 0x3C];
    u8 unk178;   /* 0x178 */
    u8 pad179[0x23C - 0x179];
} ActorRec;

typedef struct {
    u8 pad0[0xE];
    u16 unkE;    /* 0x0E */
    u8 pad10[0x14 - 0x10];
} ResEntry;

extern ActorRec D_80105AE0[];
extern ResEntry g_field_resource_entries[];


/**
 * @brief Updates the actor state when its resource query succeeds.
 *
 * @return The guard/query result, or 0x8E after a successful update.
 * @note 100% match. The function returns the value already carried in v0;
 *       that return lifetime naturally preserves the target delay-slot nops.
 */
s32 func_80094EA4(ActorRec *arg0)
{
    s32 result;

    result = D_80105AE0[arg0->unk3A].unk178 & 1;
    if (result == 0)
    {
        result = func_8009104C(arg0->unk3A, 0, 0, g_field_resource_entries[arg0->unk3B].unkE);
        if (result != 0)
        {
            result = 0x8E;
            arg0->unk2A = result;
        }
    }
    return result;
}

/**
 * @brief Updates the actor state when its resource query succeeds.
 *
 * @return The guard/query result, or 0x94 after a successful update.
 * @note 100% match. Twin of func_80094EA4 with a different success state.
 */
s32 func_80094F40(ActorRec *arg0)
{
    s32 result;

    result = D_80105AE0[arg0->unk3A].unk178 & 1;
    if (result == 0)
    {
        result = func_8009104C(arg0->unk3A, 0, 0, g_field_resource_entries[arg0->unk3B].unkE);
        if (result != 0)
        {
            result = 0x94;
            arg0->unk2A = result;
        }
    }
    return result;
}
