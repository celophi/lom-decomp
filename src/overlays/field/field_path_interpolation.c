#include "common.h"

/**
 * @file field_path_interpolation.c
 * @brief Field effect-point path interpolation: entry generation, weight
 *        evaluation, and knot/offset setup.
 *
 * Groups the six contiguous GCC 2.7.2 CDK routines at 800A1D48..800A22A8 that
 * share the D_80117Exx interpolation-parameter block and the D_801178E8 point
 * table: advance/generate entries, evaluate weights, fill knot offsets, build
 * samples, and evaluate the knot recurrence. func_800A2128 was previously kept
 * as a separate GCC 2.8.0 G0 unit, but it also matches byte-for-byte under this
 * unit's CDK configuration. See docs/decompilation/field-boundaries/map.md.
 */

/** @brief Actor record supplying base coordinates for generated entries. */
typedef struct
{
    s32 unk0;
    u8 pad4[4];
    s32 unk8;
} ArgRec;

/** @brief Field position with the vertical component left unchanged here. */
typedef struct
{
    s32 x;
    s32 y;
    s32 z;
} FieldPosition;

/** @brief Interpolation workspace matching the observed contiguous stack buffers. */
typedef struct
{
    s32 coefficients[22];
    s32 samples[12];
    s32 knots[34];
    s32 span;
} InterpolationWorkspace;

extern s32 D_801178E0;
extern u8 D_801178E8[];
extern s32 D_80117E68;
extern s32 D_80117E6C;
extern s32 D_80117E70;
extern s32 D_80117E74;
extern s32 D_80117E78;
extern s32 D_80117E7C;
extern s32 D_80117E80;
extern s32 D_80117E84;

s32 rand(void);
s32 rcos(s32 angle);
s32 rsin(s32 angle);

/* Forward declarations for routines defined later in this unit. */
void func_800A20DC(s32 *);
void func_800A22A8(s32, s32, s32 *, s32 *, s32 *);
void func_800A2128(s32 (*)[2], s32 *);

/**
 * @brief Advance an actor's animation index and drive one interpolation step.
 * @param arg0 Pointer to the actor's current index byte.
 * @param arg1 Field position that receives the interpolated components.
 * @param arg2 Effect-entry group index.
 */
void func_800A1D48(u8 *arg0, void *arg1, s32 arg2)
{
    u8 value = *arg0;

    if (value >= D_80117E6C)
    {
        *arg0 = value - (u8)D_80117E6C;
    }

    func_800A1F2C(*arg0, arg1, arg2);
}

/**
 * @brief Initialize the field effect entries associated with an actor record.
 * @param arg0 Actor record that supplies the base coordinates.
 * @param arg1 Magnitude used to offset each generated entry.
 * @param arg2 Nonzero to randomize the generated magnitude.
 * @param arg3 Effect-entry group index.
 */
void func_800A1D98(ArgRec *arg0, s32 arg1, s32 arg2, s32 arg3)
{
    s32 i;
    s32 angle;
    s32 mag;
    u8 *base;
    u8 *entry;

    i = 0;
    base = D_801178E8;
    entry = base + arg3 * 0x2C;
    D_80117E80 = 3;
    D_801178E0 = 0x14;
    D_80117E84 = 0xA;
    do
    {
        angle = rand() >> 3;
        if (arg2 != 0)
        {
            mag = (s32)((rand() | 0x4000) * arg1) >> 0xF;
        }
        else
        {
            mag = arg1;
        }
        *(s16 *)(entry + 0) = (s16)(((s32)(rcos(angle) * mag) >> 0xC) + ((s32)arg0->unk0 >> 8));
        i += 1;
        *(s16 *)(entry + 0x16) = (s16)(((s32)(rsin(angle) * mag) >> 0xC) + ((s32)arg0->unk8 >> 8));
        entry += 2;
    } while (i < 0xA);
    D_80117E70 = D_80117E84;
    D_80117E78 = D_80117E84 + D_80117E80;
    D_80117E68 = D_80117E80 + 1;
    D_80117E74 = D_80117E68 >> 1;
    D_80117E7C = D_80117E84 + (D_80117E80 * 2);
    D_80117E6C = (D_801178E0 * D_80117E84) + 1;
}

/**
 * @brief Evaluate a field point group using the generated interpolation weights.
 * @param time Input parameter scaled by D_801178E0.
 * @param position Receives the interpolated x and z components in field units.
 * @param group Index of the point group with a 0x2C-byte stride.
 * @note The moving workspace view retains the samples' 0x58-byte displacement.
 * @note Keep separate stack, map and group bases to preserve address setup.
 * @note GCC 2.7.2 CDK: 100% match, 108 instructions (432 bytes), frame 0x148.
 */
void func_800A1F2C(s32 time, FieldPosition *position, s32 group)
{
    InterpolationWorkspace work;
    s32 last;
    s32 first;
    s32 i;
    s32 x;
    s32 z;
    s32 weight;
    s32 product;
    s32 product_z;
    s32 base;
    s32 group_base;
    s32 map_base;
    u8 *point;
    InterpolationWorkspace *sample;

    func_800A20DC(work.knots);
    func_800A22A8(D_80117E68, (time << 12) / D_801178E0, &work.span, work.knots, work.coefficients);
    func_800A2128((s32 (*)[2])work.coefficients, work.samples);
    last = work.span - D_80117E74 + 1;
    first = last - D_80117E80;
    if (first < 0 || D_80117E70 - 1 < last)
    {
        first = 0;
        last = D_80117E70 - 1;
    }
    x = 0;
    z = 0;
    i = first;
    if (last >= i)
    {
        base = (s32)&work;
        sample = (InterpolationWorkspace *)(i * 4 + base);
        map_base = (s32)D_801178E8;
        group_base = group * 0x2C + map_base;
        point = (u8 *)(i * 2 + group_base);
        do
        {
            product = *(s16 *)point * sample->samples[0];
            product_z = *(s16 *)(point + 0x16) * sample->samples[0];
            x += product >> 12;
            z += product_z >> 12;
            sample = (InterpolationWorkspace *)((u8 *)sample + 4);
            i++;
            point += 2;
        } while (last >= i);
    }
    position->x = x << 8;
    position->z = z << 8;
}

/**
 * @brief Fill an output array with fixed-point offsets derived from field globals.
 * @param out Destination array for the generated 20.12 fixed-point values.
 */
void func_800A20DC(s32 *out)
{
    s32 value;
    s32 base;
    s32 bound;
    s32 i;

    i = 0;
    value = D_80117E7C;
    if (value > 0)
    {
        bound = value;
        value = D_80117E68;
        base = value - 1;
        do
        {
            *out++ = (i - base) << 12;
            i++;
        } while (i < bound);
    }
}

/**
 * @brief Build interpolation samples from the selected coefficient column.
 * @param source Two-word coefficient records.
 * @param output Destination sample array.
 */
void func_800A2128(s32 (*source)[2], s32 *output)
{
    s32 selector;
    s32 index;
    s32 *output_cursor;
    s32 value0;
    s32 value1;

    index = 0;
    if (D_80117E74 > 0)
    {
        s32 count;
        s32 offset;

        count = D_80117E74;
        output_cursor = output;
        selector = (D_80117E68 - 1) & 1;
        offset = D_80117E70;
        do
        {
            value1 = source[index + count - 1][selector];
            value0 = source[index + offset + count - 1][selector];
            index++;
            *output_cursor = value1 + value0;
            output_cursor++;
        } while (index < count);
    }

    index = 0;
    if (D_80117E70 - D_80117E80 > 0)
    {
        s32 output_offset;
        s32 source_offset;
        s32 count;

        output_offset = D_80117E74;
        source_offset = D_80117E80;
        count = D_80117E70 - D_80117E80;
        selector = (D_80117E68 - 1) & 1;
        do
        {
            s32 output_index;
            s32 source_index;

            output_index = index + output_offset;
            source_index = index + source_offset;
            index++;
            output[output_index] = source[source_index][selector];
        } while (index < count);
    }

    index = 0;
    if (D_80117E74 - 1 > 0)
    {
        s32 output_offset;
        s32 count;
        s32 source_offset;

        output_offset = D_80117E74;
        count = D_80117E74 - 1;
        source_offset = D_80117E70;
        selector = (D_80117E68 - 1) & 1;
        do
        {
            s32 source_index;

            source_index = index + source_offset;
            output[source_index - output_offset + 1] = source[source_index][selector] + source[index][selector];
            index++;
        } while (index < count);
    }
}

/**
 * @brief Evaluate fixed-point interpolation weights for a knot sequence.
 * @param order Number of interpolation levels, including the initial span test.
 * @param parameter Parameter to evaluate against the knot sequence.
 * @param span Receives the selected knot span.
 * @param knots Knot values used by the interpolation recurrence.
 * @param workspace Interleaved two-column weight buffer for successive levels.
 * @note Weights use 0x1000 as unity and alternate columns on each level.
 * @note Keep separate knot pairs to preserve the original register allocation.
 * @note GCC 2.7.2 CDK matches all 173 instructions (692 bytes).
 */
void func_800A22A8(s32 order, s32 parameter, s32 *span, s32 *knots, s32 *workspace)
{
    s32 i;
    s32 level;
    s32 left;
    s32 right;
    s32 lower;
    s32 upper;
    s32 second_lower;
    s32 second_upper;

    for (i = 0; i < D_80117E78 + 1; i++)
    {
        ((s32 (*)[2])workspace)[i][0] = 0;
    }
    for (i = 0; i < D_80117E78; i++)
    {
        if (parameter >= knots[i] && parameter < knots[i + 1])
        {
            ((s32 (*)[2])workspace)[i][0] = 0x1000;
            *span = i;
        }
    }
    if (parameter >= knots[D_80117E78 - 1] && parameter <= knots[D_80117E78] + 1)
    {
        ((s32 (*)[2])workspace)[D_80117E78 - 1][0] = 0x1000;
        *span = D_80117E78 - 1;
    }
    for (level = 1; level < order; level++)
    {
        for (i = 0; i < D_80117E78 + 1; i++)
        {
            ((s32 (*)[2])workspace)[i][level & 1] = 0;
        }
        for (i = *span - level; i <= *span; i++)
        {
            right = 0;
            left = right;
            lower = knots[i + 1];
            upper = knots[i + level + 1];
            if (lower != upper)
            {
                right = ((upper - parameter) * ((s32 (*)[2])workspace)[i + 1][(level - 1) & 1]) / (upper - lower);
            }
            second_lower = knots[i];
            second_upper = knots[i + level];
            if (second_lower != second_upper)
            {
                left = ((parameter - second_lower) * ((s32 (*)[2])workspace)[i][(level - 1) & 1]) / (second_upper - second_lower);
            }
            ((s32 (*)[2])workspace)[i][level & 1] = right + left;
        }
    }
}
