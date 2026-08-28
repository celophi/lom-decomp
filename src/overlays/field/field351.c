#include "common.h"

/** @brief 0x40-byte record in the D_80122B74 records[] array. */
typedef struct
{
    u8 flag;      /* 0x00 activation flag */
    u8 pad1[0x33];
    s32 result;   /* 0x34 cached result handle */
    u8 pad2[0x8];
} FieldRecord80122B74;

/** @brief Block pointed to by D_80122B74: header then 100 records at 0xCE0. */
typedef struct
{
    u8 header[0xCE0];
    FieldRecord80122B74 records[100];
} FieldBlock80122B74;

extern FieldBlock80122B74 *D_80122B74;

s32 func_800C38C8(FieldRecord80122B74 *arg0);

/**
 * @brief Populate cached result handles for every active D_80122B74 record.
 *
 * Walks the 100 records at offset 0xCE0: for each one flagged active whose
 * cached result is still 0, calls func_800C38C8 on the record and stores the
 * returned handle back into the record.
 *
 * @see decomp.me (100%) TODO
 */
void func_800C396C(void)
{
    u32 i;

    for (i = 0; i < 0x64; i++)
    {
        if (D_80122B74->records[i].flag != 0 && D_80122B74->records[i].result == 0)
        {
            D_80122B74->records[i].result = func_800C38C8(&D_80122B74->records[i]);
        }
    }
}
