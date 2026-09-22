/* Partial WMAP decompilation: 68.622690% (gcc280_g0). */
#include "common.h"

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

#include "sdk/libgte.h"
extern u8 D_80139258;
M2C_UNK SetPolyFT4(void *);                      /* extern */
M2C_UNK func_800571A4();                            /* extern */
M2C_UNK func_80058260();                            /* extern */
M2C_UNK func_80058298();                            /* extern */
M2C_UNK func_800582E8();                            /* extern */
M2C_UNK func_8005909C();                            /* extern */
M2C_UNK func_8005B548();                            /* extern */
s32 func_8005D494(void);
M2C_UNK func_800605B4();                            /* extern */
M2C_UNK func_80063F38();                            /* extern */
M2C_UNK func_800653EC();                            /* extern */
M2C_UNK func_800654F8();                            /* extern */
M2C_UNK func_8006CD18();                            /* extern */
M2C_UNK func_8006D870(M2C_UNK);                     /* extern */
M2C_UNK func_8006D8F0(M2C_UNK, s32 *, s32 *, s32);  /* extern */
extern s32 D_8005136C;
extern s32 D_800D06BC;
extern s32 D_800D9160;
extern s32 D_800D9168;
extern s32 D_800D916C;
extern u8 D_800D9170;
extern s32 D_800D9210;
extern s32 D_800D9218;
extern s32 D_800D9220;
extern s32 D_800D9224;
extern s32 D_800D923C;
extern s32 D_800D9240;
extern s32 D_800DBE6C;
extern s32 D_800DBE70;
extern s32 D_800DBE74;
extern s32 D_800DBE78;
extern s32 D_800DCEA0;
extern s32 D_800DCEC0;
extern s32 D_800DCEE0;
extern s32 D_800DCEE8;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_800DCEFC;
extern s32 D_800DCF08;
extern s32 D_800DCF0C;
extern s32 D_800DCF10;
extern u8 D_8010CF18;
extern u8 D_80114F18;
extern s32 D_8011CF18;
extern s32 D_8011CF20;
extern s32 D_8011CF44;
extern s32 D_8011CF50;
extern s32 D_8011CF58;
extern u8 D_8011CF60;
extern s32 D_8011CF70;
extern s32 D_8011CF74;
extern s32 D_8011CF7C;
extern s32 D_8011D0DC;
extern s32 D_8011D4F8;
extern s32 D_8011D4FC;
extern s32 D_80129540;
extern u8 D_80129548;
extern s32 D_8012954C;
extern s32 D_80129558;
extern u8 D_80129560;
extern s32 D_801391E0;
extern s32 D_80139218;
extern s32 D_80139224;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_80139248;
extern u8 D_80139258;
extern u8 D_80139278;
extern s32 D_80139280;
extern s32 D_80139868;
extern s32 D_801398A8;
extern s32 D_801398B0;
extern s32 D_801398B4;
extern s32 D_801398B8;
extern s32 D_801398BC;
extern s32 D_801398C4;
extern u8 D_801398C8;
extern s32 D_801398D0;
extern s32 D_801398D4;
extern s32 D_801398F0;
extern s32 D_801398F4;
extern s32 D_801398FC;
extern s32 D_80139900;
extern u8 D_80139950;
extern s32 D_80139960;
extern s32 D_80139978;
extern s32 D_8013997C;
extern s32 D_8013B208;
extern s32 D_8013B230;
extern s32 D_8013B254;
extern s32 D_8013B258;
extern s32 D_8013B25C;
extern u8 D_8013B260;
extern s32 D_8013B268;
extern s32 D_8013B26C;
extern s32 D_8013B274;
extern s32 D_8013B27C;
extern s32 D_8013B28C;
extern s32 D_8013B290;
extern s32 D_8013B294;
extern s32 D_80182228;
extern s32 D_80182240;
extern u8 D_80182D48;
extern s32 D_80182D70;
extern s32 D_80182D88;
extern u8 D_80182DC0;
extern s32 D_80182DD8;
extern s32 D_80182DDC;
extern s32 D_80182E00;
extern s32 D_80182E1C;
extern s32 D_80182E24;
extern s32 D_80182E30;
extern s32 D_80182E34;
extern s32 D_80182E3C;
extern s32 D_8019D6D8;
extern s32 D_801ADAE0;
extern s32 D_801ADAE8;
extern s32 D_801ADAEC;
extern s32 D_801ADAF0;
extern s32 D_801ADAF4;
extern s32 D_801ADAFC;
extern u8 D_801ADB04;
extern s32 D_801ADB90;

void func_80060918(void)
{
    void *var_v0;
    s32 *var_a1;
    s32 *var_a2;
    s32 temp_a3;
    s32 temp_a3_2;
    void *temp_a0;
    void *var_v0_2;
    void *var_v0_3;
    void *var_v1;
    void *var_v1_2;
    void *var_v1_3;

    var_a2 = &D_800D9240;
    var_a1 = &D_800D06BC;
    D_80182DD8 = 0;
    D_80139224 = 1;
    D_80182240 = 0;
    D_80182228 = 0;
    D_80139978 = -1;
    D_8013997C = 0;
    D_8011CF20 = 0;
    D_800DBE6C = -1;
    D_800DBE74 = 0;
    D_801ADAF0 = 0;
    D_8012954C = 0;
    D_800D923C = 0;
    do
    {
        M2C_FIELD(var_a2, s32 *, 0) = M2C_FIELD(var_a1, s32 *, 0);
        M2C_FIELD(var_a2, s32 *, 4) = (s32) M2C_FIELD(var_a1, s32 *, 4);
        M2C_FIELD(var_a2, s32 *, 8) = (s32) M2C_FIELD(var_a1, s32 *, 8);
        M2C_FIELD(var_a2, s32 *, 0xC) = (s32) M2C_FIELD(var_a1, s32 *, 0xC);
        var_a1 = (s32 *)((u8 *)var_a1 + 0x10);
        var_a2 = (s32 *)((u8 *)var_a2 + 0x10);
    } while (var_a1 != ((u8 *)&D_800D06BC + 0x20));
    D_80139228 = 0;
    D_8019D6D8 = 0;
    D_8013B26C = 0;
    D_80182E3C = -1;
    D_8011D4F8 = 0;
    D_80139248 = 0;
    D_80129540 = 0;
    D_80139900 = 0;
    D_801ADB90 = 1;
    D_801398B0 = 0;
    D_80182E1C = 0;
    D_801398B8 = 0;
    D_80182E00 = 0xFF;
    D_800DCEC0 = 1;
    D_800D9224 = 0;
    D_800D9160 = 0;
    D_80139280 = 0x1F800000;
    D_8013B27C = 1;
    D_801ADAFC = 1;
    D_8011CF7C = 1;
    D_801398A8 = 0;
    D_8011D0DC = 0;
    D_8013B230 = 0;
    D_801398B4 = 1;
    D_800D9220 = -1;
    D_80139868 = -1;
    D_8013B25C = 0x3C;
    D_801398BC = 0;
    D_8013B28C = 0;
    D_8013B258 = 0;
    temp_a3 = *var_a1;
    *var_a2 = temp_a3;
    M2C_FIELD(&D_80129560, void **, 0x33C) = &D_8010CF18;
    M2C_FIELD(&D_80129560, void **, 0x817C) = &D_80114F18;
    D_801ADAE8 = 0x40;
    D_8011CF50 = 1;
    D_8011CF44 = 0;
    D_800DCEFC = 0;
    D_80182D88 = 0;
    func_8006D8F0(0, var_a1, var_a2, temp_a3);
    func_800653EC();
    func_800582E8();
    func_800654F8();
    D_8011CF74 = 0;
    D_80139218 = 0;
    D_800DCEE8 = -1;
    D_8013B290 = -1;
    func_80058260();
    func_80058298();
    func_800571A4();
    func_8006CD18();
    func_8005909C();
    D_8013B268 = 0;
    D_801ADAEC = 0x80;
    D_801398D4 = 1;
    D_800D916C = 0;
    D_800D9210 = 1;
    D_801398C4 = 0;
    D_801398FC = 0;
    D_800DCEA0 = -1;
    func_800605B4();
    func_8006D870(0);
    M2C_FIELD(&D_801ADB04, s8 *, 0) = 0x80;
    M2C_FIELD(&D_801ADB04, s8 *, 1) = 0x80;
    M2C_FIELD(&D_801ADB04, s8 *, 2) = 0x80;
    D_801398F0 = 1;
    D_801ADAF4 = 0;
    D_8013B294 = 0;
    M2C_FIELD(&D_80139950, s32 *, 0) = 0;
    M2C_FIELD(&D_80139950, s32 *, 4) = 0;
    M2C_FIELD(&D_80139950, s32 *, 8) = 0x6000;
    M2C_FIELD(&D_80139278, s16 *, 0) = 0x2E0;
    M2C_FIELD(&D_80139278, s16 *, 4) = 0x1B0;
    M2C_FIELD(&D_80139278, s16 *, 2) = 0;
    M2C_FIELD(&D_80182DC0, s32 *, 0) = 8;
    M2C_FIELD(&D_80182DC0, s32 *, 4) = -0x18;
    M2C_FIELD(&D_80182DC0, s32 *, 8) = 0x6D60;
    D_80139244 = 0;
    D_8011CF70 = 0;
    D_80182D70 = 0;
    D_8011CF58 = 0;
    D_80139960 = 0;
    M2C_FIELD(&D_80182D48, s32 *, 0) = (s32) M2C_FIELD(&D_8011CF60, s32 *, 0);
    M2C_FIELD(&D_80182D48, s32 *, 4) = (s32) M2C_FIELD(&D_8011CF60, s32 *, 4);
    M2C_FIELD(&D_80182D48, s32 *, 8) = (s32) M2C_FIELD(&D_8011CF60, s32 *, 8);
    M2C_FIELD(&D_80182D48, s32 *, 0xC) = (s32) M2C_FIELD(&D_8011CF60, s32 *, 0xC);
    *(SVECTOR *)&D_801398C8 = *(SVECTOR *)&D_80139258;
    D_8011CF18 = 0;
    D_8013B208 = 0;
    D_801ADAE0 = 0;
    D_801398D0 = 0;
    D_800DCEEC = 1;
    D_800DCEF0 = 1;
    D_800D9218 = D_8005136C;
    D_80182DDC = D_8005136C;
    D_8013B254 = 0;
    D_800DBE78 = 0;
    D_800DBE70 = 2;
    D_801398F4 = 0;
    D_8011D4FC = -1;
    D_80182E24 = 0;
    D_800DCF0C = 0;
    D_800D9168 = 0;
    D_800DCEE0 = 0;
    D_80182E30 = 0;
    D_801391E0 = 0;
    D_80129558 = 0;
    D_800DCF10 = 0;
    D_800DCF08 = 0;
    D_80182E34 = 0;
    M2C_FIELD(&D_8013B260, s16 *, 2) = 0;
    M2C_FIELD(&D_80129548, s8 *, 0) = 0;
    M2C_FIELD(&D_8013B260, s16 *, 0) = 0;
    M2C_FIELD(&D_80129548, s8 *, 1) = 0;
    M2C_FIELD(&D_80129548, s8 *, 2) = 0;
    SetPolyFT4(&D_800D9170);
    var_v1 = (u8 *)&D_800D9170 + 0x78;
    var_v0 = &D_800D9170;
    M2C_FIELD(&D_800D9170, s32 *, 0x18) = 0;
    M2C_FIELD(&D_800D9170, s32 *, 0x10) = 0;
    M2C_FIELD(&D_800D9170, s32 *, 8) = 0;
    do
    {
        M2C_FIELD(var_v1, s32 *, 0) = (s32) M2C_FIELD(var_v0, s32 *, 0);
        M2C_FIELD(var_v1, s32 *, 4) = (s32) M2C_FIELD(var_v0, s32 *, 4);
        M2C_FIELD(var_v1, s32 *, 8) = (s32) M2C_FIELD(var_v0, s32 *, 8);
        M2C_FIELD(var_v1, s32 *, 0xC) = (s32) M2C_FIELD(var_v0, s32 *, 0xC);
        var_v0 += 0x10;
        var_v1 += 0x10;
    } while (var_v0 != ((u8 *)&D_800D9170 + 0x20));
    M2C_FIELD(var_v1, s32 *, 0) = (s32) M2C_FIELD(var_v0, s32 *, 0);
    M2C_FIELD(var_v1, s32 *, 4) = (s32) M2C_FIELD(var_v0, s32 *, 4);
    var_v1_2 = (u8 *)&D_800D9170 + 0x50;
    var_v0_2 = (u8 *)&D_800D9170 + 0x78;
    temp_a0 = (u8 *)&D_800D9170 + 0x98;
    do
    {
        M2C_FIELD(var_v1_2, s32 *, 0) = (s32) M2C_FIELD(var_v0_2, s32 *, 0);
        M2C_FIELD(var_v1_2, s32 *, 4) = (s32) M2C_FIELD(var_v0_2, s32 *, 4);
        M2C_FIELD(var_v1_2, s32 *, 8) = (s32) M2C_FIELD(var_v0_2, s32 *, 8);
        M2C_FIELD(var_v1_2, s32 *, 0xC) = (s32) M2C_FIELD(var_v0_2, s32 *, 0xC);
        var_v0_2 += 0x10;
        var_v1_2 += 0x10;
    } while (var_v0_2 != temp_a0);
    M2C_FIELD(var_v1_2, s32 *, 0) = (s32) M2C_FIELD(var_v0_2, s32 *, 0);
    M2C_FIELD(var_v1_2, s32 *, 4) = (s32) M2C_FIELD(var_v0_2, s32 *, 4);
    var_v1_3 = (u8 *)&D_800D9170 + 0x28;
    var_v0_3 = (u8 *)&D_800D9170 + 0x50;
    do
    {
        M2C_FIELD(var_v1_3, s32 *, 0) = (s32) M2C_FIELD(var_v0_3, s32 *, 0);
        M2C_FIELD(var_v1_3, s32 *, 4) = (s32) M2C_FIELD(var_v0_3, s32 *, 4);
        M2C_FIELD(var_v1_3, s32 *, 8) = (s32) M2C_FIELD(var_v0_3, s32 *, 8);
        M2C_FIELD(var_v1_3, s32 *, 0xC) = (s32) M2C_FIELD(var_v0_3, s32 *, 0xC);
        var_v0_3 += 0x10;
        var_v1_3 += 0x10;
    } while (var_v0_3 != ((u8 *)&D_800D9170 + 0x70));
    temp_a3_2 = M2C_FIELD(var_v0_3, s32 *, 0);
    M2C_FIELD(var_v1_3, s32 *, 0) = temp_a3_2;
    M2C_FIELD(var_v1_3, s32 *, 4) = (s32) M2C_FIELD(var_v0_3, s32 *, 4);
    D_8013B274 = func_8005D494();
    func_80063F38();
    func_8005B548();
}
