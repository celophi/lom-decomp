#include "common.h"

extern s32 D_801178E0;
extern u8 D_801178E8[];
extern s32 D_80117E68;
extern s32 D_80117E70;
extern s32 D_80117E74;
extern s32 D_80117E80;
void func_800A20DC(s32 *);
void func_800A22A8(s32, s32, s32 *, s32 *, s32 *);
void func_800A2128(u8 *, s32 *);

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
    func_800A2128((u8 *)work.coefficients, work.samples);
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
