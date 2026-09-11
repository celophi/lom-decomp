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
s8 func_800839F8(s32, s32);
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
    u8 *temp_s0;
    u8 *var_a0_2;
    u8 *var_s0_2;
    u8 *var_s1;
    u8 *var_v1_3;
    u8 *var_v1_4;
    s16 *var_s0;
    u8 *var_s3;
    s16 temp_a0;
    s16 temp_v0_3;
    s32 *var_s3_2;
    s32 var_s2;
    s32 var_s2_2;
    s32 var_s2_3;
    s32 var_s2_4;
    s32 var_s2_5;
    s32 var_s2_6;
    s32 var_s2_7;
    s8 temp_v0_2;
    u8 *var_a0_3;
    u8 *var_v1_2;
    u8 temp_v0;
    u8 *var_a0;
    u8 *var_v1;

    switch (D_80122B20)

    {
    case 1:
        if (((*(s32 *)(D_80105880 + 0x0)) | (*(s32 *)(D_80105880 + 0x1C)) | (*(s32 *)(D_80105880 + 0x38))) == 0)
        {
            D_80122B20 = 2;
            return;
        }
    case 0:
        return;
    case 2:
        func_800B0A08(1);
        D_80122B20 = 3;
        return;
    case 3:
        var_s2 = 0;
        if (D_80122B10 > 0)
        {
            var_s3 = (u8 *)D_80122B28;
            do
            {
                temp_s0 = ((*(s16 *)(var_s3 + 0x0)) * 0x54) + (u8 *)D_800FDF58;
                if ((*(s16 *)(var_s3 + 0x2)) != -1)
                {
                    (*(u8 *)(temp_s0 + 0x25)) = 0;
                    (*(s16 *)(temp_s0 + 0x2A)) = 0x8D;
                    temp_v0 = (u8) (*(s16 *)(var_s3 + 0x2)) | ((*(u8 *)(temp_s0 + 0x21)) & 0x80);
                    (*(u8 *)(temp_s0 + 0x21)) = temp_v0;
                    if (temp_v0 & 0x80)
                    {
                        (*(u8 *)(temp_s0 + 0x1B)) = 0;
                    }
                    else
                    {
                        (*(u8 *)(temp_s0 + 0x1B)) = 0x80;
                    }
                    (*(u16 *)(temp_s0 + 0x2E)) = 1;
                    (*(u8 *)(temp_s0 + 0x24)) = 1;
                    (*(s32 *)(temp_s0 + 0x1C)) = (s32) ((*(s32 *)(temp_s0 + 0x1C)) & ~0x800);
                    (*(u16 *)(temp_s0 + 0x2C)) = (u16) ((*(u16 *)(temp_s0 + 0x2C)) + 3);
                    func_8006C3FC(temp_s0);
                }
                if ((*(s16 *)(var_s3 + 0x4)) != -1)
                {
                    temp_v0_2 = func_800839F8((*(u8 *)(temp_s0 + 0x3A)), 0);
                    if ((temp_v0_2 != -1) && (func_80083EEC((*(u8 *)(temp_s0 + 0x3A)), temp_v0_2, (*(s16 *)(var_s3 + 0x4))) != 0))
                    {
                        field_start_actor_animation(temp_v0_2, 0, 0);
                        D_80105AE0[(*(u8 *)(temp_s0 + 0x3A)) * 0x23C + 0x179] = temp_v0_2;
                    }
                }
                temp_a0 = (*(s16 *)(var_s3 + 0x6));
                if (temp_a0 != -1)
                {
                    func_800A3938(temp_a0, 0x80);
                    VSync(0);
                }
                var_s2 += 1;
                var_s3 += 8;
            } while (var_s2 < D_80122B10);
        }
        D_80122B20 = 4;
        return;
    case 4:
        if (func_800B0888() == 0)
        {
            var_s2_2 = 0;
            if (D_80122B10 > 0)
            {
                var_s0 = (s16 *)D_80122B28;
loop_29:
                if (field_object_has_active_actor_tracks(*var_s0) == 0)
                {
                    var_s2_2 += 1;
                    var_s0 += 4;
                    if (var_s2_2 >= D_80122B10)
                    {

                    }

                    else

                    {
                        goto loop_29;
                    }
                }
            }
            var_s2_3 = 3;
            if (var_s2_2 == D_80122B10)
            {
                var_v1 = (u8 *)D_800FDF58 + 0xFC;
loop_34:
                if (((*(u8 *)(var_v1 + 0x25)) == 0xFF) || ((*(s16 *)(var_v1 + 0x2A)) != 0x8D))
                {
                    var_s2_3 += 1;
                    var_v1 += 0x54;
                    if (var_s2_3 >= 0xD)
                    {

                    }

                    else

                    {
                        goto loop_34;
                    }
                }
                var_s2_4 = 1;
                if (var_s2_3 == 0xD)
                {
                    var_a0 = (u8 *)D_800FDF58 + 0x54;
                    var_v1_2 = (u8 *)D_800FD818 + 0x268;
loop_40:
                    if (*var_v1_2 & 1)
                    {
                        temp_v0_3 = (*(s16 *)(var_a0 + 0x2A));
                        if (temp_v0_3 != 0xAF)
                        {
                            if (temp_v0_3 != 0xB1)
                            {
                                goto block_43;
                            }
                        }
                    }
                    else
                    {
block_43:
                        var_a0 += 0x54;
                        var_s2_4 += 1;
                        var_v1_2 += 0x268;
                        if (var_s2_4 >= 3)
                        {

                        }

                        else

                        {
                            goto loop_40;
                        }
                    }
                    var_s2_5 = 0;
                    if (var_s2_4 == 3)
                    {
                        var_a0_2 = (u8 *)D_800FDF58;
                        var_v1_3 = (u8 *)D_800FE3A0;
                        do
                        {
                            var_s2_5 += 1;
                            (*(s32 *)(var_v1_3 + 0x34)) = (s32) ((*(s32 *)(var_v1_3 + 0x34)) & 0xFF7FFFFF);
                            (*(s16 *)(var_a0_2 + 0x2A)) = 0;
                            var_a0_2 += 0x54;
                            var_v1_3 += 0x48;
                        } while (var_s2_5 < 3);
                        func_800A3938(0x79, 0x80);
                        var_s2_6 = 0;
                        var_s1 = (u8 *)D_80105AE0;
                        var_s0_2 = (u8 *)D_800FDF58;
                        var_s3_2 = D_80122B68;
                        do
                        {
                            if (*var_s3_2 != 0)
                            {
                                func_800B08FC(1, var_s2_6);
                                (*(s16 *)(var_s0_2 + 0x2A)) = 0x99;
                                (*(u16 *)(var_s0_2 + 0x2E)) = 1;
                                (*(u8 *)(var_s0_2 + 0x27)) = 0;
                                (*(u8 *)(var_s0_2 + 0x24)) = 1;
                                (*(u8 *)(var_s0_2 + 0x21)) = (u8) (((*(u8 *)(var_s0_2 + 0x21)) & 0x80) + 0x11);
                                (*(s32 *)(var_s0_2 + 0x1C)) = (s32) ((*(s32 *)(var_s0_2 + 0x1C)) & ~0x800);
                                (*(s32 *)(var_s1 + 0x174)) = (s32) ((*(s32 *)(var_s1 + 0x174)) & ~0x1800);
                                func_8006C3FC(var_s0_2);
                            }
                            var_s1 += 0x23C;
                            var_s0_2 += 0x54;
                            var_s2_6 += 1;
                            var_s3_2++;
                        } while (var_s2_6 < 2);
                        var_s2_7 = 0;
                        var_v1_4 = (u8 *)D_80105AE0;
                        var_a0_3 = (u8 *)D_800FD818;
                        do
                        {
                            if (*var_a0_3 & 1)
                            {
                                (*(u8 *)(var_v1_4 + 0x1AB)) = 0x3C;
                                (*(s32 *)(var_v1_4 + 0x174)) = (s32) ((*(s32 *)(var_v1_4 + 0x174)) | 0x8000);
                            }
                            var_v1_4 += 0x23C;
                            var_s2_7 += 1;
                            var_a0_3 += 0x268;
                        } while (var_s2_7 < 3);
                        DrawSync(0);
                        func_80084240();
                        field_restore_default_action_animation_mappings(1);
                        func_800B34D0(D_800FE754);
                        D_80122B20 = 0;
                    }
                }
            }
        }
        break;
    }
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
