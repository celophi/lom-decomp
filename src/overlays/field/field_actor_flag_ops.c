#include "common.h"

/**
 * @brief Field actor state record. Only the fields read here are known; the
 *        object index at 0x3A selects the actor's D_80105AE0 slot.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 pad1B[1];
    s32 unk1C;
    u8 pad20[1];
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 pad26[1];
    u8 unk27;
    u8 unk28;
    u8 pad29[1];
    s16 unk2A;
    s16 unk2C;
    u16 unk2E;
    s16 unk30;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 pad39[1];
    u8 unk3A;
    u8 unk3B;
    u32 unk3C;
    s32 unk40;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 pad50[4];
} FieldActorState;

/**
 * @brief Per-actor animation/geometry slot in D_80105AE0; stride 0x23C.
 */
typedef struct
{
    u8 pad0[0xC];
    s32 unkC;     /* 0xC flags; bit 0x2000 cleared by the linked-actor helpers */
    u8 pad10[0x170 - 0x10];
    u8 unk170;    /* 0x170 index of the linked actor */
    u8 pad171[0x174 - 0x171];
    u32 unk174;   /* 0x174 */
    u32 unk178;   /* 0x178 bit 1 marks a link to unk170 */
    u8 pad17C[0x23C - 0x17C];
} ActorSlotData;

/** @brief Entry of g_field_actor_slots; stride 0x244. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;   /* 0x24 */
    u8 pad25[0x23A - 0x25];
    u8 unk23A;  /* 0x23A */
    u8 pad23B[0x244 - 0x23B];
} ActorSlot;

/** @brief 0x28-byte record of the D_80107800 table; unk4 is the in-use flag. */
typedef struct
{
    u8 unk0[4];
    u8 unk4;
    u8 unk5[0x23];
} FieldUnkRecord_80086F20;

void func_8006C3FC(FieldActorState *);
s32 func_80083EEC(u8, s32, s32);
void field_start_actor_animation(s32, s32, s32);
void func_80086C00(s32 idx);
void *bcopy(const void *, void *, int);

extern ActorSlotData D_80105AE0[];
extern ActorSlot g_field_actor_slots[];
extern FieldUnkRecord_80086F20 D_80107800[];
extern s16 D_801058E0[];

/**
 * @brief Enter or leave the actor's 0x14 control state.
 *
 * With @p flag set, puts the actor into control state 0x14, keeps its slot
 * flags 0x1800 clear, sets bits 0x40800 of unk1C, and runs func_80086C00 on
 * it. With @p flag clear, resets the control state, plays animation 0x91 on
 * the actor's +0x40 object, and clears bit 0x40000 of unk1C.
 *
 * @param rec Actor state record.
 * @param flag Nonzero to enter the state, zero to leave it.
 */
void func_80086ACC(FieldActorState *rec, s32 flag)
{
    if (flag != 0)
    {
        rec->unk2E = 1;
        rec->unk24 = 1;
        rec->unk27 = 0;
        rec->unk21 = (rec->unk21 & 0x80) + 0x14;
        D_80105AE0[rec->unk3A].unk174 &= ~0x1800;
        func_8006C3FC(rec);
        rec->unk1C |= 0x40800;
        func_80086C00(rec->unk3A);
    }
    else
    {
        rec->unk27 = 0;
        rec->unk2E = 1;
        rec->unk24 = 1;
        rec->unk21 &= 0x80;
        D_80105AE0[rec->unk3A].unk174 &= ~0x1800;
        func_8006C3FC(rec);
        func_80083EEC(rec->unk3A, rec->unk3A + 0x40, 0x91);
        field_start_actor_animation(rec->unk3A + 0x40, 0, 0);
        rec->unk1C &= 0xFFFBFFFF;
    }
}

/**
 * @brief Clears a linked actor's 0x2000 flag when this actor's bit 1 is set.
 *
 * For actor @p idx, if bit 1 of its 0x178 word is set, clears bit 13 (0x2000)
 * of the 0xC flags word belonging to the actor referenced by its 0x170 byte.
 *
 * @param idx Actor slot index.
 */
void func_80086C00(s32 idx)
{
    ActorSlotData *base = D_80105AE0;
    ActorSlotData *e = &base[idx];

    if ((e->unk178 >> 1) & 1)
    {
        ActorSlotData *e2 = &base[e->unk170];
        e2->unkC &= ~0x2000;
    }
}

/**
 * @brief Play animation 0xC or 0xD on the actor's +0x40 object and set unk25.
 *
 * With @p arg1 set, plays 0xC, marks unk25 = 0xFE, and clears the linked
 * actor's 0x2000 flag as func_80086C00 does. With @p arg1 clear, plays 0xD
 * and zeroes unk25.
 *
 * @param arg0 Actor state record.
 * @param arg1 Selects the 0xC (nonzero) or 0xD (zero) path.
 */
void func_80086C70(FieldActorState *arg0, s32 arg1)
{
    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xC);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        arg0->unk25 = 0xFE;
        {
            ActorSlotData *base = D_80105AE0;
            ActorSlotData *e = &base[arg0->unk3A];
            if ((e->unk178 >> 1) & 1)
            {
                ActorSlotData *e2 = &base[e->unk170];
                e2->unkC &= ~0x2000;
            }
        }
    }
    else
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xD);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
        arg0->unk25 = 0;
    }
}

/**
 * @brief Clears a linked actor's 0x2000 flag from a record's actor index.
 *
 * Uses the 0x3A index byte of @p p to select an actor; if bit 1 of its 0x178
 * word is set, clears bit 13 (0x2000) of the 0xC flags word belonging to the
 * actor referenced by its 0x170 byte.
 *
 * @param p Actor state record viewed as bytes.
 */
void func_80086D5C(u8 *p)
{
    ActorSlotData *base = D_80105AE0;
    ActorSlotData *e = &base[p[0x3A]];

    if ((e->unk178 >> 1) & 1)
    {
        ActorSlotData *e2 = &base[e->unk170];
        e2->unkC &= ~0x2000;
    }
}

/**
 * @brief Play animation 0xE on the actor's +0x40 object, or clear its slot bytes 0x24 and 0x23A.
 * @param arg0 Actor state record.
 * @param arg1 Nonzero plays the animation, zero clears the slot bytes.
 * @see decomp.me (100%) TODO
 */
void func_80086DD0(FieldActorState *arg0, s32 arg1)
{
    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0xE);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
    else
    {
        g_field_actor_slots[arg0->unk3A + 0x40].unk24 = 0;
        g_field_actor_slots[arg0->unk3A + 0x40].unk23A = 0;
    }
}

/**
 * @brief Play animation 0x19 on the actor's +0x40 object, or clear its slot bytes 0x24 and 0x23A.
 * @param arg0 Actor state record.
 * @param arg1 Nonzero plays the animation, zero clears the slot bytes.
 * @see decomp.me (100%) TODO
 */
void func_80086E78(FieldActorState *arg0, s32 arg1)
{
    if (arg1 != 0)
    {
        func_80083EEC(arg0->unk3A, arg0->unk3A + 0x40, 0x19);
        field_start_actor_animation(arg0->unk3A + 0x40, 0, 0);
    }
    else
    {
        g_field_actor_slots[arg0->unk3A + 0x40].unk24 = 0;
        g_field_actor_slots[arg0->unk3A + 0x40].unk23A = 0;
    }
}

/**
 * @brief Clear the unk4 flag byte across all 256 records of the D_80107800
 *        table.
 */
void func_80086F20(void)
{
    s32 i;

    for (i = 0xFF; i >= 0; i--)
    {
        D_80107800[i].unk4 = 0;
    }
}

/**
 * @brief Stores a record into the first free slot of the D_80107800 table.
 *
 * Scans up to 256 records for one whose unk4 flag byte is clear, copies 0x28
 * bytes from @p src into it, and writes @p value to the parallel D_801058E0
 * half-word slot.
 *
 * @param src Source record, 0x28 bytes.
 * @param value Halfword stored in the parallel D_801058E0 slot.
 */
void func_80086F48(const void *src, s16 value)
{
    s32 i = 0;
    s16 *slot = D_801058E0;
    FieldUnkRecord_80086F20 *entry = D_80107800;

    for (; i < 0x100; i++)
    {
        if (entry->unk4 == 0)
        {
            bcopy(src, entry, 0x28);
            *slot = value;
            return;
        }
        slot++;
        entry++;
    }
}
