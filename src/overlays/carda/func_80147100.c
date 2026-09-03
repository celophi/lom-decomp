#include "common.h"

typedef struct CardaList {
    s32 unk0;
    s32 count;
    u8 entries[0x50];
} CardaList;

extern u8 *D_8012271C;
extern s32 D_80165F40;
extern u8 D_80165F48[];
extern s32 D_80165FE8;
extern u8 *D_80165FF0;
extern u8 D_80166008[];
extern void func_80016E7C();

/**
 * @brief Scan the active carda list and record entries whose resource rank is
 *        still below the cap, bumping each such resource's rank.
 * @see (100%)
 */
void func_80147100(void)
{
    u8 *base;
    CardaList *list;
    u32 i;
    u8 *p;
    u8 *resource;
    u8 *entry;
    u8 value;

    base = D_80165FF0;
    list = (CardaList *)(base + 0x300);
    D_80165FE8 = 0;
    func_80016E7C(base + 0x358, D_80166008, 0x60);
    i = 0;
    D_80165FE8 = 0;
    D_80165F40 = list->unk0;
    if (list->count != 0)
    {
        do
        {
            p = (u8 *)list + i;
            resource = D_8012271C;
            value = p[8];
            entry = resource + value;
            value = entry[0x25E0];
            if (value < 0x63U)
            {
                entry[0x25E0] = (u8)(value + 1);
                D_80165F48[D_80165FE8] = p[8];
                D_80165FE8++;
            }
            i++;
        } while (i < (u32)list->count);
    }
}
