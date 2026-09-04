#include "common.h"
#include "sdk/libgte.h"
#include "sdk/libgpu.h"

extern s32 D_800F22A0;
extern s32 D_800F22A4;
extern s32 D_800F22A8;
extern u16 D_801178D8;

#define PROJECT_POINT(_poly, _vert, _tmp) \
    (_poly)->x##_vert = (s16)(0xA0 + D_800F22A0 / 0x100 + (_tmp).vx / 0x100); \
    (_poly)->y##_vert = (s16)(0x70 + D_800F22A4 / 0x100 + (_tmp).vy / 0x100 - (_tmp).vz / 0x200 - D_800F22A8 / 0x200)

#define POLY_AT(_off) ((POLY_G4 *)(primbuf + (_off)))

#define ADD_DEPTH_ADVANCE(_depth, _expr, _type) \
    if ((_depth) < 0) { \
        addPrim(&base[0], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } else if ((_depth) >= 0x1000) { \
        addPrim(&base[0xFFF], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    } else { \
        addPrim(&base[(_expr)], (_type *)primbuf); \
        primbuf += sizeof(_type); \
    }

u8 *func_8009E66C(s32 *base, u8 *arg1, VECTOR *pos, s32 radius)
{
    VECTOR v[6];
    SVECTOR rot;
    MATRIX m0;
    MATRIX m1;
    s32 i;
    s32 x;
    s32 y;
    s32 initial_x;
    s32 initial_y;
    s32 depth;
    u32 center;
    u8 *primbuf;

    primbuf = arg1;

    rot.vx = 0;
    rot.vz = 0;
    rot.vy = D_801178D8;
    RotMatrix_gte(&rot, &m0);

    initial_x = (rcos(0) >> 4) * radius;
    initial_y = (rsin(0) >> 4) * radius;

    rot.vx = 0;
    rot.vz = 0;
    rot.vy = D_801178D8 + 0x180;
    RotMatrix_gte(&rot, &m1);

    v[0].vx = initial_x;
    v[0].vy = initial_y;
    v[0].vz = 0;
    ApplyMatrixLV(&m0, &v[0], &v[1]);
    v[0].vx = initial_x;
    v[0].vy = initial_y;
    v[0].vz = 0;
    ApplyMatrixLV(&m1, &v[0], &v[4]);

    v[0].vx = pos->vx + v[1].vx;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz + v[1].vz;
    PROJECT_POINT(POLY_AT(0x0), 0, v[0]);

    v[0].vx = pos->vx - v[1].vx;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz - v[1].vz;
    PROJECT_POINT(POLY_AT(0x24), 0, v[0]);

    v[0].vx = pos->vx - v[1].vz;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz + v[1].vx;
    PROJECT_POINT(POLY_AT(0x48), 0, v[0]);

    v[0].vx = pos->vx + v[1].vz;
    v[0].vy = pos->vy + v[1].vy;
    v[0].vz = pos->vz - v[1].vx;
    PROJECT_POINT(POLY_AT(0x6C), 0, v[0]);

    v[0].vx = pos->vx;
    v[0].vy = pos->vy;
    v[0].vz = pos->vz;
    PROJECT_POINT(POLY_AT(0x0), 1, v[0]);
    center = *(u32 *)&POLY_AT(0)->x1;
    *(u32 *)&POLY_AT(0)->x3 = center;
    *(u32 *)&POLY_AT(0x6C)->x1 = center;
    *(u32 *)&POLY_AT(0x6C)->x3 = center;
    *(u32 *)&POLY_AT(0x48)->x1 = center;
    *(u32 *)&POLY_AT(0x48)->x3 = center;
    *(u32 *)&POLY_AT(0x24)->x1 = center;
    *(u32 *)&POLY_AT(0x24)->x3 = center;

    v[0].vx = pos->vx + v[4].vx;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz + v[4].vz;
    PROJECT_POINT(POLY_AT(0x0), 2, v[0]);

    v[0].vx = pos->vx - v[4].vx;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz - v[4].vz;
    PROJECT_POINT(POLY_AT(0x24), 2, v[0]);

    v[0].vx = pos->vx - v[4].vz;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz + v[4].vx;
    PROJECT_POINT(POLY_AT(0x48), 2, v[0]);

    v[0].vx = pos->vx + v[4].vz;
    v[0].vy = pos->vy + v[4].vy;
    v[0].vz = pos->vz - v[4].vx;
    PROJECT_POINT(POLY_AT(0x6C), 2, v[0]);

    *(u32 *)&POLY_AT(0x0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
    *(u32 *)&POLY_AT(0x24)->r0 = (((v[1].vz >> 8) + 0xA0) << 8) & 0xFF00;
    *(u32 *)&POLY_AT(0x48)->r0 = ((0xA0 - (v[1].vx >> 8)) << 8) & 0xFF00;
    *(u32 *)&POLY_AT(0x6C)->r0 = (((v[1].vx >> 8) + 0xA0) << 8) & 0xFF00;

    *(u32 *)&POLY_AT(0x6C)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x48)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x24)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x0)->r1 = 0xA000;
    *(u32 *)&POLY_AT(0x24)->r3 = 0;
    *(u32 *)&POLY_AT(0x24)->r2 = 0;
    *(u32 *)&POLY_AT(0x48)->r3 = 0;
    *(u32 *)&POLY_AT(0x48)->r2 = 0;
    *(u32 *)&POLY_AT(0x6C)->r3 = 0;
    *(u32 *)&POLY_AT(0x6C)->r2 = 0;
    *(u32 *)&POLY_AT(0x0)->r3 = 0;
    *(u32 *)&POLY_AT(0x0)->r2 = 0;

    SetPolyG4(POLY_AT(0x0));
    SetPolyG4(POLY_AT(0x24));
    SetPolyG4(POLY_AT(0x48));
    SetPolyG4(POLY_AT(0x6C));
    setSemiTrans(POLY_AT(0x0), 1);
    setSemiTrans(POLY_AT(0x24), 1);
    setSemiTrans(POLY_AT(0x48), 1);
    setSemiTrans(POLY_AT(0x6C), 1);

    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);
    depth = pos->vz >> 7;
    ADD_DEPTH_ADVANCE(depth, pos->vz >> 7, POLY_G4);

    setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
    depth = (pos->vz + v[1].vz) >> 7;
    ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

    i = 1;
    do {
        x = (rcos(i << 8) >> 4) * radius;
        y = -(rsin(i << 8) >> 4) * radius;
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m0, &v[0], &v[2]);
        v[0].vx = x;
        v[0].vy = y;
        v[0].vz = 0;
        ApplyMatrixLV(&m1, &v[0], &v[5]);

        v[0].vx = pos->vx + v[1].vx;
        v[0].vy = pos->vy + v[1].vy;
        v[0].vz = pos->vz + v[1].vz;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = pos->vx + v[2].vx;
        v[0].vy = pos->vy + v[2].vy;
        v[0].vz = pos->vz + v[2].vz;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = pos->vx + v[4].vx;
        v[0].vy = pos->vy + v[4].vy;
        v[0].vz = pos->vz + v[4].vz;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = pos->vx + v[5].vx;
        v[0].vy = pos->vy + v[5].vy;
        v[0].vz = pos->vz + v[5].vz;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);

        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, POLY_G4);

        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

        v[0].vx = pos->vx - v[1].vz;
        v[0].vy = pos->vy + v[1].vy;
        v[0].vz = pos->vz + v[1].vx;
        PROJECT_POINT(POLY_AT(0), 0, v[0]);
        v[0].vx = pos->vx - v[2].vz;
        v[0].vy = pos->vy + v[2].vy;
        v[0].vz = pos->vz + v[2].vx;
        PROJECT_POINT(POLY_AT(0), 1, v[0]);
        v[0].vx = pos->vx - v[4].vz;
        v[0].vy = pos->vy + v[4].vy;
        v[0].vz = pos->vz + v[4].vx;
        PROJECT_POINT(POLY_AT(0), 2, v[0]);
        v[0].vx = pos->vx - v[5].vz;
        v[0].vy = pos->vy + v[5].vy;
        v[0].vz = pos->vz + v[5].vx;
        PROJECT_POINT(POLY_AT(0), 3, v[0]);

        *(u32 *)&POLY_AT(0)->r0 = ((0xA0 - (v[1].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r1 = ((0xA0 - (v[2].vz >> 8)) << 16) & 0xFFFFFF;
        *(u32 *)&POLY_AT(0)->r2 = 0;
        *(u32 *)&POLY_AT(0)->r3 = 0;
        SetPolyG4(POLY_AT(0));
        setSemiTrans(POLY_AT(0), 1);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, POLY_G4);

        setDrawTPage((DR_TPAGE *)primbuf, 0, 0, 0x25);
        depth = (pos->vz + v[1].vz) >> 7;
        ADD_DEPTH_ADVANCE(depth, (pos->vz + v[1].vz) >> 7, DR_TPAGE);

        i++;
        v[1].vx = v[2].vx;
        v[1].vy = v[2].vy;
        v[1].vz = v[2].vz;
        v[4].vx = v[5].vx;
        v[4].vy = v[5].vy;
        v[4].vz = v[5].vz;
    } while (i < 9);

    return primbuf;
}

#undef PROJECT_POINT
#undef POLY_AT
#undef ADD_DEPTH_ADVANCE
