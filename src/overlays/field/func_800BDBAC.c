#include "common.h"

typedef struct FieldNibbleRecord {
    u16 offset;
    u16 row;
    s32 low;
    s32 high;
} FieldNibbleRecord;

typedef struct SourceEntry {
    u8 pad[4];
    u8 packed;
} SourceEntry;

extern u8 *func_800C1E40(s32 arg0);

void func_800BDBAC(s32 unused, FieldNibbleRecord *record)
{
    u8 *base;

    base = func_800C1E40(0x13);
    if (base != NULL) {
        {
            SourceEntry *entry;
            entry = (SourceEntry *)(base + ((record->row * 0x30) +
                                           (*(volatile u16 *)&record->offset - 0x60)));
            record->low = entry->packed & 0xF;
        }
        {
            SourceEntry *entry;
            entry = (SourceEntry *)(base + ((record->row * 0x30) +
                                           (*(volatile u16 *)&record->offset - 0x60)));
            record->high = entry->packed >> 4;
        }
    }
}
