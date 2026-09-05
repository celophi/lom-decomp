#include "common.h"
#include "field_script.h"

/*
 * Helpers reached from field script handlers that act on an actor id, where
 * 0xFF means the script owner (byte 0 of g_field_script).
 */

s32 func_8008C2EC(s32 arg0, s32 arg1);
void func_8006AD04(s32 arg0, s32 arg1, s32 arg2);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
s32 func_800C2DC0(void);
s32 func_800C2D08(void);
s32 func_800C318C(s32 arg0);
s32 func_80087F44(s32 arg0, s32 *out);
void func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);
void func_8008A580(s32 arg0, s32 arg1);
void func_8008B500(s32 arg0, s32 arg1);

extern s32 D_8010AE78;
extern u8 *D_80122B78;

/**
 * @brief Forward an actor id to func_8008B1C8.
 * @param arg0 Actor id, or 0xFF for the script owner.
 */
void func_800BC268(s32 arg0)
{
    s32 var_a0;

    var_a0 = arg0;
    if (var_a0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    func_8008B1C8(var_a0);
}

/**
 * @brief Update the condition bit of the active script record from a pair query.
 *
 * Resolves @p arg0 and @p arg1 (each the script owner when 0xFF), runs
 * func_8008C2EC for the pair, and stores the result's low bit into bit 0 of
 * the active record's flags word.
 *
 * @param arg0 First actor id, or 0xFF for the script owner.
 * @param arg1 Second actor id, or 0xFF for the script owner.
 * @see decomp.me (100%) TODO
 */
void func_800BC2A0(s32 arg0, s32 arg1)
{
    FieldScriptRecordState *temp_a1;
    s32 ret;
    s32 var_a0;
    s32 var_a1;

    if (arg0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    else
    {
        var_a0 = arg0;
    }
    if (arg1 == 0xFF)
    {
        var_a1 = g_field_script->status.owner_id;
    }
    else
    {
        var_a1 = arg1;
    }
    ret = func_8008C2EC(var_a0, var_a1);
    temp_a1 = FIELD_SCRIPT_ACTIVE_RECORD_STATE();
    temp_a1->flags = (temp_a1->flags & ~1) | (ret & 1);
}

/**
 * @brief Dispatch a resolved sequence action and latch a scene-state flag.
 *
 * Runs func_800C2B14 for @p arg1, resolves @p arg0 (0xFF is the script
 * owner), and forwards the pair to func_8006AD04. When @c D_8010AE78 is set
 * it triggers func_80087FC0 and rewrites bits 17-19 of the word at
 * @c D_80122B78 + 0x400 to 0x20000. Always finishes by writing @p arg1 to
 * script variable 0x2F08.
 *
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Secondary parameter forwarded to the dispatched calls.
 * @see decomp.me (100%) TODO
 */
void func_800BC328(s32 arg0, s32 arg1)
{
    s32 var_a0;

    func_800C2B14(arg1);
    if (arg0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    else
    {
        var_a0 = arg0;
    }
    func_8006AD04(var_a0, arg1, 0);
    if (D_8010AE78 != 0)
    {
        func_80087FC0(1, 2);
        *(s32 *)(D_80122B78 + 0x400) = (*(s32 *)(D_80122B78 + 0x400) & 0xFFF9FFFF) | 0x20000;
    }
    func_800BD520(0, 0x2F08, arg1);
}

/**
 * @brief Pick a value from func_800C2DC0 or func_800C2D08, apply it to the actor when valid, and write it to script variable 0x2F00.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 1 selects func_800C2DC0, anything else func_800C2D08.
 */
void func_800BC3DC(s32 arg0, s32 arg1)
{
    s32 var_s1;
    s32 var_s0;

    if (arg0 == 0xFF)
    {
        var_s1 = g_field_script->status.owner_id;
    }
    else
    {
        var_s1 = arg0;
    }
    if (arg1 == 1)
    {
        var_s0 = func_800C2DC0();
    }
    else
    {
        var_s0 = func_800C2D08();
    }
    if (var_s0 != 0xFF)
    {
        func_8006AD04(var_s1, var_s0, 1);
    }
    func_800BD520(0, 0x2F00, var_s0);
}

/**
 * @brief Apply func_800C318C's value to the actor through func_8006AD04 and write it to script variable 0x2F00.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Value passed to func_800C318C.
 */
void func_800BC474(s32 arg0, s32 arg1)
{
    s32 value;

    value = func_800C318C(arg1);
    if (arg0 == 0xFF)
    {
        arg0 = g_field_script->status.owner_id;
        func_8006AD04(arg0, value, 1);
    }
    else
    {
        func_8006AD04(arg0, value, 1);
    }
    func_800BD520(0, 0x2F00, value);
}

/**
 * @brief Route a resolved actor index to one of two handlers, then clear bit 31 of the script status word.
 *
 * When bit 0x10000 of the word at @c D_80122B78 + 0x400 is set and the index
 * is below 3, calls func_80087E00 with func_800C2928's record; otherwise calls
 * func_80087CE0 with the index.
 *
 * @param arg0 Actor index, or 0xFF for the script owner.
 * @param arg1 Forwarded (low 16 bits) to func_800C2928.
 * @see decomp.me (100%) TODO
 */
void func_800BC4E8(s32 arg0, s32 arg1)
{
    s32 var_a0;
    u32 temp_s0;

    var_a0 = arg0;
    if (arg0 == 0xFF)
    {
        var_a0 = g_field_script->status.owner_id;
    }
    temp_s0 = var_a0 & 0xFF;
    if ((*(s32 *)(D_80122B78 + 0x400) & 0x10000) && temp_s0 < 3)
    {
        func_80087E00(temp_s0, func_800C2928(temp_s0, arg1 & 0xFFFF));
    }
    else
    {
        func_80087CE0(var_a0 & 0xFF);
    }
    g_field_script->status.word = g_field_script->status.word & 0x7FFFFFFF;
}

/**
 * @brief Call func_8005AF04 in mode 1 with 0xFF mapped to -1.
 * @param arg0 Object index.
 * @param arg1 Part index; 0xFF becomes -1.
 */
void func_800BC58C(s32 arg0, s32 arg1)
{
    if (arg1 == 0xFF)
    {
        arg1 = -1;
    }
    func_8005AF04(arg0, arg1, 1);
}

/**
 * @brief Call func_8005AF04 in mode 0 with 0xFF mapped to -1.
 * @param obj_index Object index.
 * @param part_index Part index; 0xFF becomes -1.
 */
void func_800BC5B8(s32 obj_index, s32 part_index)
{
    if (part_index == 0xFF)
    {
        part_index = -1;
    }

    func_8005AF04(obj_index, part_index, 0);
}

/**
 * @brief Re-emit an actor's position with its Y replaced by -arg1 and X and Z scaled down by 256.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Negated and used as the new Y.
 */
void func_800BC5E4(s32 arg0, s32 arg1)
{
    s32 v;
    s32 buf[4];

    if (arg0 == 0xFF)
    {
        v = g_field_script->status.owner_id;
    }
    else
    {
        v = arg0;
    }
    func_80087F44(v, buf);
    buf[1] = -arg1;
    func_80087D8C(v, buf[0] >> 8, buf[1], buf[2] >> 8);
}

/**
 * @brief Route an actor to func_8008A580 or func_8008B500 depending on bit 15 of arg1.
 * @param arg0 Actor id, or 0xFF for the script owner.
 * @param arg1 Bit 15 selects func_8008A580 with the low 15 bits; otherwise func_8008B500 gets it whole.
 */
void func_800BC65C(s32 arg0, s32 arg1)
{
    if (arg0 == 0xFF)
    {
        arg0 = g_field_script->status.owner_id;
    }

    if (arg1 & 0x8000)
    {
        func_8008A580(arg0, arg1 & 0x7FFF);
    }
    else
    {
        func_8008B500(arg0, arg1);
    }
}
