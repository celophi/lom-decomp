#include "common.h"

typedef struct
{
    u8 unk0[4];
    u8 unk4;
    u8 unk5[0x23];
} FieldUnkRecord_80086F20;

extern FieldUnkRecord_80086F20 D_80107800[];
extern s16 D_801058E0[];
extern void *bcopy(const void *, void *, int);

/**
 * @brief Stores a record into the first free slot of the D_80107800 table.
 *
 * Scans up to 256 records for one whose unk4 flag byte is clear, copies 0x28
 * bytes from @p src into it, and writes @p value to the parallel D_801058E0
 * half-word slot.
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
