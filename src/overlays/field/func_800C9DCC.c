#include "common.h"

/** @brief Byte-oriented view of a resource table entry's payload. */
typedef struct
{
    u8 pad0[4];
    u8 value;
} FieldResourceOffsetByte;

/** @brief Active menu record selected from D_80043CB8. */
typedef struct
{
    u8 pad0[0x24];
    u8 unk24;
    u8 unk25;
    u8 pad26[0x1A];
} FieldMenuRecordC9DCC;

extern FieldMenuRecordC9DCC D_80043CB8[];
extern s16 D_80122C10;

void func_800B2844(s32 arg0, u8 *arg1, u8 arg2);
u8 *func_800C1E40(s32 arg0);

/**
 * @brief Dispatch the active menu record and entries referenced by resource tables 0x103 and 0x104.
 */
void func_800C9DCC(void)
{
    FieldMenuRecordC9DCC *record;
    FieldMenuRecordC9DCC *record_base;
    s32 record_index;
    s32 unk24_value;
    s32 unk25_value;
    s32 lookup_index;
    s32 resource_index_103;
    s32 resource_index_104;
    u8 *resource_103;
    u8 *resource_104;
    s32 resource_offset;

    record_index = D_80122C10;
    record_base = D_80043CB8;
    record = record_base + record_index;
    unk24_value = record->unk24;
    unk25_value = record->unk25;
    lookup_index = (unk24_value * 0xE) + unk25_value;
    func_800B2844(0, (u8 *)record, 0xFF);

    resource_103 = func_800C1E40(0x103);
    resource_index_103 = lookup_index * 2;
    resource_offset = ((FieldResourceOffsetByte *)(resource_103 + resource_index_103))->value +
                      (((FieldResourceOffsetByte *)(func_800C1E40(0x103) + (resource_index_103 += 1)))->value << 8);
    func_800B2844(1, func_800C1E40(0x103) + (resource_offset + 4), 0xFF);

    resource_104 = func_800C1E40(0x104);
    resource_index_104 = unk25_value * 2;
    resource_offset = ((FieldResourceOffsetByte *)(resource_104 + resource_index_104))->value +
                      (((FieldResourceOffsetByte *)(func_800C1E40(0x104) + (resource_index_104 += 1)))->value << 8);
    func_800B2844(2, func_800C1E40(0x104) + (resource_offset + 4), 0xFF);
}
