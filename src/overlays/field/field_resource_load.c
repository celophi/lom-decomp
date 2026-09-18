#include "common.h"
typedef struct
{
    u8 flags;
    u8 _pad01[0x253];
    u16 resource_index;
    u8 _pad256[0x12];
} FieldResourceSlot;

typedef struct
{
    u8* start;
    u8* end;
    u8 unk8;
    u8 slot_index;
    u8 padA[4];
    s16 unkE;
    u32 flags;
} FieldResourceEntry;

typedef struct
{
    u8 _pad00[0x34];
    u32 unk34;
    u8 _pad38[0x10];
} FieldActorPartDef;

typedef struct
{
    u8 _pad000[0x174];
    s32 unk174;
    u8 _pad178[0x33];
    u8 unk1AB;
    u8 _pad1AC[0x90];
} FieldActorState;

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
    u8 unk1B;
    s32 unk1C;
    u8 pad20[0x21 - 0x20];
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 pad26[0x27 - 0x26];
    u8 unk27;
    u8 unk28;
    u8 pad29[0x2A - 0x29];
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
    u8 pad39[0x3A - 0x39];
    u8 unk3A;
    u8 unk3B;
    u32 unk3C;
    s32 unk40;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Rec80122B28;

Struct_D800FDF58 *func_80087C9C(s32 arg0);

extern s32 D_80122B10;
extern Rec80122B28 D_80122B28[];


extern s32 D_80122B68[];
extern s32 D_80122B10;
extern s32 D_80122B20;

/** @brief Clear pending actors, load state, and both pending resource IDs. */
void func_800B01FC(void)
{
    s32 i = 1;
    s32 *p = &D_80122B68[i];

    D_80122B10 = 0;
    D_80122B20 = 0;

    for (; i >= 0; i--)
    {
        *p = 0;
        p--;
    }
}

extern s32 D_80122B20;

/** @brief Start processing pending actor and resource changes.
 * @return The initial processing state, one.
 */
s32 func_800B0234(void)
{
    return D_80122B20 = 1;
}


extern FieldResourceSlot D_800FD818[];
extern u8 D_800FDF58[];
extern u8 D_800FE3A0[];
extern u8 D_80105880[];
extern u8 D_80105AE0[];
extern s32 D_800FE754;
void func_800B0A08(s32);
s32 func_800B0888(void);
void func_800B08FC(s32, s32);
void func_8006C3FC(u8 *);
s32 func_800839F8(s32, s32);
s32 func_80083EEC(s32, s32, s32);
void field_start_actor_animation(s32, s32, s32);
void func_800A3938(s32, s32);
s32 VSync(s32);
s32 DrawSync(s32);
s32 field_object_has_active_actor_tracks(s32);
void func_80084240(void);
void field_restore_default_action_animation_mappings(s32);
void func_800B34D0(s32);

/** @brief Advance pending actor changes through resource loading and installation. */
void func_800B0244(void)
{
    s32 state;
    s32 i;

    state = D_80122B20;
    if (state == 0)
    {
        goto end;
    }
    if (state == 2)
    {
        goto state_2;
    }
    if (state < 3)
    {
        if (state == 1)
        {
            goto state_1;
        }
        goto end;
    }
    if (state == 3)
    {
        goto state_3;
    }
    if (state == 4)
    {
        goto state_4;
    }
    goto end;

state_1:
    if (((*(s32 *)(D_80105880 + 0x0)) | (*(s32 *)(D_80105880 + 0x1C)) | (*(s32 *)(D_80105880 + 0x38))) == 0)
    {
        D_80122B20 = 2;
    }
    goto end;

state_2:
    func_800B0A08(1);
    D_80122B20 = 3;
    goto end;

state_3:
    {
        Struct_D800FDF58 *object;
        s32 actor_track;
        s32 none;
        u8 flags;
        u8 *actor_base;

        i = 0;
        if (D_80122B10 > 0)
        {
            none = -1;
            actor_base = D_80105AE0;
            do
            {
                object = (Struct_D800FDF58 *)D_800FDF58 + D_80122B28[i].unk0;
                if (D_80122B28[i].unk2 != none)
                {
                    object->unk25 = 0;
                    object->unk2A = 0x8D;
                    flags = (u8)D_80122B28[i].unk2 | (object->unk21 & 0x80);
                    object->unk21 = flags;
                    if (flags & 0x80)
                    {
                        object->unk1B = 0;
                    }
                    else
                    {
                        object->unk1B = 0x80;
                    }
                    object->unk2E = 1;
                    object->unk24 = 1;
                    object->unk1C &= ~0x800;
                    object->unk2C += 3;
                    func_8006C3FC((u8 *)object);
                }
                if (D_80122B28[i].unk4 != none)
                {
                    actor_track = func_800839F8(object->unk3A, 0);
                    if ((actor_track != none) && (func_80083EEC(object->unk3A, actor_track, D_80122B28[i].unk4) != 0))
                    {
                        field_start_actor_animation(actor_track, 0, 0);
                        actor_base[object->unk3A * 0x23C + 0x179] = (u8)actor_track;
                    }
                }
                if (D_80122B28[i].unk6 != none)
                {
                    func_800A3938(D_80122B28[i].unk6, 0x80);
                    VSync(0);
                }
                i += 1;
            } while (i < D_80122B10);
        }
        D_80122B20 = 4;
        goto end;
    }

state_4:
    {
        s16 scan_object_type;
        u8 *resource_slot_scan;
        u8 *object_scan_3;
        u8 *object_scan_13;
        u8 *object_records;
        s32 empty_slot;
        s32 object_type;
        u8 *resource_slots;
        s32 scan_type_a;
        s32 scan_type_b;
        s32 part_mask;

        if (func_800B0888() == 0)
        {
            i = 0;
            if (D_80122B10 > 0)
            {
                do
                {
                    if (field_object_has_active_actor_tracks(D_80122B28[i].unk0) != 0)
                    {
                        break;
                    }
                } while (++i < D_80122B10);
            }
            if (i == D_80122B10)
            {
                i = 3;
                empty_slot = 0xFF;
                object_type = 0x8D;
                object_records = (u8 *)D_800FDF58;
                object_scan_13 = object_records + 0xFC;
loop_34:
                if (((*(u8 *)(object_scan_13 + 0x25)) == empty_slot) || ((*(s16 *)(object_scan_13 + 0x2A)) != object_type))
                {
                    i += 1;
                    object_scan_13 += 0x54;
                    if (i < 0xD)
                    {
                        goto loop_34;
                    }
                }
                if (i == 0xD)
                {
                    i = 1;
                    scan_type_a = 0xAF;
                    scan_type_b = 0xB1;
                    object_records = (u8 *)D_800FDF58;
                    object_scan_3 = object_records + 0x54;
                    resource_slots = (u8 *)D_800FD818;
                    resource_slot_scan = resource_slots + 0x268;
loop_40:
                    if (*resource_slot_scan & 1)
                    {
                        scan_object_type = (*(s16 *)(object_scan_3 + 0x2A));
                        if ((scan_object_type == scan_type_a) || (scan_object_type == scan_type_b))
                        {
                            goto block_44;
                        }
                    }
                    object_scan_3 += 0x54;
                    i += 1;
                    resource_slot_scan += 0x268;
                    if (i < 3)
                    {
                        goto loop_40;
                    }
block_44:
                    if (i == 3)
                    {
                        i = 0;
                        part_mask = 0xFF7FFFFF;
                        do
                        {
                            ((FieldActorPartDef *)D_800FE3A0)[i].unk34 &= part_mask;
                            ((Struct_D800FDF58 *)D_800FDF58)[i].unk2A = 0;
                            i += 1;
                        } while (i < 3);
                        func_800A3938(0x79, 0x80);
                        i = 0;
                        do
                        {
                            if (D_80122B68[i] != 0)
                            {
                                func_800B08FC(1, i);
                                ((Struct_D800FDF58 *)D_800FDF58)[i].unk2A = 0x99;
                                ((Struct_D800FDF58 *)D_800FDF58)[i].unk2E = 1;
                                ((Struct_D800FDF58 *)D_800FDF58)[i].unk27 = 0;
                                ((Struct_D800FDF58 *)D_800FDF58)[i].unk24 = 1;
                                ((Struct_D800FDF58 *)D_800FDF58)[i].unk21 = (((Struct_D800FDF58 *)D_800FDF58)[i].unk21 & 0x80) + 0x11;
                                ((Struct_D800FDF58 *)D_800FDF58)[i].unk1C &= ~0x800;
                                ((FieldActorState *)D_80105AE0)[i].unk174 &= ~0x1800;
                                func_8006C3FC((u8 *)&((Struct_D800FDF58 *)D_800FDF58)[i]);
                            }
                            i += 1;
                        } while (i < 2);
                        i = 0;
                        do
                        {
                            if (D_800FD818[i].flags & 1)
                            {
                                ((FieldActorState *)D_80105AE0)[i].unk1AB = 0x3C;
                                ((FieldActorState *)D_80105AE0)[i].unk174 |= 0x8000;
                            }
                            i += 1;
                        } while (i < 3);
                        DrawSync(0);
                        func_80084240();
                        field_restore_default_action_animation_mappings(1);
                        func_800B34D0(D_800FE754);
                        D_80122B20 = 0;
                    }
                }
            }
        }
    }

end:
    return;
}

/**
 * @brief Update an existing pending actor entry or append a new one.
 * @param arg0 Actor identifier used to resolve the source record.
 * @param arg1 Value stored in the entry's second field.
 * @param arg2 Value stored in the entry's third field and used to select its reset value.
 * @param arg3 Value stored in the entry's fourth field.
 * @return Zero on success, or -1 when no entry can be created.
 */
s32 func_800B0710(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    Struct_D800FDF58 *rec;
    s32 i;
    Rec80122B28 *p;

    if (D_80122B10 == 8)
    {
        return -1;
    }
    rec = func_80087C9C(arg0);
    if (rec == (Struct_D800FDF58 *)-1)
    {
        return -1;
    }
    if (rec->unk25 == 0xFF)
    {
        return -1;
    }
    i = 0;
    if (D_80122B10 > 0)
    {
        do
        {
            p = &D_80122B28[i];
            if (p->unk0 == rec->unk3A)
            {
                p->unk0 = (s16)rec->unk3A;
                p->unk2 = arg1;
                p->unk4 = arg2;
                if (arg2 != -1)
                {
                    p->unk2 = 0;
                }
                p->unk6 = arg3;
                return 0;
            }
            i += 1;
        } while (i < D_80122B10);
    }
    {
        Rec80122B28 *base = D_80122B28;
        s32 idx = D_80122B10;

        p = &base[idx];
    }
    p->unk0 = (s16)rec->unk3A;
    p->unk2 = arg1;
    p->unk4 = arg2;
    if (arg2 != -1)
    {
        p->unk2 = 0xA;
    }
    D_80122B28[D_80122B10].unk6 = arg3;
    D_80122B10 += 1;
    return 0;
}


extern s32 D_80122B68[];

/** @brief Check whether either field resource slot is active.
 * @return One if a resource is active, otherwise zero.
 */
s32 func_800B0850(void)
{
    s32 i;

    for (i = 0; i < 2; i++)
    {
        if (D_80122B68[i] != 0)
        {
            return 1;
        }
    }

    return 0;
}

extern s32 cdrom_can_queue_resource(s32);

extern s32 D_80122B68[];

/**
 * @brief Checks whether either active field resource is already queued.
 *
 * @return 1 if an active resource is already queued, otherwise 0.
 */
s32 func_800B0888(void)
{
    s32 *resource_index;
    s32 i;

    i = 0;
    resource_index = D_80122B68;
    do
    {
        if (*resource_index != 0)
        {
            if (cdrom_can_queue_resource((u16)*resource_index) == 0)
            {
                return 1;
            }
        }
        i++;
        resource_index++;
    } while (i < 2);

    return 0;
}

extern u8 *D_8010D038;
extern s32 D_80122B18[];
extern s32 g_field_resource_cursor;
extern FieldResourceEntry g_field_resource_entries[];
void func_8006B354(s32);
void func_8006CB6C(u8 *, s32, s32, s32);
void func_8009C434(void);

/**
 * @brief Install a queued resource and record its allocated memory range.
 * @param arg0 Flags whose low bit selects the resource state.
 * @param arg1 Resource slot index.
 */
void func_800B08FC(s32 arg0, s32 arg1)
{
    FieldResourceEntry* entry;
    FieldResourceEntry* base;
    u32 flags;

    if (D_80122B68[arg1] != 0)
    {
        func_8006B354(arg1);
        base = g_field_resource_entries;
        entry = base + arg1;
        entry->slot_index = (u8)arg1;
        entry->unk8 = 0;
        func_8009C434();
        entry->unkE = 0x2F;
        flags = entry->flags;
        flags &= ~1;
        flags |= arg0 & 1;
        entry->flags = flags;
        entry->start = (u8*)g_field_resource_cursor;
        func_8006CB6C(D_8010D038 + (0x8000 + arg1 * 0x18000), D_80122B18[arg1], arg1, arg1);
        entry->end = (u8*)g_field_resource_cursor;
        entry->flags |= 2;
        D_80122B68[arg1] = 0;
    }
}

extern s32 cdrom_queue_read(s32, void *);



extern FieldResourceSlot D_800FD818[];
extern s32 D_80122B68[];
extern s32 D_80122B18[];
extern u8* D_8010D038;

s32 func_8006A88C(s32 slot_index, FieldResourceSlot* slot, s32 mode);

/**
 * @brief Queue CD reads for each active field resource slot.
 * @param arg0 Resource-selection mode forwarded to func_8006A88C.
 */
void func_800B0A08(s32 arg0)
{
    s32 i;
    u8* buffer;

    for (i = 0; i < 2; i++)
    {
        if (D_800FD818[i].flags & 1)
        {
            D_80122B68[i] = func_8006A88C(i, &D_800FD818[i], arg0);
            buffer = D_8010D038 + 0x8000 + i * 0x18000;
            D_800FD818[i].resource_index = (u16)D_80122B68[i];
            D_80122B18[i] = cdrom_queue_read((u16)D_80122B68[i], buffer);
        }
        else
        {
            D_80122B18[i] = 0;
            D_80122B68[i] = 0;
        }
    }
}
