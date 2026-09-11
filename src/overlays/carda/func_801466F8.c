#include "common.h"

#define CARDA_RECORD_SIZE 0x60
#define CARDA_RECORD_ARRAY_OFFSET 0x2EF4
#define CARDA_RECORD_GROWTH_OFFSET 0x2F0C

extern u8 *D_8012271C;
extern s32 D_801227C4;
extern s32 D_80165F40;
extern u8 D_80166008[];

extern void func_80016E7C(const void *source, void *destination, s32 size);
extern void func_800C1230(s32 slot);

/**
 * @brief Restore the active record and apply its pending growth value.
 */
void func_801466F8(void)
{
    u8 *record;
    u32 growth;
    u32 low_byte;
    u32 updated_growth;

    func_80016E7C(D_80166008,
                  D_8012271C +
                      (D_801227C4 * CARDA_RECORD_SIZE +
                       CARDA_RECORD_ARRAY_OFFSET),
                  CARDA_RECORD_SIZE);

    record = D_8012271C + D_801227C4 * CARDA_RECORD_SIZE;
    growth = *(u32 *)(record + CARDA_RECORD_GROWTH_OFFSET);
    low_byte = growth & 0xFF;
    growth >>= 8;
    growth += D_80165F40;
    growth <<= 8;
    updated_growth = low_byte | growth;
    *(u32 *)(record + CARDA_RECORD_GROWTH_OFFSET) = updated_growth;
    func_800C1230(D_801227C4);
}
