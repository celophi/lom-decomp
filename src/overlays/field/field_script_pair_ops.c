#include "common.h"
#include "field_script.h"
void akao_cmd_f1(void);
void akao_set_song_params(s32, s32, s32, s32);
void akao_stop_song(s32);
void field_control_animation(s32, s32, s32, s32);
void field_open_gosub_screen_sequence(void *);
void field_open_shop_mode_0(s32);
void field_run_zukan(s32);
void func_8005A67C(s32, s32);
void func_8005B0F4(s32, s32);
void func_8005B1EC(void);
void func_8005B228(s32, s32);
void func_8005B288(s32);
void func_800681C0(s32);
void func_80089980(s32);
void func_80089A68(s32);
void func_8008BD88(s32);
void func_8009C974(s32);
void func_800A37E4(void);
void func_800A38D4(void);
void func_800A5670(s32);
void func_800AD030(s32);
void func_800B22F0(s32, s32);
void func_800B31CC(s32);
void func_800B32FC(s32);
void func_800B3420(s32);
void func_800B60DC(s32);
void func_800B66F0(s32);
void func_800BCCE0(s32, s32, s32, s32);
void func_800BD520(s32, s32, s32);
void func_800BE710(s32);
void func_800C06E8(void);
void func_800C1230(s32);
void func_800C1D14(s32, s32);
void func_800C1D68(void);
void func_800C1E08(void);
void func_800C1EC8(s32, void *, s32);
void func_800C2094(s32);
s32 func_800C20D8(s32);
void func_800C2138(s32);
void func_800C21C0(s32);
s32 func_800C2264(s32);
s32 func_800C23F4(void);
s32 func_800C24BC(s32);
void func_800C25A0(s32);
s32 func_800C2724(s32);
void func_800C2848(s32, s32);
void func_800C28B8(s32);
void func_800C299C(s32);
s32 func_800C29CC(s32);
void func_800C2A88(s32);
void func_800C31BC(s32);
void func_800C35AC(s32);
s32 func_800C35E4(s32);
s32 func_800C3860(s32);
s32 func_800C3894(s32);
void func_800C396C(void);
void func_800C5704(s32);
extern u8 *D_80122B74, *D_80122B78;
extern s32 D_800473E0, D_8010AE48, D_8010D020, D_80117EC4, D_80122980;
extern s32 g_gosub_result_count, g_gosub_result_values;
extern s16 g_music_track_index;





void func_800BB9C0(s32 arg0, s32 arg1)
{
    func_800BD520(g_field_script->status.owner_id, arg0, arg1);
}

void field_control_animation(s32 list_kind, s32 index, s32 keyframe, s32 op);

void func_800BB9F4(s32 arg0, s32 arg1)
{
    field_control_animation(0, arg0, arg1, 1);
}

extern void field_find_or_load_resource_entry(s32 arg0, s32 arg1);

/**
 * @param arg0 Passed through to field_find_or_load_resource_entry.
 * @param arg1 Passed through to field_find_or_load_resource_entry.
 * @see decomp.me (100%) N/A -- trivial 8-instruction wrapper function, no scratch needed.
 */
void func_800BBA24(s32 arg0, s32 arg1)
{
    field_find_or_load_resource_entry(arg0, arg1);
}


typedef struct {
    u8 unk0;
    u8 pad1[3];
    s32 unk4;
    u8 pad8[4];
    s32 unkC;
} SeqRec;


extern s32 func_800C1FFC(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Update the low flag bit of the active sequence record.
 *
 * When @p arg0 is zero, resolves a target index (from @c g_field_script[0] when
 * @p arg1 is the 0xFF sentinel, else @p arg1), runs func_800C1FFC for that
 * index, and writes the result's low bit into bit 0 of the current record's
 * @c unkC field (the record chosen by @c g_field_script->unk4).
 *
 * @param arg0 Guard; the update runs only when zero.
 * @param arg1 Target index, or 0xFF to read the default from @c g_field_script[0].
 * @see decomp.me (100%) TODO
 */
void func_800BBA44(s32 arg0, s32 arg1)
{
    s32 var_a0;
    SeqRec *temp_a1;
    s32 ret;

    if (arg0 == 0)
    {
        if (arg1 == 0xFF)
        {
            var_a0 = g_field_script->status.owner_id;
        }
        else
        {
            var_a0 = arg1;
        }
        ret = func_800C1FFC(var_a0, 0x1100, 0x1100);
        temp_a1 = (SeqRec *)((u8 *)g_field_script + (((SeqRec *)g_field_script)->unk4 * 0xC));
        temp_a1->unkC = (temp_a1->unkC & ~1) | (ret & 1);
    }
}

/**
 * @brief Dispatch a two-operand runtime subcommand and update script or field state.
 * @see decomp.me (100%)
 */
void func_800BBAC8(u32 command, s32 operand)
{
    s32 record_offset;
    s32 actor_index;
    u8* script_record;
    u8* record;
    u8* low_flags_record;
    u8* high_flags_record;

    if (operand == 0xFF)

    {
        actor_index = g_field_script->status.owner_id;
    }
    else
    {
        actor_index = operand;
    }
    switch (command)
    {
    case 0x0:
        func_800681C0((s32)operand);
        return;
    case 0x1:
        func_800A5670((s32)operand);
        return;
    case 0x2:
        func_800C2094((s32)operand);
        return;
    case 0x3:
        func_8008BD88((s32)operand);
        return;
    case 0x4:
        func_8005B0F4((s32)operand, 1);
        return;
    case 0x5:
        func_8005B0F4((s32)operand, 0);
        return;
    case 0x6:
        record = D_80122B74 + (operand * 0xC);
        record[0x2F0] = (u8)(record[0x2F0] | 4);
        return;
    case 0x7:
        field_control_animation(0, operand, 0, 2);
        return;
    case 0x8:
        field_control_animation(0, (s32)operand, -1, 4);
        return;
    case 0x9:
        func_800A37E4();
        return;
    case 0xA:
        field_run_zukan((s32)operand);
        return;
    case 0xB:
        func_800C5704((s32)operand);
        return;
    case 0xC:
        field_open_gosub_screen_sequence(D_80122B78 + (operand * 4));
        return;
    case 0xD:
        func_800BE710((s32)operand);
        return;
    case 0xE:
        (*(s32*)(D_80122B78 + 0x404)) = (s32)((*(s32*)(D_80122B78 + 0x404)) | 0x8000);
        func_800A38D4();
        return;
    case 0xF:
        func_800C35AC((s32)operand);
        return;
    case 0x10:
        func_800C31BC((s32)operand);
        return;
    case 0x11:
        func_800C1D14((s32)actor_index, 0);
        return;
    case 0x12:
        func_800BD520(0, 0x7100, func_800C20D8((s32)operand));
        return;
    case 0x13:
        func_800C2138((s32)operand);
        return;
    case 0x14:
        func_800C21C0((s32)operand);
        return;
    case 0x15:
        func_800BD520(0, 0x7100, func_800C35E4((s32)operand));
        return;
    case 0x16:
        func_80089A68((s32)actor_index);
        return;
    case 0x17:
        func_80089980((s32)actor_index);
        return;
    case 0x18:
        func_800BD520(0, 0x7100, func_800C2264((s32)operand));
        return;
    case 0x19:
        func_800BD520(0, 0x7100, func_800C23F4());
        return;
    case 0x1A:
        func_800C2848((s32)actor_index, 2);
        return;
    case 0x1B:
        func_800B22F0(0x80, operand & 0xFFFF);
        return;
    case 0x1C:
        field_control_animation(1, operand, 0, 2);
        return;
    case 0x1D:
        field_control_animation(1, operand, -1, 1);
        return;
    case 0x1E:
        field_control_animation(2, operand, 0, 2);
        return;
    case 0x1F:
        field_control_animation(2, operand, -1, 1);
        return;
    case 0x20:
        func_8005B228((s32)operand, 1);
        return;
    case 0x21:
        func_8005B228((s32)operand, 0);
        return;
    case 0x22:
        func_800BD520(0, 0x7100, func_800C29CC((s32)operand));
        return;
    case 0x23:
        func_800B31CC((s32)actor_index);
        return;
    case 0x24:
        func_800B32FC((s32)operand);
        return;
    case 0x25:
        func_800B3420((s32)operand);
        return;
    case 0x26:
        func_800C2A88((s32)operand);
        return;
    case 0x27:
        func_800C1D68();
        return;
    case 0x28:
        func_800C1E08();
        return;
    case 0x29:
        func_800C299C((s32)operand);
        return;
    case 0x2A:
        func_800AD030((s32)operand);
        return;
    case 0x2B:
        func_800BD520(0, (s32)operand, 1);
        return;
    case 0x2C:
        func_800C28B8((s32)actor_index);
        return;
    case 0x2D:
        func_800BD520(0, 0x7100, func_800C24BC((s32)operand));
        return;
    case 0x2E:
        func_800C25A0((s32)operand);
        return;
    case 0x32:
        D_80122980 = (s32)operand;
        return;
    case 0x33:
        D_80117EC4 = (s32)operand;
        return;
    case 0x34:
        func_800BD520(0, 0x7100, func_800C2724((s32)operand));
        return;
    case 0x35:
        func_8005A67C((s32)operand, 0);
        return;
    case 0x36:
        func_8005A67C((s32)operand, 1);
        return;
    case 0x37:
        func_8009C974((s32)operand);
        return;
    case 0x38:
        func_800B66F0((s32)actor_index);
        return;
    case 0x39:
        func_800BD520(0, 0x7100, func_800C3860((s32)operand));
        return;
    case 0x3A:
        func_800BD520(0, 0x7100, func_800C3894((s32)operand));
        return;
    case 0x3B:
        if ((s32)operand >= 0x40)
        {
            akao_set_song_params(0x8001, 1, 0x2C, (s32)operand);
            return;
        }
        g_music_track_index = (s16)operand;
        return;
    case 0x3C:
    {
        typedef struct
        {
            unsigned low : 16;
            unsigned phase : 7;
            unsigned high : 9;
        } PhaseWord;
        PhaseWord* state = (PhaseWord*)(D_80122B74 + 0x2E4);
        state->phase++;
        state->phase %= 6U;
        return;
    }
    case 0x3D:
        D_8010AE48 = (s32)operand;
        return;
    case 0x3E:
        field_open_shop_mode_0((s32)operand);
        return;
    case 0x3F:
        func_800C396C();
        return;
    case 0x40:
        func_8008BD88((s32)actor_index);
        return;
    case 0x41:
        func_800C06E8();
        return;
    case 0x42:
        func_8005B1EC();
        return;
    case 0x2F:
    case 0x43:
        func_8005B288((s32)operand);
        return;
    case 0x44:
        D_8010D020 = (s32)operand;
        return;
    case 0x45:
        akao_stop_song(0);
        return;
    case 0x46:
        akao_cmd_f1();
        return;
    case 0x47:
        func_800B60DC((s32)operand);
        return;
    case 0x48:
        func_800C1230((s32)operand);
        return;
    case 0x49:
        D_800473E0 = (s32)operand;
        return;
    case 0x4A:
        script_record = (u8*)g_field_script + (g_field_script->active_record * 0xC);
        (*(s32*)(script_record + 0x10)) = (s32)(((*(s32*)(script_record + 0x10)) & 1) | (operand * 2));
        g_field_script->status.word = (s32)((s32)g_field_script->status.word & 0x7FFFFFFF);
        return;
    case 0x4B:
        g_gosub_result_count = 1;
        g_gosub_result_values = (s32)operand;
        return;
    case 0x4C:
        (*(s32*)(D_80122B74 + 0x2C)) = (s32)operand;
        return;
    case 0x4D:
        g_music_track_index = 0;
        func_800AD030(0);
        func_800BCCE0(0xFFFE, 0, 0, 0);
        return;
    case 0x4E:
        (*(s32*)(D_80122B74 + 0x28)) = (s32)((*(s32*)(D_80122B74 + 0x28)) | 0xC);
        func_800C1EC8(0, D_80122B74 + 0xE4, 0x200);
        func_800C1EC8(0, D_80122B74 + 0x2E4, 0x30C);
        actor_index = 0;
        D_80122B74[0x2E4] = 1;
        record_offset = 0;
        (*(s32*)(D_80122B74 + 0x2E8)) = (s32)((*(s32*)(D_80122B74 + 0x2E8)) | 0x10000000);
        do
        {
            low_flags_record = D_80122B74 + record_offset;
            low_flags_record[0x2F1] = (u8)(low_flags_record[0x2F1] | 0xF);
            actor_index += 1;
            high_flags_record = D_80122B74 + record_offset;
            high_flags_record[0x2F1] = (u8)(high_flags_record[0x2F1] | 0xF0);
            record_offset += 0xC;
        } while (actor_index < 0x40);
        (*(s32*)(D_80122B74 + 0x2F0)) = (s32)((*(s32*)(D_80122B74 + 0x2F0)) | 1);
        D_80122B74[0x2F3] = 1;
        (*(s32*)(D_80122B74 + 0x2F0)) = (s32)((*(s32*)(D_80122B74 + 0x2F0)) | 4);
        (*(s32*)(D_80122B74 + 0x2FC)) = (s32)((*(s32*)(D_80122B74 + 0x2FC)) | 4);
        (*(s32*)(D_80122B74 + 0x308)) = (s32)((*(s32*)(D_80122B74 + 0x308)) | 4);
        (*(s32*)(D_80122B74 + 0x314)) = (s32)((*(s32*)(D_80122B74 + 0x314)) | 4);
        (*(s32*)(D_80122B74 + 0x320)) = (s32)((*(s32*)(D_80122B74 + 0x320)) | 4);
        (*(s32*)(D_80122B74 + 0x32C)) = (s32)((*(s32*)(D_80122B74 + 0x32C)) | 4);
        (*(s32*)(D_80122B74 + 0x338)) = (s32)((*(s32*)(D_80122B74 + 0x338)) | 4);
        (*(s32*)(D_80122B74 + 0x470)) = (s32)((*(s32*)(D_80122B74 + 0x470)) | 4);
        break;
    }
}




/*
 * Helpers reached from field script handlers that act on an actor id, where
 * 0xFF means the script owner (byte 0 of g_field_script).
 */

s32 func_8008C2EC(s32 arg0, s32 arg1);
void field_activate_actor_resource_slot(s32 arg0, s32 arg1, s32 arg2);
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
 * owner), and forwards the pair to field_activate_actor_resource_slot. When @c D_8010AE78 is set
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
    field_activate_actor_resource_slot(var_a0, arg1, 0);
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
        field_activate_actor_resource_slot(var_s1, var_s0, 1);
    }
    func_800BD520(0, 0x2F00, var_s0);
}

/**
 * @brief Apply func_800C318C's value to the actor through field_activate_actor_resource_slot and write it to script variable 0x2F00.
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
        field_activate_actor_resource_slot(arg0, value, 1);
    }
    else
    {
        field_activate_actor_resource_slot(arg0, value, 1);
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



/** @brief One eight-byte shop entry built from a packed resource word. */
typedef struct
{
    u16 item;
    u16 unused;
    u32 price;
} ScriptShopEntry;

u8 *func_800C1E40(s32);
s32 func_800C38C8(u8 *);
void field_open_shop_mode_1(u32, ScriptShopEntry *, u8 *, s32);

/**
 * @brief Build a shop inventory from a packed item list and open shop mode one.
 * @param list_index Index of the packed shop list to load.
 * @param price_scale Scale applied to each generated item price.
 */
void func_800BC6B0(s32 list_index, s32 price_scale)
{
    u8 *lists;
    u8 *list;
    u8 *items;
    ScriptShopEntry entries[32];
    s32 index;
    s32 price;
    u32 scaled;
    u8 *item;

    lists = func_800C1E40(0xA);
    list = lists + *(u32 *)(lists + list_index * 4 + 4);
    items = func_800C1E40(5);
    index = 0;

    if (*(u32 *)list != 0)
    {
        do
        {
            if ((*(u32 *)(list + index * 4 + 4) >> 8) & 1)
            {
                entries[index].item = *(u8 *)(list + index * 4 + 4) | 0x8000;
                item = items + ((*(u8 *)(list + index * 4 + 4) << 6) + 4);
                entries[index].unused = 0;
                price = func_800C38C8(item);
                *(s32 *)(item + 0x34) = price;
                scaled = (u32)(price * price_scale) >> 3;
                entries[index].price = scaled;
            }
            else
            {
                entries[index].item = *(u8 *)(list + index * 4 + 4);
                entries[index].unused = 0;
                scaled = (s32)((*(u32 *)(list + index * 4 + 4) >> 9) * price_scale) >> 3;
                entries[index].price = scaled;
            }
            index++;
        } while ((u32)index < *(u32 *)list);
    }

    field_open_shop_mode_1(*(u32 *)list, entries, items + 4, 2);
}



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
void field_set_all_actor_render_state(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4);
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
    func_80087F44(idx, (s32 *)&sp18);
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
