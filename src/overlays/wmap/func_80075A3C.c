#include "common.h"

/** @brief World-map 8-byte slot: only the +4 pointer field is written here. */
typedef struct
{
    s32 field_00;
    void *field_04;
} WmapSlot8;

/** @brief World-map 0x14-byte slot: only the leading halfword is cleared here. */
typedef struct
{
    s16 field_00;
    u8 pad_02[0x12];
} WmapSlot14;

extern s32 D_801B0FD0;
extern WmapSlot8 D_80139988[];
extern s32 D_8011D538;
extern WmapSlot14 D_801AFBD0[];
extern s32 D_801B258C;
extern s32 D_801B2588;
extern void func_80075AC0(void);

/** @brief World-map step: reset a run of slot tables then advance the sub-counter. */
void func_80075A3C(void)
{
    s32 i;

    D_801B0FD0 = 0x18;
    for (i = 0; i < 0x18; i++)
    {
        D_801AFBD0[i].field_00 = 0;
        D_80139988[i + 0xCC].field_04 = &D_8011D538;
    }
    D_801B258C = 0x30;
    D_801B2588 += 1;
    func_80075AC0();
}
