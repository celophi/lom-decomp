#include "common.h"
extern u8 *D_80122B78;
extern u8 *g_field_script;
s32 func_80087F44(s32, s32 *);
s32 func_80087D8C(s32, s32, s32, s32);


/** @brief Camera command mode, actor reference, and explicit coordinates. */
typedef struct
{
    u16 unk0, unk2;
    s32 unk4, unk8, unkC;
} Command;
/** @brief Camera position fields used to preserve the current scroll origin. */
typedef struct
{
    s32 unk0, unk4, unk8, unkC;
} Camera;
/** @brief Saved scroll coordinates in the field state. */
typedef struct
{
    u8 pad[0x424];
    s32 unk424, unk428;
} State;
/** @brief Scene dimensions used to clamp the camera position. */
typedef struct
{
    s16 width;
    u16 height;
} Bounds;
extern s32 func_80087F44(s32, s32 *);
extern s32 D_8010AE74, D_8010CFD8, D_8010CFDC;
extern Camera *D_80122B70;

extern u8 *g_field_script;
/**
 * @brief Apply a camera command using explicit, actor-relative, or saved coordinates.
 * @param arg0 Unused script dispatcher argument.
 * @param arg1 Camera command and parameters.
 */
void func_800BE0E0(s32 arg0, Command *arg1)
{
    s32 position[3];
    Bounds *bounds;
    s32 *destination;
    State *state;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1_2;
    s32 var_v1;
    s32 var_v1_2;
    u16 temp_v1;
    s32 var_a0;

    temp_v1 = arg1->unk2;
    switch (temp_v1)
    { /* irregular */
    case 0:
        temp_v1_2 = (s32)-D_80122B70->unk4 >> 8;
        ((State *)D_80122B78)->unk424 = temp_v1_2;
        D_8010CFD8 = temp_v1_2;
        temp_v0 = (s32)-D_80122B70->unkC >> 9;
        ((State *)D_80122B78)->unk428 = temp_v0;
        D_8010CFDC = temp_v0;
        if ((temp_v1_2 | temp_v0) == 0)
        {
            D_8010CFDC = 1;
        }
        D_8010AE74 = 0;
        return;
    case 1:
        D_8010CFD8 = arg1->unk4;
        D_8010CFDC = arg1->unk8;
        goto block_26;
    case 2:
        bounds = (Bounds *)0x801ED400;
        if (arg1->unk0 == 0xFF)
        {
            var_a0 = *g_field_script;
        }
        else
        {
            var_a0 = arg1->unk0;
        }
        func_80087F44(var_a0, position);
        temp_a0 = (position[0] >> 8) - 0xA0;
        destination = &D_8010CFD8;
        if (temp_a0 > 0)
        {
            var_v1 = bounds->width - 0x140;
            if (temp_a0 < var_v1)
            {
                var_v1 = temp_a0;
            }
            *destination = var_v1;
        }
        else
        {
            *destination = 0;
        }
        temp_a0_2 = ((s32)(position[2] - position[1]) >> 8) - 0xE0;
        destination = &D_8010CFDC;
        if (temp_a0_2 > 0)
        {
            var_v1_2 = (s16)bounds->height - 0x1C0;
            if (temp_a0_2 < var_v1_2)
            {
                var_v1_2 = temp_a0_2;
            }
            *destination = var_v1_2;
        }
        else
        {
            *destination = 0;
        }
        temp_v0_2 = D_8010CFDC / 2;
        D_8010CFDC = temp_v0_2;
        if ((D_8010CFD8 | temp_v0_2) == 0)
        {
            D_8010CFDC = 1;
        }
    block_26:
        D_8010AE74 = arg1->unkC;
        return;
    case 3:
        state = (State *)D_80122B78;
        D_8010AE74 = arg1->unkC;
        D_8010CFD8 = state->unk424;
        D_8010CFDC = state->unk428;
        return;
    }
}


typedef struct
{
    s16 unk0;
    s16 unk2;
    s32 unk4;
    s32 unk8;
    s8 unkC[4];
    s32 unk10;
} UnkStruct800BE2F0;

void func_800BE2F0(s32 arg0, UnkStruct800BE2F0* arg1)
{
    arg1->unk0 = func_800C33E4(arg1->unk4, arg1->unk8, &arg1->unk10);
}

typedef struct
{
    /* 0x0 */ u16 unk0;
    /* 0x4 */ s32 unk4;
    /* 0x8 */ s32 unk8;
    /* 0xC */ s32 unkC;
} UnkStruct800BE324;

extern u8 *g_field_script;

extern s32 func_80087D8C(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

void func_800BE324(s32 arg0, UnkStruct800BE324 *arg1)
{
    s32 var_a0;

    if (arg1->unk0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    else
    {
        var_a0 = arg1->unk0;
    }

    func_80087D8C(var_a0, arg1->unk4, -arg1->unk8, arg1->unkC);
}


typedef struct {
    u16 unk0;
    u16 unk2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} SomeStruct;

extern u8 *g_field_script;
extern void func_8006B984(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/**
 * @brief Dispatch a command record, decoding its priority flag.
 *
 * Resolves the record's target index (@c g_field_script[0] when @c unk0 is the 0xFF
 * sentinel), decodes @c unk2: when bit 7 is set the priority flag is 1 and the
 * value is the low 7 bits, otherwise the flag is 0 and the value is @c unk2 in
 * full. Forwards the record's three payload words plus flag, value, and target
 * index to func_8006B984.
 *
 * @param arg0 Unused.
 * @param arg1 Command record.
 * @see decomp.me (100%) TODO
 */
void func_800BE37C(s32 arg0, SomeStruct *arg1)
{
    s32 var_a0;
    u16 temp_v1;
    s32 var_v0;
    s32 var_a3;

    if (arg1->unk0 == 0xFF)
    {
        var_a0 = *g_field_script;
    }
    else
    {
        var_a0 = arg1->unk0;
    }
    temp_v1 = arg1->unk2;
    var_a3 = 1;
    if (temp_v1 & 0x80)
    {
        var_v0 = temp_v1 & 0x7F;
    }
    else
    {
        var_a3 = 0;
        var_v0 = arg1->unk2;
    }
    func_8006B984(arg1->unk4, arg1->unk8, arg1->unkC, var_a3, var_v0, var_a0);
}


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
            func_80087614(id, ((StructB78B800BE404 *)D_80122B78)->unk403);
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

/** @brief Field command record containing a selector and resolved position. */
typedef struct
{
    u16 unk0;
    u16 pad2;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} FieldPositionCommand;

/** @brief Three-component field position returned by func_80087F44. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;


/**
 * @brief Resolves a field position and stores it in a command record.
 *
 * @param arg0 Unused command argument.
 * @param command Destination record; selector 0xFF uses the script owner.
 */
void func_800BE550(s32 arg0, FieldPositionCommand *command)
{
    s32 index;
    FieldPosition position;

    if (command->unk0 == 0xFF)
    {
        index = *g_field_script;
    }
    else
    {
        index = command->unk0;
    }

    func_80087F44(index, (s32 *)&position);
    command->unk4 = position.x;
    command->unk8 = -position.y;
    command->unkC = position.z;
}

