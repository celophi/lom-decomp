#include "wmap_map_display.h"
#include "wmap_land_effect_loader.h"
#include "wmap_resource_support.h"
#include "wmap_sequence_runtime.h"
#include "cdrom.h"

/** @brief Upload the selected effect resources and register its callback. */
void func_800591A8(u32 selection)
{

extern WmapLandDisplay D_80182508;
extern s32 D_801ADAF8;
extern u8 D_80182E40[];
extern u8 D_8018B240[];
extern u8 D_80193640[];
extern s32 func_800706B0(s32);
extern s32 func_800712C0(s32);
extern s32 func_8007369C(s32);
extern s32 func_80074FB4(s32);
extern s32 func_80077560(s32);
extern s32 func_8007998C(s32);
extern s32 func_8007B910(s32);
extern s32 func_8007DC14(s32);
extern s32 func_8007F398(s32);
extern s32 func_80081BA0(s32);
extern s32 func_8008437C(s32);
extern s32 func_80085A70(s32);
extern s32 func_8008759C(s32);
extern s32 func_80089134(s32);
extern s32 func_8008B25C(s32);
extern s32 func_8008D4B4(s32);
extern s32 func_8008FD1C(s32);
extern s32 func_80091C0C(s32);
extern s32 func_80093E98(s32);
extern s32 func_80095F34(s32);
extern s32 func_80097C50(s32);
extern s32 func_8009B8E4(s32);
extern s32 func_8009E250(s32);
extern s32 func_800A1604(s32);
extern s32 func_800A3CC8(s32);
extern s32 func_800AEAB8(s32);
extern s32 func_800B7290(s32);
extern s32 func_800BAE24(s32);
extern s32 func_800BD83C(s32);

    D_801ADAF8 = 0;
    cdrom_wait_queue_empty();
    func_800651B4(D_80182E40);
    func_800651B4(D_8018B240);
    if (selection != 2)
    {
        func_800651B4(D_80193640);
    }
    if (selection == 0x16)
    {
        wmap_resolve_land_image(&D_80182508);
    }
    else
    {
        wmap_resolve_land_image(&g_wmap_land_display[selection]);
    }
    switch (selection)
    {
    case 33:
        func_8006CAC0(func_800BD83C);
        break;
    case 24:
        func_8006CAC0(func_800BAE24);
        break;
    case 31:
        func_8006CAC0(func_800B7290);
        break;
    case 25:
        func_8006CAC0(func_800AEAB8);
        break;
    case 22:
        func_8006CAC0(func_800A1604);
        break;
    case 19:
        func_8006CAC0(func_800A3CC8);
        break;
    case 23:
        func_8006CAC0(func_8009E250);
        break;
    case 27:
        func_8006CAC0(func_8009B8E4);
        break;
    case 16:
        func_8006CAC0(func_80097C50);
        break;
    case 0:
        func_8006CAC0(func_80091C0C);
        break;
    case 9:
        func_8006CAC0(func_80095F34);
        break;
    case 8:
        func_8006CAC0(func_80093E98);
        break;
    case 10:
        func_8006CAC0(func_8008FD1C);
        break;
    case 5:
        func_8006CAC0(func_8008D4B4);
        break;
    case 17:
        func_8006CAC0(func_8008B25C);
        break;
    case 21:
        func_8006CAC0(func_80089134);
        break;
    case 3:
        func_8006CAC0(func_8008759C);
        break;
    case 11:
        func_8006CAC0(func_80085A70);
        break;
    case 12:
        func_8006CAC0(func_8008437C);
        break;
    case 30:
        func_8006CAC0(func_80081BA0);
        break;
    case 7:
        func_8006CAC0(func_8007F398);
        break;
    case 2:
        func_8006CAC0(func_8007DC14);
        break;
    case 15:
        func_8006CAC0(func_8007B910);
        break;
    case 32:
        func_8006CAC0(func_8007998C);
        break;
    case 26:
        func_8006CAC0(func_80077560);
        break;
    case 1:
        func_8006CAC0(func_8007369C);
        break;
    case 18:
        func_8006CAC0(func_800706B0);
        break;
    case 4:
        func_8006CAC0(func_800712C0);
        break;
    case 13:
        func_8006CAC0(func_80074FB4);
        break;
    default:
        func_8006CAC0(func_8006D244);
        break;
    }
}
