#include "common.h"


extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern s8 D_800459B3;

typedef struct
{
    s16 unk0;
    u8 pad2[0x12 - 2];
    s16 unk12;
} StructD80122C0A;

extern StructD80122C0A D_80122C0A;

/**
 * @brief Swap the selected layout owner in the packed three-slot ordering.
 * @note Nonmatching C. The input ordering must contain the selected owners;
 * the target leaves the found index unset if the first search fails.
 */
void func_800C68C8(void)
{
    s32 sp[3];
    s32 slot_idx;
    s32 found_idx;
    s32 flags;
    s32 d0;
    s8 key;

    flags = g_menuLayoutBuffer[0x29DB];
    slot_idx = 3;
    found_idx = 3;
    sp[0] = flags & 3;
    sp[1] = (flags >> 2) & 3;
    sp[2] = (flags >> 4) & 3;
    key = (s8)g_menuLayoutBuffer[0x29D7];
    d0 = D_80122C00;
    if (g_menuLayoutBuffer[d0 + 0x29D8] != key)
    {
        s32 i;
        s32 t1;

        i = 0;
        do
        {
            if (key == g_menuLayoutBuffer[i + 0x29D8])
            {
                found_idx = i;
            }
            i += 1;
        } while (i < 3);
        i = 0;
        do
        {
            if (sp[i] == found_idx)
            {
                t1 = i;
            }
            i += 1;
        } while (i < 3);
        i = 0;
        do
        {
            if (sp[i] == d0)
            {
                slot_idx = i;
            }
            i += 1;
        } while (i < 3);
        sp[t1] = d0;
        sp[slot_idx] = found_idx;
        D_800459B3 = sp[0] + (sp[1] * 4) + (sp[2] * 0x10);
    }
    D_80122C0A.unk0 = slot_idx;
    D_80122C0A.unk12 = (s16)found_idx;
}


extern s16 D_80122C10;
extern u8 D_80043CB8[];

void func_800B2844(s32 arg0, u8* arg1, u8 arg2);

/** @brief Dispatch the selected shared equipment record. */
void func_800C69F4(void)
{
    func_800B2844(0, D_80043CB8 + (D_80122C10 << 6), 0xFF);
}


extern u8 g_menuLayoutBuffer[];
extern s16 D_80122C10;

/**
 * @brief Counts active menu-layout slots whose packed field matches D_80122C10.
 *
 * Scans all 100 slots of @c g_menuLayoutBuffer (stride 0x40). A slot counts
 * when its active byte at +0xCE0 is nonzero and bits 8-9 of the packed word at
 * +0xCF4 equal the value in @c D_80122C10 (read once up front). The total is
 * written back to @c D_80122C10.
 */
void func_800C6A30(void)
{
    s32 i;
    s32 count;
    u8 *p;
    u32 field;
    s32 target_val;

    target_val = D_80122C10;
    count = 0;
    for (i = 0; i < 0x64; i++)
    {
        p = &g_menuLayoutBuffer[i * 0x40];
        if (p[0xCE0] != 0)
        {
            field = *(u32 *) &p[0xCF4];
            field = (field >> 8) & 3;
            if (field == target_val)
            {
                count++;
            }
        }
    }
    D_80122C10 = count;
}


extern u8 g_menuLayoutBuffer[];
extern u8 D_80043CB8[];

extern u8 *func_800A9060(void);
extern void func_800A8F8C(u8 *dst, u8 *src);

/** @brief Fill available record destinations and advance the layout counter. */
void func_800C6A90(void)
{
    while (func_800A9060() != 0)
    {
        func_800A8F8C(func_800A9060(), D_80043CB8);
    }

    g_menuLayoutBuffer[0x29D5] += 9;
}


typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} Rec16;

typedef struct
{
    u8 pad[0xE8];
} Struct0xE8;

extern Struct0xE8 D_80051CE4;
extern Struct0xE8 D_80051DCC;
extern s16 D_80122C1C;
extern u8 g_menuLayoutBuffer[];

void func_800C3BB0(void);

/**
 * @brief Reconcile active logic blocks with the selected layout class.
 * @note Nonmatching C. Copies both 0xE8-byte tables; only the first is read
 * by the visible ownership check. Preserve the second copy from the target.
 */
void func_800C6AF0(void)
{
    Struct0xE8 sp10;
    Struct0xE8 spF8;
    Rec16 *v0;
    Rec16 *v1;
    u8 *a0;
    s32 a1;
    s32 key;
    u32 v1w;
    s32 v0w;
    u8 *rec;

    v1 = (Rec16 *)&sp10;
    v0 = (Rec16 *)&D_80051CE4;
    do
    {
        s32 t4, t5, t6, t7;
        t4 = v0->unk0;
        t5 = v0->unk4;
        t6 = v0->unk8;
        t7 = v0->unkC;
        v1->unk0 = t4;
        v1->unk4 = t5;
        v1->unk8 = t6;
        v1->unkC = t7;
        v0++;
        v1++;
    } while (v0 != (Rec16 *)((u8 *)&D_80051CE4 + 0xE0));
    {
        s32 t4, t5;
        t4 = v0->unk0;
        t5 = v0->unk4;
        v1->unk0 = t4;
        v1->unk4 = t5;
    }

    v1 = (Rec16 *)&spF8;
    v0 = (Rec16 *)&D_80051DCC;
    do
    {
        s32 t4, t5, t6, t7;
        t4 = v0->unk0;
        t5 = v0->unk4;
        t6 = v0->unk8;
        t7 = v0->unkC;
        v1->unk0 = t4;
        v1->unk4 = t5;
        v1->unk8 = t6;
        v1->unkC = t7;
        v0++;
        v1++;
    } while (v0 != (Rec16 *)((u8 *)&D_80051DCC + 0xE0));
    {
        s32 t4, t5;
        t4 = v0->unk0;
        t5 = v0->unk4;
        v1->unk0 = t4;
        v1->unk4 = t5;
    }

    key = ((s8 *)g_menuLayoutBuffer)[0x29D7];
    if (key < 3)
    {
        rec = g_menuLayoutBuffer + key * 0x14C;
        a0 = (u8 *)-0x10;
        g_menuLayoutBuffer[0xAA9] = (u8)D_80122C1C;
        *(s32 *)(rec + 0x2B50) = (*(s32 *)(rec + 0x2B50) & ~0xF) | ((u8)D_80122C1C & 0xF);
        a1 = 0;
        if (g_menuLayoutBuffer[0x29D6] != 0)
        {
            a0 = g_menuLayoutBuffer;
            do
            {
                v1w = *(u32 *)(a0 + 0x29DC);
                if (((v1w >> 0x10) & 1) == 1 && (v1w & 3) == key)
                {
                    v0w = *(s32 *)((u8 *)&sp10 + (v1w & 0xFC));
                    if (v0w != 0 && v0w != D_80122C1C)
                    {
                        *(u32 *)(a0 + 0x29DC) = (v1w & 0xFFFEFFFF) | 3;
                    }
                }
                a1 = a1 + 1;
                a0 = a0 + 4;
            } while (a1 < g_menuLayoutBuffer[0x29D6]);
        }
        func_800C3BB0();
    }
}


/**
 * @brief View of g_menuLayoutBuffer used by func_800C6C80: a packed-word
 *        cursor at 0x29D6 followed by the word table at 0x29DC.
 */
typedef struct
{
    u8 pad0[0x29D6];
    u8 index;
    u8 pad29D7[5];
    u32 words[1];
} MenuLayoutBuffer;

/**
 * @brief Three-word parameter block forwarded to field_open_gosub_screen_sequence.
 */
typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
} UnkStruct80051EB4;

typedef struct
{
    u8 pad[0xCF4];
    s32 unkCF4;
} MenuRec;

typedef struct
{
    s16 mystic_card_ids[3];
} FieldMysticCardSlots;

typedef struct
{
    s16 unk0;
    s16 unk2;
} FieldObjState;

/**
 * @brief Placeholder layout for the record returned by func_800C1E40; only the
 *        byte at offset 4 is read here.
 */
typedef struct
{
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
} Rec;

#define MENU_LAYOUT ((MenuLayoutBuffer *)g_menuLayoutBuffer)

void func_800C9ED4();
void field_open_gosub_screen_sequence(UnkStruct80051EB4 *arg0);
void func_800AD030(s32 arg0);
void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);
u8 *func_800C1E40(s32 arg0);


extern u8 D_800459AE;
extern UnkStruct80051EB4 D_80051EB4;
extern s32 D_80122C00;
extern s16 D_80122C06[];
extern FieldObjState D_80122C0C;
extern u16 D_80122C0E;
extern s16 D_80122C10;
extern s16 D_80122C1C;
extern s32 g_gosub_result_values[];
extern u8 g_menuLayoutBuffer[];

/**
 * @brief Append a packed word built from D_80122C18..D_80122C1C to the menu word table.
 */
void func_800C6C80(void)
{
    s16 *p = &D_80122C1C;
    s32 arg0 = p[0];
    s32 arg1 = p[-1];
    s32 arg2 = p[-2];

    if (arg0 == 0xFF)
    {
        D_800459AE = 0;
        return;
    }

    if (MENU_LAYOUT->index < 0x28)
    {
        MENU_LAYOUT->words[MENU_LAYOUT->index] =
            (MENU_LAYOUT->words[MENU_LAYOUT->index] & ~0xFC) | ((arg0 & 0x3F) << 2);
        MENU_LAYOUT->words[MENU_LAYOUT->index] =
            (MENU_LAYOUT->words[MENU_LAYOUT->index] & ~0xF00) | ((arg1 & 0xF) << 8);
        MENU_LAYOUT->words[MENU_LAYOUT->index] =
            (MENU_LAYOUT->words[MENU_LAYOUT->index] & ~0xF000) | ((arg2 & 0xF) << 12);
        MENU_LAYOUT->words[MENU_LAYOUT->index] |= 3;
        MENU_LAYOUT->words[MENU_LAYOUT->index] &= ~0x10000;
        MENU_LAYOUT->index++;
    }
}

/**
 * @brief Forward D_80122C1C to func_800C9ED4.
 */
void func_800C6DA0(void)
{
    func_800C9ED4(D_80122C1C);
}

/**
 * @brief Copy the D_80051EB4 constant onto the stack and forward it to field_open_gosub_screen_sequence.
 */
void func_800C6DC8(void)
{
    UnkStruct80051EB4 local;

    local = D_80051EB4;
    field_open_gosub_screen_sequence(&local);
}

/**
 * @brief Call func_800AD030 with argument 0.
 */
void func_800C6E08(void)
{
    func_800AD030(0);
}

/**
 * @brief Resolve the current menu record's song slot and latch it.
 *
 * Reads the packed field @c unkCF4 of the record selected by @c D_80122C10,
 * decoding a base index (@c bits 10-15) offset by mode (@c bits 8-9): +0 for
 * mode 0, +0xB for mode 1, +0x17 otherwise. When the result is the 0xFF
 * sentinel it triggers akao_set_song_params and stores 0; otherwise the resolved
 * slot is written back to @c D_80122C10.
 *
 * @see decomp.me (100%) TODO
 */
void func_800C6E28(void)
{
    u32 rec;
    s32 mode;
    s32 v;

    rec = ((MenuRec *)(g_menuLayoutBuffer + D_80122C10 * 0x40))->unkCF4;
    mode = (rec >> 8) & 3;
    if (mode == 0)
    {
        v = (rec >> 10) & 0x3F;
    }
    else if (mode == 1)
    {
        v = ((rec >> 10) & 0x3F) + 0xB;
    }
    else
    {
        v = ((rec >> 10) & 0x3F) + 0x17;
    }
    if (v == 0xFF)
    {
        akao_set_song_params(0x8002, 0x22, 0, 0);
        v = 0;
    }
    D_80122C10 = v;
}

/**
 * @brief Invalidates matching menu slots and preserves the remaining IDs.
 *
 * Loads three menu IDs from the active 0x40-byte layout record, then scans
 * the three halfword slots at D_80122C00. Matching slots are replaced with
 * 0xFF and the corresponding menu ID is invalidated before the final IDs are
 * written to D_80122C06.
 */
void func_800C6EBC(void)
{
    s32 i;
    s16 *slot;
    u8 a, b, c;
    u8 *p;
    s16 value;
    s32 invalid;

    i = 0;
    invalid = 0xFF;
    slot = ((FieldMysticCardSlots *)&D_80122C00)->mystic_card_ids;
    {
        s16 *idxp;
        u8 *base;

        idxp = &D_80122C10;
        base = g_menuLayoutBuffer;
        p = base + (*idxp << 6);
    }
    a = p[0xD00];
    b = p[0xD01];
    c = p[0xD02];
    do
    {
        value = *slot;
        if (value != invalid)
        {
            if (value == a)
            {
                *slot = invalid;
                a = 0xFF;
            }
            else if (value == b)
            {
                *slot = invalid;
                b = 0xFF;
            }
            else if (value == c)
            {
                *slot = invalid;
                c = 0xFF;
            }
        }
        i++;
        slot++;
    } while (i < 3);
    D_80122C06[0] = a;
    D_80122C06[1] = b;
    D_80122C06[2] = c;
}

/**
 * @brief Load the gosub-selected equipment's visible Mystic Card IDs.
 */
void func_800C6F60(void)
{
    s32 *selection_results;
    u8 *layout_buffer;
    u8 *equipment_record;

    selection_results = g_gosub_result_values;
    layout_buffer = g_menuLayoutBuffer;
    equipment_record = (u8 *)((selection_results[0] * 64) + (s32)layout_buffer);
    ((FieldMysticCardSlots *)&D_80122C00)->mystic_card_ids[0] = equipment_record[0xD00];
    ((FieldMysticCardSlots *)&D_80122C00)->mystic_card_ids[1] = equipment_record[0xD01];
    ((FieldMysticCardSlots *)&D_80122C00)->mystic_card_ids[2] = equipment_record[0xD02];
}

/**
 * @brief Look up the current object's halfword in resource 0x102 and store it in D_80122C0E.
 * @see decomp.me (100%)
 */
void func_800C6F9C(void)
{
    s16 temp_s0;
    u8 *p;
    s32 offset;
    u16 result;

    temp_s0 = D_80122C0C.unk0;
    if (temp_s0 == 0xFF)
    {
        result = 0xFFFF;
    }
    else
    {
        if (D_80122C0C.unk2 == 0)
        {
            p = func_800C1E40(0x102);
            offset = temp_s0 * 4;
        }
        else
        {
            p = func_800C1E40(0x102);
            offset = temp_s0 * 4;
            offset = offset | 2;
        }
        result = *(u16 *)(p + offset + 4);
    }
    D_80122C0E = result;
}

/**
 * @brief Decode a little-endian offset from resource 0x101 and dispatch the referenced entry.
 * @see decomp.me (100%) TODO
 */
void func_800C7014(void)
{
    s32 idx;
    s32 k;
    u8 *p1;
    s32 value;

    idx = D_80122C0C.unk0;
    p1 = func_800C1E40(0x101);
    k = idx * 2;
    value = ((Rec *)(p1 + k))->unk4 +
            (((Rec *)(func_800C1E40(0x101) + (k += 1)))->unk4 << 8);
    func_800B2844(0, func_800C1E40(0x101) + (value + 4), 0xFF);
}
