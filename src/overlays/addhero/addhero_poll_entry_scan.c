#include "common.h"

extern s32 D_8016093C;
extern s32 D_801609A4;
extern s32 D_801609A8;
extern s32 D_801609AC;
extern s32 D_80164A5C;
extern s32 D_80164B1C;
extern u8 D_80164B60[];

/**
 * @brief Directory entry as seen by the entry-scan pass: only the size field
 *        at offset 0x18 is used here.
 */
typedef struct AddheroDirEntry {
    u8 pad[0x18];
    s32 size;
} AddheroDirEntry;

/**
 * @brief Advance one step of the add-hero entry load scan for the given page.
 *
 * If the current entry's streamed resource is ready, hand it off and advance
 * the entry index. Otherwise poll the stream, and once the scan has stalled or
 * completed, total the number of loaded blocks across all scanned entries and
 * either rank/commit a selection or defer, depending on how much has loaded.
 *
 * @param page Page index whose entry block is being scanned.
 * @return 1 if an entry was consumed this step, 0 otherwise.
 * @see decomp.me (100.00%)
 */
s32 func_80145C34(s32 page)
{
    s32 i;
    s32 sum;
    s32 offset;
    s32 selected;
    s32 page_offset;
    s32 count;
    s32 cond;

    page_offset = page * 0x320;
    if (func_8001684C((void *)(D_80164B60 + page_offset + D_801609A4 * 0x28)) != 0)
    {
        func_800B0170((void *)(D_80164B60 + page_offset + D_801609A4 * 0x28));
        D_801609A4 += 1;
        return 1;
    }

    func_800AA02C();
    if ((D_8016093C == 0) && (func_80144A28() == 0))
    {
        do {
            D_801609A4 = 0xF8;
        } while (0);
    }
    else
    {
        i = 0;
        sum = 0;
        D_80164B1C = 0;
        count = D_801609A4;
        if (count > 0)
        {
            u8 *entries;
            do { entries = D_80164B60; } while (0);
            offset = D_801609A8 * 0x320;
            do
            {
                sum += ((AddheroDirEntry *)(offset + (s32)entries))->size / 8192;
                i++;
                offset += 0x28;
            } while (i < count);
        }
        cond = sum >= 0xE;
        if (cond != 0)
        {
            selected = func_801447B4(sum, i, count);
            if (func_80144A28() == 0)
            {
                D_801609A4 = 0xFA;
                D_80164A5C = 0;
            }
            else
            {
                if (D_8016093C != 0)
                {
                    D_801609AC = 0;
                }
                D_801609AC = selected;
                func_80140CFC();
            }
        }
        else
        {
            D_80164B1C = 1;
            selected = func_801447B4(sum, i, count);
            if (func_80144A28() == 0)
            {
                D_801609AC = 0;
                func_80140CFC();
                D_80164A5C = 0;
            }
            else
            {
                if (D_8016093C != 0)
                {
                    D_801609AC = 0;
                }
                D_801609AC = selected;
                func_80140CFC();
            }
        }
    }
    return 0;
}
