#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "common.h"
#include "cdrom.h"
#include "sdk/libgte.h"

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    u8 field_00;
    u8 field_01;
    u8 field_02;
} WmapColor3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    s16 field_04;
} WmapShort3;

typedef struct
{
    s16 field_00;
    s16 field_02;
    u16 field_04;
    s16 field_06;
} WmapShort4;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
} WmapInt3;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
} WmapAlignedQuad;

typedef struct
{
    s32 words[9];
} WmapBlock36;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef struct
{
    s32 value;
    u8 pad[0x24];
} WmapCell;

typedef struct
{
    s32 field_00;
    s32 field_04;
    s32 field_08;
    s32 field_0C;
    s32 field_10;
    s32 field_14;
    s32 field_18;
    s32 field_1C;
    s32 field_20;
    s32 state_24;
    s32 field_28;
    s32 field_2C;
    s32 field_30;
    s32 field_34;
    s32 field_38;
    s32 field_3C;
    s32 field_40;
    s32 field_44;
    s32 field_48;
    s32 tail_state;
} WmapState;

typedef union
{
    struct
    {
        s16 field_00;
        s16 field_02;
    } fields;
    s32 value;
} WmapSignedPair16;

typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[2];
    u8 field_06;
    u8 pad_07[7];
    s16 field_0E;
    s16 field_10;
    u8 pad_12[0x10];
    s16 field_22;
    s16 field_24;
    s16 field_26;
    u8 pad_28[4];
} WmapConfigA;

typedef void (*WmapHandler)(void);

extern s32 D_80054A18[];
extern WmapHandler D_800D7314[];
extern WmapHandler D_800D732C[];
extern WmapHandler D_800D7384[];
extern WmapHandler D_800D7394[];
extern WmapHandler D_800D73A4[];
extern WmapHandler D_800D73BC[];
extern WmapHandler D_800D73D4[];
extern WmapHandler D_800D73EC[];
extern WmapHandler D_800D7404[];
extern WmapHandler D_800D741C[];
extern WmapHandler D_800D7434[];
extern WmapHandler D_800D744C[];
extern WmapHandler D_800D745C[];
extern WmapHandler D_800D7474[];
extern WmapHandler D_800D748C[];
extern WmapHandler D_800D753C[];
extern WmapHandler D_800D7554[];
extern WmapHandler D_800D7564[];
extern WmapHandler D_800D757C[];
extern WmapHandler D_800D7594[];
extern WmapHandler D_800D75AC[];
extern WmapHandler D_800D75C4[];
extern WmapHandler D_800D75D4[];
extern WmapHandler D_800D75E4[];
extern WmapHandler D_800D75F4[];
extern WmapHandler D_800D760C[];
extern WmapHandler D_800D7624[];
extern WmapHandler D_800D763C[];
extern WmapHandler D_800D7654[];
extern WmapHandler D_800D766C[];
extern WmapHandler D_800D7684[];
extern WmapHandler D_800D769C[];
extern WmapHandler D_800D76AC[];
extern WmapHandler D_800D76C4[];
extern WmapHandler D_800D76DC[];
extern WmapHandler D_800D76F4[];
extern WmapHandler D_800D770C[];
extern WmapHandler D_800D7774[];
extern WmapHandler D_800D778C[];
extern WmapHandler D_800D77A4[];
extern WmapHandler D_800D77BC[];
extern WmapHandler D_800D77D4[];
extern WmapHandler D_800D77E4[];
extern WmapHandler D_800D77F4[];
extern WmapHandler D_800D7804[];
extern WmapHandler D_800D7814[];
extern WmapHandler D_800D7824[];
extern WmapHandler D_800D78BC[];
extern WmapHandler D_800D78CC[];
extern WmapHandler D_800D78A4[];
extern WmapHandler D_800D788C[];
extern WmapHandler D_800D7874[];
extern WmapHandler D_800D7854[];
extern WmapHandler D_800D783C[];
extern WmapHandler D_800D7924[];
extern WmapHandler D_800D793C[];
extern WmapHandler D_800D7954[];
extern WmapHandler D_800D796C[];
extern WmapHandler D_800D7984[];
extern WmapHandler D_800D799C[];
extern WmapHandler D_800D79B4[];
extern WmapHandler D_800D79CC[];
extern WmapHandler D_800D79E4[];
extern WmapHandler D_800D79FC[];
extern WmapHandler D_800D7A0C[];
extern WmapHandler D_800D7A2C[];
extern u8 D_800D95D8[];
extern u8 D_800D9D68[];
extern u8 D_800DA028[];
extern u8 D_800DAA78[];
extern u8 D_800DB310[];
extern WmapConfigA D_800DB4C8[];
extern s32 D_800DBE70;
extern WmapBlock36 D_800D06BC;
extern WmapBlock36 D_800D9240;
extern WmapAlignedQuad D_800DCEC8;
extern s32 D_800DCEB0;
extern WmapShort3 D_800DCEB8;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_800DCF18[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9318;
extern WmapConfigA D_800D9370;
extern WmapConfigA D_800D939C;
extern WmapConfigA D_800D93C8;
extern WmapConfigA D_800D93F4;
extern WmapConfigA D_800D9420;
extern s32 D_800D9158;
extern s32 D_800D9228;
extern WmapConfigA D_800D9268[];
extern WmapPair16 D_8011CF4C;
extern s32* D_8011CF1C;
extern s32* D_8011CF24;
extern s32* D_8011CF28;
extern s32* D_8011CF2C;
extern s32* D_8011CF30;
extern s32* D_8011CF34;
extern s32* D_8011CF38;
extern s32* D_8011CF3C;
extern s32 D_8011D4FC;
extern s32 D_8011D500;
extern s32 D_8011D510;
extern s32 D_8011D530;
extern u8 D_8011D538;
extern u8 D_8011F538;
extern u8 D_80121538;
extern u8 D_80123538;
extern u8 D_80125538;
extern u8 D_80127538;
extern s32 D_80139228;
extern s32 D_80139234;
extern WmapInt3 D_80139200;
extern WmapShort3 D_80139210;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139244;
extern s32 D_8013924C;
extern s32 D_80139250;
extern WmapPair D_80139258;
extern s32 D_80139260;
extern s32 D_80139264;
extern s32 D_80139268;
extern s32 D_8013926C;
extern WmapState* D_80139280;
extern WmapCell D_80139290[][6];
extern VECTOR D_80139870;
extern s32 D_8013986C;
extern s32 D_801398D0;
extern u8 D_80139A28[];
extern u8 D_80139B88[];
extern u8 D_80139C08[];
extern u8 D_80139DE8[];
extern u8 D_80139F78[];
extern WmapAlignedQuad D_80139950;
extern WmapInt3 D_80139968;
extern s32 D_80139978;
extern u8 D_80139988[];
extern u8 D_801399A8;
extern void* D_801399AC;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern u8 D_801399C0;
extern void* D_801399C4;
extern u8 D_801399C8;
extern void* D_801399CC;
extern u8 D_801399D0;
extern void* D_801399D4;
extern u8 D_801399D8;
extern void* D_801399DC;
extern s32 D_8013B20C;
extern s32 D_8013B208;
extern s32 D_8013B254;
extern WmapPair D_8013B238;
extern WmapPair D_8013B240;
extern s32 D_8013B294;
extern s32 D_8013B29C;
extern WmapSignedPair16 D_80182D58;
extern WmapPair16 D_80182D60;
extern u8 D_80182E40;
extern WmapColor3 D_80182D74;
extern WmapColor3 D_80182D80;
extern WmapColor3 D_80182D8C;
extern WmapColor3 D_80182D94;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182DD8;
extern s32 D_80182DE4;
extern s32 D_80182DE8;
extern s32 D_80182DEC;
extern s32 D_80182DF0;
extern s32 D_80182DF4;
extern s32 D_80182E38;
extern VECTOR D_80182DC0;
extern VECTOR D_8011CF60;
extern u8 D_8018B240;
extern s32 D_801ADAE0;
extern s32 D_801ADAF4;
extern u8 D_801AFBD0[];
extern VECTOR D_801B2478;
extern WmapPair D_801B2490;
extern WmapPair D_801B2498;
extern SVECTOR D_801B24A0;
extern WmapPair D_801B24A8;
extern s32 D_801B24B4;
extern s32 D_801B25D8;
extern s32 D_801B25DC;
extern s32 D_801B25E0;
extern VECTOR D_801B2650;
extern WmapPair D_801B2670;
extern WmapPair D_801B2678;
extern s32 D_801B2FD8;
extern s32 D_801B2FDC;
extern s32 D_801B2FE0;
extern s32 D_801B2FE4;
extern s32 D_801B2FE8;
extern s32 D_801B2FEC;
extern s32 D_801B2FF0;
extern s32 D_801B2FF4;
extern s32 D_801B2FF8;
extern s32 D_801B2FFC;
extern s32 D_801B3000;
extern s32 D_801B3004;
extern s32 D_801B3008;
extern s32 D_801B300C;
extern s32 D_801B3010;
extern s32 D_801B3014;
extern s32 D_801B3018;
extern s32 D_801B301C;
extern s32 D_801B3020;
extern s32 D_801B3024;
extern s32 D_801B3028;
extern s32 D_801B302C;
extern s32 D_801B3030;
extern s32 D_801B3034;
extern s32 D_801B3038;
extern s32 D_801B303C;
extern s32 D_801B3040;
extern s32 D_801B3044;
extern s32 D_801B3048;
extern s32 D_801B304C;
extern s32 D_801B3050;
extern s32 D_801B3054;
extern s32 D_801B3058;
extern s32 D_801B305C;
extern s32 D_801B3060;
extern s32 D_801B3064;
extern s32 D_801B3068;
extern s32 D_801B306C;
extern s32 D_801B3070;
extern s32 D_801B3074;
extern s32 D_801B3078;
extern s32 D_801B307C;
extern s32 D_801B3080;
extern s32 D_801B3084;
extern s32 D_801B3088;
extern s32 D_801B308C;
extern s32 D_801B3090;
extern s32 D_801B3094;
extern s32 D_801B3098;
extern s32 D_801B309C;
extern s32 D_801B30A0;
extern s32 D_801B30A4;
extern s32 D_801B30A8;
extern s32 D_801B30AC;
extern s32 D_801B30B0;
extern s32 D_801B30B4;
extern s32 D_801B30B8;
extern s32 D_801B30BC;
extern s32 D_801B30C0;
extern s32 D_801B30C4;
extern s32 D_801B30C8;
extern s32 D_801B30CC;
extern s32 D_801B30D0;
extern s32 D_801B30D4;
extern s32 D_801B30D8;
extern s32 D_801B30DC;
extern s32 D_801B30E0;
extern s32 D_801B30E4;
extern s32 D_801B30E8;
extern s32 D_801B30EC;
extern s32 D_801B30F0;
extern s32 D_801B30F4;
extern WmapPair D_801B3118;
extern SVECTOR D_801B3120;
extern s32 D_801B3128;
extern s32 D_801B312C;
extern s32 D_801B3130;
extern s32 D_801B3134;
extern s32 D_801B3138;
extern s32 D_801B313C;
extern s32 D_801B3140;
extern s32 D_801B3144;
extern s32 D_801B3148;
extern s32 D_801B314C;
extern s32 D_801B3150;
extern s32 D_801B3154;
extern s32 D_801B3158;
extern s32 D_801B315C;
extern s32 D_801B3160;
extern s32 D_801B3164;
extern s32 D_801B3168;
extern s32 D_801B316C;
extern s32 D_801B3170;
extern s32 D_801B3174;
extern s32 D_801B3178;
extern s32 D_801B317C;
extern s32 D_801B3180;
extern s32 D_801B3184;
extern s32 D_801B3188;
extern s32 D_801B318C;
extern s32 D_801B3190;
extern s32 D_801B3194;
extern s32 D_801B3198;
extern s32 D_801B319C;
extern s32 D_801B31A0;
extern s32 D_801B31A4;
extern s32 D_801B31A8;
extern s32 D_801B31AC;
extern s32 D_801B31B0;
extern s32 D_801B31B4;
extern s32 D_801B31B8;
extern s32 D_801B31BC;
extern s32 D_801B31C0;
extern s32 D_801B31C4;
extern s32 D_801B31C8;
extern s32 D_801B31CC;
extern s32 D_801B31D0;
extern s32 D_801B31D4;
extern s32 D_801B31D8;
extern s32 D_801B31DC;
extern s32 D_801B31E0;
extern s32 D_801B31E4;
extern s32 D_801B31E8;
extern s32 D_801B31EC;
extern s32 D_801B31F0;
extern s32 D_801B31F4;
extern s32 D_801B31F8;
extern s32 D_801B31FC;
extern s32 D_801B3200;
extern s32 D_801B3204;
extern s32 D_801B3208;
extern s32 D_801B320C;
extern s32 D_801B3210;
extern s32 D_801B3214;
extern s32 D_801B3218;
extern s32 D_801B321C;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3238;
extern s32 D_801B323C;

extern void akao_cmd_a9(s32, s32);
extern void func_80066F9C(void*, s32, s32, s32, s32);
extern void func_800675F0(s32*, s32, s32, s32, s32, s32, s32, s32, s32, s32);
extern void func_8008ECF8(s32, s32, s32*, void*);
extern void func_800B1E7C(void);
extern void func_800B1F80(void);
extern void func_800B37EC(void);
extern void func_800B38EC(void);
extern void func_800B35F0(void);
extern void func_800B36F0(void);
extern void func_800B4E18(void);
extern void func_800B4FBC(void);
extern void func_800B39E8(void);
extern void func_800B3AE8(void);
extern void func_800B3BE4(void);
extern void func_800B3CE0(void);
extern void func_800B3DD8(void);
extern void func_800B3EDC(void);
extern void func_800B3FDC(void);
extern void func_800B40DC(void);
extern void func_800B41D8(void);
extern void func_800B42DC(void);
extern void func_800B43DC(void);
extern void func_800B5C7C(void);
extern void func_800B619C(void);
extern void func_800B6284(void);
extern void func_800B6558(void);
extern void func_800B6644(void);
extern void func_800B6730(void);
extern void func_800B681C(void);
extern void func_800B6918(void);
extern void func_800B6A10(void);
extern void func_800B6B0C(void);
extern void func_800B6C04(void);
extern void func_800B6D00(void);
extern void func_800B6DF8(void);
extern void func_800B6EF4(void);
extern void func_800B6FEC(void);
extern void func_800B70CC(void);
extern void func_800BA108(void);
extern void func_800BA408(void);
extern void func_800BA580();
extern void func_800BA66C();
extern void func_800BA758();
extern void func_800BAAB0();
extern void func_800BAB80();
extern void func_800BA1BC();
extern void func_800B9F1C();
extern void func_800B9FE4();
extern void func_800B8794(void);
extern void func_800B8988(void);
extern void func_800B95CC(void);
extern void func_800B96B8(void);
extern void func_800B9850(void);
extern void func_800B9938(void);
extern void func_800B9B08(void);
extern void func_800B9D74(void);
extern void func_800BAEF8();
extern void func_800BAF34();
extern void func_800BAF78();
extern void func_800BAFB4();
extern s32 func_800BAFCC(s32);
extern s32 func_800BB610(s32);
extern s32 func_800BB87C(s32);
extern s32 func_800BBAE8(s32);
extern void func_800BCDE0();
extern void func_800BCCF0();
extern void func_800BCAC8();
extern void func_800BC8C4();
extern void func_800BAC44();
extern void func_800BAD38();
extern void func_800BD258();
extern void func_800BD354();
extern void func_800BD750(void);
extern s32 func_800BD998(s32);
extern void func_800BD934(void);
extern void func_800BD970(void);
extern s32 func_800BE1A8(s32);
extern s32 func_800BE40C(s32);
extern s32 func_800BE678(s32);
extern s32 func_800BE8E4(s32);
extern s32 func_800BEB50(s32);
extern s32 func_800BED50(s32);
extern s32 func_800BEF58(s32);
extern s32 func_800BF160(s32);
extern s32 func_800BF368(s32);
extern s32 func_800BDF40(s32);
extern void func_800BE04C(void);
extern void func_800BE114(void);
extern void func_800BE2B0(void);
extern void func_800BE378(void);
extern void func_800BE51C(void);
extern void func_800BE5E4(void);
extern void func_800BE788(void);
extern void func_800BE850(void);
extern void func_800BE9F4(void);
extern void func_800BEABC(void);
extern void func_800BECB0(void);
extern void func_800BEEB4(void);
extern void func_800BF0BC(void);
extern void func_800BF1F0(void);
extern void func_800BF2C4(void);
extern void func_800BEFE8(void);
extern void func_800BFEEC(void);
extern void func_800BFF28(void);
extern void func_800BFF98(void);
extern void func_800BFFD8(void);
extern s32 func_800C0034(s32);
extern void func_800C06DC(void);
extern void func_800C071C(void);
extern void func_800C075C(void);
extern void func_800C079C(void);
extern s32 func_800BC01C(s32);
extern s32 func_800BC1BC(s32);
extern s32 func_800BC360(s32);
extern s32 func_800BC4B8(s32);
extern s32 func_800BC60C(s32);
extern s32 func_800BC764(s32);
extern s32 func_800BC964(s32);
extern void func_800BC7F4(void);
extern void func_800BC9F4(void);
extern void func_800BCBFC(void);
extern s32 func_800BCB6C(s32);
extern s32 func_800BCEA8(s32);
extern s32 func_800BCFD4(s32);
extern s32 func_800BD12C(s32);
extern s32 func_800BBD54(s32);
extern void func_800BB720();
extern void func_800BB7E8();
extern void func_800BB98C();
extern void func_800BBA54();
extern void func_800BBBF8();
extern void func_800BBCC0();
extern void func_800BBF88();
extern void func_800BC128();
extern void func_800BC2CC();

extern s32 func_800B7438(s32);
extern s32 func_800B7E7C(s32);
extern s32 func_800B8084(s32);
extern s32 func_800B822C(s32);
extern s32 func_800B8B80(s32);
extern s32 func_800B8CD8(s32);
extern s32 func_800B8E30(s32);
extern s32 func_800B8F8C(s32);
extern s32 func_800B90E8(s32);
extern s32 func_800B9244(s32);
extern s32 func_800B99F8(s32);
extern s32 func_800B9B9C(s32);
extern s32 func_800BA078(s32);
extern s32 func_800B84D0(s32);
extern s32 func_800B862C(s32);
extern s32 func_800B8824(s32);
extern s32 func_800B8A2C(s32);
extern s32 func_800B93A0(s32);
extern s32 func_800B94FC(s32);
extern s32 func_800B977C(s32);
extern s32 func_800B9E08(s32);
extern s32 func_800B5050(s32);
extern s32 func_800B51AC(s32);
extern s32 func_800B4EAC(s32);
extern s32 func_800B5308(s32);
extern s32 func_800B4D08(s32);
extern s32 func_800B571C(s32);
extern s32 func_800B55C0(s32);
extern s32 func_800B5878(s32);
extern s32 func_800B5B2C(s32);
extern s32 func_800B4774(s32);
extern void func_800B4688(void);
extern void func_800B470C(void);
extern void func_800B4748(void);

void func_800B7FE0(void);
void func_800B8198(void);
void func_800B8338(void);
void func_800B840C(void);
void func_800B7364(void);
void func_800B73A0(void);
void func_800B73E4(void);
void func_800B7420(void);

void func_800B3368(void)
{
    D_80182DEC = 1;
    D_801B24A8 = D_80139258;
    D_8013924C = 0;
    D_801B2FDC = 0xA7;
    D_801B2FD8++;
    func_800B1E7C();
}

void func_800B33E4(void)
{
    D_801B2FDC = 8;
    D_801B2FD8++;
    func_800B1F80();
}

void func_800B341C(void)
{
    D_801B2FD8++;
}

/**
 * @see decomp.me (100%)
 */
void func_800B35F0(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_800675F0(D_8011CF1C, (D_80139234 / 0x10) & 3, 5, 0x37, 0x7800, 1, D_801B24B4, -0x5A, -0xA, -1);
    D_80139234 += 4;
    value = D_801B24B4 + 2;
    D_801B24B4 = value;
    if (value >= 0x82)
    {
        D_801B24B4 = 0x81;
    }
    timer = D_801B3004;
    ((WmapShort4*)&D_801B2490)->field_04 += 0;
    next_timer = timer - 1;
    D_801B3004 = next_timer;
    if (next_timer == 0)
    {
        D_801B3000++;
    }
}

void func_800B36F0(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;
    s32* timer_ptr;

    func_8006CFA8(&D_80182DC0, &D_801B2490);
    func_800675F0(D_8011CF1C, (D_80139234 / 0x10) & 3, 5, 0x37, 0x7800, 1, D_801B24B4, -0x5A, -0xA, -1);
    value = D_801B24B4 - 4;
    D_801B24B4 = value;
    if (value < 0)
    {
        D_801B24B4 = 0;
    }
    timer_ptr = &D_801B3004;
    D_80139234 += 4;
    ((WmapShort4*)&D_801B2490)->field_04 += 0;
    timer = *timer_ptr;
    next_timer = timer - 1;
    *timer_ptr = next_timer;
    if (next_timer == 0)
    {
        D_801B3000++;
    }
}

void func_800B37EC(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_800675F0(D_8011CF24, (D_8013923C / 0x10) & 3, 5, 0x37, 0x7800, 1, D_80182DE4, 0x5F, -0x14, -1);
    D_8013923C += 4;
    value = D_80182DE4 + 4;
    D_80182DE4 = value;
    if (value >= 0x72)
    {
        D_80182DE4 = 0x71;
    }
    timer = D_801B300C;
    ((WmapShort4*)&D_801B2498)->field_04 += 0;
    next_timer = timer - 1;
    D_801B300C = next_timer;
    if (next_timer == 0)
    {
        D_801B3008++;
    }
}

void func_800B38EC(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;
    s32* timer_ptr;

    func_8006CFA8(&D_80182DC0, &D_801B2498);
    func_800675F0(D_8011CF24, (D_8013923C / 0x10) & 3, 5, 0x37, 0x7800, 1, D_80182DE4, 0x5F, -0x14, -1);
    value = D_80182DE4 - 4;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    timer_ptr = &D_801B300C;
    D_8013923C += 4;
    ((WmapShort4*)&D_801B2498)->field_04 += 0;
    timer = *timer_ptr;
    next_timer = timer - 1;
    *timer_ptr = next_timer;
    if (next_timer == 0)
    {
        D_801B3008++;
    }
}

void func_800B39E8(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;

    func_8006CFA8(&D_80182DC0, &D_801B24A0);
    func_800675F0(D_8011CF28, (D_80139240 / 0x10) & 3, 5, 0x37, 0x7800, 1, D_80182DE8, 0x14, 0xA, -1);
    D_80139240 += 4;
    value = D_80182DE8 + 2;
    D_80182DE8 = value;
    if (value >= 0x82)
    {
        D_80182DE8 = 0x81;
    }
    timer = D_801B3014;
    ((WmapShort4*)&D_801B24A0)->field_04 += 0;
    next_timer = timer - 1;
    D_801B3014 = next_timer;
    if (next_timer == 0)
    {
        D_801B3010++;
    }
}

void func_800B3AE8(void)
{
    s32 value;
    s32 timer;
    s32 next_timer;
    s32* timer_ptr;

    func_8006CFA8(&D_80182DC0, &D_801B24A0);
    func_800675F0(D_8011CF28, (D_80139240 / 0x10) & 3, 5, 0x37, 0x7800, 1, D_80182DE8, 0x14, 0xA, -1);
    value = D_80182DE8 - 4;
    D_80182DE8 = value;
    if (value < 0)
    {
        D_80182DE8 = 0;
    }
    timer_ptr = &D_801B3014;
    D_80139240 += 4;
    ((WmapShort4*)&D_801B24A0)->field_04 += 0;
    timer = *timer_ptr;
    next_timer = timer - 1;
    *timer_ptr = next_timer;
    if (next_timer == 0)
    {
        D_801B3010++;
    }
}

void func_800B3BE4(void)
{
    s32 timer;
    s32 next_timer;
    s32 value;

    func_8006CFA8(&D_80182DC0, &D_801B24A8);
    func_800675F0(D_8011CF2C, (D_8013924C / 0x10) & 3, 0xA, 0x37, 0x7800, 1, D_80182DEC, 0, 0x14, -1);
    D_8013924C += 4;
    value = D_80182DEC + 8;
    D_80182DEC = value;
    if (value >= 0x82)
    {
        D_80182DEC = 0x81;
    }
    timer = D_801B301C;
    ((WmapShort4*)&D_801B24A8)->field_04 += 0;
    next_timer = timer - 1;
    D_801B301C = next_timer;
    if (next_timer == 0)
    {
        D_801B3018++;
    }
}

void func_800B3CE0(void)
{
    s32 timer;
    s32 next_timer;
    s32 value;
    s32* timer_ptr;

    func_8006CFA8(&D_80182DC0, &D_801B24A8);
    func_800675F0(D_8011CF2C, (D_8013924C / 0x10) & 3, 0xA, 0x37, 0x7800, 1, D_80182DEC, 0, 0x14, -1);
    value = D_80182DEC - 2;
    D_80182DEC = value;
    if (value < 0)
    {
        D_80182DEC = 0;
    }
    timer_ptr = &D_801B301C;
    D_8013924C += 4;
    ((WmapShort4*)&D_801B24A8)->field_04 += 0;
    timer = *timer_ptr;
    next_timer = timer - 1;
    *timer_ptr = next_timer;
    if (next_timer == 0)
    {
        D_801B3018++;
    }
}

void func_800B3DD8(void)
{
    s32 value;
    s32 timer;

    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF30, (D_80139250 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_80182DF0, 0, -0xA, -1);
    D_80139250 += 0x10;
    value = D_80182DF0 + 8;
    D_80182DF0 = value;
    if (value >= 0x82)
    {
        D_80182DF0 = 0x81;
    }
    timer = D_801B3024 - 1;
    ((u16*)&D_8013B238)[2] += 0x20;
    D_801B3024 = timer;
    if (timer == 0)
    {
        D_801B3020++;
    }
}

void func_800B3EDC(void)
{
    s32 value;
    s32 timer;

    func_8006CFA8(&D_80182DC0, &D_8013B238);
    func_800675F0(D_8011CF30, (D_80139250 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_80182DF0, 0, -0xA, -1);
    value = D_80182DF0 - 2;
    D_80182DF0 = value;
    if (value < 0)
    {
        D_80182DF0 = 0;
    }
    D_80139250 += 0x10;
    timer = D_801B3024 - 1;
    D_801B3024 = timer;
    ((u16*)&D_8013B238)[2] += 0x20;
    if (timer == 0)
    {
        D_801B3020++;
    }
}

void func_800B3FDC(void)
{
    s32 value;
    s32 timer;

    func_8006CFA8(&D_80182DC0, &D_8013B240);
    func_800675F0(D_8011CF34, (D_80139260 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
    D_80139260 += 8;
    value = D_80182DF4 + 2;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    timer = D_801B302C - 1;
    ((u16*)&D_8013B240)[2] += 4;
    D_801B302C = timer;
    if (timer == 0)
    {
        D_801B3028++;
    }
}

void func_800B40DC(void)
{
    s32 value;
    s32 timer;

    func_8006CFA8(&D_80182DC0, &D_8013B240);
    func_800675F0(D_8011CF34, (D_80139260 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 - 4;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    D_80139260 += 8;
    timer = D_801B302C - 1;
    D_801B302C = timer;
    ((u16*)&D_8013B240)[2] += 4;
    if (timer == 0)
    {
        D_801B3028++;
    }
}

void func_800B41D8(void)
{
    s32 value;
    s32 timer;

    func_8006CFA8(&D_80182DC0, &D_801B2670);
    func_800675F0(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_801B25D8, 0, -0xA, -1);
    D_80139264 += 8;
    value = D_801B25D8 + 8;
    D_801B25D8 = value;
    if (value >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    timer = D_801B3034 - 1;
    ((u16*)&D_801B2670)[2] += 4;
    D_801B3034 = timer;
    if (timer == 0)
    {
        D_801B3030++;
    }
}

void func_800B42DC(void)
{
    s32 value;
    s32 timer;

    func_8006CFA8(&D_80182DC0, &D_801B2670);
    func_800675F0(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x35, 0x7800, 1, D_801B25D8, 0, -0xA, -1);
    value = D_801B25D8 - 2;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    D_80139264 += 8;
    timer = D_801B3034 - 1;
    D_801B3034 = timer;
    ((u16*)&D_801B2670)[2] += 4;
    if (timer == 0)
    {
        D_801B3030++;
    }
}

void func_800B43DC(void)
{
    s32 value;
    s32 fade;
    s32 timer;
    MATRIX matrix;

    value = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = value;
    if (value < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &matrix);
    TransMatrix(&matrix, &D_8011CF60);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    if (D_80182DE8 != 0)
    {
        func_800675F0(D_800DCF18, 0, 4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        fade = D_80182DE8 - 8;
        D_80182DE8 = fade;
        if (fade < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    timer = D_801B303C - 1;
    D_801B303C = timer;
    if (timer == 0)
    {
        D_801B3038++;
    }
}

s32 func_800B45B8(s32 reset)
{
    if (reset != 0)
    {
        D_801B2FE0 = 1;
        D_801B2FE4 = 1;
        return 1;
    }
    if ((u32)D_801B2FE0 >= 6)
    {
        return 0;
    }
    D_800D7314[D_801B2FE0]();
    return 1;
}

void func_800B4630(void)
{
    D_801B2FE0 = 1;
    D_801B2FE4 = 1;
}

void func_800B4648(void)
{
    if (D_801398D0 != 2)
    {
        D_801B2FE0++;
        func_800B4688();
    }
}

void func_800B4688(void)
{
    cdrom_wait_queue_empty();
    D_8011CF4C.field_00 = 0x94;
    D_8011CF4C.field_02 = 0x31;
    D_80182E38 = 4;
    D_800D9228 = 0xFF;
    D_8011D500 = 0xFF;
    func_8006CAC0(func_800B4774);
    D_8013B20C = 1;
    D_801B2FE0++;
    func_800B470C();
}

void func_800B470C(void)
{
    if (D_8013B20C == 0)
    {
        D_801B2FE0++;
        func_800B4748();
    }
}

void func_800B4748(void)
{
    D_8013B294 = 1;
    D_80139228 = 2;
    D_801B2FE0++;
}

s32 func_800B4774(s32 reset)
{
    if (reset != 0)
    {
        D_801B2FE8 = 1;
        D_801B2FEC = 1;
        return 1;
    }
    if ((u32)D_801B2FE8 >= 0x16)
    {
        return 0;
    }
    D_800D732C[D_801B2FE8]();
    return 1;
}

void func_800B47EC(void)
{
    D_801B2FE8 = 1;
    D_801B2FEC = 1;
}

void func_800B4804(void)
{
    D_8013B208 = 1;
    func_800652A8(0x38, 0x80);
    D_8011D500 = 0x7F;
    func_8006CAC0(func_800B5878);
    func_8006CAC0(func_800B55C0);
    func_8006683C(0x904060);
    D_801ADAF4 = 0xE;
    func_8006CAC0(func_800B5B2C);
    D_801B2FEC = 0x5A;
    D_801B2FE8++;
}

void func_800B4894(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B48C8(void)
{
    func_8006CAC0(func_800B4D08);
    D_801B2FEC = 4;
    D_801B2FE8++;
}

void func_800B4904(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B4938(void)
{
    func_8006CAC0(func_800B4D08);
    D_801ADAF4 = 9;
    D_801B2FEC = 0x77;
    D_801B2FE8++;
}

void func_800B4980(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B49B4(void)
{
    D_8011D500 = 1;
    func_8006CAC0(func_800B571C);
    D_801B2FEC = 0x14;
    D_801B2FE8++;
}

void func_800B49FC(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B4A30(void)
{
    func_8006CAC0(func_800B4EAC);
    D_801B2FEC = 0x24;
    D_801B2FE8++;
}

void func_800B4A6C(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B4AA0(void)
{
    D_801ADAF4 = 0x11;
    func_8006683C(0x755085);
    D_8011D500 = 0x23;
    func_8006CAC0(func_800B5308);
    D_801B2FEC = 0x14;
    D_801B2FE8++;
}

void func_800B4B00(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B4B34(void)
{
    D_801ADAF4 = 0xE;
    D_801B2FEC = 4;
    D_801B2FE8++;
}

void func_800B4B60(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B4B94(void)
{
    func_8006683C(0x654080);
    D_801ADAF4 = 0xA;
    func_8006CAC0(func_800B5050);
    D_801B2FEC = 0x1E;
    D_801B2FE8++;
}

void func_800B4BE8(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B4C1C(void)
{
    func_8006CAC0(func_800B51AC);
    D_801B2FEC = 0x64;
    D_801B2FE8++;
}

void func_800B4C58(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B4C8C(void)
{
    D_8011D500 = 0xFF;
    D_801B2FEC = 0x70;
    D_801B2FE8++;
}

void func_800B4CB8(void)
{
    s32 timer;

    timer = D_801B2FEC - 1;
    D_801B2FEC = timer;
    if (timer == 0)
    {
        D_801B2FE8++;
    }
}

void func_800B4CEC(void)
{
    D_8013B20C = 0;
    D_801B2FE8++;
}

s32 func_800B4D08(s32 reset)
{
    if (reset != 0)
    {
        D_801B2FF0 = 1;
        D_801B2FF4 = 1;
        return 1;
    }
    if ((u32)D_801B2FF0 >= 4)
    {
        return 0;
    }
    D_800D7384[D_801B2FF0]();
    return 1;
}

void func_800B4D80(void)
{
    D_801B2FF0 = 1;
    D_801B2FF4 = 1;
}

void func_800B4D98(void)
{
    D_801399AC = &D_8011D538;
    D_800D9318.field_06 = 0xF;
    D_800D9318.field_10 = -1;
    D_800D9318.field_26 = 0x10;
    D_800D9318.field_22 = 1;
    D_800D9318.field_02 = 0;
    D_800D9318.field_0E = 0;
    D_800D9318.field_24 = 0x81;
    D_801B2FF4 = 0x10;
    D_801B2FF0++;
    func_800B4E18();
}

void func_800B4E18(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9318, &D_801399A8);
    func_80066F9C(&D_800D9318, *(s32*)&D_8011CF4C, 0xF, 2, 0);
    timer = D_801B2FF4 - 1;
    D_801B2FF4 = timer;
    if (timer == 0)
    {
        D_801B2FF0++;
    }
}

void func_800B4E94(void)
{
    D_801B2FF0++;
}

s32 func_800B4EAC(s32 reset)
{
    if (reset != 0)
    {
        D_801B2FF8 = 1;
        D_801B2FFC = 1;
        return 1;
    }
    if ((u32)D_801B2FF8 >= 4)
    {
        return 0;
    }
    D_800D7394[D_801B2FF8]();
    return 1;
}

void func_800B4F24(void)
{
    D_801B2FF8 = 1;
    D_801B2FFC = 1;
}

void func_800B4F3C(void)
{
    s32 value;

    D_801399B4 = &D_8011D538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_0E = value = 1;
    D_800D9344.field_10 = -value;
    D_800D9344.field_26 = 2;
    D_800D9344.field_24 = value;
    D_800D9344.field_02 = 0;
    D_800D9344.field_22 = 0x81;
    D_801B2FFC = 0x118;
    D_801B2FF8++;
    func_800B4FBC();
}

void func_800B4FBC(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, *(s32*)&D_8011CF4C, 0xF, 5, 0);
    timer = D_801B2FFC - 1;
    D_801B2FFC = timer;
    if (timer == 0)
    {
        D_801B2FF8++;
    }
}

void func_800B5038(void)
{
    D_801B2FF8++;
}

s32 func_800B5050(s32 reset)
{
    if (reset != 0)
    {
        D_801B3000 = 1;
        D_801B3004 = 1;
        return 1;
    }
    if ((u32)D_801B3000 >= 6)
    {
        return 0;
    }
    D_800D73A4[D_801B3000]();
    return 1;
}

void func_800B50C8(void)
{
    D_801B3000 = 1;
    D_801B3004 = 1;
}

void func_800B50E0(void)
{
    D_801B24B4 = 1;
    D_801B2490 = D_80139258;
    D_80139234 = 0;
    D_801B3004 = 0xA9;
    D_801B3000++;
    func_800B35F0();
}

void func_800B515C(void)
{
    D_801B3004 = 0x20;
    D_801B3000++;
    func_800B36F0();
}

void func_800B5194(void)
{
    D_801B3000++;
}

s32 func_800B51AC(s32 reset)
{
    if (reset != 0)
    {
        D_801B3008 = 1;
        D_801B300C = 1;
        return 1;
    }
    if ((u32)D_801B3008 >= 6)
    {
        return 0;
    }
    D_800D73BC[D_801B3008]();
    return 1;
}

void func_800B5224(void)
{
    D_801B3008 = 1;
    D_801B300C = 1;
}

void func_800B523C(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B300C = 0xA8;
    D_801B3008++;
    func_800B37EC();
}

void func_800B52B8(void)
{
    D_801B300C = 0x20;
    D_801B3008++;
    func_800B38EC();
}

void func_800B52F0(void)
{
    D_801B3008++;
}

s32 func_800B5308(s32 reset)
{
    if (reset != 0)
    {
        D_801B3010 = 1;
        D_801B3014 = 1;
        return 1;
    }
    if ((u32)D_801B3010 >= 6)
    {
        return 0;
    }
    D_800D73D4[D_801B3010]();
    return 1;
}

void func_800B5380(void)
{
    D_801B3010 = 1;
    D_801B3014 = 1;
}

void func_800B5398(void)
{
    D_80182DE8 = 1;
    *(WmapPair*)&D_801B24A0 = D_80139258;
    D_80139240 = 0;
    D_801B3014 = 0xBE;
    D_801B3010++;
    func_800B39E8();
}

void func_800B5414(void)
{
    D_801B3014 = 0x20;
    D_801B3010++;
    func_800B3AE8();
}

void func_800B544C(void)
{
    D_801B3010++;
}

s32 func_800B5464(s32 reset)
{
    if (reset != 0)
    {
        D_801B3018 = 1;
        D_801B301C = 1;
        return 1;
    }
    if ((u32)D_801B3018 >= 6)
    {
        return 0;
    }
    D_800D73EC[D_801B3018]();
    return 1;
}

void func_800B54DC(void)
{
    D_801B3018 = 1;
    D_801B301C = 1;
}

void func_800B54F4(void)
{
    D_80182DEC = 1;
    D_801B24A8 = D_80139258;
    D_8013924C = 0;
    D_801B301C = 0x1E;
    D_801B3018++;
    func_800B3BE4();
}

void func_800B5570(void)
{
    D_801B301C = 0x40;
    D_801B3018++;
    func_800B3CE0();
}

void func_800B55A8(void)
{
    D_801B3018++;
}

s32 func_800B55C0(s32 reset)
{
    if (reset != 0)
    {
        D_801B3020 = 1;
        D_801B3024 = 1;
        return 1;
    }
    if ((u32)D_801B3020 >= 6)
    {
        return 0;
    }
    D_800D7404[D_801B3020]();
    return 1;
}

void func_800B5638(void)
{
    D_801B3020 = 1;
    D_801B3024 = 1;
}

void func_800B5650(void)
{
    D_80182DF0 = 1;
    D_8013B238 = D_80139258;
    D_80139250 = 0;
    D_801B3024 = 0xB4;
    D_801B3020++;
    func_800B3DD8();
}

void func_800B56CC(void)
{
    D_801B3024 = 0x40;
    D_801B3020++;
    func_800B3EDC();
}

void func_800B5704(void)
{
    D_801B3020++;
}

s32 func_800B571C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3028 = 1;
        D_801B302C = 1;
        return 1;
    }
    if ((u32)D_801B3028 >= 6)
    {
        return 0;
    }
    D_800D741C[D_801B3028]();
    return 1;
}

void func_800B5794(void)
{
    D_801B3028 = 1;
    D_801B302C = 1;
}

void func_800B57AC(void)
{
    D_80182DF4 = 1;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B302C = 0xF8;
    D_801B3028++;
    func_800B3FDC();
}

void func_800B5828(void)
{
    D_801B302C = 0x20;
    D_801B3028++;
    func_800B40DC();
}

void func_800B5860(void)
{
    D_801B3028++;
}

s32 func_800B5878(s32 reset)
{
    if (reset != 0)
    {
        D_801B3030 = 1;
        D_801B3034 = 1;
        return 1;
    }
    if ((u32)D_801B3030 >= 6)
    {
        return 0;
    }
    D_800D7434[D_801B3030]();
    return 1;
}

void func_800B58F0(void)
{
    D_801B3030 = 1;
    D_801B3034 = 1;
}

void func_800B5908(void)
{
    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139264 = 0;
    D_801B3034 = 0xB4;
    D_801B3030++;
    func_800B41D8();
}

void func_800B5984(void)
{
    D_801B3034 = 0x40;
    D_801B3030++;
    func_800B42DC();
}

void func_800B59BC(void)
{
    D_801B3030++;
}

s32 func_800B59D4(s32 reset)
{
    if (reset != 0)
    {
        D_801B3038 = 1;
        D_801B303C = 1;
        return 1;
    }
    if ((u32)D_801B3038 >= 4)
    {
        return 0;
    }
    D_800D744C[D_801B3038]();
    return 1;
}

void func_800B5A4C(void)
{
    D_801B3038 = 1;
    D_801B303C = 1;
}

void func_800B5A64(void)
{
    *(WmapPair*)&D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B303C = 0x10;
    D_801B3038++;
    func_800B43DC();
}

void func_800B5B14(void)
{
    D_801B3038++;
}

s32 func_800B5B2C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3040 = 1;
        D_801B3044 = 1;
        return 1;
    }
    if ((u32)D_801B3040 >= 6)
    {
        return 0;
    }
    D_800D745C[D_801B3040]();
    return 1;
}

void func_800B5BA4(void)
{
    D_801B3040 = 1;
    D_801B3044 = 1;
}

void func_800B5BBC(void)
{
    s32 timer;

    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, (u8*)D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    timer = D_801B3044 - 1;
    D_801B3044 = timer;
    if (timer == 0)
    {
        D_801B3040++;
    }
}

void func_800B5C34(void)
{
    D_801B3044 = 0x20;
    D_80139280[1].field_28 = 0x270F;
    D_801B3040++;
    func_800B5C7C();
}

void func_800B5C7C(void)
{
    s32 timer;

    func_8006AEE0();
    func_8008ECF8(0x14, 0x78, D_8011CF2C, (u8*)D_80139280 + 0x78);
    func_8006534C(0x25, 5);
    timer = D_801B3044 - 1;
    D_801B3044 = timer;
    if (timer == 0)
    {
        D_801B3040++;
    }
}

void func_800B5CF4(void)
{
    D_801B3040++;
}

void func_800B5D0C(void)
{
    D_8013986C = -1;
    D_801ADAF4 = 0;
    D_80182D74.field_00 = 0xB4;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x96;
    D_80182D80.field_00 = 0xB4;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x96;
    D_80182D8C.field_00 = 0xB4;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0x96;
    D_80182D94.field_00 = 0xB4;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0x96;
    D_801B3054 = 0x38;
    D_801B3050++;
}

void func_800B5D98(void)
{
    D_80139244 = 1;
    func_8006CAC0(func_800B8A2C);
    D_801ADAF4 = 0;
    D_80182D74.field_00 = 0xC8;
    D_80182D74.field_01 = 0xC8;
    D_80182D74.field_02 = 0xC8;
    D_80182D80.field_00 = 0xC8;
    D_80182D80.field_01 = 0xC8;
    D_80182D80.field_02 = 0xC8;
    D_80182D8C.field_00 = 0xC8;
    D_80182D8C.field_01 = 0xC8;
    D_80182D8C.field_02 = 0xC8;
    D_80182D94.field_00 = 0xC8;
    D_80182D94.field_01 = 0xC8;
    D_80182D94.field_02 = 0xC8;
    D_801B3054 = 0xF;
    D_801B3050++;
}

void func_800B5E3C(void)
{
    func_8006CAC0(func_800B94FC);
    func_8006CAC0(func_800B977C);
    func_8006CAC0(func_800B93A0);
    func_8006CAC0(func_800B862C);
    func_8006CAC0(func_800B84D0);
    D_801ADAF4 = 0;
    D_80182D74.field_00 = 0x60;
    D_80182D74.field_01 = 0xC;
    D_80182D74.field_02 = 0xC8;
    D_80182D80.field_00 = 0x60;
    D_80182D80.field_01 = 0xC;
    D_80182D80.field_02 = 0xC8;
    D_80182D8C.field_00 = 0xA0;
    D_80182D8C.field_01 = 0xA0;
    D_80182D8C.field_02 = 0xA0;
    D_80182D94.field_00 = 0xA0;
    D_80182D94.field_01 = 0xA0;
    D_80182D94.field_02 = 0xA0;
    D_801B3054 = 0x14;
    D_801B3050++;
}

void func_800B5F10(void)
{
    D_801ADAF4 = 0;
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0x64;
    D_80182D74.field_02 = 0x78;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0x64;
    D_80182D80.field_02 = 0x78;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0x64;
    D_80182D8C.field_02 = 0x78;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0x64;
    D_80182D94.field_02 = 0x78;
    D_8013B29C = 0;
    func_8006683C(0x701050);
    D_8013986C = 0;
    D_801ADAF4 = 0;
    func_8006CAC0(func_800B9E08);
    D_801B3054 = 0x58;
    D_801B3050++;
}

void func_800B5FD4(void)
{
    func_8006CAC0(func_800B8824);
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    D_800D9240 = D_800D06BC;
    D_801B3054 = 0x78;
    D_801B3050++;
}

void func_800B619C(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    func_800675F0(D_8011CF1C, (D_80139234 / 0x10) & 3, 0xA, 0x36, 0x7940, 0x1001, D_801B24B4, 0, 0, -1);
    D_80139234 += 0x10;
    value = D_801B24B4 + 2;
    D_801B24B4 = value;
    if (value >= 0x82)
    {
        D_801B24B4 = 0x81;
    }
    ((WmapShort4*)&D_801B2490)->field_04 += 0x40;
    timer = D_801B3074 - 1;
    D_801B3074 = timer;
    if (timer == 0)
    {
        D_801B3070++;
    }
}

void func_800B6284(void)
{
    s32 value;
    s32 timer;
    WmapShort4* position;

    func_8006AEE0();
    func_800675F0(D_8011CF1C, (D_80139234 / 0x10) & 3, 0xA, 0x36, 0x7940, 0x1001, D_801B24B4, 0, 0, -1);
    value = D_801B24B4 - 2;
    D_801B24B4 = value;
    if (value < 0)
    {
        D_801B24B4 = 0;
    }
    position = (WmapShort4*)&D_801B2490;
    D_80139234 += 0x10;
    timer = D_801B3074 - 1;
    D_801B3074 = timer;
    position->field_04 += 0x40;
    if (timer == 0)
    {
        D_801B3070++;
    }
}

void func_800B6558(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = value;
    if (value < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    func_8006D150(&D_801B24A0);
    if (D_80182DE8 != 0)
    {
        func_800675F0(D_8011CF34, 0, 4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        fade = D_80182DE8 - 1;
        D_80182DE8 = fade;
        if (fade < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    timer = D_801B308C - 1;
    D_801B308C = timer;
    if (timer == 0)
    {
        D_801B3088++;
    }
}

void func_800B6644(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_801B2478.vz - 0xDAC;
    D_801B2478.vz = value;
    if (value < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }
    PushMatrix();
    func_8006D150(&D_801B24A8);
    if (D_80182DEC != 0)
    {
        func_800675F0(D_8011CF3C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, 0, 0, -1);
        fade = D_80182DEC - 4;
        D_80182DEC = fade;
        if (fade < 0)
        {
            D_80182DEC = 0;
        }
    }
    PopMatrix();
    timer = D_801B3094 - 1;
    D_801B3094 = timer;
    if (timer == 0)
    {
        D_801B3090++;
    }
}

void func_800B6730(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_80139870.vz - 0xDAC;
    D_80139870.vz = value;
    if (value < 0x2710)
    {
        D_80139870.vz = 0x2710;
    }
    PushMatrix();
    func_8006D150(&D_8013B238);
    if (D_80182DF0 != 0)
    {
        func_800675F0(D_8011CF2C, 0, 4, 0x35, 0x7800, 1, D_80182DF0, 0, 0, -1);
        fade = D_80182DF0 - 2;
        D_80182DF0 = fade;
        if (fade < 0)
        {
            D_80182DF0 = 0;
        }
    }
    PopMatrix();
    timer = D_801B309C - 1;
    D_801B309C = timer;
    if (timer == 0)
    {
        D_801B3098++;
    }
}

void func_800B681C(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_801B2498);
    func_800675F0(D_800DCF18, (D_8013923C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_80182DE4, 6, -0x18, -1);
    D_8013923C += 0x10;
    value = D_80182DE4 + 4;
    D_80182DE4 = value;
    if (value >= 0x82)
    {
        D_80182DE4 = 0x81;
    }
    ((u16*)&D_801B2498)[2] += 0x10;
    timer = D_801B30A4 - 1;
    D_801B30A4 = timer;
    if (timer == 0)
    {
        D_801B30A0++;
    }
}

void func_800B6918(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_801B2498);
    func_800675F0(D_800DCF18, (D_8013923C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_80182DE4, 6, -0x18, -1);
    value = D_80182DE4 - 0x10;
    D_80182DE4 = value;
    if (value < 0)
    {
        D_80182DE4 = 0;
    }
    D_8013923C += 0x10;
    timer = D_801B30A4 - 1;
    D_801B30A4 = timer;
    ((u16*)&D_801B2498)[2] += 0x10;
    if (timer == 0)
    {
        D_801B30A0++;
    }
}

void func_800B6A10(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_801B3118);
    func_800675F0(D_800DCF18, (D_8013926C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_801B25E0, 4, -0x14, -1);
    D_8013926C -= 0x10;
    value = D_801B25E0 + 2;
    D_801B25E0 = value;
    if (value >= 0x82)
    {
        D_801B25E0 = 0x81;
    }
    ((u16*)&D_801B3118)[2] += 0x10;
    timer = D_801B30AC - 1;
    D_801B30AC = timer;
    if (timer == 0)
    {
        D_801B30A8++;
    }
}

void func_800B6B0C(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_801B3118);
    func_800675F0(D_800DCF18, (D_8013926C / 0x10) & 7, 0xA, 0x36, 0x7880, 0x1001, D_801B25E0, 4, -0x14, -1);
    value = D_801B25E0 - 0x80;
    D_801B25E0 = value;
    if (value < 0)
    {
        D_801B25E0 = 0;
    }
    D_8013926C -= 0x10;
    timer = D_801B30AC - 1;
    D_801B30AC = timer;
    ((u16*)&D_801B3118)[2] += 0x10;
    if (timer == 0)
    {
        D_801B30A8++;
    }
}

void func_800B6C04(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_8013B240);
    func_800675F0(D_8011CF28, (D_80139260 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_80182DF4, 4, -0x14, -1);
    D_80139260 -= 0x20;
    value = D_80182DF4 + 2;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    ((u16*)&D_8013B240)[2] += 0x90;
    timer = D_801B30B4 - 1;
    D_801B30B4 = timer;
    if (timer == 0)
    {
        D_801B30B0++;
    }
}

void func_800B6D00(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_8013B240);
    func_800675F0(D_8011CF28, (D_80139260 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_80182DF4, 4, -0x14, -1);
    value = D_80182DF4 - 8;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    D_80139260 -= 0x20;
    timer = D_801B30B4 - 1;
    D_801B30B4 = timer;
    ((u16*)&D_8013B240)[2] += 0x90;
    if (timer == 0)
    {
        D_801B30B0++;
    }
}

void func_800B6DF8(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_801B2670);
    func_800675F0(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 6, -0x18, -1);
    D_80139264 += 0x10;
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x82)
    {
        D_801B25D8 = 0x81;
    }
    ((u16*)&D_801B2670)[2] += 0x60;
    timer = D_801B30BC - 1;
    D_801B30BC = timer;
    if (timer == 0)
    {
        D_801B30B8++;
    }
}

void func_800B6EF4(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_801B2670);
    func_800675F0(D_8011CF38, (D_80139264 / 0x10) & 3, 0xA, 0x36, 0x7900, 0x1001, D_801B25D8, 6, -0x18, -1);
    value = D_801B25D8 - 0x10;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    D_80139264 += 0x10;
    timer = D_801B30BC - 1;
    D_801B30BC = timer;
    ((u16*)&D_801B2670)[2] += 0x60;
    if (timer == 0)
    {
        D_801B30B8++;
    }
}

void func_800B71A8(void)
{
    s32 index;
    s32 config_offset;
    s32 screen_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = D_80139988;
    resource = &D_80121538;
    screen_offset = 0x640;
    config_offset = 0xFA0;
    D_80139280[2].field_00 = 0x28;
    D_80139280[2].field_04 = 0xC8;
    D_80139280[2].field_08 = -0x136;
    D_80139280[2].field_0C = 0x28;
    D_80139280[2].field_10 = -0x64;
    D_80139280[2].field_14 = 0x28;
    D_80139280[2].field_18 = 0;
    D_80139280[2].field_1C = 0x2D;
    D_80139280[2].field_20 = 0x81;
    D_80139280[2].state_24 = 1;
    D_80139280[2].field_28 = 0x20;
    D_80139280[2].field_2C = 4;
    D_80139280[2].field_30 = 0xF;
    D_80139280[2].field_34 = 0;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 0x28);

    D_801B30F4 = 0x64;
    D_801B30F0++;
    func_800BA108();
}

s32 func_800B7290(s32 reset)
{
    if (reset != 0)
    {
        D_801B3048 = 1;
        D_801B304C = 1;
        return 1;
    }

    if ((u32)D_801B3048 >= 6)
    {
        return 0;
    }

    D_800D7474[D_801B3048]();
    return 1;
}

void func_800B7308(void)
{
    D_801B3048 = 1;
    D_801B304C = 1;
}

void func_800B7320(void)
{
    func_8006CAC0(func_8006C81C);
    D_8013B20C = 1;
    D_801B3048++;
    func_800B7364();
}

void func_800B7364(void)
{
    if (D_8013B20C == 0)
    {
        D_801B3048++;
        func_800B73A0();
    }
}

void func_800B73A0(void)
{
    func_8006CAC0(func_800B7438);
    D_8013B20C = 1;
    D_801B3048++;
    func_800B73E4();
}

void func_800B73E4(void)
{
    if (D_8013B20C == 0)
    {
        D_801B3048++;
        func_800B7420();
    }
}

void func_800B7420(void)
{
    D_801B3048++;
}

s32 func_800B7438(s32 reset)
{
    if (reset != 0)
    {
        D_801B3050 = 1;
        D_801B3054 = 1;
        return 1;
    }

    if ((u32)D_801B3050 >= 0x2C)
    {
        return 0;
    }

    D_800D748C[D_801B3050]();
    return 1;
}

void func_800B74B0(void)
{
    D_801B3050 = 1;
    D_801B3054 = 1;
}

void func_800B74C8(void)
{
    func_800652A8(0x34, 0x80);
    D_8013B208 = 1;
    func_8006CAC0(func_800B9B9C);
    D_801B3054 = 0xF;
    D_801B3050++;
}

void func_800B751C(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7550(void)
{
    func_8006CAC0(func_800BA078);
    D_801B3054 = 0xF;
    D_801B3050++;
}

void func_800B758C(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B75C0(void)
{
    func_8006CAC0(func_800B8084);
    D_801B3054 = 0x16;
    D_801B3050++;
}

void func_800B75FC(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7630(void)
{
    func_8006CAC0(func_800B9244);
    D_801ADAF4 = 0xC;
    func_8006683C(0x704060);
    D_801B3054 = 0xB;
    D_801B3050++;
}

void func_800B7684(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B76B8(void)
{
    D_801ADAE0 = 1;
    D_801B3054 = 0x28;
    D_801B3050++;
}

void func_800B76E4(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7718(void)
{
    func_8006CAC0(func_800B99F8);
    func_8006683C(0x352030);
    func_8006CAC0(func_800B8E30);
    D_801B3054 = 0xA;
    D_801B3050++;
}

void func_800B776C(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B77A0(void)
{
    D_8013B254 = 2;
    D_8013B29C = 1;
    D_801B3054 = 4;
    D_801B3050++;
}

void func_800B77D8(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B780C(void)
{
    func_8006683C(0);
    D_801ADAF4 = 1;
    D_801B3054 = 0xF;
    D_801B3050++;
}

void func_800B7850(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7884(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B78B8(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B78EC(void)
{
    func_8006CBD8(func_8006C0EC);
    D_80139210.field_00 = 0x132;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0x1C2;
    D_80139968.field_00 = 0x26C;
    D_80139968.field_04 = 0;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0x132;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = -0x1C2;
    D_80139200.field_00 = 0x26C;
    D_80139200.field_04 = 0;
    D_80139200.field_08 = 0;
    D_801B3054 = 0xA;
    D_801B3050++;
}

void func_800B7988(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B79BC(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B79F0(void)
{
    D_80139210.field_00 = 0;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0;
    D_80139968.field_00 = 5;
    D_80139968.field_04 = 0;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = 0;
    D_80139200.field_00 = 0;
    D_80139200.field_04 = 0;
    D_80139200.field_08 = 0;
    D_801B3054 = 0x1E;
    D_801B3050++;
}

void func_800B7A64(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7A98(void)
{
    func_8006CAC0(func_800B822C);
    D_801B3054 = 0xA0;
    D_801B3050++;
}

void func_800B7AD4(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7B08(void)
{
    func_8006CAC0(func_800B8B80);
    D_801B3054 = 7;
    D_801B3050++;
}

void func_800B7B44(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7B78(void)
{
    func_8006CAC0(func_800B8F8C);
    D_801B3054 = 0x14;
    D_801B3050++;
}

void func_800B7BB4(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7BE8(void)
{
    func_8006CAC0(func_800B7E7C);
    D_801B3054 = 0x28;
    D_801B3050++;
}

void func_800B7C24(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7C58(void)
{
    func_8006CAC0(func_800B90E8);
    D_801ADAF4 = 0;
    D_80182D74.field_00 = 0;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0;
    D_80182D80.field_00 = 0;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    D_801B3054 = 0x1E;
    D_801B3050++;
}

void func_800B7CEC(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7D20(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7D54(void)
{
    func_8006CAC0(func_800B8CD8);
    func_8006683C(0x808080);
    D_801ADAF4 = 0x10;
    D_80139244 = 0;
    D_801B3054 = 0x1C;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B3050++;
}

void func_800B7DF8(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7E2C(void)
{
    s32 timer;

    timer = D_801B3054 - 1;
    D_801B3054 = timer;
    if (timer == 0)
    {
        D_801B3050++;
    }
}

void func_800B7E60(void)
{
    D_8013B20C = 0;
    D_801B3050++;
}

s32 func_800B7E7C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3058 = 1;
        D_801B305C = 1;
        return 1;
    }

    if ((u32)D_801B3058 >= 6)
    {
        return 0;
    }

    D_800D753C[D_801B3058]();
    return 1;
}

void func_800B7EF4(void)
{
    D_801B3058 = 1;
    D_801B305C = 1;
}

void func_800B7F0C(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x28, 0, 0x7F, 4, 0, (u8*)D_80139280 + 0x78);
    timer = D_801B305C - 1;
    D_801B305C = timer;
    if (timer == 0)
    {
        D_801B3058++;
    }
}

void func_800B7F98(void)
{
    D_801B305C = 0x20;
    D_80139280[1].field_3C = -1;
    D_801B3058++;
    func_800B7FE0();
}

void func_800B7FE0(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x28, 0, 0x7F, 4, 0, (u8*)D_80139280 + 0x78);
    timer = D_801B305C - 1;
    D_801B305C = timer;
    if (timer == 0)
    {
        D_801B3058++;
    }
}

void func_800B806C(void)
{
    D_801B3058++;
}

s32 func_800B8084(s32 reset)
{
    if (reset != 0)
    {
        D_801B3060 = 1;
        D_801B3064 = 1;
        return 1;
    }

    if ((u32)D_801B3060 >= 4)
    {
        return 0;
    }

    D_800D7554[D_801B3060]();
    return 1;
}

void func_800B80FC(void)
{
    D_801B3060 = 1;
    D_801B3064 = 1;
}

void func_800B8114(void)
{
    D_801399B4 = &D_8011F538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_0E = 3;
    D_800D9344.field_10 = -1;
    D_800D9344.field_26 = 8;
    D_800D9344.field_22 = 0x81;
    D_800D9344.field_02 = 0;
    D_800D9344.field_24 = 1;
    D_801B3064 = 0x64;
    D_801B3060++;
    func_800B8198();
}

void func_800B8198(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, *(s32*)&D_8011CF4C, 0xF, 2, 0);
    timer = D_801B3064 - 1;
    D_801B3064 = timer;
    if (timer == 0)
    {
        D_801B3060++;
    }
}

void func_800B8214(void)
{
    D_801B3060++;
}

s32 func_800B822C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3068 = 1;
        D_801B306C = 1;
        return 1;
    }

    if ((u32)D_801B3068 >= 6)
    {
        return 0;
    }

    D_800D7564[D_801B3068]();
    return 1;
}

void func_800B82A4(void)
{
    D_801B3068 = 1;
    D_801B306C = 1;
}

void func_800B82BC(void)
{
    D_801399BC = &D_8011D538;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 2;
    D_800D9370.field_22 = 0x81;
    D_800D9370.field_02 = 0;
    D_800D9370.field_06 = 0;
    D_800D9370.field_0E = 0;
    D_800D9370.field_24 = 1;
    D_801B306C = 0x10E;
    D_801B3068++;
    func_800B8338();
}

void func_800B8338(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, *(s32*)&D_8011CF4C, 0x10, 8, 1);
    timer = D_801B306C - 1;
    D_801B306C = timer;
    if (timer == 0)
    {
        D_801B3068++;
    }
}

void func_800B83B8(void)
{
    D_800D9370.field_26 = 2;
    D_800D9370.field_22 = 0;
    D_80139268 = 0;
    D_801B306C = 0x40;
    D_801B3068++;
    func_800B840C();
}

void func_800B840C(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, *(s32*)&D_8011CF4C, 0x10, 8, 1);
    D_800D9370.field_06 = D_80139268 >> 4;
    D_80139268 += 0x10;
    if (D_80139268 >= 0xF1)
    {
        D_80139268 = 0xF0;
    }
    timer = D_801B306C - 1;
    D_801B306C = timer;
    if (timer == 0)
    {
        D_801B3068++;
    }
}

void func_800B84B8(void)
{
    D_801B3068++;
}

s32 func_800B84D0(s32 reset)
{
    if (reset != 0)
    {
        D_801B3070 = 1;
        D_801B3074 = 1;
        return 1;
    }

    if ((u32)D_801B3070 >= 6)
    {
        return 0;
    }

    D_800D757C[D_801B3070]();
    return 1;
}

void func_800B8548(void)
{
    D_801B3070 = 1;
    D_801B3074 = 1;
}

void func_800B8560(void)
{
    D_801B24B4 = 1;
    D_801B2490 = D_80139258;
    D_80139234 = 0;
    D_801B3074 = 0x92;
    D_801B3070++;
    func_800B619C();
}

void func_800B85DC(void)
{
    D_801B3074 = 0x40;
    D_801B3070++;
    func_800B6284();
}

void func_800B8614(void)
{
    D_801B3070++;
}

s32 func_800B862C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3078 = 1;
        D_801B307C = 1;
        return 1;
    }

    if ((u32)D_801B3078 >= 6)
    {
        return 0;
    }

    D_800D7594[D_801B3078]();
    return 1;
}

void func_800B86A4(void)
{
    D_801B3078 = 1;
    D_801B307C = 1;
}

void func_800B86BC(void)
{
    s32 timer;

    func_8006ADD0(&D_80182DC0, &D_801B3120);
    func_8006C448(D_80139280);
    D_801B3120.vz += 0x18;
    timer = D_801B307C - 1;
    D_801B307C = timer;
    if (timer == 0)
    {
        D_801B3078++;
    }
}

void func_800B8734(void)
{
    s32 i;
    WmapConfigA* configs;

    configs = (WmapConfigA*)D_800D95D8;
    for (i = 0; i < 0x28; i++)
    {
        configs[i].field_22 = 0;
        configs[i].field_26 = 8;
    }
    D_801B307C = 0x10;
    D_801B3078++;
    func_800B8794();
}

void func_800B8794(void)
{
    s32 timer;

    func_8006ADD0(&D_80182DC0, &D_801B3120);
    func_8006C448(D_80139280);
    D_801B3120.vz += 0x18;
    timer = D_801B307C - 1;
    D_801B307C = timer;
    if (timer == 0)
    {
        D_801B3078++;
    }
}

void func_800B880C(void)
{
    D_801B3078++;
}

s32 func_800B8824(s32 reset)
{
    if (reset != 0)
    {
        D_801B3080 = 1;
        D_801B3084 = 1;
        return 1;
    }

    if ((u32)D_801B3080 >= 6)
    {
        return 0;
    }

    D_800D75AC[D_801B3080]();
    return 1;
}

void func_800B889C(void)
{
    D_801B3080 = 1;
    D_801B3084 = 1;
}

void func_800B88B4(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x20, 0, 0x7F, 2, 0, D_80139280 + 1);
    timer = D_801B3084 - 1;
    D_801B3084 = timer;
    if (timer == 0)
    {
        D_801B3080++;
    }
}

void func_800B8940(void)
{
    D_801B3084 = 0x20;
    D_80139280[1].field_14 = -1;
    D_801B3080++;
    func_800B8988();
}

void func_800B8988(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D9D68, D_80139B88, 0x20, 0, 0x7F, 2, 0, D_80139280 + 1);
    timer = D_801B3084 - 1;
    D_801B3084 = timer;
    if (timer == 0)
    {
        D_801B3080++;
    }
}

void func_800B8A14(void)
{
    D_801B3080++;
}

s32 func_800B8A2C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3088 = 1;
        D_801B308C = 1;
        return 1;
    }

    if ((u32)D_801B3088 >= 4)
    {
        return 0;
    }

    D_800D75C4[D_801B3088]();
    return 1;
}

void func_800B8AA4(void)
{
    D_801B3088 = 1;
    D_801B308C = 1;
}

void func_800B8ABC(void)
{
    *(WmapPair*)&D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B308C = 0x80;
    D_801B3088++;
    func_800B6558();
}

void func_800B8B68(void)
{
    D_801B3088++;
}

s32 func_800B8B80(s32 reset)
{
    if (reset != 0)
    {
        D_801B3090 = 1;
        D_801B3094 = 1;
        return 1;
    }

    if ((u32)D_801B3090 >= 4)
    {
        return 0;
    }

    D_800D75D4[D_801B3090]();
    return 1;
}

void func_800B8BF8(void)
{
    D_801B3090 = 1;
    D_801B3094 = 1;
}

void func_800B8C10(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B3094 = 0x20;
    D_801B3090++;
    func_800B6644();
}

void func_800B8CC0(void)
{
    D_801B3090++;
}

s32 func_800B8CD8(s32 reset)
{
    if (reset != 0)
    {
        D_801B3098 = 1;
        D_801B309C = 1;
        return 1;
    }

    if ((u32)D_801B3098 >= 4)
    {
        return 0;
    }

    D_800D75E4[D_801B3098]();
    return 1;
}

void func_800B8D50(void)
{
    D_801B3098 = 1;
    D_801B309C = 1;
}

void func_800B8D68(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    D_801B309C = 0x40;
    D_801B3098++;
    func_800B6730();
}

void func_800B8E18(void)
{
    D_801B3098++;
}

s32 func_800B8E30(s32 reset)
{
    if (reset != 0)
    {
        D_801B30A0 = 1;
        D_801B30A4 = 1;
        return 1;
    }

    if ((u32)D_801B30A0 >= 6)
    {
        return 0;
    }

    D_800D75F4[D_801B30A0]();
    return 1;
}

void func_800B8EA8(void)
{
    D_801B30A0 = 1;
    D_801B30A4 = 1;
}

void func_800B8EC0(void)
{
    D_80182DE4 = 1;
    D_801B2498 = D_80139258;
    D_8013923C = 0;
    D_801B30A4 = 0x5A;
    D_801B30A0++;
    func_800B681C();
}

void func_800B8F3C(void)
{
    D_801B30A4 = 8;
    D_801B30A0++;
    func_800B6918();
}

void func_800B8F74(void)
{
    D_801B30A0++;
}

s32 func_800B8F8C(s32 reset)
{
    if (reset != 0)
    {
        D_801B30A8 = 1;
        D_801B30AC = 1;
        return 1;
    }

    if ((u32)D_801B30A8 >= 6)
    {
        return 0;
    }

    D_800D760C[D_801B30A8]();
    return 1;
}

void func_800B9004(void)
{
    D_801B30A8 = 1;
    D_801B30AC = 1;
}

void func_800B901C(void)
{
    D_801B25E0 = 1;
    D_801B3118 = D_80139258;
    D_8013926C = 0;
    D_801B30AC = 0xB6;
    D_801B30A8++;
    func_800B6A10();
}

void func_800B9098(void)
{
    D_801B30AC = 1;
    D_801B30A8++;
    func_800B6B0C();
}

void func_800B90D0(void)
{
    D_801B30A8++;
}

s32 func_800B90E8(s32 reset)
{
    if (reset != 0)
    {
        D_801B30B0 = 1;
        D_801B30B4 = 1;
        return 1;
    }

    if ((u32)D_801B30B0 >= 6)
    {
        return 0;
    }

    D_800D7624[D_801B30B0]();
    return 1;
}

void func_800B9160(void)
{
    D_801B30B0 = 1;
    D_801B30B4 = 1;
}

void func_800B9178(void)
{
    D_80182DF4 = 0x81;
    D_8013B240 = D_80139258;
    D_80139260 = 0;
    D_801B30B4 = 0x68;
    D_801B30B0++;
    func_800B6C04();
}

void func_800B91F4(void)
{
    D_801B30B4 = 0x10;
    D_801B30B0++;
    func_800B6D00();
}

void func_800B922C(void)
{
    D_801B30B0++;
}

s32 func_800B9244(s32 reset)
{
    if (reset != 0)
    {
        D_801B30B8 = 1;
        D_801B30BC = 1;
        return 1;
    }

    if ((u32)D_801B30B8 >= 6)
    {
        return 0;
    }

    D_800D763C[D_801B30B8]();
    return 1;
}

void func_800B92BC(void)
{
    D_801B30B8 = 1;
    D_801B30BC = 1;
}

void func_800B92D4(void)
{
    D_801B25D8 = 1;
    D_801B2670 = D_80139258;
    D_80139264 = 0;
    D_801B30BC = 0x87;
    D_801B30B8++;
    func_800B6DF8();
}

void func_800B9350(void)
{
    D_801B30BC = 8;
    D_801B30B8++;
    func_800B6EF4();
}

void func_800B9388(void)
{
    D_801B30B8++;
}

s32 func_800B93A0(s32 reset)
{
    if (reset != 0)
    {
        D_801B30C0 = 1;
        D_801B30C4 = 1;
        return 1;
    }

    if ((u32)D_801B30C0 >= 6)
    {
        return 0;
    }

    D_800D7654[D_801B30C0]();
    return 1;
}

void func_800B9418(void)
{
    D_801B30C0 = 1;
    D_801B30C4 = 1;
}

void func_800B9430(void)
{
    D_801B25DC = 1;
    D_801B2678 = D_80139258;
    D_80139268 = 0;
    D_801B30C4 = 0xF0;
    D_801B30C0++;
    func_800B6FEC();
}

void func_800B94AC(void)
{
    D_801B30C4 = 0x20;
    D_801B30C0++;
    func_800B70CC();
}

void func_800B94E4(void)
{
    D_801B30C0++;
}

s32 func_800B94FC(s32 reset)
{
    if (reset != 0)
    {
        D_801B30C8 = 1;
        D_801B30CC = 1;
        return 1;
    }

    if ((u32)D_801B30C8 >= 6)
    {
        return 0;
    }

    D_800D766C[D_801B30C8]();
    return 1;
}

void func_800B9574(void)
{
    D_801B30C8 = 1;
    D_801B30CC = 1;
}

void func_800B958C(void)
{
    D_80139240 = 0;
    D_801B30CC = 0xF8;
    D_801B30C8++;
    func_800B95CC();
}

void func_800B95CC(void)
{
    s32 timer;

    func_8006AEE0();
    func_800675F0(D_8011CF30, 0, 0xB0, 0x35, 0x7800, 1, D_80139240, -0x32, 0, -1);
    D_80139240 += 8;
    if (D_80139240 >= 0x81)
    {
        D_80139240 = 0x80;
    }

    timer = D_801B30CC - 1;
    D_801B30CC = timer;
    if (timer == 0)
    {
        D_801B30C8++;
    }
}

void func_800B9680(void)
{
    D_801B30CC = 0x10;
    D_801B30C8++;
    func_800B96B8();
}

void func_800B96B8(void)
{
    s32 timer;

    func_8006AEE0();
    func_800675F0(D_8011CF30, 0, 0xB0, 0x35, 0x7800, 1, D_80139240, -0x32, 0, -1);
    D_80139240 -= 8;
    if (D_80139240 < 0)
    {
        D_80139240 = 0;
    }

    timer = D_801B30CC - 1;
    D_801B30CC = timer;
    if (timer == 0)
    {
        D_801B30C8++;
    }
}

void func_800B9764(void)
{
    D_801B30C8++;
}

s32 func_800B977C(s32 reset)
{
    if (reset != 0)
    {
        D_801B30D0 = 1;
        D_801B30D4 = 1;
        return 1;
    }

    if ((u32)D_801B30D0 >= 6)
    {
        return 0;
    }

    D_800D7684[D_801B30D0]();
    return 1;
}

void func_800B97F4(void)
{
    D_801B30D0 = 1;
    D_801B30D4 = 1;
}

void func_800B980C(void)
{
    D_8013924C = 1;
    D_801B30D4 = 0x70;
    D_801B30D0++;
    func_800B9850();
}

void func_800B9850(void)
{
    s32 timer;

    func_8006AEE0();
    func_800675F0(D_8011CF24, 0, 0xA, 0x35, 0x7840, 1, D_8013924C, 0, 0, -1);
    D_8013924C += 8;
    if (D_8013924C >= 0x82)
    {
        D_8013924C = 0x81;
    }

    timer = D_801B30D4 - 1;
    D_801B30D4 = timer;
    if (timer == 0)
    {
        D_801B30D0++;
    }
}

void func_800B9900(void)
{
    D_801B30D4 = 0x20;
    D_801B30D0++;
    func_800B9938();
}

void func_800B9938(void)
{
    s32 timer;

    func_8006AEE0();
    func_800675F0(D_8011CF24, 0, 0xA, 0x35, 0x7840, 1, D_8013924C, 0, 0, -1);
    D_8013924C -= 4;
    if (D_8013924C < 0)
    {
        D_8013924C = 0;
    }

    timer = D_801B30D4 - 1;
    D_801B30D4 = timer;
    if (timer == 0)
    {
        D_801B30D0++;
    }
}

void func_800B99E0(void)
{
    D_801B30D0++;
}

s32 func_800B99F8(s32 reset)
{
    if (reset != 0)
    {
        D_801B30D8 = 1;
        D_801B30DC = 1;
        return 1;
    }

    if ((u32)D_801B30D8 >= 4)
    {
        return 0;
    }

    D_800D769C[D_801B30D8]();
    return 1;
}

void func_800B9A70(void)
{
    D_801B30D8 = 1;
    D_801B30DC = 1;
}

void func_800B9A88(void)
{
    D_801399C4 = &D_8011D538;
    D_800D939C.field_06 = 0xF;
    D_800D939C.field_0E = 1;
    D_800D939C.field_10 = -1;
    D_800D939C.field_26 = 4;
    D_800D939C.field_02 = 0;
    D_800D939C.field_22 = 0x61;
    D_800D939C.field_24 = 1;
    D_801B30DC = 0x64;
    D_801B30D8++;
    func_800B9B08();
}

void func_800B9B08(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D939C;
    func_8006CC4C(config, &D_801399C0);
    func_80066F9C(config, *(s32*)&D_8011CF4C, 0x2B, 8, 0);
    timer = D_801B30DC - 1;
    D_801B30DC = timer;
    if (timer == 0)
    {
        D_801B30D8++;
    }
}

void func_800B9B84(void)
{
    D_801B30D8++;
}

s32 func_800B9B9C(s32 reset)
{
    if (reset != 0)
    {
        D_801B30E0 = 1;
        D_801B30E4 = 1;
        return 1;
    }

    if ((u32)D_801B30E0 >= 6)
    {
        return 0;
    }

    D_800D76AC[D_801B30E0]();
    return 1;
}

void func_800B9C14(void)
{
    D_801B30E0 = 1;
    D_801B30E4 = 1;
}

void func_800B9CAC(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93C8;
    func_8006CC4C(config, &D_801399C8);
    func_80066F9C(config, *(s32*)&D_8011CF4C, 0x10, 8, 0);
    timer = D_801B30E4 - 1;
    D_801B30E4 = timer;
    if (timer == 0)
    {
        D_801B30E0++;
    }
}

void func_800B9D28(void)
{
    D_800D93C8.field_26 = 8;
    D_800D93C8.field_22 = 0;
    D_801B30E4 = 0x10;
    D_801B30E0++;
    func_800B9D74();
}

void func_800B9D74(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93C8;
    func_8006CC4C(config, &D_801399C8);
    func_80066F9C(config, *(s32*)&D_8011CF4C, 0x10, 8, 0);
    timer = D_801B30E4 - 1;
    D_801B30E4 = timer;
    if (timer == 0)
    {
        D_801B30E0++;
    }
}

void func_800B9DF0(void)
{
    D_801B30E0++;
}

s32 func_800B9E08(s32 reset)
{
    if (reset != 0)
    {
        D_801B30E8 = 1;
        D_801B30EC = 1;
        return 1;
    }

    if ((u32)D_801B30E8 >= 6)
    {
        return 0;
    }

    D_800D76C4[D_801B30E8]();
    return 1;
}

void func_800B9E80(void)
{
    D_801B30E8 = 1;
    D_801B30EC = 1;
}

void func_800B9E98(void)
{
    D_801399D4 = &D_8011D538;
    D_800D93F4.field_06 = 0xF;
    D_800D93F4.field_0E = 3;
    D_800D93F4.field_10 = -1;
    D_800D93F4.field_26 = 2;
    D_800D93F4.field_22 = 0x81;
    D_800D93F4.field_02 = 0;
    D_800D93F4.field_24 = 1;
    D_801B30EC = 0xB4;
    D_801B30E8++;
    func_800B9F1C();
}

void func_800B9F1C(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93F4;
    func_8006CC4C(config, &D_801399D0);
    func_80066F9C(config, *(s32*)&D_8011CF4C, 0x10, 8, 0);
    timer = D_801B30EC - 1;
    D_801B30EC = timer;
    if (timer == 0)
    {
        D_801B30E8++;
    }
}

void func_800B9F98(void)
{
    D_800D93F4.field_26 = 4;
    D_800D93F4.field_22 = 0;
    D_801B30EC = 0x20;
    D_801B30E8++;
    func_800B9FE4();
}

void func_800B9FE4(void)
{
    s32 timer;
    WmapConfigA* config;

    config = &D_800D93F4;
    func_8006CC4C(config, &D_801399D0);
    func_80066F9C(config, *(s32*)&D_8011CF4C, 0x10, 8, 0);
    timer = D_801B30EC - 1;
    D_801B30EC = timer;
    if (timer == 0)
    {
        D_801B30E8++;
    }
}

void func_800BA060(void)
{
    D_801B30E8++;
}

s32 func_800BA078(s32 reset)
{
    if (reset != 0)
    {
        D_801B30F0 = 1;
        D_801B30F4 = 1;
        return 1;
    }

    if ((u32)D_801B30F0 >= 6)
    {
        return 0;
    }

    D_800D76DC[D_801B30F0]();
    return 1;
}

void func_800BA0F0(void)
{
    D_801B30F0 = 1;
    D_801B30F4 = 1;
}

void func_800BA108(void)
{
    s32 timer;

    func_8006C448(&D_80139280[2]);
    timer = D_801B30F4 - 1;
    D_801B30F4 = timer;
    if (timer == 0)
    {
        D_801B30F0++;
    }
}

void func_800BA15C(void)
{
    s32 i;

    for (i = 0; i < 0x28; i++)
    {
        D_800DB4C8[i].field_22 = 0;
        D_800DB4C8[i].field_26 = 0x20;
    }
    D_801B30F4 = 4;
    D_801B30F0++;
    func_800BA1BC();
}

void func_800BA1BC(void)
{
    s32 timer;

    func_8006C448(&D_80139280[2]);
    timer = D_801B30F4 - 1;
    D_801B30F4 = timer;
    if (timer == 0)
    {
        D_801B30F0++;
    }
}

void func_800BA210(void)
{
    D_801B30F0++;
}

void func_800BA228(void)
{
    func_8006CAC0(func_800BC360);
    D_80139244 = 1;
    func_8006CAC0(func_800BB87C);
    func_8006CAC0(func_800BB610);
    func_8006CAC0(func_800BBAE8);
    func_8006683C(0x701040);
    D_801ADAF4 = 0;
    D_80182D74.field_00 = 0x38;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x18;
    D_80182D80.field_00 = 0x38;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x18;
    D_80182D8C.field_00 = 0x38;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0x18;
    D_80182D94.field_00 = 0x38;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0x18;
    D_801B3134 = 8;
    D_801B3130++;
}

void func_800BA300(void)
{
    func_8006CAC0(func_800BCB6C);
    func_8006CBD8(func_8006C0EC);
    D_80139210.field_00 = 5;
    D_80139210.field_02 = 0;
    D_80139210.field_04 = 0;
    D_80139968.field_00 = 0;
    D_80139968.field_04 = 2;
    D_80139968.field_08 = 0;
    D_800DCEB8.field_00 = 0xFA;
    D_800DCEB8.field_02 = 0;
    D_800DCEB8.field_04 = 0;
    D_80139200.field_00 = 0;
    D_80139200.field_04 = 0x64;
    D_80139200.field_08 = 0;
    D_801ADAF4 = 0;
    D_80182D74.field_00 = 0x38;
    D_80182D74.field_01 = 0;
    D_80182D74.field_02 = 0x18;
    D_80182D80.field_00 = 0x38;
    D_80182D80.field_01 = 0;
    D_80182D80.field_02 = 0x18;
    D_80182D8C.field_00 = 0;
    D_80182D8C.field_01 = 0;
    D_80182D8C.field_02 = 0;
    D_80182D94.field_00 = 0;
    D_80182D94.field_01 = 0;
    D_80182D94.field_02 = 0;
    D_801B3134 = 0x70;
    D_801B3130++;
}

void func_800BA580(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = value;
    if (value < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }
    PushMatrix();
    func_8006D150(&D_801B24A0);
    if (D_80182DE8 != 0)
    {
        func_800675F0(D_800DCF18, 0, 4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        fade = D_80182DE8 - 2;
        D_80182DE8 = fade;
        if (fade < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();
    timer = D_801B316C - 1;
    D_801B316C = timer;
    if (timer == 0)
    {
        D_801B3168++;
    }
}

void func_800BA66C(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_801B2478.vz - 0xDAC;
    D_801B2478.vz = value;
    if (value < 0x2710)
    {
        D_801B2478.vz = 0x2710;
    }
    PushMatrix();
    func_8006D150(&D_801B24A8);
    if (D_80182DEC != 0)
    {
        func_800675F0(D_8011CF1C, 0, 4, 0x35, 0x7800, 1, D_80182DEC, 0, 0, -1);
        fade = D_80182DEC - 1;
        D_80182DEC = fade;
        if (fade < 0)
        {
            D_80182DEC = 0;
        }
    }
    PopMatrix();
    timer = D_801B3174 - 1;
    D_801B3174 = timer;
    if (timer == 0)
    {
        D_801B3170++;
    }
}

void func_800BA758(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_80139870.vz - 0xDAC;
    D_80139870.vz = value;
    if (value < 0x2710)
    {
        D_80139870.vz = 0x2710;
    }
    PushMatrix();
    func_8006D150(&D_8013B238);
    if (D_80182DF0 != 0)
    {
        func_800675F0(D_8011CF24, 0, 4, 0x35, 0x7800, 1, D_80182DF0, 0, 0, -1);
        fade = D_80182DF0 - 8;
        D_80182DF0 = fade;
        if (fade < 0)
        {
            D_80182DF0 = 0;
        }
    }
    PopMatrix();
    timer = D_801B317C - 1;
    D_801B317C = timer;
    if (timer == 0)
    {
        D_801B3178++;
    }
}

void func_800BA910(void)
{
    s32 index;
    s32 screen_offset;
    s32 config_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = D_80139988;
    resource = &D_80121538;
    screen_offset = 0x280;
    config_offset = 0x640;
    D_80139280->field_2C = 1;
    D_80139280->field_30 = 4;
    D_80139280->field_34 = 0x40;
    D_80139280->field_3C = 3;
    D_80139280->field_40 = -0x1C2;
    D_80139280->field_44 = 0x50;
    D_80139280->field_48 = 8;
    D_80139280->tail_state = 2;
    D_80139280->field_38 = 0;
    D_80139280[1].field_00 = 0x61A8;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 0x1E);

    D_801B318C = 0x5A;
    D_801B3188++;
    func_800BC9F4();
}

void func_800BAAB0(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    func_800675F0(D_8011CF28, (D_80139240 >> 4) & 7, 0xA, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 + 8;
    D_80182DF4 = value;
    if (value >= 0x82)
    {
        D_80182DF4 = 0x81;
    }
    D_80139240 += 0x10;
    timer = D_801B319C - 1;
    D_801B319C = timer;
    if (timer == 0)
    {
        D_801B3198++;
    }
}

void func_800BAB80(void)
{
    s32 value;
    s32 timer;

    func_8006AEE0();
    func_800675F0(D_8011CF28, (D_80139240 >> 4) & 7, 0xA, 0x35, 0x7800, 1, D_80182DF4, 0, 0, -1);
    value = D_80182DF4 - 4;
    D_80182DF4 = value;
    if (value < 0)
    {
        D_80182DF4 = 0;
    }
    D_80139240 += 0x10;
    timer = D_801B319C - 1;
    D_801B319C = timer;
    if (timer == 0)
    {
        D_801B3198++;
    }
}

void func_800BAC44(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_801B2670);
    func_800675F0(D_8011CF2C, (D_8013924C >> 4) & 7, 0xB0, 0x35, 0x7800, 1, D_801B25D8, 4, -0x44, -1);
    value = D_801B25D8 + 2;
    D_801B25D8 = value;
    if (value >= 0x81)
    {
        D_801B25D8 = 0x80;
    }
    D_8013924C += 0x18;
    timer = D_801B31A4 - 1;
    D_801B31A4 = timer;
    ((u16*)&D_801B2670)[2] += 0x28;
    if (timer == 0)
    {
        D_801B31A0++;
    }
}

void func_800BAD38(void)
{
    s32 value;
    s32 timer;

    func_8006D150(&D_801B2670);
    func_800675F0(D_8011CF2C, (D_8013924C >> 4) & 7, 0xB0, 0x35, 0x7800, 1, D_801B25D8, 4, -0x44, -1);
    value = D_801B25D8 - 0x10;
    D_801B25D8 = value;
    if (value < 0)
    {
        D_801B25D8 = 0;
    }
    D_8013924C += 0x18;
    timer = D_801B31A4 - 1;
    D_801B31A4 = timer;
    ((u16*)&D_801B2670)[2] += 0x28;
    if (timer == 0)
    {
        D_801B31A0++;
    }
}

s32 func_800BAE24(s32 reset)
{
    if (reset != 0)
    {
        D_801B3128 = 1;
        D_801B312C = 1;
        return 1;
    }

    if ((u32)D_801B3128 >= 6)
    {
        return 0;
    }

    D_800D76F4[D_801B3128]();
    return 1;
}

void func_800BAE9C(void)
{
    D_801B3128 = 1;
    D_801B312C = 1;
}

void func_800BAEB4(void)
{
    s32 value;

    func_8006CAC0(func_8006C81C);
    D_8013B20C = value = 1;
    D_801B3128 += value;
    func_800BAEF8();
}

void func_800BAEF8(void)
{
    if (D_8013B20C == 0)
    {
        D_801B3128++;
        func_800BAF34();
    }
}

void func_800BAF34(void)
{
    s32 value;

    func_8006CAC0(func_800BAFCC);
    D_8013B20C = value = 1;
    D_801B3128 += value;
    func_800BAF78();
}

void func_800BAF78(void)
{
    if (D_8013B20C == 0)
    {
        D_801B3128++;
        func_800BAFB4();
    }
}

void func_800BAFB4(void)
{
    D_801B3128++;
}

s32 func_800BAFCC(s32 reset)
{
    if (reset != 0)
    {
        D_801B3130 = 1;
        D_801B3134 = 1;
        return 1;
    }

    if ((u32)D_801B3130 >= 0x1A)
    {
        return 0;
    }

    D_800D770C[D_801B3130]();
    return 1;
}

void func_800BB044(void)
{
    D_801B3130 = 1;
    D_801B3134 = 1;
}

void func_800BB05C(void)
{
    s32 value;

    D_8013B208 = value = 1;
    func_800652A8(0x35, 0x80);
    D_8013B29C = value;
    D_801B3134 = 4;
    D_801B3130 += value;
}

void func_800BB0B4(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB0E8(void)
{
    func_8006CAC0(func_800BBD54);
    func_8006683C(0x605060);
    D_801ADAF4 = 0xA;
    D_801B3134 = 0x12;
    D_801B3130++;
}

void func_800BB13C(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB170(void)
{
    D_801ADAE0 = 1;
    D_801ADAF4 = 5;
    D_801B3134 = 0xE;
    D_801B3130++;
}

void func_800BB1A8(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB1DC(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB210(void)
{
    func_8006CAC0(func_800BC764);
    D_801B3134 = 4;
    D_801B3130++;
}

void func_800BB24C(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB280(void)
{
    func_8006CAC0(func_800BCEA8);
    D_801B3134 = 0x5C;
    D_801B3130++;
}

void func_800BB2BC(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB2F0(void)
{
    func_8006CAC0(func_800BD12C);
    D_801B3134 = 0x1E;
    D_801B3130++;
}

void func_800BB32C(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB360(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB394(void)
{
    func_8006CAC0(func_800BCFD4);
    D_801B3134 = 0x5A;
    D_801B3130++;
}

void func_800BB3D0(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB404(void)
{
    func_8006CAC0(func_800BC4B8);
    D_801ADAF4 = 7;
    D_80139244 = 0;
    func_8006683C(0x703080);
    D_8013B29C = 0;
    func_8006CAC0(func_800BC01C);
    func_8006CAC0(func_800BC1BC);
    D_801B3134 = 4;
    D_801B3130++;
}

void func_800BB480(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB4B4(void)
{
    func_8006CAC0(func_800BC964);
    D_801B3134 = 0xB7;
    D_801B3130++;
}

void func_800BB4F0(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB524(void)
{
    func_8006CAC0(func_800BC60C);
    func_8006683C(0x808080);
    D_801ADAF4 = 0x10;
    D_801B3134 = 0x3C;
    D_80139290[D_8011D510][D_8011D530].value = D_8011D4FC | 0x100;
    D_801B3130++;
}

void func_800BB5C0(void)
{
    s32 timer;

    timer = D_801B3134 - 1;
    D_801B3134 = timer;
    if (timer == 0)
    {
        D_801B3130++;
    }
}

void func_800BB5F4(void)
{
    D_8013B20C = 0;
    D_801B3130++;
}

s32 func_800BB610(s32 reset)
{
    if (reset != 0)
    {
        D_801B3138 = 1;
        D_801B313C = 1;
        return 1;
    }

    if ((u32)D_801B3138 >= 6)
    {
        return 0;
    }

    D_800D7774[D_801B3138]();
    return 1;
}

void func_800BB688(void)
{
    D_801B3138 = 1;
    D_801B313C = 1;
}

void func_800BB6A0(void)
{
    D_801399AC = &D_8011F538;
    D_800D9318.field_06 = 0xF;
    D_800D9318.field_10 = -1;
    D_800D9318.field_26 = 4;
    D_800D9318.field_22 = 0x81;
    D_800D9318.field_02 = 0;
    D_800D9318.field_0E = 0;
    D_800D9318.field_24 = 1;
    D_801B313C = 0x88;
    D_801B3138++;
    func_800BB720();
}

void func_800BB720(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9318, &D_801399A8);
    func_80066F9C(&D_800D9318, *(s32*)&D_8011CF4C, 0xB, 2, 0);
    timer = D_801B313C - 1;
    D_801B313C = timer;
    if (timer == 0)
    {
        D_801B3138++;
    }
}

void func_800BB79C(void)
{
    D_800D9318.field_22 = 0;
    D_800D9318.field_26 = 8;
    D_801B313C = 0x10;
    D_801B3138++;
    func_800BB7E8();
}

void func_800BB7E8(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9318, &D_801399A8);
    func_80066F9C(&D_800D9318, *(s32*)&D_8011CF4C, 0xB, 2, 0);
    timer = D_801B313C - 1;
    D_801B313C = timer;
    if (timer == 0)
    {
        D_801B3138++;
    }
}

void func_800BB864(void)
{
    D_801B3138++;
}

s32 func_800BB87C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3140 = 1;
        D_801B3144 = 1;
        return 1;
    }

    if ((u32)D_801B3140 >= 6)
    {
        return 0;
    }

    D_800D778C[D_801B3140]();
    return 1;
}

void func_800BB8F4(void)
{
    D_801B3140 = 1;
    D_801B3144 = 1;
}

void func_800BB90C(void)
{
    s32 value;

    D_801399B4 = &D_8011F538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_0E = value = 1;
    D_800D9344.field_10 = -value;
    D_800D9344.field_26 = 8;
    D_800D9344.field_24 = value;
    D_800D9344.field_02 = 0;
    D_800D9344.field_22 = 0x81;
    D_801B3144 = 0x88;
    D_801B3140++;
    func_800BB98C();
}

void func_800BB98C(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, *(s32*)&D_8011CF4C, 0xB, 2, 0);
    timer = D_801B3144 - 1;
    D_801B3144 = timer;
    if (timer == 0)
    {
        D_801B3140++;
    }
}

void func_800BBA08(void)
{
    D_800D9344.field_26 = 8;
    D_800D9344.field_22 = 0;
    D_801B3144 = 0x10;
    D_801B3140++;
    func_800BBA54();
}

void func_800BBA54(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, *(s32*)&D_8011CF4C, 0xB, 2, 0);
    timer = D_801B3144 - 1;
    D_801B3144 = timer;
    if (timer == 0)
    {
        D_801B3140++;
    }
}

void func_800BBAD0(void)
{
    D_801B3140++;
}

s32 func_800BBAE8(s32 reset)
{
    if (reset != 0)
    {
        D_801B3148 = 1;
        D_801B314C = 1;
        return 1;
    }

    if ((u32)D_801B3148 >= 6)
    {
        return 0;
    }

    D_800D77A4[D_801B3148]();
    return 1;
}

void func_800BBB60(void)
{
    D_801B3148 = 1;
    D_801B314C = 1;
}

void func_800BBB78(void)
{
    D_801399BC = &D_8011F538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_0E = 2;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 8;
    D_800D9370.field_02 = 0;
    D_800D9370.field_22 = 0x81;
    D_800D9370.field_24 = 0x81;
    D_801B314C = 0x88;
    D_801B3148++;
    func_800BBBF8();
}

void func_800BBBF8(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, *(s32*)&D_8011CF4C, 0xB, 2, 0);
    timer = D_801B314C - 1;
    D_801B314C = timer;
    if (timer == 0)
    {
        D_801B3148++;
    }
}

void func_800BBC74(void)
{
    D_800D9370.field_26 = 2;
    D_800D9370.field_22 = 0;
    D_801B314C = 0x40;
    D_801B3148++;
    func_800BBCC0();
}

void func_800BBCC0(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, *(s32*)&D_8011CF4C, 0xB, 2, 0);
    timer = D_801B314C - 1;
    D_801B314C = timer;
    if (timer == 0)
    {
        D_801B3148++;
    }
}

void func_800BBD3C(void)
{
    D_801B3148++;
}

s32 func_800BBD54(s32 reset)
{
    if (reset != 0)
    {
        D_801B3150 = 1;
        D_801B3154 = 1;
        return 1;
    }

    if ((u32)D_801B3150 >= 6)
    {
        return 0;
    }

    D_800D77BC[D_801B3150]();
    return 1;
}

void func_800BBDCC(void)
{
    D_801B3150 = 1;
    D_801B3154 = 1;
}

void func_800BBE9C(void)
{
    s32 timer;

    func_8006CC4C(&D_800D939C, &D_801399C0);
    func_80066F9C(&D_800D939C, *(s32*)&D_8011CF4C, 0xB, 2, 0);
    if (D_80139268 > 0)
    {
        func_800BA408();
    }
    timer = D_801B3154 - 1;
    D_80139268--;
    D_801B3154 = timer;
    if (timer == 0)
    {
        D_801B3150++;
    }
}

void func_800BBF3C(void)
{
    D_800D939C.field_26 = 8;
    D_800D939C.field_22 = 0;
    D_801B3154 = 0x10;
    D_801B3150++;
    func_800BBF88();
}

void func_800BBF88(void)
{
    s32 timer;

    func_8006CC4C(&D_800D939C, &D_801399C0);
    func_80066F9C(&D_800D939C, *(s32*)&D_8011CF4C, 0xB, 2, 0);
    timer = D_801B3154 - 1;
    D_801B3154 = timer;
    if (timer == 0)
    {
        D_801B3150++;
    }
}

void func_800BC004(void)
{
    D_801B3150++;
}

s32 func_800BC01C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3158 = 1;
        D_801B315C = 1;
        return 1;
    }

    if ((u32)D_801B3158 >= 4)
    {
        return 0;
    }

    D_800D77D4[D_801B3158]();
    return 1;
}

void func_800BC094(void)
{
    D_801B3158 = 1;
    D_801B315C = 1;
}

void func_800BC0AC(void)
{
    D_801399B4 = &D_8011D538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_10 = -1;
    D_800D9344.field_26 = 8;
    D_800D9344.field_02 = 0;
    D_800D9344.field_0E = 0;
    D_800D9344.field_22 = 0x80;
    D_800D9344.field_24 = 0;
    D_801B315C = 0xDC;
    D_801B3158++;
    func_800BC128();
}

void func_800BC128(void)
{
    s32 timer;

    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, *(s32*)&D_8011CF4C, 0x1F, 0xC, 0);
    timer = D_801B315C - 1;
    D_801B315C = timer;
    if (timer == 0)
    {
        D_801B3158++;
    }
}

void func_800BC1A4(void)
{
    D_801B3158++;
}

s32 func_800BC1BC(s32 reset)
{
    if (reset != 0)
    {
        D_801B3160 = 1;
        D_801B3164 = 1;
        return 1;
    }

    if ((u32)D_801B3160 >= 4)
    {
        return 0;
    }

    D_800D77E4[D_801B3160]();
    return 1;
}

void func_800BC234(void)
{
    D_801B3160 = 1;
    D_801B3164 = 1;
}

void func_800BC24C(void)
{
    s32 value;

    D_801399D4 = &D_8011D538;
    D_800D93F4.field_06 = 0xF;
    D_800D93F4.field_0E = value = 1;
    D_800D93F4.field_10 = -value;
    D_800D93F4.field_26 = 8;
    D_800D93F4.field_24 = value;
    D_800D93F4.field_02 = 0;
    D_800D93F4.field_22 = 0x81;
    D_801B3164 = 0xBD;
    D_801B3160++;
    func_800BC2CC();
}

void func_800BC2CC(void)
{
    s32 timer;

    func_8006CC4C(&D_800D93F4, &D_801399D0);
    func_80066F9C(&D_800D93F4, *(s32*)&D_8011CF4C, 0x2D, 0x1E, 0);
    timer = D_801B3164 - 1;
    D_801B3164 = timer;
    if (timer == 0)
    {
        D_801B3160++;
    }
}

void func_800BC348(void)
{
    D_801B3160++;
}

s32 func_800BC360(s32 reset)
{
    if (reset != 0)
    {
        D_801B3168 = 1;
        D_801B316C = 1;
        return 1;
    }

    if ((u32)D_801B3168 >= 4)
    {
        return 0;
    }

    D_800D77F4[D_801B3168]();
    return 1;
}

void func_800BC3D8(void)
{
    D_801B3168 = 1;
    D_801B316C = 1;
}

void func_800BC3F0(void)
{
    *(WmapPair*)&D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B316C = 0x40;
    D_801B3168++;
    func_800BA580();
}

void func_800BC4A0(void)
{
    D_801B3168++;
}

s32 func_800BC4B8(s32 reset)
{
    if (reset != 0)
    {
        D_801B3170 = 1;
        D_801B3174 = 1;
        return 1;
    }

    if ((u32)D_801B3170 >= 4)
    {
        return 0;
    }

    D_800D7804[D_801B3170]();
    return 1;
}

void func_800BC530(void)
{
    D_801B3170 = 1;
    D_801B3174 = 1;
}

void func_800BC548(void)
{
    D_801B24A8 = D_80139258;
    D_801B2478 = D_80182DC0;
    D_80182DEC = 0x80;
    D_801B2478.vz = 0xAFC8;
    D_801B3174 = 0x80;
    D_801B3170++;
    func_800BA66C();
}

void func_800BC5F4(void)
{
    D_801B3170++;
}

s32 func_800BC60C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3178 = 1;
        D_801B317C = 1;
        return 1;
    }

    if ((u32)D_801B3178 >= 4)
    {
        return 0;
    }

    D_800D7814[D_801B3178]();
    return 1;
}

void func_800BC684(void)
{
    D_801B3178 = 1;
    D_801B317C = 1;
}

void func_800BC69C(void)
{
    D_8013B238 = D_80139258;
    D_80139870 = D_80182DC0;
    D_80182DF0 = 0x80;
    D_80139870.vz = 0xAFC8;
    D_801B317C = 0x10;
    D_801B3178++;
    func_800BA758();
}

void func_800BC74C(void)
{
    D_801B3178++;
}

s32 func_800BC764(s32 reset)
{
    if (reset != 0)
    {
        D_801B3180 = 1;
        D_801B3184 = 1;
        return 1;
    }

    if ((u32)D_801B3180 >= 6)
    {
        return 0;
    }

    D_800D7824[D_801B3180]();
    return 1;
}

void func_800BC7DC(void)
{
    D_801B3180 = 1;
    D_801B3184 = 1;
}

void func_800BC7F4(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x3C, 0xFF, 1, 2, 0, D_80139280);
    timer = D_801B3184 - 1;
    D_801B3184 = timer;
    if (timer == 0)
    {
        D_801B3180++;
    }
}

void func_800BC87C(void)
{
    D_801B3184 = 0x80;
    D_80139280->field_14 = -1;
    D_801B3180++;
    func_800BC8C4();
}

void func_800BC8C4(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x3C, 0xFF, 1, 2, 0, D_80139280);
    timer = D_801B3184 - 1;
    D_801B3184 = timer;
    if (timer == 0)
    {
        D_801B3180++;
    }
}

void func_800BC94C(void)
{
    D_801B3180++;
}

s32 func_800BC964(s32 reset)
{
    if (reset != 0)
    {
        D_801B3188 = 1;
        D_801B318C = 1;
        return 1;
    }

    if ((u32)D_801B3188 >= 6)
    {
        return 0;
    }

    D_800D783C[D_801B3188]();
    return 1;
}

void func_800BC9DC(void)
{
    D_801B3188 = 1;
    D_801B318C = 1;
}

void func_800BC9F4(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x1E, 0xFF, 1, 4, 0, (u8*)D_80139280 + 0x28);
    timer = D_801B318C - 1;
    D_801B318C = timer;
    if (timer == 0)
    {
        D_801B3188++;
    }
}

void func_800BCA80(void)
{
    D_801B318C = 0x40;
    D_80139280->field_3C = -1;
    D_801B3188++;
    func_800BCAC8();
}

void func_800BCAC8(void)
{
    s32 timer;

    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x1E, 0xFF, 1, 4, 0, (u8*)D_80139280 + 0x28);
    timer = D_801B318C - 1;
    D_801B318C = timer;
    if (timer == 0)
    {
        D_801B3188++;
    }
}

void func_800BCB54(void)
{
    D_801B3188++;
}

s32 func_800BCB6C(s32 reset)
{
    if (reset != 0)
    {
        D_801B3190 = 1;
        D_801B3194 = 1;
        return 1;
    }

    if ((u32)D_801B3190 >= 8)
    {
        return 0;
    }

    D_800D7854[D_801B3190]();
    return 1;
}

void func_800BCBE4(void)
{
    D_801B3190 = 1;
    D_801B3194 = 1;
}

void func_800BCBFC(void)
{
    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    D_801B25DC += 8;
    timer = D_801B3194 - 1;
    D_801B3194 = timer;
    if (timer == 0)
    {
        D_801B3190++;
    }
}

void func_800BCCB8(void)
{
    D_801B3194 = 0x12;
    D_801B3190++;
    func_800BCCF0();
}

void func_800BCCF0(void)
{
    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    timer = D_801B3194 - 1;
    D_801B3194 = timer;
    if (timer == 0)
    {
        D_801B3190++;
    }
}

void func_800BCDA0(void)
{
    D_800DCEB0 = 0;
    D_801B3194 = 0x64;
    D_801B3190++;
    func_800BCDE0();
}

void func_800BCDE0(void)
{
    s32 timer;

    func_8006B328(0x78, 0x9C, 2, -1, -3, -8, 0, 0x1F, -0xB4, 0x190, -0xA0, 0x190, 0, 0x80, 0, 8, 2);
    timer = D_801B3194 - 1;
    D_801B3194 = timer;
    if (timer == 0)
    {
        D_801B3190++;
    }
}

void func_800BCE90(void)
{
    D_801B3190++;
}

s32 func_800BCEA8(s32 reset)
{
    if (reset != 0)
    {
        D_801B3198 = 1;
        D_801B319C = 1;
        return 1;
    }

    if ((u32)D_801B3198 >= 6)
    {
        return 0;
    }

    D_800D7874[D_801B3198]();
    return 1;
}

void func_800BCF20(void)
{
    D_801B3198 = 1;
    D_801B319C = 1;
}

void func_800BCF38(void)
{
    D_80139240 = 0;
    D_80182DF4 = 1;
    D_801B319C = 0x5A;
    D_801B3198++;
    func_800BAAB0();
}

void func_800BCF84(void)
{
    D_801B319C = 0x20;
    D_801B3198++;
    func_800BAB80();
}

void func_800BCFBC(void)
{
    D_801B3198++;
}

s32 func_800BCFD4(s32 reset)
{
    if (reset != 0)
    {
        D_801B31A0 = 1;
        D_801B31A4 = 1;
        return 1;
    }

    if ((u32)D_801B31A0 >= 6)
    {
        return 0;
    }

    D_800D788C[D_801B31A0]();
    return 1;
}

void func_800BD04C(void)
{
    D_801B31A0 = 1;
    D_801B31A4 = 1;
}

void func_800BD064(void)
{
    D_8013924C = 0;
    D_801B2670 = D_80139258;
    D_801B25D8 = 0;
    D_801B31A4 = 0x58;
    D_801B31A0++;
    func_800BAC44();
}

void func_800BD0DC(void)
{
    D_801B31A4 = 8;
    D_801B31A0++;
    func_800BAD38();
}

void func_800BD114(void)
{
    D_801B31A0++;
}

s32 func_800BD12C(s32 reset)
{
    if (reset != 0)
    {
        D_801B31A8 = 1;
        D_801B31AC = 1;
        return 1;
    }

    if ((u32)D_801B31A8 >= 6)
    {
        return 0;
    }

    D_800D78A4[D_801B31A8]();
    return 1;
}

void func_800BD1A4(void)
{
    D_801B31A8 = 1;
    D_801B31AC = 1;
}

void func_800BD1BC(void)
{
    D_801399DC = &D_80121538;
    D_80139250 = 0x780;
    D_800D9420.field_06 = 0xF;
    D_800D9420.field_10 = -1;
    D_800D9420.field_26 = 4;
    D_800D9420.field_22 = 0x81;
    D_800D9420.field_24 = 1;
    D_800D9420.field_02 = 0;
    D_800D9420.field_0E = 0;
    D_801B31AC = 0xE1;
    D_80182D58.value = *(s32*)&D_8011CF4C;
    D_801B31A8++;
    func_800BD258();
}

void func_800BD258(void)
{
    s32 value;
    s32 adjusted;
    s32 timer;

    func_8006CC4C(&D_800D9420, &D_801399D8);
    func_80066F9C(&D_800D9420, D_80182D58.value, 8, 0x14, 0);

    value = D_80139250;
    adjusted = value;
    if (value < 0)
    {
        adjusted = value + 0xF;
    }
    D_80182D58.fields.field_02 = adjusted >> 4;
    if (value >= 0x321)
    {
        D_80139250 = value - 0x10;
    }

    timer = D_801B31AC - 1;
    D_801B31AC = timer;
    if (timer == 0)
    {
        D_801B31A8++;
    }
}

void func_800BD308(void)
{
    D_800D9420.field_26 = 8;
    D_800D9420.field_22 = 0;
    D_801B31AC = 0x10;
    D_801B31A8++;
    func_800BD354();
}

void func_800BD354(void)
{
    s32 value;
    s32 adjusted;
    s32 timer;

    func_8006CC4C(&D_800D9420, &D_801399D8);
    func_80066F9C(&D_800D9420, D_80182D58.value, 8, 0x14, 0);

    value = D_80139250;
    adjusted = value;
    if (value < 0)
    {
        adjusted = value + 0xF;
    }
    D_80182D58.fields.field_02 = adjusted >> 4;
    if (value >= 0x321)
    {
        D_80139250 = value - 0x10;
    }

    timer = D_801B31AC - 1;
    D_801B31AC = timer;
    if (timer == 0)
    {
        D_801B31A8++;
    }
}

void func_800BD404(void)
{
    D_801B31A8++;
}

void func_800BD5B8(void)
{
    s32 index;
    s32 screen_offset;
    s32 config_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = D_80139988;
    resource = &D_80125538;
    screen_offset = 0x460;
    config_offset = 0xAF0;
    D_80139280[1].field_04 = 0;
    D_80139280[1].field_08 = 0;
    D_80139280[1].field_0C = 0x80;
    D_80139280[1].field_10 = 0;
    D_80139280[1].field_14 = 3;
    D_80139280[1].field_18 = 0x3E8;
    D_80139280[1].field_1C = 0x8C;
    D_80139280[1].field_20 = 0x13;
    D_80139280[1].state_24 = 0;
    D_80139280[1].field_28 = 0x32C8;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 34);

    D_801B31FC = 102;
    D_801B31F8++;
    func_800BEFE8();
}

void func_800BD680(void)
{
    s32 index;
    s32 screen_offset;
    s32 config_offset;
    u8* config_base;
    u8* screen_base;
    u8* resource;
    u8* screen_entry;
    s16* config_entry;

    index = 0;
    config_base = D_801AFBD0;
    screen_base = D_80139988;
    resource = &D_80121538;
    screen_offset = 0x5F0;
    config_offset = 0xED8;
    D_80139280[2].field_04 = 1;
    D_80139280[2].field_08 = 3;
    D_80139280[2].field_0C = 0x40;
    D_80139280[2].field_10 = 0;
    D_80139280[2].field_14 = 3;
    D_80139280[2].field_18 = -0x1C2;
    D_80139280[2].field_1C = 0xBE;
    D_80139280[2].field_20 = 0x2C;
    D_80139280[2].state_24 = 5;
    D_80139280[2].field_28 = 0x6D60;

    do
    {
        screen_entry = (u8*)(screen_offset + (s32)screen_base);
        screen_offset += 8;
        config_entry = (s16*)(config_offset + (s32)config_base);
        config_offset += 0x14;
        index++;
        *config_entry = 0;
        *(u8**)(screen_entry + 4) = resource;
    } while (index < 40);

    D_801B3204 = 120;
    D_801B3200++;
    func_800BF1F0();
}

void func_800BD750(void)
{
    s32 value;
    s32 fade;
    s32 timer;

    value = D_801B2650.vz - 0xDAC;
    D_801B2650.vz = value;
    if (value < 0x2710)
    {
        D_801B2650.vz = 0x2710;
    }

    PushMatrix();
    func_8006D150(&D_801B24A0);
    if (D_80182DE8 != 0)
    {
        func_800675F0(D_800DCF18, 0, 4, 0x35, 0x7800, 1, D_80182DE8, 0, 0, -1);
        fade = D_80182DE8 - 2;
        D_80182DE8 = fade;
        if (fade < 0)
        {
            D_80182DE8 = 0;
        }
    }
    PopMatrix();

    timer = D_801B320C - 1;
    D_801B320C = timer;
    if (timer == 0)
    {
        D_801B3208++;
    }
}

s32 func_800BD83C(s32 reset)
{
    if (reset != 0)
    {
        D_801B31B0 = 1;
        D_801B31B4 = 1;
        return 1;
    }

    if ((u32)D_801B31B0 >= 4)
    {
        return 0;
    }

    D_800D78BC[D_801B31B0]();
    return 1;
}

void func_800BD8B4(void)
{
    D_801B31B0 = 1;
    D_801B31B4 = 1;
}

void func_800BD8CC(void)
{
    D_800DBE70 = 0;
    func_8006D190();
    D_8011CF4C.field_00 = 0xA4;
    D_8011CF4C.field_02 = 0x69;
    func_8006CAC0(func_800BD998);
    D_8013B20C = 1;
    D_801B31B0++;
    func_800BD934();
}

void func_800BD934(void)
{
    if (D_8013B20C == 0)
    {
        D_801B31B0++;
        func_800BD970();
    }
}

void func_800BD970(void)
{
    D_8013B294 = 1;
    D_80139228 = 1;
    D_801B31B0++;
}

s32 func_800BD998(s32 reset)
{
    if (reset != 0)
    {
        D_801B31B8 = 1;
        D_801B31BC = 1;
        return 1;
    }

    if ((u32)D_801B31B8 >= 0x16)
    {
        return 0;
    }

    D_800D78CC[D_801B31B8]();
    return 1;
}

void func_800BDA10(void)
{
    D_801B31B8 = 1;
    D_801B31BC = 1;
}

void func_800BDA28(void)
{
    D_8013B208 = 1;
    D_801ADAE0 = 1;
    func_800652A8(0x36, 0x80);
    D_801B31BC = 0xF;
    D_801B31B8++;
}

void func_800BDA78(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDAAC(void)
{
    func_8006CAC0(func_800BF160);
    D_801B31BC = 0x23;
    D_801B31B8++;
}

void func_800BDAE8(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDB1C(void)
{
    func_8006CAC0(func_800BF368);
    func_8006683C(0x703040);
    D_801ADAF4 = 0xA;
    D_801B31BC = 0x19;
    D_801B31B8++;
}

void func_800BDB70(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDBA4(void)
{
    func_8006CAC0(func_800BDF40);
    func_8006CAC0(func_800BED50);
    D_801B31BC = 0x20;
    D_801B31B8++;
}

void func_800BDBEC(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDC20(void)
{
    D_80139978 = -1;
    func_8006CAC0(func_800BE40C);
    D_801B31BC = 0x36;
    D_801B31B8++;
}

void func_800BDC68(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDC9C(void)
{
    func_8006CAC0(func_800BEB50);
    D_801B31BC = 0x45;
    D_801B31B8++;
}

void func_800BDCD8(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDD0C(void)
{
    func_8006CAC0(func_800BE1A8);
    D_801B31BC = 0x12;
    D_801B31B8++;
}

void func_800BDD48(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDD7C(void)
{
    func_8006CAC0(func_800BEF58);
    D_801B31BC = 0x2B;
    D_801B31B8++;
}

void func_800BDDB8(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDDEC(void)
{
    func_8006CAC0(func_800BE8E4);
    func_8006CAC0(func_800BE678);
    func_8006CAC0(func_800BF368);
    D_801B31BC = 0x64;
    D_801B31B8++;
}

void func_800BDE40(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDE74(void)
{
    D_80139290[D_8011D510][D_8011D530].value = 0x121;
    akao_cmd_a9(0x3C, 0);
    D_801B31BC = 0x5A;
    D_801B31B8++;
}

void func_800BDEF0(void)
{
    if (--D_801B31BC == 0)
    {
        D_801B31B8++;
    }
}

void func_800BDF24(void)
{
    D_8013B20C = 0;
    D_801B31B8++;
}

s32 func_800BDF40(s32 reset)
{
    if (reset != 0)
    {
        D_801B31C0 = 1;
        D_801B31C4 = 1;
        return 1;
    }

    if ((u32)D_801B31C0 >= 6)
    {
        return 0;
    }

    D_800D7924[D_801B31C0]();
    return 1;
}

void func_800BDFB8(void)
{
    D_801B31C0 = 1;
    D_801B31C4 = 1;
}

void func_800BDFD0(void)
{
    D_801399B4 = &D_80127538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_10 = -1;
    D_800D9344.field_26 = 4;
    D_800D9344.field_02 = 0;
    D_800D9344.field_0E = 0;
    D_800D9344.field_22 = 0x80;
    D_800D9344.field_24 = 0;
    D_801B31C4 = 0xA0;
    D_801B31C0++;
    func_800BE04C();
}

void func_800BE04C(void)
{
    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, *(s32*)&D_8011CF4C, 0x2C, 8, 0);
    if (--D_801B31C4 == 0)
    {
        D_801B31C0++;
    }
}

void func_800BE0C8(void)
{
    D_800D9344.field_26 = 0x80;
    D_800D9344.field_22 = 0;
    D_801B31C4 = 1;
    D_801B31C0++;
    func_800BE114();
}

void func_800BE114(void)
{
    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, *(s32*)&D_8011CF4C, 0x2C, 8, 0);
    if (--D_801B31C4 == 0)
    {
        D_801B31C0++;
    }
}

void func_800BE190(void)
{
    D_801B31C0++;
}

s32 func_800BE1A8(s32 reset)
{
    if (reset != 0)
    {
        D_801B31C8 = 1;
        D_801B31CC = 1;
        return 1;
    }

    if ((u32)D_801B31C8 >= 6)
    {
        return 0;
    }

    D_800D793C[D_801B31C8]();
    return 1;
}

void func_800BE220(void)
{
    D_801B31C8 = 1;
    D_801B31CC = 1;
}

void func_800BE238(void)
{
    D_801399BC = &D_80123538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_10 = -1;
    D_800D9370.field_02 = 0;
    D_800D9370.field_0E = 0;
    D_800D9370.field_26 = 0x80;
    D_800D9370.field_22 = 0x80;
    D_800D9370.field_24 = 0;
    D_801B31CC = 0x3E;
    D_801B31C8++;
    func_800BE2B0();
}

void func_800BE2B0(void)
{
    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, *(s32*)&D_8011CF4C, 0x2C, 8, 0);
    if (--D_801B31CC == 0)
    {
        D_801B31C8++;
    }
}

void func_800BE32C(void)
{
    D_800D9370.field_26 = 0x80;
    D_800D9370.field_22 = 0;
    D_801B31CC = 1;
    D_801B31C8++;
    func_800BE378();
}

void func_800BE378(void)
{
    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, *(s32*)&D_8011CF4C, 0x2C, 8, 0);
    if (--D_801B31CC == 0)
    {
        D_801B31C8++;
    }
}

void func_800BE3F4(void)
{
    D_801B31C8++;
}

s32 func_800BE40C(s32 reset)
{
    if (reset != 0)
    {
        D_801B31D0 = 1;
        D_801B31D4 = 1;
        return 1;
    }

    if ((u32)D_801B31D0 >= 6)
    {
        return 0;
    }

    D_800D7954[D_801B31D0]();
    return 1;
}

void func_800BE484(void)
{
    D_801B31D0 = 1;
    D_801B31D4 = 1;
}

void func_800BE49C(void)
{
    D_801399C4 = &D_80121538;
    D_800D939C.field_06 = 0xF;
    D_800D939C.field_10 = -1;
    D_800D939C.field_26 = 0x80;
    D_800D939C.field_22 = 0x81;
    D_800D939C.field_02 = 0;
    D_800D939C.field_0E = 0;
    D_800D939C.field_24 = 1;
    D_801B31D4 = 0xBC;
    D_801B31D0++;
    func_800BE51C();
}

void func_800BE51C(void)
{
    func_8006CC4C(&D_800D939C, &D_801399C0);
    func_80066F9C(&D_800D939C, *(s32*)&D_8011CF4C, 0x2E, 0xC, 0);
    if (--D_801B31D4 == 0)
    {
        D_801B31D0++;
    }
}

void func_800BE598(void)
{
    D_800D939C.field_26 = 0x80;
    D_800D939C.field_22 = 0;
    D_801B31D4 = 1;
    D_801B31D0++;
    func_800BE5E4();
}

void func_800BE5E4(void)
{
    func_8006CC4C(&D_800D939C, &D_801399C0);
    func_80066F9C(&D_800D939C, *(s32*)&D_8011CF4C, 0x2E, 0xC, 0);
    if (--D_801B31D4 == 0)
    {
        D_801B31D0++;
    }
}

void func_800BE660(void)
{
    D_801B31D0++;
}

s32 func_800BE678(s32 reset)
{
    if (reset != 0)
    {
        D_801B31D8 = 1;
        D_801B31DC = 1;
        return 1;
    }

    if ((u32)D_801B31D8 >= 6)
    {
        return 0;
    }

    D_800D796C[D_801B31D8]();
    return 1;
}

void func_800BE6F0(void)
{
    D_801B31D8 = 1;
    D_801B31DC = 1;
}

void func_800BE708(void)
{
    s32 value;

    D_801399CC = &D_80121538;
    D_800D93C8.field_06 = 0xF;
    D_800D93C8.field_0E = value = 1;
    D_800D93C8.field_10 = -value;
    D_800D93C8.field_26 = 8;
    D_800D93C8.field_24 = value;
    D_800D93C8.field_02 = 0;
    D_800D93C8.field_22 = 0x81;
    D_801B31DC = 0x65;
    D_801B31D8++;
    func_800BE788();
}

void func_800BE788(void)
{
    func_8006CC4C(&D_800D93C8, &D_801399C8);
    func_80066F9C(&D_800D93C8, *(s32*)&D_8011CF4C, 0x2E, 0xC, 0);
    if (--D_801B31DC == 0)
    {
        D_801B31D8++;
    }
}

void func_800BE804(void)
{
    D_800D93C8.field_26 = 0x80;
    D_800D93C8.field_22 = 0;
    D_801B31DC = 1;
    D_801B31D8++;
    func_800BE850();
}

void func_800BE850(void)
{
    func_8006CC4C(&D_800D93C8, &D_801399C8);
    func_80066F9C(&D_800D93C8, *(s32*)&D_8011CF4C, 0x2E, 0xC, 0);
    if (--D_801B31DC == 0)
    {
        D_801B31D8++;
    }
}

void func_800BE8CC(void)
{
    D_801B31D8++;
}

s32 func_800BE8E4(s32 reset)
{
    if (reset != 0)
    {
        D_801B31E0 = 1;
        D_801B31E4 = 1;
        return 1;
    }

    if ((u32)D_801B31E0 >= 6)
    {
        return 0;
    }

    D_800D7984[D_801B31E0]();
    return 1;
}

void func_800BE95C(void)
{
    D_801B31E0 = 1;
    D_801B31E4 = 1;
}

void func_800BE974(void)
{
    D_801399D4 = &D_80121538;
    D_800D93F4.field_06 = 0xF;
    D_800D93F4.field_0E = 2;
    D_800D93F4.field_10 = -1;
    D_800D93F4.field_26 = 4;
    D_800D93F4.field_02 = 0;
    D_800D93F4.field_22 = 0x80;
    D_800D93F4.field_24 = 0;
    D_801B31E4 = 0x66;
    D_801B31E0++;
    func_800BE9F4();
}

void func_800BE9F4(void)
{
    func_8006CC4C(&D_800D93F4, &D_801399D0);
    func_80066F9C(&D_800D93F4, *(s32*)&D_8011CF4C, 0x2C, 8, 0);
    if (--D_801B31E4 == 0)
    {
        D_801B31E0++;
    }
}

void func_800BEA70(void)
{
    D_800D93F4.field_26 = 2;
    D_800D93F4.field_22 = 0;
    D_801B31E4 = 0x40;
    D_801B31E0++;
    func_800BEABC();
}

void func_800BEABC(void)
{
    func_8006CC4C(&D_800D93F4, &D_801399D0);
    func_80066F9C(&D_800D93F4, *(s32*)&D_8011CF4C, 0x2C, 8, 0);
    if (--D_801B31E4 == 0)
    {
        D_801B31E0++;
    }
}

void func_800BEB38(void)
{
    D_801B31E0++;
}

s32 func_800BEB50(s32 reset)
{
    if (reset != 0)
    {
        D_801B31E8 = 1;
        D_801B31EC = 1;
        return 1;
    }

    if ((u32)D_801B31E8 >= 6)
    {
        return 0;
    }

    D_800D799C[D_801B31E8]();
    return 1;
}

void func_800BEBC8(void)
{
    D_801B31E8 = 1;
    D_801B31EC = 1;
}

void func_800BEBE0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x32, 0xFF, 1, 8, 0, D_80139280);
    if (--D_801B31EC == 0)
    {
        D_801B31E8++;
    }
}

void func_800BEC68(void)
{
    D_801B31EC = 0x20;
    D_80139280->field_14 = -1;
    D_801B31E8++;
    func_800BECB0();
}

void func_800BECB0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800D95D8, D_80139A28, 0x32, 0xFF, 1, 8, 0, D_80139280);
    if (--D_801B31EC == 0)
    {
        D_801B31E8++;
    }
}

void func_800BED38(void)
{
    D_801B31E8++;
}

s32 func_800BED50(s32 reset)
{
    if (reset != 0)
    {
        D_801B31F0 = 1;
        D_801B31F4 = 1;
        return 1;
    }

    if ((u32)D_801B31F0 >= 6)
    {
        return 0;
    }

    D_800D79B4[D_801B31F0]();
    return 1;
}

void func_800BEDC8(void)
{
    D_801B31F0 = 1;
    D_801B31F4 = 1;
}

void func_800BEDE0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x28, 0xFF, 1, 2, 0, &D_80139280->field_28);
    if (--D_801B31F4 == 0)
    {
        D_801B31F0++;
    }
}

void func_800BEE6C(void)
{
    D_801B31F4 = 0x80;
    D_80139280->field_3C = -1;
    D_801B31F0++;
    func_800BEEB4();
}

void func_800BEEB4(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DA028, D_80139C08, 0x28, 0xFF, 1, 2, 0, &D_80139280->field_28);
    if (--D_801B31F4 == 0)
    {
        D_801B31F0++;
    }
}

void func_800BEF40(void)
{
    D_801B31F0++;
}

s32 func_800BEF58(s32 reset)
{
    if (reset != 0)
    {
        D_801B31F8 = 1;
        D_801B31FC = 1;
        return 1;
    }

    if ((u32)D_801B31F8 >= 6)
    {
        return 0;
    }

    D_800D79CC[D_801B31F8]();
    return 1;
}

void func_800BEFD0(void)
{
    D_801B31F8 = 1;
    D_801B31FC = 1;
}

void func_800BEFE8(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAA78, D_80139DE8, 0x22, 0xFF, 1, 2, 0, D_80139280 + 1);
    if (--D_801B31FC == 0)
    {
        D_801B31F8++;
    }
}

void func_800BF074(void)
{
    D_801B31FC = 0x80;
    D_80139280[1].field_14 = -1;
    D_801B31F8++;
    func_800BF0BC();
}

void func_800BF0BC(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DAA78, D_80139DE8, 0x22, 0xFF, 1, 2, 0, D_80139280 + 1);
    if (--D_801B31FC == 0)
    {
        D_801B31F8++;
    }
}

void func_800BF148(void)
{
    D_801B31F8++;
}

s32 func_800BF160(s32 reset)
{
    if (reset != 0)
    {
        D_801B3200 = 1;
        D_801B3204 = 1;
        return 1;
    }

    if ((u32)D_801B3200 >= 6)
    {
        return 0;
    }

    D_800D79E4[D_801B3200]();
    return 1;
}

void func_800BF1D8(void)
{
    D_801B3200 = 1;
    D_801B3204 = 1;
}

void func_800BF1F0(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB310, D_80139F78, 0x28, 0xFF, 1, 4, 0, D_80139280 + 2);
    if (--D_801B3204 == 0)
    {
        D_801B3200++;
    }
}

void func_800BF27C(void)
{
    D_801B3204 = 0x40;
    D_80139280[2].field_14 = -1;
    D_801B3200++;
    func_800BF2C4();
}

void func_800BF2C4(void)
{
    func_8006AEE0();
    func_8006A2FC(D_800DB310, D_80139F78, 0x28, 0xFF, 1, 4, 0, D_80139280 + 2);
    if (--D_801B3204 == 0)
    {
        D_801B3200++;
    }
}

void func_800BF350(void)
{
    D_801B3200++;
}

s32 func_800BF368(s32 reset)
{
    if (reset != 0)
    {
        D_801B3208 = 1;
        D_801B320C = 1;
        return 1;
    }

    if ((u32)D_801B3208 >= 4)
    {
        return 0;
    }

    D_800D79FC[D_801B3208]();
    return 1;
}

void func_800BF3E0(void)
{
    D_801B3208 = 1;
    D_801B320C = 1;
}

void func_800BF3F8(void)
{
    *(WmapPair*)&D_801B24A0 = D_80139258;
    D_801B2650 = D_80182DC0;
    D_80182DE8 = 0x80;
    D_801B2650.vz = 0xAFC8;
    D_801B320C = 0x40;
    D_801B3208++;
    func_800BD750();
}

void func_800BF4A8(void)
{
    D_801B3208++;
}

void func_800BF4C0(void)
{
    MATRIX matrix;
    s32 value;

    value = D_801B2650.vz + 0x7D0;
    D_801B2650.vz = value;
    if (value > 0xBB80)
    {
        D_801B2650.vz = 0xBB80;
    }
    PushMatrix();
    RotMatrix(&D_801B24A0, &matrix);
    TransMatrix(&matrix, &D_801B2650);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    func_800675F0(D_800DCF18, 0, 4, 0xB6, 0x7880, 1, D_80182DE8, 0, 0, -1);
    value = D_80182DE8 + 2;
    D_80182DE8 = value;
    if (value >= 0x41)
    {
        D_80182DE8 = 0x10080;
    }
    PopMatrix();
    if (--D_801B322C == 0)
    {
        D_801B3228++;
    }
}

void func_800BF5D4(void)
{
    MATRIX matrix;
    SVECTOR* rotation = &D_801B24A0;
    VECTOR* translation;

    rotation->vy += D_80139240;
    D_80139240 += 4;
    if (D_80139240 >= 0x101)
    {
        D_80139240 = 0x100;
    }
    D_8013923C -= 2;
    PushMatrix();
    translation = &D_801B2650;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    func_800675F0(D_800DCF18, 0, 4, 0xB6, 0x7880, 1, 0x10080, 0, D_8013923C >> 4, -1);
    PopMatrix();
    if (--D_801B322C == 0)
    {
        D_801B3228++;
    }
}

void func_800BF6F8(void)
{
    MATRIX matrix;
    SVECTOR* rotation = &D_801B24A0;
    VECTOR* translation;

    rotation->vy += D_80139240;
    D_80139240++;
    if (D_80139240 >= 0x81)
    {
        D_80139240 = 0x80;
    }
    D_8013923C -= 0x12C;
    PushMatrix();
    translation = &D_801B2650;
    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
    func_800675F0(D_800DCF18, 0, 4, 0xB6, 0x7880, 1, 0x10080, 0, D_8013923C >> 4, -1);
    PopMatrix();
    if (--D_801B322C == 0)
    {
        D_801B3228++;
    }
}

void func_800BF81C(void)
{
    D_801398D0 = 0;
    if (func_8006D0F0(D_80054A18[(D_80182DD8 - 3) % 5], &D_800DCEF8, &D_800DCF00) != 0)
    {
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
        D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    }
    D_801B3230++;
    func_800C06DC();
}

void func_800BF920(void)
{
    D_801398D0 = 0;
    if (func_8006D0F0(D_80054A18[(D_80182DD8 - 2) % 5], &D_800DCEF8, &D_800DCF00) != 0)
    {
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
        D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    }
    D_801B3230++;
    func_800C071C();
}

void func_800BFA24(void)
{
    D_801398D0 = 0;
    if (func_8006D0F0(D_80054A18[(D_80182DD8 - 1) % 5], &D_800DCEF8, &D_800DCF00) != 0)
    {
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
        D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    }
    D_801B3230++;
    func_800C075C();
}

void func_800BFB28(void)
{
    D_801398D0 = 0;
    if (func_8006D0F0(D_80054A18[D_80182DD8 % 5], &D_800DCEF8, &D_800DCF00) != 0)
    {
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
        D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    }
    D_801B3230++;
    func_800C079C();
}

void func_800BFC28(void)
{
    s32 x;
    s32 y;
    WmapConfigA* config = &D_800D9344;

    if (D_80182D58.fields.field_02 < 0x78)
    {
        D_80182D60.field_00 -= 0x80;
        D_80182D60.field_02 += 0x168;
    }

    x = D_80182D60.field_00;
    if (x < 0)
    {
        x += 0xF;
    }
    y = D_80182D60.field_02;
    D_80182D58.fields.field_00 = (x >> 4) + 0xE6;
    if (y < 0)
    {
        y += 0xF;
    }
    D_80182D58.fields.field_02 = y >> 4;
    func_8006CC4C(config, &D_801399B0);
    func_80066F9C(config, D_80182D58.value, 8, 8, 0);
    if (--D_801B323C == 0)
    {
        D_801B3238++;
    }
}

s32 func_800BFD18(s32 reset)
{
    if (reset != 0)
    {
        D_801B3218 = 1;
        D_801B321C = 1;
        return 1;
    }

    if ((u32)D_801B3218 >= 8)
    {
        return 0;
    }

    D_800D7A0C[D_801B3218]();
    return 1;
}

void func_800BFD90(void)
{
    D_801B3218 = 1;
    D_801B321C = 1;
}

void func_800BFDA8(void)
{
    s32 field_00;
    s32 field_04;

    D_8011CF4C.field_00 = 0xA4;
    D_8011CF4C.field_02 = 0x69;
    D_800DCEC8 = D_80139950;
    field_00 = D_800DCEEC;
    field_04 = D_800DCEF0;
    D_800DCEF0 = 1;
    D_800DCEEC = 1;
    D_801B3210 = field_00;
    D_801B3214 = field_04;
    func_800A89DC(0x22);
    D_801B321C = 0x1E;
    D_801B3218++;
}

void func_800BFE54(void)
{
    if (--D_801B321C == 0)
    {
        D_801B3218++;
    }
}

void func_800BFE88(void)
{
    cdrom_wait_queue_empty();
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    func_8006CAC0(func_800C0034);
    D_8013B20C = 1;
    D_801B3218++;
    func_800BFEEC();
}

void func_800BFEEC(void)
{
    if (D_8013B20C == 0)
    {
        D_801B3218++;
        func_800BFF28();
    }
}

void func_800BFF28(void)
{
    D_801398D0 = 2;
    D_80182D68 = D_800DCEC8.field_00 - D_80139950.field_00;
    D_80182D78 = D_800DCEC8.field_04 - D_80139950.field_04;
    D_801B3218++;
    func_800BFF98();
}

void func_800BFF98(void)
{
    if (D_801398D0 != 2)
    {
        D_801B3218++;
        func_800BFFD8();
    }
}

void func_800BFFD8(void)
{
    D_8013B208 = 0;
    func_80064094();
    D_8013B294 = 1;
    D_800DCEEC = D_801B3210;
    D_800DCEF0 = D_801B3214;
    D_801B3218++;
}

s32 func_800C0034(s32 reset)
{
    if (reset != 0)
    {
        D_801B3220 = 1;
        D_801B3224 = 1;
        return 1;
    }

    if ((u32)D_801B3220 >= 0xE)
    {
        return 0;
    }

    D_800D7A2C[D_801B3220]();
    return 1;
}

void func_800C00AC(void)
{
    D_801B3220 = 1;
    D_801B3224 = 1;
}
