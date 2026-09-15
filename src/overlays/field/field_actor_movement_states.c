/** @file field_actor_movement_states.c
 * @brief Dispatch actor movement and trigger states and start checked animations.
 */

/* field_actor_movement_states */
/* func_80092AD8 */
#include "common.h"

/**
 * @brief Record fields consumed by the FIELD movement-state update.
 */
typedef struct
{
    u8 pad0[4];
    s32 state_value;
    u8 pad8[0x21 - 8];
    u8 state_flags;
    u8 pad22[0x54 - 0x22];
} FieldStateRecord;

void func_8006C3FC();
void func_80092C24(u8 *rec, s32 arg1);
void func_80097FA0(void *arg0, void *arg1, s32 arg2);

/**
 * @brief Advance selected FIELD movement states and request animation 0x1A.
 * @param entry Record containing the signed state value and state flags.
 * @return Zero while the state is being advanced, or one when it is complete.
 * @note The high state bit selects horizontal displacement direction.
 * @note Local assembly match: 100% with GCC 2.7.2 CDK (83 instructions).
 * @see decomp.me WIP
 */
s32 func_80092AD8(FieldStateRecord *entry)
{
    s32 state_value;
    s32 unused_value;
    s32 timer;
    /**
     * @brief Three-axis displacement stored in scratchpad RAM.
     */
    struct Vector
    {
        s32 x;
        s32 y;
        s32 z;
    } *scratch;

    scratch = (struct Vector *)0x1F800000;
    switch ((u32)(u8)(entry->state_flags & 0x7F) - 8)
    {
        case 0: /* state 8 */
            entry->state_flags = (entry->state_flags & 0x80) | 9;
            func_8006C3FC(entry);
            return 0;
        case 53: /* state 61 */
            state_value = entry->state_value;
            if (state_value < -0xC00)
            {
                entry->state_value = state_value + 0xC00;
                return 0;
            }
            else
            {
                entry->state_value = 0;
                entry->state_flags = (entry->state_flags & 0x80) | 9;
                func_8006C3FC(entry);
                return 0;
            }
        case 64: /* state 72 */
        case 65: /* state 73 */
            state_value = entry->state_value;
            if (state_value < -0xC00)
            {
                if (entry->state_flags & 0x80)
                {
                    scratch->x = 0x200;
                }
                else
                {
                    scratch->x = -0x200;
                }
                scratch->z = 0;
                scratch->y = 0;
                timer = entry->state_value;
                func_80097FA0(entry, scratch, 1);
                timer += 0xC00;
                entry->state_value = timer;
                return 0;
            }
            else if (state_value == 0)
            {
                break;
            }
            else if (state_value < -0xA)
            {
                entry->state_value = -0xA;
                func_80092C24((u8 *)entry, 0x1A);
            }
            else
            {
                entry->state_value = state_value + 1;
            }
            return 0;
        case 1:  /* state 9  */
        case 50: /* state 58 */
        case 51: /* state 59 */
        case 52: /* state 60 */
        case 62: /* state 70 */
        case 63: /* state 71 */
        case 70: /* state 78 */
        case 71: /* state 79 */
            func_80092C24((u8 *)entry, 0x1A);
            return 1;
        default:
            break;
    }
    return 1;
}


/* func_80092C24 */
#include "common.h"

extern s32 func_800839F8(s32 arg0, s32 arg1);
s32 func_80083EEC();
void field_start_actor_animation();

/**
 * @brief Starts an actor's animation when its slot resolves and passes a check.
 *
 * Resolves the actor slot for @p rec's 0x3A id via func_800839F8; if valid and
 * func_80083EEC (given @p arg1) succeeds, starts that slot's animation.
 */
void func_80092C24(u8 *rec, s32 arg1)
{
    s32 v = func_800839F8(rec[0x3A], 0);

    if (v != -1)
    {
        if (func_80083EEC(rec[0x3A], v, arg1))
        {
            field_start_actor_animation(v, 0, 0);
        }
    }
}


/* func_80092C98 */
#include "common.h"

/** @brief Field actor record (0x54 bytes); only the fields this handler touches are named. */

typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 pad8[0x16 - 0x8];
    s16 unk16;
    u8 pad18[0x1C - 0x18];
    s32 unk1C;
    u8 unk20;
    u8 unk21;
    u8 pad22[0x27 - 0x22];
    u8 unk27;
    u8 pad28[0x2A - 0x28];
    s16 unk2A;
    u8 pad2C[0x34 - 0x2C];
    u8 unk34;
    u8 unk35;
    u8 unk36;
    s8 unk37;
    s8 unk38;
    u8 unk39;
    u8 unk3A;
    u8 pad3B[0x54 - 0x3B];
} FieldRec;

/** @brief D_80105AE0 slot record (stride 0x23C). */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
    u8 pad10[0x170 - 0x10];
    u8 unk170;
    u8 pad171[0x178 - 0x171];
    u32 unk178;
    u8 pad17C[0x23C - 0x17C];
} Slot23C;

/** @brief D_800FD818 object entry (stride 0x268). */
typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 pad2[0x268 - 0x2];
} Entry268;

/** @brief D_8010A038 animation record (stride 0x190); unk5A is written as both a u16 and its low byte. */
typedef struct
{
    u16 unk0;
    u8 pad2[0x8 - 0x2];
    u16 unk8;
    u8 padA[0x58 - 0xA];
    u16 unk58;
    union
    {
        u16 h;
        struct
        {
            u8 lo;
            u8 hi;
        } b;
    } unk5A;
    u16 unk5C;
    u16 unk5E;
    u8 pad60[0x190 - 0x60];
} Anim190;

extern Slot23C D_80105AE0[];
extern Entry268 D_800FD818[];
extern Anim190 D_8010A038[];

void field_start_actor_animation();
void func_8006C3FC();
s32 func_800839F8(s32 arg0, s32 arg1);
s32 func_80083EEC();
void func_8008A9D8(s32 arg0, s32 arg1, s32 arg2);
s32 func_8008AABC(s32 a, s32 b);
void func_8008BC5C(FieldRec *rec);
void func_8008E690(FieldRec *rec);
s32 func_80091728(u8 index, s32 kind, FieldRec *rec);
s32 func_80091914(FieldRec *rec, u8 index);
void func_80096334(FieldRec *rec);
void func_800A2DD8(u8 index);

/** @brief Program the animation record for object @p idx (fields 0x58..0x5E). */
#define SET_ANIM(idx, v58, v5C, v5E)                                  \
    D_8010A038[idx].unk5A.h &= 0xFBFF;                                \
    D_8010A038[idx].unk58 = v58;                                      \
    D_8010A038[idx].unk5A.b.lo = 0xFF;                                \
    D_8010A038[idx].unk5C = v5C;                                      \
    D_8010A038[idx].unk5E = v5E;                                      \
    D_8010A038[idx].unk5A.h &= 0xFCFF;

/** @brief Interpolated step offset (unk37..unk38 scaled by unk34/unk35), in 1/256 units. */
#define STEP_OFFSET(rec) \
    ((rec->unk37 + (rec->unk38 - rec->unk37) * rec->unk34 / rec->unk35) << 8)

/**
 * @brief Per-frame state handler for a field actor's opcode 0x86 / trigger-kind states.
 *
 * With no pending flags in unk1C, first resolves the 0x3D transition when the
 * current animation matches, then dispatches on the opcode (unk21 & 0x7F) by
 * trigger kind (func_80091728 kinds 3, 1/0, 2), programming the D_8010A038
 * animation record and queueing the follow-up state via func_8008E690.
 *
 * @param rec Field actor record.
 * @return Never set; the declared non-void return keeps v0 live at the epilogue,
 *         which is what the original codegen shows (all exits are bare returns).
 * @see decomp.me (100%) TODO
 */
s32 func_80092C98(FieldRec *rec)
{
    s32 targets;
    s32 tmp;
    s32 anim;
    s32 anim_id;
    s32 index;

    if (rec->unk1C & 0x1FF)
    {
        return;
    }
    if (rec->unk2A == 0x86)
    {
        tmp = rec->unk21 & 0x7F;
        if (tmp == 0x3D)
        {
            anim = func_80091914(rec, rec->unk3A);
            if (D_8010A038[rec->unk3A].unk8 == tmp && anim == 0x185)
            {
                rec->unk2A = anim;
                rec->unk4 -= STEP_OFFSET(rec);
                func_8008E690(rec);
                func_800A2DD8(rec->unk3A);
                rec->unk2A = 0x9B;
                return;
            }
            else if (D_8010A038[rec->unk3A].unk0 == 0x3D && anim == 0x85)
            {
                rec->unk2A = anim;
                rec->unk4 -= STEP_OFFSET(rec);
                func_8008E690(rec);
                func_800A2DD8(rec->unk3A);
                rec->unk2A = 0x9B;
                return;
            }
        }
    }
    if (func_80091728(rec->unk3A, 3, rec) != 0)
    {
        switch (rec->unk21 & 0x7F)
        {
        case 0x2F:
        case 0x44:
            rec->unk2A = 0x885;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x3E:
            rec->unk2A = 0xA85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x38:
            rec->unk2A = 0xA85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x3A:
            SET_ANIM(rec->unk3A, 0x4F, 0x25, 0);
            rec->unk2A = 0xB85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x39:
            SET_ANIM(rec->unk3A, 0x4F, 0x25, 0);
            rec->unk2A = 0xB85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x34:
            SET_ANIM(rec->unk3A, 0x51, 0x27, 0);
            rec->unk2A = 0xB85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
            break;
        case 0x8:
        case 0x3B:
        case 0x3C:
        case 0x3D:
            if (rec->unk27 < 3)
            {
                return;
            }
            rec->unk21 = (rec->unk21 & 0x80) | 0x49;
            rec->unk4 -= STEP_OFFSET(rec);
            func_8006C3FC(rec);
            func_800A2DD8(rec->unk3A);
            rec->unk2A = 0x96;
            rec->unk16 = 1;
            rec->unk34 = 1;
            rec->unk35 = 1;
            return;
        case 0x35:
            if ((D_80105AE0[rec->unk3A].unk178 >> 1) & 1)
            {
                D_80105AE0[D_80105AE0[rec->unk3A].unk170].unkC &= ~0x2000;
                rec->unk2A = 0;
                func_80096334(rec);
                tmp = func_800839F8(rec->unk3A, 0);
                if (tmp != -1)
                {
                    if (func_8008AABC(rec->unk3A, D_80105AE0[rec->unk3A].unk170) != 0)
                    {
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(rec->unk3A, D_80105AE0[rec->unk3A].unk170, 0xD);
                        }
                        else
                        {
                            func_8008A9D8(rec->unk3A, D_80105AE0[rec->unk3A].unk170, 0xC);
                        }
                        index = rec->unk3A;
                        anim_id = 0x64;
                        if (D_800FD818[index].unk1 == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(D_80105AE0[rec->unk3A].unk170, rec->unk3A, 0x18);
                        }
                        else
                        {
                            func_8008A9D8(D_80105AE0[rec->unk3A].unk170, rec->unk3A, 0x17);
                        }
                        index = rec->unk3A;
                        anim_id = 0x65;
                        if (D_800FD818[index].unk1 == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (func_80083EEC(index, tmp, anim_id) != 0)
                    {
                        targets = D_80105AE0[rec->unk3A].unk170;
                        field_start_actor_animation(tmp, 1, &targets);
                    }
                    func_800A2DD8(rec->unk3A);
                }
                func_8008BC5C(rec);
            }
            return;
        default:
            return;
        }
    }
    else if (func_80091728(rec->unk3A, 1, rec) != 0 || func_80091728(rec->unk3A, 0, rec) != 0)
    {
        tmp = func_80091728(rec->unk3A, 1, rec) != 0;
        switch (rec->unk21 & 0x7F)
        {
        case 0x25:
        {
            s32 base;
            s32 actor_offset;
            s32 track_offset;
            s32 offset;
            base = (s32)D_8010A038;
            track_offset = tmp * 8;
            actor_offset = rec->unk3A * 0x190;
            offset = track_offset + actor_offset + base;
            if (*(u16 *)offset == 8 || *(u16 *)offset == 0x3C)
            {
                rec->unk2A = 0x985;
                func_8008E690(rec);
                func_800A2DD8(rec->unk3A);
            }
            break;
        }
        case 0x31:
        {
            s32 base;
            s32 track_offset;
            s32 actor_offset;
            s32 offset;
            base = (s32)D_8010A038;
            track_offset = tmp * 8;
            actor_offset = rec->unk3A * 0x190;
            offset = track_offset + actor_offset + base;
            if (*(u16 *)offset == 8)
            {
                SET_ANIM(rec->unk3A, 0x3C, 0, 1);
                rec->unk2A = 0xB85;
                func_8008E690(rec);
                func_800A2DD8(rec->unk3A);
            }
            break;
        }
        }
    }
    else if (func_80091728(rec->unk3A, 2, rec) != 0)
    {
        if ((rec->unk21 & ~0x80) == 0x34)
        {
            SET_ANIM(rec->unk3A, 0x50, 0x26, 0);
            rec->unk2A = 0xB85;
            func_8008E690(rec);
            func_800A2DD8(rec->unk3A);
        }
        if ((rec->unk21 & ~0x80) == 0x35)
        {
            if ((D_80105AE0[rec->unk3A].unk178 >> 1) & 1)
            {
                D_80105AE0[D_80105AE0[rec->unk3A].unk170].unkC &= ~0x2000;
                rec->unk2A = 0;
                func_80096334(rec);
                tmp = func_800839F8(rec->unk3A, 0);
                if (tmp != -1)
                {
                    if (func_8008AABC(rec->unk3A, D_80105AE0[rec->unk3A].unk170) != 0)
                    {
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(rec->unk3A, D_80105AE0[rec->unk3A].unk170, 0xD);
                        }
                        else
                        {
                            func_8008A9D8(rec->unk3A, D_80105AE0[rec->unk3A].unk170, 0xC);
                        }
                        index = rec->unk3A;
                        anim_id = 0x64;
                        if (D_800FD818[index].unk1 == 8)
                        {
                            anim_id = 0x61;
                        }
                    }
                    else
                    {
                        if (D_800FD818[rec->unk3A].unk1 == 8)
                        {
                            func_8008A9D8(D_80105AE0[rec->unk3A].unk170, rec->unk3A, 0x18);
                        }
                        else
                        {
                            func_8008A9D8(D_80105AE0[rec->unk3A].unk170, rec->unk3A, 0x17);
                        }
                        index = rec->unk3A;
                        anim_id = 0x65;
                        if (D_800FD818[index].unk1 == 8)
                        {
                            anim_id = 0x63;
                        }
                    }
                    if (func_80083EEC(index, tmp, anim_id) != 0)
                    {
                        targets = D_80105AE0[rec->unk3A].unk170;
                        field_start_actor_animation(tmp, 1, &targets);
                    }
                }
            }
            func_800A2DD8(rec->unk3A);
        }
    }
}
