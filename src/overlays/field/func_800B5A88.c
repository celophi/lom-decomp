#include "common.h"
/** @brief Action record with an owner byte and packed eligibility fields. */
typedef struct
{
    u32 pad0;
    union
    {
        u32 word;
        struct
        {
            u8 id;
            u8 rest[3];
        } bytes;
    } info;
    u16 pad8, flags_a;
} Record;
/** @brief Current action context and its descriptor and actor record pointers. */
typedef struct
{
    u8 pad[0x14];
    u32 flags;
    u32 pad18;
    u32 *descriptor;
    u32 pad20;
    Record *record;
} Context;
s32 func_8008ADB4(u8);
s32 func_800B4CE4(Record *, s32);
extern u8 *D_80122B74;
extern Context *D_80123FB0;

/**
 * @brief Classify the current action and mark eligible follow-up actions.
 * @return 0 when unavailable, 1 for the class test, 2 for immediate handling, or 3 for a marked
 * follow-up.
 * @note 100% match with GCC 2.8: 115 instructions, 460 bytes.
 */
s32 func_800B5A88(void)
{
    s32 action;
    u32 mode;
    u32 flags;
    Record *record;
    u8 *source;
    action = func_8008ADB4(D_80123FB0->record->info.bytes.id);
    switch (*D_80123FB0->descriptor & 15)
    {
    case 1:
        if (action == 0x3B)
        {
            return 2;
        }
        if (action == 0x22)
        {
            record = D_80123FB0->record;
            mode = record->info.word & 0xFC00;
            if (mode == 0 || mode == 0x400)
            {
                source = D_80122B74 + record->info.bytes.id * 0x250;
                if ((u32)(((*(u32 *)(source + 0x654) >> 10) & 63) - 6) < 2)
                {
                    return 1;
                }
            }
        }
        break;
    case 4:
        flags = D_80123FB0->flags;
        if ((flags >> 1) & 1)
        {
            return 2;
        }
        if ((D_80123FB0->record->flags_a & 0x10) && (u32)(action - 10) < 2)
        {
            D_80123FB0->flags = flags | 4;
            return 3;
        }
        if (func_800B4CE4(D_80123FB0->record, 0x34))
        {
            D_80123FB0->flags |= 4;
            return 3;
        }
        break;
    case 5:
        if (func_800B4CE4(D_80123FB0->record, 0x35))
        {
            D_80123FB0->flags |= 4;
            return 3;
        }
        break;
    case 2:
    case 3:
    case 6:
    default:
        break;
    }
    return 0;
}
