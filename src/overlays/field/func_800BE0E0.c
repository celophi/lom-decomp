#include "common.h"

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
extern State *D_80122B78;
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
        D_80122B78->unk424 = temp_v1_2;
        D_8010CFD8 = temp_v1_2;
        temp_v0 = (s32)-D_80122B70->unkC >> 9;
        D_80122B78->unk428 = temp_v0;
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
        state = D_80122B78;
        D_8010AE74 = arg1->unkC;
        D_8010CFD8 = state->unk424;
        D_8010CFDC = state->unk428;
        return;
    }
}
