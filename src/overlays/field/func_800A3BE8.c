#include "common.h"
#include "akao.h"

/** @brief Header preceding the copied field sound-bank tables. */
typedef struct
{
    /** @brief Byte offset from this header to the current copied table. */
    s32 table_offset;
    /** @brief Unknown field cleared when loading a new bank list. */
    s32 unk_04;
    /** @brief Unknown field cleared when loading a new bank list. */
    s32 unk_08;
} FieldBankCopyHeader;

extern FieldBankCopyHeader D_80119F00;
extern u8 *D_8010D038;
extern u8 D_800EC398[];

void cdrom_queue_read(s32 id, void *dst);
void cdrom_wait_queue_empty(void);
s32 func_80022EF8(void *bank_id, s32 arg1, s32 arg2);
void akao_upload_bank_blocking(AkaoBankHeader *bank, s32 wait_for_completion);

/**
 * @brief Load field sound-bank tables and upload their associated audio banks.
 * @param bank_id Resource index; -2 preserves state and -1 clears only the header.
 */
void func_800A3BE8(s32 bank_id)
{
    FieldBankCopyHeader *header;
    u8 *src;
    u8 *cursor;
    u8 *copy_cursor;
    u8 *sub_block;
    u8 *sub_block_end;
    u8 *flag;
    u8 *header_base;
    s32 *offsets;
    s32 resource_id;
    s32 count;
    s32 i;

    if (bank_id == -2)
    {
        return;
    }

    offsets = (s32 *)&D_80119F00;
    offsets[2] = 0;
    offsets[1] = 0;
    D_80119F00.table_offset = 0;

    resource_id = bank_id + 0x51;
    if (bank_id == -1)
    {
        return;
    }

    resource_id = (u16)resource_id;
    header = (FieldBankCopyHeader *)offsets;
    if (bank_id != 0)
    {
        header = (FieldBankCopyHeader *)offsets;
    }
    cursor = (u8 *)header + sizeof(*header);
    offsets = (s32 *)D_8010D038;

    cdrom_queue_read(resource_id, offsets);
    src = (u8 *)offsets;
    offsets = NULL;
    cdrom_wait_queue_empty();

    i = 0;
    offsets = (s32 *)(src + 4);
    do
    {
        count = *(volatile s32 *)src;
    } while (0);

    if (count <= 0)
    {
        return;
    }

    header_base = (u8 *)header;

    flag = (u8 *)bank_id;
    flag += (s32)D_800EC398;

    do
    {
        header->table_offset = (s32)(((cursor - header_base) >> 2) * 4);

        sub_block = src + (s32)(sub_block = (u8 *)*offsets);
        sub_block_end = sub_block + *(s32 *)(sub_block + (*(s32 *)sub_block) * 4);

        copy_cursor = cursor;
        if (sub_block != sub_block_end)
        {
            do
            {
                *copy_cursor = *sub_block;
                sub_block++;
                copy_cursor++;
            } while (sub_block != sub_block_end);
        }

        cursor = copy_cursor;
        if (*flag == 0)
        {
            akao_upload_bank_blocking((AkaoBankHeader *)sub_block_end, 1);
            return;
        }

        func_80022EF8(sub_block_end, i, 1);
        i++;
        offsets++;
    } while (i < count);
}
