#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    u8 pad8[0x23C - 8];
} Struct_D80105AE0;
extern u8 *D_80122B78;
extern s32 *D_80123FB0;
extern void func_800966F0(s32, void *);
extern Struct_D80105AE0 *func_80087F0C(s32);


extern u8 *D_80122B78;

/**
 * @see decomp.me (100%) TODO
 */
void func_800B4390(void)
{
    s32 i;
    s32 off;

    for (i = 3; i < *(u16 *)(D_80122B78 + 0x400); i++)
    {
        off = i * 0x94;
        func_80087F0C((D_80122B78 + off)[0x430]);
    }
}


extern u8 *D_80122B78;
extern u8 *D_80122B74;

void func_80087614(s32 arg0, s32 arg1);
void func_800B28E0(s32 arg0, s32 arg1, s32 arg2);

s32 func_80087FC0(s32 arg0, s32 arg1);

/**
 * @brief Notify matching actors and refresh the first three actor slots.
 * @param arg0 Value matched against actor flags and stored in the field state.
 */
void func_800B4410(s32 arg0)
{
    s32 record_offset;
    s32 index;
    s32 slot_offset;

    for (index = 3; index < *(u16 *)(D_80122B78 + 0x400); index++)
    {
        record_offset = index * 0x94;
        if ((*(u32 *)(D_80122B78 + record_offset + 0x4C0) & 0xF) == arg0)
        {
            func_80087614(*(u8 *)(D_80122B78 + record_offset + 0x430), arg0);
            func_800B28E0(*(u8 *)(D_80122B78 + record_offset + 0x430), 0xD, 0);
        }
    }

    {
        u8 *state = D_80122B78;
        *(u32 *)(state + 0x400) |= 0x10000;
        *(u8 *)(state + 0x403) = arg0;
        func_800966F0(arg0, state);
    }

    for (index = 0; index < 3; index++)
    {
        slot_offset = index * 0x250;
        record_offset = index * 0x94;
        if ((*(u8 *)(D_80122B74 + slot_offset + 0x608) >> 7) != 0)
        {
            func_80087FC0(index, 0);
        }
        else
        {
            *(u16 *)(D_80122B78 + record_offset + 0x436) = 0xFFFF;
            func_80087FC0(index, 2);
            func_800B28E0(*(u8 *)(D_80122B78 + record_offset + 0x430), 0xF, 0);
        }
    }

    func_800B28E0(0x80, 0xD, 0);
}




extern u8 *D_80122B78;
extern s32 *D_80123FB0;


void func_800B28E0(s32, s32, s32);


/**
 * @brief Mark the field state busy and refresh matching actor records.
 */
void func_800B4584(void)
{
    s32 i;
    s32 off;
    Struct_D80105AE0 *rec;
    s32 *p;

    p = D_80123FB0;
    *p |= 0x80000000;
    func_800966F0(0, p);
    func_800B28E0(0x80, 0xD, 3);

    for (i = 3; i < *(u16 *)(D_80122B78 + 0x400); i++)
    {
        off = i * 0x94;
        if (((*(u32 *)(D_80122B78 + off + 0x4C0)) & 0xF) == *(u8 *)(D_80122B78 + 0x403))
        {
            func_800B28E0((D_80122B78 + off)[0x430], 0xD, 3);
        }
    }

    for (i = 0; i < 3; i++)
    {
        rec = func_80087F0C(i);
        if (rec != (Struct_D80105AE0 *)-1)
        {
            rec->unk4 = rec->unk0;
        }
    }
}

/** @brief Field header and actor event fields used by the reset routine. */
typedef struct
{
    u8 pad[0x400];
    union
    {
        u32 word;
        u16 count;
        struct
        {
            u8 low[3];
            u8 high;
        } bytes;
    } header;
    u8 pad404[0x2C];
    u8 id;
    u8 pad431[5];
    u16 enabled;
    u16 event;
} State;

extern u8 *D_80122B74;
extern s32 D_8010D020, g_layout_flag;

extern void func_800C1D14(s32, s32);

extern void akao_cmd_c1(s32, s32, s32);
/**
 * @brief Reset the first three actor event lists and dispatch initialization events.
 * @note Sends audio command C1 for layouts 3, 34, 35, 37, 43, 45, 46, and 47.
 * @note Best current match: 93.089290% with GCC 2.8.
 */
void func_800B4684(void)
{
    s32 i, offset, source_offset, inner;
    u32 j;
    State *state;
    D_80123FB0 = 0;
    ((State *)D_80122B78)->header.word &= 0xFFFEFFFF;
    ((State *)D_80122B78)->header.bytes.high = 0;
    i = 0;
    offset = 0;
    source_offset = 0;
    do
    {
        if (*((u8 *)((s32)D_80122B74 + source_offset) + 0x608) >> 7)
        {
            func_80087FC0(i, 0);
        }
        else
        {
            j = 0;
            state = (State *)D_80122B78;
            inner = offset;
            ((State *)((u8 *)state + offset))->enabled = 0;
            do
            {
                ((State *)((u8 *)state + inner))->event = 0xFFFF;
                j++;
                inner += 2;
            } while (j < 16);
            func_80087FC0(i, 1);
            func_800C1D14(i, 0);
        }
        offset += 0x94;
        i++;
        source_offset += 0x250;
    } while (i < 3);
    i = 0;
    if (((State *)D_80122B78)->header.count != 0)
    {
        offset = 0;
        do
        {
            func_800B28E0(((State *)((s32)D_80122B78 + offset))->id, 13, 1);
            i++;
            offset += 0x94;
        } while (i < ((State *)D_80122B78)->header.count);
    }
    if (D_8010D020 != 0)
    {
        D_8010D020 = 0;
    }
    else
    {
        switch (g_layout_flag)
        {
        case 3:
        case 34:
        case 35:
        case 37:
        case 43:
        case 45:
        case 46:
        case 47:
            akao_cmd_c1(0, 0x40, 0);
            break;
        }
    }
}
