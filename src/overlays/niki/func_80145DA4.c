#include "common.h"

extern s32 D_80164AE8;
extern s32 D_80164B78;
extern s32 D_80164B70;
extern s32 D_80164B7C;
extern s32 D_80164F14;
extern s32 D_80164FD4;
extern u8 D_80165018[];

/**
 * @brief Directory entry as seen by the entry-scan pass: only the size field
 *        at offset 0x18 is used here.
 */
typedef struct NikiDirEntry {
    u8 pad[0x18];
    s32 size;
} NikiDirEntry;

/**
 * @brief Advance one step of the NIKI entry load scan for the given page.
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
s32 func_80145DA4(s32 page)
{
    s32 i;
    s32 sum;
    s32 offset;
    s32 selected;
    s32 page_offset;
    s32 count;
    s32 cond;

    page_offset = page * 0x320;
    if (func_8001684C((void *)(D_80165018 + page_offset + D_80164B78 * 0x28)) != 0)
    {
        func_800B0170((void *)(D_80165018 + page_offset + D_80164B78 * 0x28));
        D_80164B78 += 1;
        return 1;
    }

    func_800AA02C();
    if ((D_80164AE8 == 0) && (func_80144BF8() == 0))
    {
        do {
            D_80164B78 = 0xF8;
        } while (0);
    }
    else
    {
        i = 0;
        sum = 0;
        D_80164FD4 = 0;
        count = D_80164B78;
        if (count > 0)
        {
            u8 *entries;
            do { entries = D_80165018; } while (0);
            offset = D_80164B70 * 0x320;
            do
            {
                sum += ((NikiDirEntry *)(offset + (s32)entries))->size / 8192;
                i++;
                offset += 0x28;
            } while (i < count);
        }
        cond = sum >= 0xE;
        if (cond != 0)
        {
            selected = func_80144984(sum, i, count);
            if (func_80144BF8() == 0)
            {
                D_80164B78 = 0xFA;
                D_80164F14 = 0;
            }
            else
            {
                D_80164B7C = selected;
                func_80140CC8();
            }
        }
        else
        {
            D_80164FD4 = 1;
            selected = func_80144984(sum, i, count);
            if (func_80144BF8() == 0)
            {
                D_80164B7C = 0;
                func_80140CC8();
                D_80164F14 = 0;
            }
            else
            {
                D_80164B7C = selected;
                func_80140CC8();
            }
        }
    }
    return 0;
}
