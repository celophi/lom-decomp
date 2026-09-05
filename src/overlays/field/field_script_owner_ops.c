#include "common.h"
#include "field_script.h"

/*
 * Helpers reached from field script handlers. Most take an actor id where
 * 0xFF means the script owner; the owner id is byte 0 of g_field_script.
 */

/** @brief Actor record from func_800C1B60; unk90 holds the flag word. */
typedef struct
{
    u8 pad[0x90];
    s32 unk90;
} SomeStruct;

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} Struct80087F44;

/** @brief View of D_80122B74 exposing the word at 0x2C. */
typedef struct
{
    s8 pad[0x2C];
    s32 unk2C;
} UnkStruct80122B74;

#define FIELD_B74 ((UnkStruct80122B74 *)D_80122B74)

SomeStruct *func_800C1B60(s32 arg0);
void func_80089AE4(s32 arg0, s32 arg1);
s32 func_80087770(s32 arg0, s32 arg1);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
void func_800A3904(s32 arg0, s32 arg1, s32 arg2);
void func_800A3938();
s32 func_800878B4(s32 arg0);
void func_8006B8DC(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
void func_800C28B8(s32 arg0);
void func_80087A9C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5,
                   s32 arg6, s32 arg7, s32 arg8, s32 arg9);
void func_800B0710(s32, s32, s32, s32);

extern u8 *D_80122B74;
extern u8 *D_80122B78;

/**
 * @brief Resolve a target index and clear two flag bits on its state record.
 *
 * When @p arg0 is the sentinel 0xFF the index is the script owner; otherwise
 * it is @p arg0 itself. The resolved index selects a state record via
 * func_800C1B60, whose @c unk90 field has bits 31 and 29 cleared, then the
 * index and @p arg1 are dispatched to func_80089AE4.
 *
 * @param arg0 Target index, or 0xFF for the script owner.
 * @param arg1 Forwarded to func_80089AE4.
 * @see decomp.me (100%) TODO
 */
void func_800BC7EC(s32 arg0, s32 arg1)
{
    s32 var_s0;
    SomeStruct *temp_v0;

    if (arg0 == 0xFF)
    {
        var_s0 = g_field_script->status.owner_id;
    }
    else
    {
        var_s0 = arg0;
    }
    temp_v0 = func_800C1B60(var_s0);
    temp_v0->unk90 &= 0x7FFFFFFF;
    temp_v0->unk90 &= 0xDFFFFFFF;
    func_80089AE4(var_s0, arg1);
}

/**
 * @brief Write func_80087770's result for two actors to script variable 0x7100.
 * @param arg0 First actor id, or 0xFF for the script owner.
 * @param arg1 Second actor id, or 0xFF for the script owner.
 */
void func_800BC86C(s32 arg0, s32 arg1)
{
    arg0 = (arg0 == 0xFF) ? g_field_script->status.owner_id : arg0;
    arg1 = (arg1 == 0xFF) ? g_field_script->status.owner_id : arg1;

    func_800BD520(0, 0x7100, func_80087770(arg0, arg1));
}

/**
 * @brief Copy script variable arg1 to script variable arg0 in the owner's slot.
 * @param arg0 Destination variable id.
 * @param arg1 Source variable id (low 16 bits).
 */
void func_800BC8CC(s32 arg0, s32 arg1)
{
    func_800BD520(g_field_script->status.owner_id, arg0, func_800BD414(g_field_script->status.owner_id, arg1 & 0xFFFF));
}

/**
 * @brief Call func_800A3904 in mode 1 with arg0 clamped to 0x7F and arg1 defaulting to 1.
 * @param arg0 Value clamped to 0x7F.
 * @param arg1 Count; 0 becomes 1.
 */
void func_800BC91C(s32 arg0, s32 arg1)
{
    arg1 = (arg1 != 0) ? arg1 : 1;
    if (arg0 >= 0x80)
    {
        arg0 = 0x7F;
    }
    func_800A3904(1, arg1, arg0);
}

/**
 * @brief Thin stack-frame wrapper around func_800A3858.
 */
void func_800BC960(void)
{
    func_800A3858();
}

/**
 * @brief Call func_800A3904 in mode 0 with arg0 clamped to 0x7F and arg1 defaulting to 1.
 * @param arg0 Value clamped to 0x7F.
 * @param arg1 Count; 0 becomes 1.
 */
void func_800BC980(s32 arg0, s32 arg1)
{
    arg1 = (arg1 != 0) ? arg1 : 1;
    if (arg0 >= 0x80)
    {
        arg0 = 0x7F;
    }
    func_800A3904(0, arg1, arg0);
}

/**
 * @brief Store a 2-bit value into bits 4-5 of the 0xC-byte layout record's status byte.
 * @param arg0 Record index.
 * @param arg1 Value; only the low 2 bits are stored.
 */
void func_800BC9C4(s32 arg0, s32 arg1)
{
    u8 *temp_v1;

    temp_v1 = D_80122B74 + arg0 * 0xC;
    temp_v1[0x2F0] = (temp_v1[0x2F0] & 0xCF) | ((arg1 & 3) << 4);
}

/**
 * @brief Fetch an actor's position, scale it down by 256, and forward it to func_80087680.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Forwarded to func_80087680.
 */
void func_800BC9F8(s32 arg0, s32 arg1)
{
    Struct80087F44 sp18;
    s32 idx;

    if (arg0 == 0xFF)
    {
        idx = g_field_script->status.owner_id;
    }
    else
    {
        idx = arg0;
    }
    func_80087F44(idx, &sp18);
    {
        s32 x = sp18.unk0 >> 8;
        s32 y = sp18.unk4 >> 8;
        s32 z = sp18.unk8 >> 8;
        func_80087680(idx, arg1, D_80122B78[0x403], x, y, z);
    }
}

/**
 * @brief Forward two values to func_800A3938.
 * @param arg0 Forwarded unchanged.
 * @param arg1 Forwarded unchanged.
 */
void func_800BCA88(s32 arg0, s32 arg1)
{
    func_800A3938(arg0, arg1);
}

/**
 * @brief Write the layout buffer's word at 0x2C to script variable arg0.
 * @param arg0 Script variable id.
 */
void func_800BCAA8(s32 arg0)
{
    func_800BD520(0, arg0, FIELD_B74->unk2C);
}

/**
 * @brief Write func_800878B4's result for an actor to script variable arg0 in that actor's slot.
 * @param arg0 Script variable id.
 * @param arg1 Actor id, or 0xFF for the script owner.
 */
void func_800BCAD8(s32 arg0, s32 arg1)
{
    s32 v;

    if (arg1 == 0xFF)
    {
        v = g_field_script->status.owner_id;
    }
    else
    {
        v = arg1;
    }
    func_800BD520(v, arg0, func_800878B4(v));
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB40(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB48(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB50(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB58(void)
{
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCB60(void)
{
}

/**
 * @brief Forward four values to akao_set_song_params.
 * @param flags AKAO flags word.
 * @param duration Duration.
 * @param field_id Field id.
 * @param sub_id Sub id.
 */
void func_800BCB68(s32 flags, s32 duration, s32 field_id, s32 sub_id)
{
    akao_set_song_params(flags, duration, field_id, sub_id);
}

/**
 * @brief Split bit 7 of arg0 into a flag and forward the rest to func_8006B8DC.
 * @param arg0 Id with an optional 0x80 flag bit.
 * @param arg1 Forwarded as the first argument.
 * @param arg2 Forwarded as the second argument.
 * @param arg3 Forwarded as the third argument.
 */
void func_800BCB88(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 flag;

    if (arg0 & 0x80)
    {
        flag = 1;
        arg0 &= 0x7F;
    }
    else
    {
        flag = 0;
    }
    func_8006B8DC(arg1, arg2, arg3, flag, arg0);
}

/**
 * @brief Split bit 7 of arg0 into a flag, run func_800C28B8 on the id, then forward everything to func_80087A9C.
 * @param arg0 Actor id with an optional 0x80 flag bit.
 * @param arg1 Forwarded to func_80087A9C.
 * @param arg2 Forwarded to func_80087A9C.
 * @param arg3 Forwarded to func_80087A9C.
 */
void func_800BCBD0(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 var_s0;
    s32 var_s1;

    if (arg0 & 0x80)
    {
        var_s1 = 1;
        var_s0 = arg0 & 0x7F;
    }
    else
    {
        var_s1 = 0;
        var_s0 = arg0;
    }
    func_800C28B8(var_s0);
    func_80087A9C(var_s0, arg1, arg2, arg3, 0, -1, -1, -1, 0, var_s1);
}

/**
 * @brief Empty function; no-op.
 */
void func_800BCC6C(void)
{
}

/**
 * @brief Forward four values to func_800B0710, mapping 0xFF to the script owner in the first and to -1 in the rest.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 0xFF becomes -1.
 * @param arg2 0xFF becomes -1.
 * @param arg3 0xFF becomes -1.
 */
void func_800BCC74(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 v;

    if (arg0 == 0xFF)
    {
        v = g_field_script->status.owner_id;
    }
    else
    {
        v = arg0;
    }
    func_800B0710(v,
                  (arg1 == 0xFF) ? -1 : arg1,
                  (arg2 == 0xFF) ? -1 : arg2,
                  (arg3 == 0xFF) ? -1 : arg3);
}
