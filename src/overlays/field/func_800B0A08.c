#include "common.h"

typedef struct
{
    u8 flags;
    u8 _pad01[0x253];
    u16 resource_index;
    u8 _pad256[0x12];
} FieldResourceSlot;

extern FieldResourceSlot D_800FD818[];
extern s32 D_80122B68[];
extern s32 D_80122B18[];
extern u8* D_8010D038;

s32 func_8006A88C(s32 slot_index, FieldResourceSlot* slot, s32 mode);

/**
 * @brief Queue CD reads for each active field resource slot.
 * @param arg0 Resource-selection mode forwarded to func_8006A88C.
 */
void func_800B0A08(s32 arg0)
{
    s32 i;
    u8* buffer;

    for (i = 0; i < 2; i++)
    {
        if (D_800FD818[i].flags & 1)
        {
            D_80122B68[i] = func_8006A88C(i, &D_800FD818[i], arg0);
            buffer = D_8010D038 + 0x8000 + i * 0x18000;
            D_800FD818[i].resource_index = (u16)D_80122B68[i];
            D_80122B18[i] = cdrom_queue_read((u16)D_80122B68[i], buffer);
        }
        else
        {
            D_80122B18[i] = 0;
            D_80122B68[i] = 0;
        }
    }
}
