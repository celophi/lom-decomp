#include "common.h"

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
void func_800B2844(s32 arg0, void *arg1, s32 arg2);

extern u8 D_800459AE;
extern UnkStruct80051EB4 D_80051EB4;
extern FieldMysticCardSlots D_80122C00;
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
    slot = D_80122C00.mystic_card_ids;
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
    D_80122C00.mystic_card_ids[0] = equipment_record[0xD00];
    D_80122C00.mystic_card_ids[1] = equipment_record[0xD01];
    D_80122C00.mystic_card_ids[2] = equipment_record[0xD02];
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
