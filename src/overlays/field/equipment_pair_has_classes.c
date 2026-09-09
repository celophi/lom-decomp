#include "common.h"

extern u8 g_menuLayoutBuffer[];
/** @brief Equipment record view exposing the packed class word. */
typedef struct
{
    u8 pad[0xCF4];
    u32 flags;
} EquipmentView;

/**
 * @brief Test whether two equipment records supply one item of each requested class.
 * @param class_a Class required of one record.
 * @param class_b Class required of the other record.
 * @param record_indices Two equipment-record indices.
 * @return 1 when distinct records satisfy the pair, otherwise 0.
 * @note Reuse j for the decoded class and the later inner-loop index; this
 * reproduces the target's temporary allocation without forcing registers.
 * @note GCC 2.8.0 G0: 100% match, 71 instructions (284 bytes).
 */
s32 equipment_pair_has_classes(s32 class_a, s32 class_b, s32 *record_indices)
{
    s32 classes[2];
    s32 i;
    s32 j;

    i = 0;
    do
    {
        j = (((EquipmentView *)(g_menuLayoutBuffer + (*record_indices << 6)))->flags >> 10) & 0x3F;
        classes[i] = j;
        if (((((EquipmentView *)(g_menuLayoutBuffer + (*record_indices << 6)))->flags >> 8) & 3) == 1)
        {
            classes[i] = j + 11;
        }
        if (((((EquipmentView *)(g_menuLayoutBuffer + (*record_indices << 6)))->flags >> 8) & 3) == 2)
        {
            classes[i] += 23;
        }
        i++;
        record_indices++;
    } while (i < 2);
    i = 0;
    do
    {
        if (classes[i] == class_a)
        {
            j = 0;
            do
            {
                if (classes[j] == class_b && j != i)
                {
                    return 1;
                }
                j++;
            } while (j < 2);
        }
        i++;
    } while (i < 2);
    return 0;
}
