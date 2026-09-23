#include "wmap_special_effect_34.h"
#include "wmap_sprite_render.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "wmap_effect_primitives.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_resources.h"
#include "cdrom.h"
#include "sdk/libgte.h"

void func_800BF4C0(void)
{
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
extern void func_800BFEEC__for_func_800BF4C0(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BF4C0(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BF4C0(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BF4C0(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BF4C0(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BF4C0(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BF4C0(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BF4C0(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BF4C0(void) __asm__("func_800C079C");
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
extern void func_800BFEEC__for_func_800BF5D4(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BF5D4(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BF5D4(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BF5D4(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BF5D4(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BF5D4(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BF5D4(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BF5D4(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BF5D4(void) __asm__("func_800C079C");
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
extern void func_800BFEEC__for_func_800BF6F8(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BF6F8(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BF6F8(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BF6F8(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BF6F8(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BF6F8(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BF6F8(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BF6F8(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BF6F8(void) __asm__("func_800C079C");
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
extern void func_800BFEEC__for_func_800BF81C(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BF81C(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BF81C(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BF81C(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BF81C(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BF81C(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BF81C(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BF81C(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BF81C(void) __asm__("func_800C079C");
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

    D_801398D0 = 0;
    if (func_8006D0F0(D_80054A18[(D_80182DD8 - 3) % 5], &D_800DCEF8, &D_800DCF00) != 0)
    {
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
        D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    }
    D_801B3230++;
    func_800C06DC__for_func_800BF81C();
}

void func_800BF920(void)
{
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
extern void func_800BFEEC__for_func_800BF920(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BF920(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BF920(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BF920(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BF920(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BF920(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BF920(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BF920(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BF920(void) __asm__("func_800C079C");
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

    D_801398D0 = 0;
    if (func_8006D0F0(D_80054A18[(D_80182DD8 - 2) % 5], &D_800DCEF8, &D_800DCF00) != 0)
    {
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
        D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    }
    D_801B3230++;
    func_800C071C__for_func_800BF920();
}

void func_800BFA24(void)
{
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
extern void func_800BFEEC__for_func_800BFA24(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFA24(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFA24(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFA24(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFA24(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFA24(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFA24(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFA24(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFA24(void) __asm__("func_800C079C");
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

    D_801398D0 = 0;
    if (func_8006D0F0(D_80054A18[(D_80182DD8 - 1) % 5], &D_800DCEF8, &D_800DCF00) != 0)
    {
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
        D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    }
    D_801B3230++;
    func_800C075C__for_func_800BFA24();
}

void func_800BFB28(void)
{
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
extern void func_800BFEEC__for_func_800BFB28(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFB28(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFB28(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFB28(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFB28(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFB28(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFB28(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFB28(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFB28(void) __asm__("func_800C079C");
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

    D_801398D0 = 0;
    if (func_8006D0F0(D_80054A18[D_80182DD8 % 5], &D_800DCEF8, &D_800DCF00) != 0)
    {
        D_801398D0 = 2;
        D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
        D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    }
    D_801B3230++;
    func_800C079C__for_func_800BFB28();
}

void func_800BFC28(void)
{
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
extern void func_800BFEEC__for_func_800BFC28(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFC28(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFC28(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFC28(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFC28(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFC28(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFC28(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFC28(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFC28(void) __asm__("func_800C079C");
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
extern void func_800BFEEC__for_func_800BFD18(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFD18(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFD18(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFD18(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFD18(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFD18(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFD18(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFD18(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFD18(void) __asm__("func_800C079C");
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
extern void func_800BFEEC__for_func_800BFD90(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFD90(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFD90(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFD90(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFD90(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFD90(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFD90(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFD90(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFD90(void) __asm__("func_800C079C");
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

    D_801B3218 = 1;
    D_801B321C = 1;
}

void func_800BFDA8(void)
{
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
extern void func_800BFEEC__for_func_800BFDA8(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFDA8(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFDA8(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFDA8(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFDA8(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFDA8(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFDA8(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFDA8(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFDA8(void) __asm__("func_800C079C");
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
extern void func_800BFEEC__for_func_800BFE54(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFE54(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFE54(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFE54(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFE54(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFE54(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFE54(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFE54(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFE54(void) __asm__("func_800C079C");
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

    if (--D_801B321C == 0)
    {
        D_801B3218++;
    }
}

void func_800BFE88(void)
{
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
extern void func_800BFEEC__for_func_800BFE88(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFE88(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFE88(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFE88(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFE88(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFE88(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFE88(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFE88(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFE88(void) __asm__("func_800C079C");
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

    cdrom_wait_queue_empty();
    func_800651B4(&D_80182E40);
    func_800651B4(&D_8018B240);
    func_8006CAC0(func_800C0034__for_func_800BFE88);
    D_8013B20C = 1;
    D_801B3218++;
    func_800BFEEC__for_func_800BFE88();
}

void func_800BFEEC(void)
{
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
extern void func_800BFF28__for_func_800BFEEC(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFEEC(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFEEC(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFEEC(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFEEC(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFEEC(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFEEC(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFEEC(void) __asm__("func_800C079C");
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

    if (D_8013B20C == 0)
    {
        D_801B3218++;
        func_800BFF28__for_func_800BFEEC();
    }
}

void func_800BFF28(void)
{
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
extern void func_800BFEEC__for_func_800BFF28(void) __asm__("func_800BFEEC");
extern void func_800BFF28(void);
extern void func_800BFF98__for_func_800BFF28(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800BFF28(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFF28(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFF28(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFF28(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFF28(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFF28(void) __asm__("func_800C079C");
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

    D_801398D0 = 2;
    D_80182D68 = D_800DCEC8.field_00 - D_80139950.field_00;
    D_80182D78 = D_800DCEC8.field_04 - D_80139950.field_04;
    D_801B3218++;
    func_800BFF98__for_func_800BFF28();
}

void func_800BFF98(void)
{
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
extern void func_800BFEEC__for_func_800BFF98(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFF98(void) __asm__("func_800BFF28");
extern void func_800BFF98(void);
extern void func_800BFFD8__for_func_800BFF98(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800BFF98(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFF98(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFF98(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFF98(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFF98(void) __asm__("func_800C079C");
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

    if (D_801398D0 != 2)
    {
        D_801B3218++;
        func_800BFFD8__for_func_800BFF98();
    }
}

void func_800BFFD8(void)
{
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
extern void func_800BFEEC__for_func_800BFFD8(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800BFFD8(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800BFFD8(void) __asm__("func_800BFF98");
extern void func_800BFFD8(void);
extern s32 func_800C0034__for_func_800BFFD8(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800BFFD8(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800BFFD8(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800BFFD8(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800BFFD8(void) __asm__("func_800C079C");
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

    D_8013B208 = 0;
    wmap_reset_after_transition();
    D_8013B294 = 1;
    D_800DCEEC = D_801B3210;
    D_800DCEF0 = D_801B3214;
    D_801B3218++;
}

s32 func_800C0034(s32 reset)
{
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
extern void func_800BFEEC__for_func_800C0034(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800C0034(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800C0034(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800C0034(void) __asm__("func_800BFFD8");
extern s32 func_800C0034(s32);
extern void func_800C06DC__for_func_800C0034(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800C0034(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800C0034(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800C0034(void) __asm__("func_800C079C");
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
extern void func_800BFEEC__for_func_800C00AC(void) __asm__("func_800BFEEC");
extern void func_800BFF28__for_func_800C00AC(void) __asm__("func_800BFF28");
extern void func_800BFF98__for_func_800C00AC(void) __asm__("func_800BFF98");
extern void func_800BFFD8__for_func_800C00AC(void) __asm__("func_800BFFD8");
extern s32 func_800C0034__for_func_800C00AC(s32) __asm__("func_800C0034");
extern void func_800C06DC__for_func_800C00AC(void) __asm__("func_800C06DC");
extern void func_800C071C__for_func_800C00AC(void) __asm__("func_800C071C");
extern void func_800C075C__for_func_800C00AC(void) __asm__("func_800C075C");
extern void func_800C079C__for_func_800C00AC(void) __asm__("func_800C079C");
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

    D_801B3220 = 1;
    D_801B3224 = 1;
}

void func_800C00C4(void)
{
extern s32 D_8013924C;
extern s32 D_8013B208;
extern s32 D_80139234;
extern s32 D_801B3220;
extern s32 func_800C0474__for_func_800C00C4(s32) __asm__("func_800C0474");
extern s32 func_800C064C__for_func_800C00C4(s32) __asm__("func_800C064C");
extern void func_800C0150__for_func_800C00C4(void) __asm__("func_800C0150");
extern void func_800C018C__for_func_800C00C4(void) __asm__("func_800C018C");

    D_8013B208 = 1;
    func_800652A8(0x3B, 0x80);
    func_8006683C(0x703040);
    g_wmap_backdrop_target_level = 0xA;
    D_80139234 = 1;
    D_8013924C = 1;
    func_8006CAC0(func_800C0474__for_func_800C00C4);
    func_8006CAC0(func_800C064C__for_func_800C00C4);
    D_801B3220++;
    func_800C0150__for_func_800C00C4();
}

/**
 * @brief Advance this sequence one step while its gate flag is clear.
 */
void func_800C0150(void)
{
extern s32 D_8013924C;
extern s32 D_8013B208;
extern s32 D_80139234;
extern s32 D_801B3220;
extern s32 func_800C0474__for_func_800C0150(s32) __asm__("func_800C0474");
extern s32 func_800C064C__for_func_800C0150(s32) __asm__("func_800C064C");
extern void func_800C0150(void);
extern void func_800C018C__for_func_800C0150(void) __asm__("func_800C018C");

    if (D_80139234 == 0)
    {
        D_801B3220 += 1;
        func_800C018C__for_func_800C0150();
    }
}

/**
 * @brief Prepare sequence coordinates for the selected world-map entry and advance the step.
 */
void func_800C018C(void)
{
typedef struct
{
    s32 field_00;
    s32 field_04;
} WmapAlignedPair;

extern const s32 D_80054A18[];
extern s32 D_800DCEF8;
extern s32 D_800DCF00;
extern s32 D_801398D0;
extern WmapAlignedPair D_80139950;
extern s32 D_80182D68;
extern s32 D_80182D78;
extern s32 D_80182DD8;
extern s32 D_801B3220;
extern void func_800C0254__for_func_800C018C(void) __asm__("func_800C0254");

    func_8006D0F0(D_80054A18[D_80182DD8 - 4], &D_800DCEF8, &D_800DCF00);
    D_801398D0 = 2;
    D_80182D68 = ((D_800DCEF8 - 1) * 0x30) - D_80139950.field_00;
    D_80182D78 = ((D_800DCF00 - 1) * 0x30) - D_80139950.field_04;
    D_801B3220++;
    func_800C0254__for_func_800C018C();
}

/**
 * @brief Advance this sequence one step unless its gate flag hit the stop value.
 */
void func_800C0254(void)
{
extern s32 D_801398D0;
extern s32 D_801B3220;
extern void func_800C0294__for_func_800C0254(void) __asm__("func_800C0294");

    if (D_801398D0 != 2)
    {
        D_801B3220 += 1;
        func_800C0294__for_func_800C0254();
    }
}

/** @brief World-map step handler: bump the step counter and run the next step. */
void func_800C0294(void)
{
extern s32 D_801B3220;
extern void func_800C02C0__for_func_800C0294(void) __asm__("func_800C02C0");

    D_801B3220 += 1;
    func_800C02C0__for_func_800C0294();
}

void func_800C02C0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C02C0(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C02C0(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C02C0(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C02C0(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C02C0(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C02C0(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C02C0(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C02C0(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C02C0(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C02C0(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C02C0(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C02C0(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C02C0(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C02C0(void) __asm__("func_800C0BC8");

    if (D_8013924C == 0)
    {
        D_801B3220++;
        func_800C02FC__for_func_800C02C0();
    }
}

void func_800C02FC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C02FC(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C02FC(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C02FC(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C02FC(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C02FC(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C02FC(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C02FC(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C02FC(void) __asm__("func_800C07DC");
extern void func_800C02FC(void);
extern s32 func_800C07F8__for_func_800C02FC(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C02FC(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C02FC(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C02FC(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C02FC(void) __asm__("func_800C0BC8");

    func_8006CAC0(func_800C07F8__for_func_800C02FC);
    D_801B3224 = 0xC;
    D_801B3220++;
}

void func_800C0338(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0338(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0338(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0338(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0338(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0338(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0338(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0338(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0338(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0338(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0338(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0338(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0338(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0338(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0338(void) __asm__("func_800C0BC8");

    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C036C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C036C(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C036C(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C036C(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C036C(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C036C(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C036C(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C036C(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C036C(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C036C(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C036C(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C036C(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C036C(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C036C(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C036C(void) __asm__("func_800C0BC8");

    func_8006CAC0(func_800C09F4__for_func_800C036C);
    D_801B3224 = 0x5A;
    D_801B3220++;
}

void func_800C03A8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C03A8(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C03A8(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C03A8(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C03A8(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C03A8(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C03A8(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C03A8(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C03A8(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C03A8(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C03A8(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C03A8(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C03A8(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C03A8(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C03A8(void) __asm__("func_800C0BC8");

    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C03DC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C03DC(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C03DC(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C03DC(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C03DC(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C03DC(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C03DC(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C03DC(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C03DC(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C03DC(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C03DC(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C03DC(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C03DC(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C03DC(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C03DC(void) __asm__("func_800C0BC8");

    func_8006683C(0x808080);
    g_wmap_backdrop_target_level = 0x10;
    D_801B3224 = 0x3C;
    D_801B3220++;
}

void func_800C0424(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0424(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0424(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0424(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0424(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0424(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0424(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0424(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0424(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0424(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0424(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0424(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0424(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0424(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0424(void) __asm__("func_800C0BC8");

    if (--D_801B3224 == 0)
    {
        D_801B3220++;
    }
}

void func_800C0458(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0458(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0458(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0458(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0458(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0458(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0458(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0458(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0458(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0458(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0458(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0458(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0458(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0458(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0458(void) __asm__("func_800C0BC8");

    D_8013B20C = 0;
    D_801B3220++;
}

s32 func_800C0474(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0474(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0474(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0474(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0474(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0474(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0474(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0474(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0474(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0474(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0474(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0474(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0474(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0474(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0474(void) __asm__("func_800C0BC8");

    if (reset != 0)
    {
        D_801B3228 = 1;
        D_801B322C = 1;
        return 1;
    }

    if ((u32)D_801B3228 >= 8)
    {
        return 0;
    }

    D_800D7A64[D_801B3228]();
    return 1;
}

void func_800C04EC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C04EC(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C04EC(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C04EC(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C04EC(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C04EC(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C04EC(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C04EC(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C04EC(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C04EC(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C04EC(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C04EC(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C04EC(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C04EC(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C04EC(void) __asm__("func_800C0BC8");

    D_801B3228 = 1;
    D_801B322C = 1;
}

void func_800C0504(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0504(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0504(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0504(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0504(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0504(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0504(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0504(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0504(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0504(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0504(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0504(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0504(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0504(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0504(void) __asm__("func_800C0BC8");

    D_801B24A0 = D_80139258;
    D_801B2650 = D_8011CF60;
    D_80182DE8 = 0;
    D_801B2650.vz = 0x1388;
    D_80139240 = 0;
    D_8013923C = 0;
    D_801B322C = 0x3C;
    D_801B3228++;
    func_800BF4C0__for_func_800C0504();
}

void func_800C05C0(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C05C0(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C05C0(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C05C0(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C05C0(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C05C0(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C05C0(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C05C0(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C05C0(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C05C0(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C05C0(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C05C0(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C05C0(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C05C0(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C05C0(void) __asm__("func_800C0BC8");

    D_801B322C = 0xB4;
    D_801B3228++;
    func_800BF5D4__for_func_800C05C0();
}

void func_800C05F8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C05F8(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C05F8(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C05F8(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C05F8(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C05F8(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C05F8(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C05F8(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C05F8(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C05F8(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C05F8(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C05F8(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C05F8(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C05F8(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C05F8(void) __asm__("func_800C0BC8");

    D_801B322C = 0x1E;
    D_801B3228++;
    func_800BF6F8__for_func_800C05F8();
}

void func_800C0630(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0630(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0630(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0630(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0630(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0630(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0630(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0630(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0630(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0630(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0630(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0630(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0630(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0630(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0630(void) __asm__("func_800C0BC8");

    D_8013924C = 0;
    D_801B3228++;
}

s32 func_800C064C(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C064C(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C064C(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C064C(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C064C(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C064C(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C064C(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C064C(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C064C(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C064C(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C064C(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C064C(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C064C(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C064C(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C064C(void) __asm__("func_800C0BC8");

    if (reset != 0)
    {
        D_801B3230 = 1;
        D_801B3234 = 1;
        return 1;
    }

    if ((u32)D_801B3230 >= 0xA)
    {
        return 0;
    }

    D_800D7A84[D_801B3230]();
    return 1;
}

void func_800C06C4(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C06C4(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C06C4(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C06C4(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C06C4(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C06C4(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C06C4(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C06C4(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C06C4(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C06C4(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C06C4(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C06C4(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C06C4(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C06C4(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C06C4(void) __asm__("func_800C0BC8");

    D_801B3230 = 1;
    D_801B3234 = 1;
}

void func_800C06DC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C06DC(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C06DC(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C06DC(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C06DC(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C06DC(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C06DC(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C06DC(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C06DC(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C06DC(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C06DC(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C06DC(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C06DC(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C06DC(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C06DC(void) __asm__("func_800C0BC8");

    if (D_801398D0 != 2)
    {
        D_801B3230++;
        func_800BF920__for_func_800C06DC();
    }
}

void func_800C071C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C071C(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C071C(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C071C(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C071C(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C071C(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C071C(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C071C(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C071C(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C071C(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C071C(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C071C(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C071C(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C071C(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C071C(void) __asm__("func_800C0BC8");

    if (D_801398D0 != 2)
    {
        D_801B3230++;
        func_800BFA24__for_func_800C071C();
    }
}

void func_800C075C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C075C(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C075C(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C075C(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C075C(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C075C(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C075C(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C075C(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C075C(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C075C(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C075C(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C075C(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C075C(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C075C(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C075C(void) __asm__("func_800C0BC8");

    if (D_801398D0 != 2)
    {
        D_801B3230++;
        func_800BFB28__for_func_800C075C();
    }
}

void func_800C079C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C079C(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C079C(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C079C(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C079C(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C079C(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C079C(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C079C(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C079C(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C079C(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C079C(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C079C(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C079C(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C079C(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C079C(void) __asm__("func_800C0BC8");

    if (D_801398D0 != 2)
    {
        D_801B3230++;
        func_800C07DC__for_func_800C079C();
    }
}

void func_800C07DC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C07DC(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C07DC(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C07DC(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C07DC(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C07DC(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C07DC(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C07DC(void) __asm__("func_800BFC28");
extern void func_800C07DC(void);
extern void func_800C02FC__for_func_800C07DC(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C07DC(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C07DC(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C07DC(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C07DC(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C07DC(void) __asm__("func_800C0BC8");

    D_80139234 = 0;
    D_801B3230++;
}

s32 func_800C07F8(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C07F8(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C07F8(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C07F8(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C07F8(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C07F8(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C07F8(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C07F8(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C07F8(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C07F8(void) __asm__("func_800C02FC");
extern s32 func_800C07F8(s32);
extern s32 func_800C09F4__for_func_800C07F8(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C07F8(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C07F8(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C07F8(void) __asm__("func_800C0BC8");

    if (reset != 0)
    {
        D_801B3238 = 1;
        D_801B323C = 1;
        return 1;
    }

    if ((u32)D_801B3238 >= 6)
    {
        return 0;
    }

    D_800D7AAC[D_801B3238]();
    return 1;
}

void func_800C0870(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0870(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0870(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0870(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0870(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0870(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0870(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0870(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0870(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0870(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0870(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0870(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0870(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0870(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0870(void) __asm__("func_800C0BC8");

    D_801B3238 = 1;
    D_801B323C = 1;
}

void func_800C0888(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0888(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0888(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0888(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0888(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0888(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0888(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0888(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0888(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0888(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0888(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0888(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0888(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0888(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0888(void) __asm__("func_800C0BC8");

    D_801399B4 = &D_8011F538;
    D_800D9344.field_06 = 0xF;
    D_800D9344.field_0E = 1;
    D_800D9344.field_10 = -1;
    D_800D9344.field_02 = 0;
    D_800D9344.field_26 = 0;
    D_800D9344.field_22 = 0x80;
    D_800D9344.field_24 = 0x80;
    D_80182D60.field_00 = 0;
    D_80182D60.field_02 = 0;
    D_801B323C = 0x64;
    D_801B3238++;
    func_800BFC28__for_func_800C0888();
}

void func_800C0914(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0914(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0914(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0914(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0914(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0914(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0914(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0914(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0914(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0914(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0914(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0914(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0914(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0914(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0914(void) __asm__("func_800C0BC8");

    D_800D9344.field_26 = 4;
    D_800D9344.field_22 = 0;
    D_801B323C = 0x20;
    D_801B3238++;
    func_800C0960__for_func_800C0914();
}

void func_800C0960(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0960(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0960(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0960(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0960(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0960(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0960(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0960(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0960(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0960(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0960(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0960(s32) __asm__("func_800C09F4");
extern void func_800C0960(void);
extern void func_800C0B00__for_func_800C0960(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0960(void) __asm__("func_800C0BC8");

    func_8006CC4C(&D_800D9344, &D_801399B0);
    func_80066F9C(&D_800D9344, D_80182D58, 8, 8, 0);
    if (--D_801B323C == 0)
    {
        D_801B3238++;
    }
}

void func_800C09DC(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C09DC(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C09DC(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C09DC(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C09DC(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C09DC(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C09DC(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C09DC(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C09DC(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C09DC(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C09DC(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C09DC(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C09DC(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C09DC(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C09DC(void) __asm__("func_800C0BC8");

    D_801B3238++;
}

s32 func_800C09F4(s32 reset)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C09F4(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C09F4(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C09F4(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C09F4(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C09F4(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C09F4(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C09F4(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C09F4(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C09F4(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C09F4(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4(s32);
extern void func_800C0960__for_func_800C09F4(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C09F4(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C09F4(void) __asm__("func_800C0BC8");

    if (reset != 0)
    {
        D_801B3240 = 1;
        D_801B3244 = 1;
        return 1;
    }

    if ((u32)D_801B3240 >= 6)
    {
        return 0;
    }

    D_800D7AC4[D_801B3240]();
    return 1;
}

void func_800C0A6C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0A6C(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0A6C(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0A6C(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0A6C(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0A6C(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0A6C(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0A6C(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0A6C(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0A6C(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0A6C(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0A6C(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0A6C(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0A6C(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0A6C(void) __asm__("func_800C0BC8");

    D_801B3240 = 1;
    D_801B3244 = 1;
}

void func_800C0A84(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0A84(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0A84(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0A84(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0A84(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0A84(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0A84(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0A84(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0A84(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0A84(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0A84(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0A84(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0A84(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0A84(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0A84(void) __asm__("func_800C0BC8");

    D_801399BC = &D_8011F538;
    D_800D9370.field_06 = 0xF;
    D_800D9370.field_10 = -1;
    D_800D9370.field_26 = 0x10;
    D_800D9370.field_02 = 0;
    D_800D9370.field_0E = 0;
    D_800D9370.field_22 = 0x80;
    D_800D9370.field_24 = 0;
    D_801B3244 = 0x3E;
    D_801B3240++;
    func_800C0B00__for_func_800C0A84();
}

void func_800C0B00(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0B00(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0B00(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0B00(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0B00(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0B00(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0B00(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0B00(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0B00(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0B00(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0B00(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0B00(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0B00(void) __asm__("func_800C0960");
extern void func_800C0B00(void);
extern void func_800C0BC8__for_func_800C0B00(void) __asm__("func_800C0BC8");

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 8, 8, 0);
    if (--D_801B3244 == 0)
    {
        D_801B3240++;
    }
}

void func_800C0B7C(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0B7C(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0B7C(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0B7C(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0B7C(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0B7C(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0B7C(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0B7C(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0B7C(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0B7C(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0B7C(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0B7C(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0B7C(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0B7C(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0B7C(void) __asm__("func_800C0BC8");

    D_800D9370.field_26 = 2;
    D_800D9370.field_22 = 0;
    D_801B3244 = 0x40;
    D_801B3240++;
    func_800C0BC8__for_func_800C0B7C();
}

void func_800C0BC8(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0BC8(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0BC8(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0BC8(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0BC8(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0BC8(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0BC8(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0BC8(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0BC8(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0BC8(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0BC8(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0BC8(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0BC8(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0BC8(void) __asm__("func_800C0B00");
extern void func_800C0BC8(void);

    func_8006CC4C(&D_800D9370, &D_801399B8);
    func_80066F9C(&D_800D9370, D_8011CF4C, 8, 8, 0);
    if (--D_801B3244 == 0)
    {
        D_801B3240++;
    }
}

void func_800C0C44(void)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0C44(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0C44(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0C44(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0C44(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0C44(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0C44(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0C44(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0C44(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0C44(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0C44(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0C44(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0C44(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0C44(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0C44(void) __asm__("func_800C0BC8");

    D_801B3240++;
}

void func_800C0C5C(VECTOR* translation, SVECTOR* rotation)
{
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

typedef struct
{
    s16 field_00;
    s16 field_02;
} WmapPair16;

typedef struct
{
    s32 field_00;
    s32 field_04;
} __attribute__((packed)) WmapPair;

typedef void (*WmapHandler)(void);

extern WmapHandler D_800D7A64[];
extern WmapHandler D_800D7A84[];
extern WmapHandler D_800D7AAC[];
extern WmapHandler D_800D7AC4[];
extern WmapConfigA D_800D9344;
extern WmapConfigA D_800D9370;
extern VECTOR D_8011CF60;
extern u8 D_8011F538;
extern s32 D_80182D58;
extern WmapPair16 D_80182D60;
extern s32 D_80182DE8;
extern WmapPair D_80139258;
extern s32 D_8013923C;
extern s32 D_80139240;
extern s32 D_80139234;
extern s32 D_8013924C;
extern s32 D_8013B20C;
extern s32 D_801398D0;
extern u8 D_801399B0;
extern void* D_801399B4;
extern u8 D_801399B8;
extern void* D_801399BC;
extern s32 D_8011CF4C;
extern WmapPair D_801B24A0;
extern VECTOR D_801B2650;
extern s32 D_801B3220;
extern s32 D_801B3224;
extern s32 D_801B3228;
extern s32 D_801B322C;
extern s32 D_801B3230;
extern s32 D_801B3234;
extern s32 D_801B3238;
extern s32 D_801B323C;
extern s32 D_801B3240;
extern s32 D_801B3244;

extern void func_800BF4C0__for_func_800C0C5C(void) __asm__("func_800BF4C0");
extern void func_800BF5D4__for_func_800C0C5C(void) __asm__("func_800BF5D4");
extern void func_800BF6F8__for_func_800C0C5C(void) __asm__("func_800BF6F8");
extern void func_800BF920__for_func_800C0C5C(void) __asm__("func_800BF920");
extern void func_800BFA24__for_func_800C0C5C(void) __asm__("func_800BFA24");
extern void func_800BFB28__for_func_800C0C5C(void) __asm__("func_800BFB28");
extern void func_800BFC28__for_func_800C0C5C(void) __asm__("func_800BFC28");
extern void func_800C07DC__for_func_800C0C5C(void) __asm__("func_800C07DC");
extern void func_800C02FC__for_func_800C0C5C(void) __asm__("func_800C02FC");
extern s32 func_800C07F8__for_func_800C0C5C(s32) __asm__("func_800C07F8");
extern s32 func_800C09F4__for_func_800C0C5C(s32) __asm__("func_800C09F4");
extern void func_800C0960__for_func_800C0C5C(void) __asm__("func_800C0960");
extern void func_800C0B00__for_func_800C0C5C(void) __asm__("func_800C0B00");
extern void func_800C0BC8__for_func_800C0C5C(void) __asm__("func_800C0BC8");

    MATRIX matrix;

    RotMatrix(rotation, &matrix);
    TransMatrix(&matrix, translation);
    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);
}
