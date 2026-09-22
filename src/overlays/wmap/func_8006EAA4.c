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
extern s32 D_80182DEC;
extern s32 D_801B0FD0;
extern s32 D_801B2450;
extern s32 D_801B2454;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern void func_80070368(void);

/** @brief Configure effect parameters and reset its animation resource slots. */
void func_8006EAA4(void)
{
    s32 i;

    D_801B0FD0 = 16;
    D_80139234 = 3;
    D_8013923C = 1;
    D_80139240 = 56;
    D_8013924C = 0;
    D_80139250 = 1;
    D_80139260 = 5000;
    D_80139264 = 60;
    D_80139268 = 17;
    D_8013926C = 1;
    D_80139284 = 0;
    D_80182DEC = 127;
    for (i = 0; i < 16; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 104].field_04 = D_8011F538;
    }
    D_801B2454 = 32;
    D_801B2450++;
    func_80070368();
}
