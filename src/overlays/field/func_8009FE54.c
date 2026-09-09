#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern s32 D_801178D8;

#define PROJECT_POINT(_poly, _vert, _tmp) \
    (_poly)->x##_vert = (s16)(0xA0 + D_800F22A0 / 0x100 + (_tmp).vx / 0x100); \
    (_poly)->y##_vert = (s16)(0x70 + D_800F22A4 / 0x100 + (_tmp).vy / 0x100 - (_tmp).vz / 0x200 - D_800F22A8 / 0x200)

#define POLY_AT(_off) ((POLY_G4 *)(primbuf + (_off)))

#define ADD_DEPTH_ADVANCE(_depth, _expr, _type) \
    if ((_depth) < 0) \
    { \
        addPrim(&base[0], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } \
    else if ((_depth) >= 0x1000) \
    { \
        addPrim(&base[0xFFF], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } \
    else \
    { \
        addPrim(&base[(_expr)], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    }

/**
 * @brief Append four rotating strips of shaded, translucent quads.
 * @param base Ordering table with 4096 depth buckets.
 * @param arg1 Next free primitive-buffer byte.
 * @param pos World-space center in fixed-point coordinates.
 * @param radius Radius used to construct the strips.
 * @return First free byte after the appended primitives.
 * @note Matches 100% with gcc272_cdk: 814 instructions, 3256 bytes.
 * @note Keep the vector array and unused matrix to preserve the stack layout.
 */
u8 *func_8009FE54(s32 *base, u8 *arg1, VECTOR *pos, s32 radius)
{
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX m1;
    VECTOR p0;
    VECTOR p1;
    s32 distance;
    s32 i;
    s32 j;
    s32 x;
    s32 y;
    s32 inner_x;
    s32 inner_y;
    s32 angle;
    s32 depth;
    u8 *primbuf;

    primbuf = arg1;
    i = 0;
    do
    {
        distance = (radius >> 1) + 0x40;
        p0.vx = pos->vx;
        p0.vy = pos->vy;
        p0.vz = pos->vz;
        p1.vx = pos->vx;
        p1.vy = pos->vy;
        p1.vz = pos->vz;
        angle = i << 10;
        x = (rcos(angle - D_801178D8) >> 4) * distance;
        y = (rsin(angle - D_801178D8) >> 4) * distance;
        p0.vx += x;
        p0.vz += y;
        x = (rcos(angle - D_801178D8 - 0x100) >> 4) * distance;
        y = (rsin(angle - D_801178D8 - 0x100) >> 4) * distance;
        p1.vx += x;
        p1.vz += y;
        rot.vx = 0;
        rot.vz = 0;
        rot.vy = D_801178D8 + angle;
        RotMatrix_gte(&rot, &m0);
        x = ((rcos(0) >> 4) * radius) >> 1;
        y = ((rsin(0) >> 4) * radius) >> 1;
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[1]);
        v[0].vx = -radius * 0x80;
        v[0].vy = 0;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[2]);
        v[0].vx = p0.vx + v[1].vx;
        v[0].vy = p0.vy + v[1].vy;
        v[0].vz = p0.vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = p0.vx + v[2].vx;
        v[0].vy = p0.vy + v[2].vy;
        v[0].vz = p0.vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = p1.vx + v[1].vx;
        v[0].vy = p1.vy + v[1].vy;
        v[0].vz = p1.vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = p1.vx + v[2].vx;
        v[0].vy = p1.vy + v[2].vy;
        v[0].vz = p1.vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);
        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 8) & 0xFF00;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 8) & 0xFF00;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (p0.vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, POLY_G4);
        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
        depth = (p0.vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, DR_TPAGE);
        j = 1;
        do
        {
            inner_x = ((rcos(j << 8) >> 4) * radius) >> 1;
            inner_y = (-(rsin(j << 8) >> 4) * radius) >> 1;
            v[0].vx = inner_x;
            v[0].vy = inner_y;
            v[0].vz = 0;
            ApplyMatrixLV(&m0, &v[0], &v[2]);
            v[0].vx = p0.vx + v[1].vx;
            v[0].vy = p0.vy + v[1].vy;
            v[0].vz = p0.vz + v[1].vz;
            PROJECT_POINT(POLY_AT(0), 0, v[0]);
            v[0].vx = p0.vx + v[2].vx;
            v[0].vy = p0.vy + v[2].vy;
            v[0].vz = p0.vz + v[2].vz;
            PROJECT_POINT(POLY_AT(0), 1, v[0]);
            v[0].vx = p1.vx + v[1].vx;
            v[0].vy = p1.vy + v[1].vy;
            v[0].vz = p1.vz + v[1].vz;
            PROJECT_POINT(POLY_AT(0), 2, v[0]);
            v[0].vx = p1.vx + v[2].vx;
            v[0].vy = p1.vy + v[2].vy;
            v[0].vz = p1.vz + v[2].vz;
            PROJECT_POINT(POLY_AT(0), 3, v[0]);
            *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
            *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
            *(u32 *)&POLY_AT(0)->r2 = 0;
            *(u32 *)&POLY_AT(0)->r3 = 0;
            SetPolyG4(POLY_AT(0));
            setSemiTrans(POLY_AT(0), 1);
            depth = (p0.vz + v[1].vz) >> 7;
            ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, POLY_G4);
            setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
            depth = (p0.vz + v[1].vz) >> 7;
            ADD_DEPTH_ADVANCE(depth, (p0.vz + v[1].vz) >> 7, DR_TPAGE);
            j++;
            v[1].vx = v[2].vx;
            v[1].vy = v[2].vy;
            v[1].vz = v[2].vz;
        } while (j < 9);
        i++;
    } while (i < 4);
    return primbuf;
}
