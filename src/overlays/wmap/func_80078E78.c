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

extern s32 D_80139234;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_8013924C;
extern s32 D_80139250;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern s32 D_80139284;
extern s32 D_801B24B4;
extern s32 D_801B0FD0;
extern s32 D_801B2698;
extern s32 D_801B269C;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011D538[];
extern void func_8007A32C(void);

/** @brief Configure effect parameters and reset its animation resource slots. */
void func_80078E78(void)
{
    s32 i;

    D_801B0FD0 = 24;
    D_801B24B4 = 127;
    D_80139234 = 1;
    D_8013923C = 10;
    D_80139240 = 20;
    D_8013924C = 2;
    D_80139250 = 2;
    D_80139260 = 1500;
    D_80139264 = 100;
    D_80139268 = 19;
    D_8013926C = 0;
    D_80139284 = 1000;
    for (i = 0; i < 24; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 104].field_04 = D_8011D538;
    }
    D_801B269C = 40;
    D_801B2698++;
    func_8007A32C();
}
