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
extern s32 D_80182DF4;
extern s32 D_801B0FD0;
extern s32 D_801B2458;
extern s32 D_801B245C;
extern WmapSlot8 D_80139988[];
extern WmapSlot14 D_801AFBD0[];
extern u8 D_8011F538[];
extern void func_80070490(void);

/** @brief Configure effect parameters and reset its animation resource slots. */
void func_8006EBB0(void)
{
    s32 i;

    D_801B0FD0 = 40;
    D_80139234 = 6;
    D_8013923C = 2;
    D_80139240 = 32;
    D_8013924C = 2;
    D_80139250 = 1;
    D_80139260 = 5000;
    D_80139264 = 120;
    D_80139268 = 17;
    D_8013926C = 2;
    D_80139284 = 0;
    D_80182DF4 = 127;
    for (i = 0; i < 40; i++)
    {
        D_801AFBD0[i + D_80139264].field_00 = 0;
        D_80139988[i + 204].field_04 = D_8011F538;
    }
    D_801B245C = 48;
    D_801B2458++;
    func_80070490();
}
