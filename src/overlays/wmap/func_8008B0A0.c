/* Partial WMAP decompilation: 97.333336% (gcc280_g0). */
#include "common.h"

/** @brief World-map actor configuration. */
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

/** @brief Per-actor motion and animation parameters. */
typedef struct
{
    s16 state;
    s16 angle;
    s32 x;
    s32 z;
    s16 scale;
    s16 field_0E;
    s32 field_10;
} WmapMotion;

/** @brief Animation resource slot. */
typedef struct
{
    s32 field_00;
    void *resource;
} WmapResource;

/** @brief Position and velocity halfwords for an effect particle. */
typedef struct
{
    s16 x, y, z, pad_06;
    s16 vx, vy, vz, pad_0E;
} WmapParticle;
extern WmapConfigA D_800D9268[];
extern WmapParticle D_800E4F18[];
extern WmapResource D_80139988[];
extern u8 D_80121538[];
extern s32 D_801B2A00;
extern s32 D_801B2A04;
extern s32 rand(void);
extern void func_8008C684(void);

/** @brief Initialize particle positions, velocities and animation resources. */
void func_8008B0A0(void)
{
    s32 i;
    WmapConfigA *actor;
    WmapParticle *particle;
    WmapParticle *particles = D_800E4F18;

    for (i = 100; i < 124; i++)
    {
        actor = &D_800D9268[i];
        particle = &particles[i];
        particle->x = ((rand() * 200) >> 15) - 100;
        particle->y = ((rand() * 200) >> 15) - 100;
        particle->z = ((rand() * 150) >> 15) - 150;
        particle->vx = ((rand() << 6) >> 15) - 20;
        particle->vy = ((rand() << 6) >> 15) - 20;
        particle->vz = ((rand() << 6) >> 15) - 20;
        actor->field_02 = 0;
        actor->field_06 = 15;
        actor->field_0E = (rand() * 3) >> 15;
        actor->field_10 = -1;
        actor->field_26 = 4;
        actor->field_22 = 129;
        actor->field_24 = 1;
        D_80139988[i].resource = D_80121538;
    }
    particle = &particles[i];
    particle->x = -100;
    particle->y = -100;
    particle->z = -150;
    particle->vx = 300;
    particle->vy = 300;
    particle->vz = 300;
    D_801B2A04 = 112;
    D_801B2A00++;
    func_8008C684();
}
