#include "common.h"

typedef struct
{
    u8 pad0[0x4];
    s32 unk4;
    u8 pad8[0x1C - 0x8];
    s32 unk1C;
    u8 pad20[0x21 - 0x20];
    u8 unk21;
    u8 pad22[0x24 - 0x22];
    u8 unk24;
    u8 pad25[0x27 - 0x25];
    u8 unk27;
    u8 unk28;
    u8 pad29[0x2A - 0x29];
    s16 unk2A;
    u8 pad2C[0x2E - 0x2C];
    u16 unk2E;
    u8 pad30[0x3A - 0x30];
    u8 unk3A;
} FieldActorRecord;

typedef struct
{
    u8 pad0[0x260];
    s16 unk260;
    u8 pad262[0x268 - 0x262];
} FieldRecord800FD818;

typedef struct
{
    u8 pad0[0x174];
    u32 unk174;
    u8 pad178[0x23C - 0x178];
} FieldAnimationRecord;

typedef struct
{
    u8 pad0[0x244];
} FieldActorSlot;

extern FieldRecord800FD818 D_800FD818[];
extern FieldAnimationRecord D_80105AE0[];
extern FieldActorSlot D_800FB3C8[];

void func_8008BC5C(FieldActorRecord *record);
void func_8008BCF8(FieldActorRecord *record);
void func_8006C3FC(FieldActorRecord *record);
void field_stop_actor_animations_for_object(FieldActorRecord *record, s32 force);
void func_80083BC0(FieldActorRecord *record, void *slot, s32 force);
void func_800952DC(FieldActorRecord *record, s32 value);
void func_80084424(s32 index);

/**
 * @brief Reset an actor record and reinitialize its associated animation state.
 * @param record Actor record to reset.
 * @param clear_slot Nonzero to clear the associated D_800FD818 entry when its index is below three.
 */
void func_8008BE38(FieldActorRecord *record, s32 clear_slot)
{
    FieldAnimationRecord *animation_base;
    FieldAnimationRecord *animation;
    s32 animation_mask;

    func_8008BC5C(record);
    func_8008BCF8(record);
    record->unk2A = 0x8E;
    if (clear_slot != 0 && record->unk3A < 3)
    {
        D_800FD818[record->unk3A].unk260 = 0;
    }

    record->unk28 = 0xFF;
    animation_base = D_80105AE0;
    record->unk2E = 1;
    record->unk24 = 1;
    record->unk4 = 0;
    record->unk27 = 0;
    record->unk21 = (record->unk21 & 0x80) + 0x1D;

    animation = &animation_base[record->unk3A];
    animation_mask = -0x1801;
    animation->unk174 &= animation_mask;
    func_8006C3FC(record);

    field_stop_actor_animations_for_object(record, 1);
    func_80083BC0(record, &D_800FB3C8[record->unk3A], 1);
    record->unk1C |= 0x800;
    func_800952DC(record, 0);
    func_80084424(record->unk3A);
}
