#include "common.h"

/** @brief Partial RetC1B60 layout used by func_800C1A18. */
typedef struct
{
    u8 unk0[2];
    u16 unk2;
} RetC1B60;

/** @brief Partial SlotRec layout used by func_800C1A18. */
typedef struct
{
    u8 pad0[2];
    u16 unk2;
    s32 unk4;
    u8 pad8[0x44 - 8];
} SlotRec;

/** @brief Partial FieldStateB layout used by func_800C1A18. */
typedef struct
{
    u8 pad0[8];
    SlotRec *unk8;
} FieldStateB;

RetC1B60 *func_800C1B60(void *arg0);
void func_800C2138(s32 arg0);
u8 *func_800A9060(void);
void func_800A8F8C(u8 *dst, u8 *src);
void func_800A8D8C(u8 *arg0, u8 arg1);

extern FieldStateB *D_80123FB0;
extern u16 D_800F0E98[];

/**
 * @brief Resolve a command to a slot record or table entry and submit its data.
 * @param arg0 Unused.
 * @param arg1 Command source passed to func_800C1B60.
 * @note WIP: two target instructions and temporary-register differences remain.
 */
void func_800C1A18(void *arg0, void *arg1)
{
    RetC1B60 *ret;
    u16 flags;
    s32 code;
    s32 i;
    s32 count;
    SlotRec *table;
    SlotRec *cursor;
    s32 offset;
    u8 *found;
    u8 *handle;
    u8 *arg0_2;
    u8 arg1_2;

    ret = func_800C1B60(arg1);
    flags = ret->unk2;
    code = flags & 0xFF;
    if (!(flags & 0x8000))
    {
        s32 key;

        found = NULL;
        table = D_80123FB0->unk8;
        offset = 0;
        count = table->unk2;
        i = offset;
        if (count != 0)
        {
            s32 n;
            SlotRec *base;

            key = flags & 0xFFFF;
            base = table;
            n = count;
            cursor = base;
            i = 0;
            do
            {
                if (cursor->unk4 == key)
                {
                    found = (u8 *)base + offset + 8;
                    goto found_match;
                }
                cursor = (SlotRec *)((u8 *)cursor + 0x44);
                offset += 0x44;
                i += 1;
            } while (i < n);
found_match:;
        }
        if (found == NULL)
        {
            return;
        }
        handle = func_800A9060();
        if (handle == NULL)
        {
            return;
        }
        func_800A8F8C(handle, found);
        arg0_2 = handle;
        arg1_2 = 0;
        func_800A8D8C(arg0_2, arg1_2);
        return;
    }
    else
    {
        func_800C2138(code);
        arg0_2 = (u8 *)D_800F0E98 + D_800F0E98[code];
        arg1_2 = 1;
    }
    func_800A8D8C(arg0_2, arg1_2);
}
