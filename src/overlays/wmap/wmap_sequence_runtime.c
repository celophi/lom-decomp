#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "sdk/inline_c.h"
#include "sdk/gte_dmpsx_compat.h"

/** @brief Animation fields within a world-map actor. */
typedef struct
{
    u8 unknown_00[0xE];
    s16 sequence;
    s16 previous_sequence;
    u8 unknown_12[2];
    u8* cursor;
    u8* sequence_start;
    u8* frame_data;
    s16 remaining;
} WmapAnimation;

/** @brief Resource slot containing the animation data block. */
typedef struct
{
    s32 unknown_00;
    u8* data;
} WmapAnimationResource;

/** @brief World-map actor configuration with its original field layout. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

/** @brief Map translation and projection scale. */
typedef struct
{
    s32 x, y, scale, pad;
} WmapTransform;

/** @brief Record with a leading value and a 40-byte stride. */
typedef struct
{
    s32 value;
    u8 unknown_4[36];
} WmapValueRecord;

/** @brief Two signed coordinates stored consecutively. */
typedef struct
{
    s16 x;
    s16 y;
} WmapCoordinatePair;

extern s32 D_8011CF44;
extern s32 D_801B0FD8[];
extern WmapSequenceCallback D_801B1018[];
extern void func_80064F14(void);
extern s32 D_8013B20C;
extern u32 D_801B10A0;
extern u32 D_801B1098;
extern s32 D_801B109C;
extern void (*D_800D0A44[])(void);
extern s32 D_8011D4FC;
extern s32 D_8013B254;
extern s32 D_80182E34;
extern s32 D_801ADAEC;
extern s32 D_800DBE70;
extern s32 D_800DBE78;
extern s32 D_8013B208;
extern s32 D_801B1058[];
extern WmapSequenceCallback D_801B1078[];
extern WmapConfigA D_800D9268[];
extern s32 D_80139988[];
extern void func_800675F0(u8*, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern s32 D_8011D510;
extern s32 D_8011D530;
extern WmapTransform D_80139950;
extern s32 D_800D923C;
extern s32* D_80139280;
extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern WmapValueRecord D_80139290[][6];
extern VECTOR D_8011CF60;
extern WmapCoordinatePair D_8011CF4C;
extern SVECTOR D_80139278;
extern s32 D_801B10A4;
extern void (*D_800D0FAC[])(void);
extern u32 D_801B10A8;
extern s32 D_801B10AC;
extern void (*D_800D0FBC[])(void);
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern void func_80064094(void);

static void func_8006D1AC(VECTOR* translation, SVECTOR* rotation);
static s32 func_8006D328(s32 arg0);

/**
 * @brief Register and initialize a callback in the first free slot.
 */
void func_8006C754(void)
{
    s32 i;
    WmapSequenceCallback callback;

    callback = func_8006C81C;
    for (i = 0; i < 14; i++)
    {
        if (D_801B0FD8[i] == 0)
        {
            D_801B1018[i] = callback;
            D_801B0FD8[i] = 1;
            (D_801B1018[i])(1);
            D_8011CF44++;
            break;
        }
    }
    if (i == 14)
    {
        func_80064F14();
    }
    D_8013B20C = 1;
    D_801B10A0++;
    func_8006D2D4();
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or a reset occurred, 0 if the index was out of range.
 */
s32 func_8006C81C(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B1098 = 1;
        D_801B109C = 1;
        return 1;
    }

    if (D_801B1098 < 0xA)
    {
        D_800D0A44[D_801B1098]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006C894(void)
{
    D_801B1098 = 1;
    D_801B109C = 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006C8AC(void)
{
    if (--D_801B109C == 0)
    {
        D_801B1098 += 1;
    }
}

/** @brief Set world-map flags and advance to a two-tick delay. */
void func_8006C8E0(void)
{
    if (D_8011D4FC != 0x1F)
    {
        D_8013B254 = 2;
    }
    D_80182E34 = 3;
    D_801ADAEC = 0;
    D_801B109C = 2;
    D_801B1098 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006C930(void)
{
    if (--D_801B109C == 0)
    {
        D_801B1098 += 1;
    }
}

/**
 * @brief Reset a world-map step slot: arm its wait and advance the step index.
 */
void func_8006C964(void)
{
    D_801B109C = 4;
    D_801B1098 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006C984(void)
{
    if (--D_801B109C == 0)
    {
        D_801B1098 += 1;
    }
}

/** @brief Set world-map flags, start an eight-tick delay, and advance the state. */
void func_8006C9B8(void)
{
    D_800DBE70 = 0;
    D_8013B208 = 1;
    D_800DBE78 = 1;
    D_801B109C = 8;
    D_801B1098 += 1;
}

/**
 * @brief Tick the sequence wait timer; advance the step counter when it expires.
 */
void func_8006C9F4(void)
{
    if (--D_801B109C == 0)
    {
        D_801B1098 += 1;
    }
}

/** @brief Run active callbacks and decrement the active count when a callback finishes. */
void func_8006CA28(void)
{
    s32 i;
    s32 result;

    i = 0;
    for (; i < 14; i++)
    {
        if (D_801B0FD8[i] != 0)
        {
            result = D_801B1018[i](0);
            D_801B0FD8[i] = result;
            if (result == 0)
            {
                D_8011CF44--;
            }
        }
    }
}

/**
 * @brief Register and initialize a callback in the first free slot.
 * @param callback Callback receiving one for initialization and zero for updates.
 */
void func_8006CAC0(WmapSequenceCallback callback)
{
    s32 i;

    for (i = 0; i < 14; i++)
    {
        if (D_801B0FD8[i] == 0)
        {
            D_801B1018[i] = callback;
            D_801B0FD8[i] = 1;
            (D_801B1018[i])(1);
            D_8011CF44++;
            break;
        }
    }
    if (i == 14)
    {
        func_80064F14();
    }
}

/** @brief Run each active callback and retain its returned active state. */
void func_8006CB60(void)
{
    s32 (**callback)(s32);
    s32* active;
    s32 i;

    i = 0;
    active = D_801B1058;
    callback = D_801B1078;
    do
    {
        if (*active != 0)
        {
            *active = (*callback)(0);
        }
        active++;
        i++;
        callback++;
    } while (i < 8);
}

/**
 * @brief Install and initialize a callback in the first inactive slot.
 * @param callback Callback receiving one for initialization and zero for updates.
 */
void func_8006CBD8(WmapSequenceCallback callback)
{
    WmapSequenceCallback* slot;
    s32* active;
    s32 i;

    i = 0;
    active = D_801B1058;
    slot = D_801B1078;
next_slot:
    i++;
    if (*active == 0)
    {
        *slot = callback;
        *active = 1;
        *active = (*slot)(1);
        return;
    }
    active++;
    slot++;
    if (i >= 8)
    {
        return;
    }
    goto next_slot;
}

/**
 * @brief Advance the selected animation, wrapping when its end marker is reached.
 * @param actor_data Actor animation state and frame pointers.
 * @param resource_data Animation block containing sequence and frame offsets.
 * @return New frame index, or -1 when the current frame is still active.
 */
s32 func_8006CC4C(void* actor_data, void* resource_data)
{
    WmapAnimation* actor = actor_data;
    WmapAnimationResource* resource = resource_data;
    u8* data;
    s16* offsets;
    u8* cursor;
    s32 result;
    s32 frame;
    s32 offset;

    result = -1;
    offsets = (s16*)resource->data;
    data = (u8*)offsets;
    if (actor->previous_sequence != actor->sequence)
    {
        actor->previous_sequence = (u16)actor->sequence;
        offset = offsets[actor->sequence];
        actor->remaining = 1;
        cursor = (u8*)offsets + offset;
        actor->sequence_start = cursor;
        actor->cursor = cursor;
    }
    if (actor->remaining != 255)
    {
        actor->remaining = (u16)actor->remaining - 1;
    }
    if (actor->remaining == 0)
    {
        cursor = actor->cursor;
        frame = cursor[0];
        actor->remaining = cursor[1];
        if (frame == 255)
        {
            cursor = actor->sequence_start;
            actor->cursor = cursor;
            frame = cursor[0];
            actor->remaining = cursor[1];
        }
        actor->cursor += 4;
        actor->frame_data = data + ((s16*)(frame * 2 + data))[32];
        result = frame;
    }
    return result;
}

/** @brief Initialize actor indices and clear the callback state tables. */
void func_8006CD18(void)
{
    s32 index;
    s32* callback_state;
    s32* active;

    for (index = 0; index < 256; index++)
    {
        D_800D9268[index].field_00 = index;
        D_80139988[index * 2] = index;
        D_800D9268[index].field_02 = -1;
    }
    index = 0xD;
    callback_state = D_801B0FD8;
    callback_state += index;
    do
    {
        *callback_state = 0;
        index -= 1;
        callback_state--;
    } while (index >= 0);
    index = 7;
    active = D_801B1058;
    active += index;
    do
    {
        *active = 0;
        index -= 1;
        active--;
    } while (index >= 0);
}

/**
 * @brief Draw a world-map resource without screen offsets or an extra Z divisor.
 * @param resource_table Resource table.
 * @param resource_index Resource to draw.
 * @param ot_index Ordering-table index.
 * @param tpage Texture page.
 * @param clut Color lookup table.
 * @param blend_mode Blending mode.
 * @param color_scale Color scale.
 */
void func_8006CD98(u8* resource_table, s32 resource_index, s32 ot_index, s32 tpage, s32 clut, s32 blend_mode, s32 color_scale)
{
    func_800675F0(resource_table, resource_index, ot_index, tpage, clut, blend_mode, color_scale, 0, 0, -1);
}

/** @brief Project the current map position through the active GTE matrix. */
void func_8006CDDC(void)
{
    SVECTOR position;

    position.vz = 0;
    position.vx = (((D_8011D510 - 1) * 160 - D_80139950.x * 0x14000 / D_80139950.scale) * 0x6000) / D_80139950.scale;
    position.vy = (((D_8011D530 - 1) * 160 - D_80139950.y * 0x14000 / D_80139950.scale) * 0x6000) / D_80139950.scale;
    gte_ldv0(&position);
    gte_rtps();
}

/**
 * @brief Scale packed RGB channels, preserving the high byte.
 * @param color Packed RGB value and high byte.
 * @param scale Scale in units of 1/128, or -1 to preserve the color.
 * @return Packed scaled color; channel results wrap to eight bits.
 */
s32 func_8006CF40(CVECTOR color, s32 scale)
{
    /* The packet color word is passed by value and returned in packed form. */
    CVECTOR* channels = &color;
    s32 red;
    s32 green;
    s32 blue;
    if (scale == -1)
    {
        return *(s32*)&color;
    }
    else
    {
        red = channels->r * scale;
        green = channels->g * scale;
        blue = channels->b * scale;
        channels->r = red >> 7;
        channels->g = green >> 7;
        channels->b = blue >> 7;
        return *(s32*)&color;
    }
}

/**
 * @brief Select the world-map transform helper using the current mode.
 * @param translation Translation passed to the selected helper.
 * @param rotation Rotation passed to the selected helper.
 */
void func_8006CFA8(VECTOR* translation, SVECTOR* rotation)
{
    if (D_800D923C != 0)
    {
        func_8006ADD0(translation, rotation);
        return;
    }
    func_8006D1AC(translation, rotation);
}

/**
 * @brief Forward six arguments with the final option cleared.
 * @param arg0 First forwarded argument.
 * @param arg1 Second forwarded argument.
 * @param arg2 Third forwarded argument.
 * @param arg3 Fourth forwarded argument.
 * @param arg4 Fifth forwarded argument.
 * @param arg5 Sixth forwarded argument.
 */
void func_8006CFE4(void* arg0, void* arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    func_8006D014(arg0, arg1, arg2, arg3, arg4, arg5, 0);
}

/**
 * @brief Copy the global effect parameters into the descriptor and draw it.
 * @param actor Actor configuration passed to the drawing helper.
 * @param resource Resource slot passed to the drawing helper.
 * @param arg2 TODO: drawing parameter meaning unknown.
 * @param arg3 TODO: drawing parameter meaning unknown.
 * @param arg4 TODO: drawing parameter meaning unknown.
 * @param arg5 TODO: drawing parameter meaning unknown.
 * @param arg6 TODO: drawing parameter meaning unknown.
 */
void func_8006D014(void* actor, void* resource, s32 arg2, s32 arg3, s32 arg4, s32 arg5, s32 arg6)
{
    D_80139280[1] = D_80139234;
    D_80139280[2] = D_8013923C;
    D_80139280[3] = D_80139240;
    D_80139280[4] = D_8013924C;
    D_80139280[5] = D_80139250;
    D_80139280[6] = D_80139260;
    D_80139280[7] = D_80139264;
    D_80139280[8] = D_80139268;
    D_80139280[9] = D_8013926C;
    D_80139280[10] = D_80139284;
    func_8006A2FC(actor, resource, arg2, arg3, arg4, arg5, arg6, D_80139280);
}

/**
 * @brief Find a value in the six-by-six world-map record table.
 * @param value Value to find.
 * @param row_out Receives the first matching row.
 * @param column_out Receives the first matching column.
 * @return One if found, otherwise zero; outputs are unchanged on failure.
 */
s32 func_8006D0F0(s32 value, s32* row_out, s32* column_out)
{
    s32 row;
    s32 column;
    for (column = 0; column < 6; column++)
    {
        for (row = 0; row < 6; row++)
        {
            if (D_80139290[row][column].value == value)
            {
                *row_out = row;
                *column_out = column;
                return 1;
            }
        }
    }
    return 0;
}

/**
 * @brief Install a rotation matrix with the world-map translation.
 * @param rotation Rotation angles used to build the matrix.
 */
void func_8006D150(SVECTOR* rotation)
{
    MATRIX matrix;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}

/**
 * @brief Set the world-map coordinate pair to its default position.
 */
void func_8006D190(void)
{
    D_8011CF4C.x = 0xA4;
    D_8011CF4C.y = 0x69;
}

/**
 * @brief Compose the global and local transforms and install the result in the GTE.
 * @param translation Translation for the global rotation matrix.
 * @param rotation Local rotation angles.
 */
static void func_8006D1AC(VECTOR* translation, SVECTOR* rotation)
{
    MATRIX matrices[2];

    RotMatrix(&D_80139278, &matrices[0]);
    TransMatrix(&matrices[0], translation);
    SetRotMatrix(&matrices[0]);
    SetTransMatrix(&matrices[0]);
    RotMatrix(rotation, &matrices[1]);
    TransMatrix(&matrices[1], &D_8011CF60);
    CompMatrix(&matrices[0], &matrices[1], &matrices[1]);
    SetRotMatrix(&matrices[1]);
    SetTransMatrix(&matrices[1]);
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
s32 func_8006D244(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B10A0 = 1;
        D_801B10A4 = 1;
        return 1;
    }

    if (D_801B10A0 < 0x4)
    {
        D_800D0FAC[D_801B10A0]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006D2BC(void)
{
    D_801B10A0 = 1;
    D_801B10A4 = 1;
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8006D2D4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B10A0 += 1;
        func_8006D310();
    }
}

/**
 * @brief Increment a world-map state counter.
 */
void func_8006D310(void)
{
    D_801B10A0 += 1;
}

/**
 * @brief Dispatch the current world-map sequence step, or reset it.
 * @param arg0 Non-zero forces a reset of the step counters.
 * @return 1 if a step ran or reset, 0 if the step index was out of range.
 */
static s32 func_8006D328(s32 arg0)
{
    s32 result;

    if (arg0 != 0)
    {
        D_801B10A8 = 1;
        D_801B10AC = 1;
        return 1;
    }

    if (D_801B10A8 < 0x6)
    {
        D_800D0FBC[D_801B10A8]();
        result = 1;
    }
    else
    {
        result = 0;
    }
    return result;
}

/**
 * @brief Set two adjacent world-map state flags.
 */
void func_8006D3A0(void)
{
    D_801B10A8 = 1;
    D_801B10AC = 1;
}

/** @brief World-map step handler: bump the step counter and run the next step. */
void func_8006D3B8(void)
{
    D_801B10A8 += 1;
    func_8006D3E4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_8006D3E4(void)
{
    if (D_8011CF44 == 0)
    {
        D_801B10A8 += 1;
        func_8006D420();
    }
}

/** @brief Compute the map-coordinate offset and advance the sequence. */
void func_8006D420(void)
{

    D_801398D0 = 2;
    D_80182D68 = D_80139950.x - ((D_800DCEF8 - 1) * 0x30);
    D_80182D78 = D_80139950.y - ((D_800DCF00 - 1) * 0x30);
    D_801B10A8 += 1;
    func_8006D4B0();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_8006D4B0(void)
{
    if (D_801398D0 != 2)
    {
        D_801B10A8 += 1;
        func_8006D4F0();
    }
}

/** @brief Clear the world-map flag, run setup, and advance the state. */
void func_8006D4F0(void)
{
    D_8013B208 = 0;
    func_80064094();
    D_801B10A8 += 1;
}
