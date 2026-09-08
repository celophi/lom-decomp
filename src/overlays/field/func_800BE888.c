#include "common.h"

typedef struct
{
    s32 owner;
    u8 mode;
    u8 primary_record;
    u8 secondary_record;
    u8 tertiary_record;
} FieldSequenceConfig;

extern FieldSequenceConfig *D_80123FC4;

void func_800C21C0(s32 arg0);
void func_800BEF74(void);

/**
 * @brief Initialize and flush the shared field sequence configuration.
 * @param owner Owning object or handle stored in the sequence configuration.
 * @param mode Sequence mode selector.
 * @param primary_record Primary record index.
 * @param secondary_record Secondary record index.
 */
void func_800BE888(s32 owner, s32 mode, s32 primary_record, s32 secondary_record)
{
    FieldSequenceConfig *config;
    s32 i;

    func_800C21C0(secondary_record);

    config = D_80123FC4;
    config->mode = mode;
    i = 0;
    config->owner = owner;
    D_80123FC4->primary_record = primary_record;
    D_80123FC4->secondary_record = secondary_record;
    D_80123FC4->tertiary_record = 0xFF;

    for (; i < 8; i++)
    {
        u8 *flag_entry;
        u8 *value_entry;

        flag_entry = (u8 *)D_80123FC4;
        flag_entry += i;
        flag_entry[0x20] = (flag_entry[0x20] & 0xF0) | 4;
        value_entry = (u8 *)D_80123FC4;
        value_entry += i;
        value_entry[0x50] = 4;
    }

    for (i = 0; i < 6; i++)
    {
        u8 *entry;

        entry = (u8 *)D_80123FC4;
        entry += i;
        entry[0x28] = 0xFF;
    }

    ((u8 *)D_80123FC4)[0x2E] = ((u8 *)D_80123FC4)[5] << 4;
    ((u8 *)D_80123FC4)[0x2F] = (((u8 *)D_80123FC4)[5] << 4) + 0xC;
    ((u8 *)D_80123FC4)[0x30] = (((u8 *)D_80123FC4)[5] << 4) + 0xD;
    ((u8 *)D_80123FC4)[0x31] = (((u8 *)D_80123FC4)[5] << 4) + 0xE;
    ((u8 *)D_80123FC4)[0x32] = (((u8 *)D_80123FC4)[5] << 4) + 0xF;
    ((u8 *)D_80123FC4)[0x33] = 0xFF;

    func_800BEF74();
}
