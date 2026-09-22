#include "wmap_map_display.h"
#include "wmap_view_effects.h"
#include "wmap_resource_support.h"
#include "wmap_main.h"
#include "common.h"
#include "sdk/libgpu.h"
#include "sdk/libgte.h"
#include "sdk/libetc.h"
#include "wmap_map_labels.h"
#include "wmap_sequence_runtime.h"
#include "wmap_effect_backdrop.h"
#include "wmap_frame_render.h"

#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8*)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)
#define M2C_BITWISE(type, expr) ((type)(expr))

typedef s32 M2C_UNK;
typedef s8 M2C_UNK8;
typedef s16 M2C_UNK16;
typedef s32 M2C_UNK32;

/** @brief Rectangle used by world-map image transfers. */
typedef struct
{
    s16 x;
    s16 y;
    s16 w;
    s16 h;
} WmapRect;

/** @brief World-map resource cache entry. */
typedef struct
{
    s32 unk0;
    s32 resource_id;
    u32 age;
    s32 busy;
    u8* image;
} WmapCacheEntry;

/** @brief World-map resource state and cache slot. */
typedef struct
{
    u8 pad0[2];
    s16 resource_id;
    u8 pad4;
    u8 state;
    u8 pad6[8];
    s16 unkE;
    s16 slot;
    u8 pad12[2];
    s32 busy;
    u8 pad18[0x14];
} WmapResource;

/** @brief Partially identified world-map drawing environment. */
typedef struct
{
    u8 pad0[0xC];
    s16 tw_x;
    s16 tw_y;
    u8 pad10[0x4C];
} WmapDrawEnv;

/** @brief World-map display environment storage. */
typedef struct
{
    u8 data[0x14];
} WmapDispEnv;

/** @brief World-map render context and primitive allocation cursor. */
typedef struct
{
    WmapDrawEnv draw_env;
    WmapDispEnv disp_env;
    u32 ot[0xB3];
    u8* prim_cursor;
} WmapRenderContext;

/** @brief World-map tile and its cached neighboring layout data. */
typedef struct
{
    s32 tile;
    s16 field_04;
    s16 field_06;
    u8 neighbors[32];
} WmapTile;

/** @brief Per-tile display state. */
typedef struct
{
    s16 field_00;
    s16 field_02;
    u8 pad_04[24];
} WmapTileDisplay;

/** @brief Actor storage containing the map display mode. */
typedef struct
{
    u8 pad_00[0x26];
    s16 display_mode;
    u8 pad_28[4];
} WmapActor;

/** @brief Motion state for a world-map actor. */
typedef struct
{
    s16 state;
    u8 pad_02[0x12];
} WmapMotion;

extern WmapRenderContext* D_801398EC;
extern void* D_800D0454;
extern s32 D_800DCEDC;
extern u8 D_80129560[];
M2C_UNK akao_cmd_f0();
M2C_UNK akao_cmd_f1();
extern u8 D_80051A80;
extern s32 D_800D0550;
extern s32 D_80182228;
extern s32 D_80182240;
extern u8 D_80139258;
extern void func_8005909C(void);
M2C_UNK func_8005B548();
s32 func_8005D494(void);
extern s32 D_8005136C;
extern s32 D_800D06BC;
extern s32 D_800D9160;
extern s32 D_800D9168;
extern s32 D_800D916C;
extern POLY_FT4 D_800D9170;
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
extern M2C_UNK D_8010CF18;
extern M2C_UNK D_80114F18;
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
extern s32 D_801391E0;
extern s32 D_80139218;
extern s32 D_80139224;
extern s32 D_80139228;
extern s32 D_80139244;
extern s32 D_80139248;
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
extern M2C_UNK D_80139950;
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
extern u8 D_80182D48;
extern s32 D_80182D70;
extern s32 D_80182D88;
extern u8 D_80182DC0;
extern u32 D_80182DD8;
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
extern u8 D_800D0554;
extern u8 D_800D05F4;
extern s32 D_800D06E0;
extern s32 D_800D921C;
extern u8 D_80182D74;
extern u8 D_80182D75;
extern u8 D_80182D76;
extern u8 D_80182D80;
extern u8 D_80182D81;
extern u8 D_80182D82;
extern u8 D_80182D8C;
extern u8 D_80182D8D;
extern u8 D_80182D8E;
extern u8 D_80182D94;
extern u8 D_80182D95;
extern u8 D_80182D96;
extern POLY_FT4 D_800D0694;
extern u8 D_800D0698;
M2C_UNK akao_play_sfx_from_buffer(s32, M2C_UNK, M2C_UNK, M2C_UNK);
void cdrom_queue_read();
M2C_UNK cdrom_wait_queue_empty();
extern s32 D_800CB204;
extern s32 D_800CB224;
extern s32 D_800CB23C;
extern s32 D_800CB254;
extern SPRT D_800D06E4;
extern u8 D_800D06F8;
extern s32 D_8013922C;
extern s32 D_801398C0;
extern u8 D_8019D6E0;
extern s32 D_80139230;
extern s32 D_80182DB4;
extern s32 D_80182DE0;
extern u8 D_800D0544;
extern WmapTile D_80139290[6][6];
extern u8 D_8019D248[];
extern s32 D_800CB248;
extern u8 D_800D0A08[];
extern s32 D_800D9228;
extern s32 D_800D9238;
extern s32 D_800DBE68;
extern s32 D_800DBE7C;
extern u8 D_800DCF18[];
extern u16 D_800DCF2C[];
extern WmapCacheEntry D_8011CF88[];
extern s32 D_8011D0E0;
extern s32 D_8011D0E4;
extern s32 D_8011D500;
extern M2C_UNK D_80129538;
extern u8 D_801295BC[];
extern s32 D_80139238;
extern s32 D_80139834;
extern s32 D_8013986C;
extern M2C_UNK D_80139870;
extern M2C_UNK D_80139888;
extern u32 D_801398F8;
extern u8 D_80139908[];
extern u16 D_8013B210;
extern s32 D_8013B234;
extern M2C_UNK D_8013B238;
extern M2C_UNK D_8013B240;
extern s32 D_8013B24C;
extern u16 D_8013B2A0;
extern u16 D_8013C628;
extern WmapResource D_80182248[];
extern s32 D_80182D5C;
extern s32 D_80182DD4;
extern s32 D_80182E38;
extern s32 D_801ADAF8;
extern s32 D_801ADB08;
extern u16 D_801ADBA0;
extern M2C_UNK D_801B2478;
extern M2C_UNK D_801B24A8;
extern M2C_UNK func_800BFD18;
extern u8 D_80051A88[];
extern s32 D_800DCE98;
extern s32 D_800DCE9C;
extern WmapTileDisplay D_8011D108[6][6];
extern s32 D_80139830;
extern s32 D_80182E20;
extern s32 func_8005D670(s32, s32);
extern s32 func_8005D554(u32, s32);
extern void func_8005D6B8(s32, s32, void*);
extern s16 func_8005B8C8(s32, s32, s32);
extern WmapActor D_800D9268[];
extern WmapMotion D_801AFBD0[];
extern void akao_cmd_c2(s32, s32, s32, s32);
extern s32 D_8011D52C;
extern s32 D_80129550;

void func_80060720(void);
void func_80060918(void);
s32 func_80060F04(s32 initialize);
s32 func_80061878(s32 initialize);
void func_80061A2C(s32 value);
void func_80061F18(s32* state);
void func_80061FF8(s32* state);
s32 func_800623CC(void);
void func_80063BE0(void);
void func_80063F38(void);

/** @brief Configure the two world-map frame buffers and clear their ordering tables. */
void func_80060720(void)
{
    SetGeomOffset(160, 120);
    D_800DCEDC = 0x4000;
    SetGeomScreen(0x4000);
    SetDispMask(1);
    SetDefDrawEnv((DRAWENV*)D_80129560, 0, 8, 320, 224);
    SetDefDrawEnv((DRAWENV*)(D_80129560 + 0x7E40), 0, 248, 320, 224);
    SetDefDispEnv((DISPENV*)(D_80129560 + 0x5C), 0, 240, 320, 240);
    SetDefDispEnv((DISPENV*)(D_80129560 + 0x7E9C), 0, 0, 320, 240);
    *(s16*)(D_80129560 + 0x7E40) = 8;
    ((DRAWENV*)D_80129560)->clip.x = 8;
    *(s16*)(D_80129560 + 0x7E44) = 312;
    ((DRAWENV*)D_80129560)->clip.w = 312;
    ClearOTagR((u_long*)(D_80129560 + 0x70), 179);
    ClearOTagR((u_long*)(D_80129560 + 0x7EB0), 179);
}

/** @brief Run the world-map overlay and return its next game state.
 * @return Next top-level game state.
 */
s32 run_world_map(void)
{
    RECT sp10;
    s32 var_v0;

    sp10 = *(RECT*)&D_80051A80;
    akao_cmd_f0();
    akao_cmd_f1();
    func_80060720();
    M2C_FIELD(&D_80129560, s8*, 0x18) = 0;
    M2C_FIELD(&D_80129560, s8*, 0x7E58) = 0;
    D_800D0550 = func_800623CC();
    akao_cmd_f0();
    akao_cmd_f1();
    if (D_800D0550 == 2)
    {
        DrawSync(0);
        VSync(0);
        SetDispMask(0);
        ResetGraph(0);
        ClearImage(&sp10, 0, 0, 0);
        return 2;
    }
    if (D_80182228 != 0)
    {
        return 9;
    }
    var_v0 = 0xA;
    if (D_80182240 == 0)
    {
        var_v0 = D_800D0550;
    }
    return var_v0;
}

/** @brief Initialize world-map state, resource slots, and packet templates. */
void func_80060918(void)
{
    void* var_v0;
    s32* var_a1;
    s32* var_a2;
    s32 temp_a3;
    s32 temp_a3_2;
    void* temp_a0;
    void* var_v0_2;
    void* var_v0_3;
    void* var_v1;
    void* var_v1_2;
    void* var_v1_3;

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
        M2C_FIELD(var_a2, s32*, 0) = M2C_FIELD(var_a1, s32*, 0);
        M2C_FIELD(var_a2, s32*, 4) = (s32)M2C_FIELD(var_a1, s32*, 4);
        M2C_FIELD(var_a2, s32*, 8) = (s32)M2C_FIELD(var_a1, s32*, 8);
        M2C_FIELD(var_a2, s32*, 0xC) = (s32)M2C_FIELD(var_a1, s32*, 0xC);
        var_a1 = (s32*)((u8*)var_a1 + 0x10);
        var_a2 = (s32*)((u8*)var_a2 + 0x10);
    } while (var_a1 != ((u8*)&D_800D06BC + 0x20));
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
    M2C_FIELD(&D_80129560, void**, 0x33C) = &D_8010CF18;
    M2C_FIELD(&D_80129560, void**, 0x817C) = &D_80114F18;
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
    M2C_FIELD(&D_801ADB04, s8*, 0) = 0x80;
    M2C_FIELD(&D_801ADB04, s8*, 1) = 0x80;
    M2C_FIELD(&D_801ADB04, s8*, 2) = 0x80;
    D_801398F0 = 1;
    D_801ADAF4 = 0;
    D_8013B294 = 0;
    M2C_FIELD(&D_80139950, s32*, 0) = 0;
    M2C_FIELD(&D_80139950, s32*, 4) = 0;
    M2C_FIELD(&D_80139950, s32*, 8) = 0x6000;
    M2C_FIELD(&D_80139278, s16*, 0) = 0x2E0;
    M2C_FIELD(&D_80139278, s16*, 4) = 0x1B0;
    M2C_FIELD(&D_80139278, s16*, 2) = 0;
    M2C_FIELD(&D_80182DC0, s32*, 0) = 8;
    M2C_FIELD(&D_80182DC0, s32*, 4) = -0x18;
    M2C_FIELD(&D_80182DC0, s32*, 8) = 0x6D60;
    D_80139244 = 0;
    D_8011CF70 = 0;
    D_80182D70 = 0;
    D_8011CF58 = 0;
    D_80139960 = 0;
    M2C_FIELD(&D_80182D48, s32*, 0) = (s32)M2C_FIELD(&D_8011CF60, s32*, 0);
    M2C_FIELD(&D_80182D48, s32*, 4) = (s32)M2C_FIELD(&D_8011CF60, s32*, 4);
    M2C_FIELD(&D_80182D48, s32*, 8) = (s32)M2C_FIELD(&D_8011CF60, s32*, 8);
    M2C_FIELD(&D_80182D48, s32*, 0xC) = (s32)M2C_FIELD(&D_8011CF60, s32*, 0xC);
    *(SVECTOR*)&D_801398C8 = *(SVECTOR*)&D_80139258;
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
    M2C_FIELD(&D_8013B260, s16*, 2) = 0;
    M2C_FIELD(&D_80129548, s8*, 0) = 0;
    M2C_FIELD(&D_8013B260, s16*, 0) = 0;
    M2C_FIELD(&D_80129548, s8*, 1) = 0;
    M2C_FIELD(&D_80129548, s8*, 2) = 0;
    SetPolyFT4(&D_800D9170);
    var_v1 = (u8*)&D_800D9170 + 0x78;
    var_v0 = &D_800D9170;
    M2C_FIELD(&D_800D9170, s32*, 0x18) = 0;
    M2C_FIELD(&D_800D9170, s32*, 0x10) = 0;
    M2C_FIELD(&D_800D9170, s32*, 8) = 0;
    do
    {
        M2C_FIELD(var_v1, s32*, 0) = (s32)M2C_FIELD(var_v0, s32*, 0);
        M2C_FIELD(var_v1, s32*, 4) = (s32)M2C_FIELD(var_v0, s32*, 4);
        M2C_FIELD(var_v1, s32*, 8) = (s32)M2C_FIELD(var_v0, s32*, 8);
        M2C_FIELD(var_v1, s32*, 0xC) = (s32)M2C_FIELD(var_v0, s32*, 0xC);
        var_v0 += 0x10;
        var_v1 += 0x10;
    } while (var_v0 != ((u8*)&D_800D9170 + 0x20));
    M2C_FIELD(var_v1, s32*, 0) = (s32)M2C_FIELD(var_v0, s32*, 0);
    M2C_FIELD(var_v1, s32*, 4) = (s32)M2C_FIELD(var_v0, s32*, 4);
    var_v1_2 = (u8*)&D_800D9170 + 0x50;
    var_v0_2 = (u8*)&D_800D9170 + 0x78;
    temp_a0 = (u8*)&D_800D9170 + 0x98;
    do
    {
        M2C_FIELD(var_v1_2, s32*, 0) = (s32)M2C_FIELD(var_v0_2, s32*, 0);
        M2C_FIELD(var_v1_2, s32*, 4) = (s32)M2C_FIELD(var_v0_2, s32*, 4);
        M2C_FIELD(var_v1_2, s32*, 8) = (s32)M2C_FIELD(var_v0_2, s32*, 8);
        M2C_FIELD(var_v1_2, s32*, 0xC) = (s32)M2C_FIELD(var_v0_2, s32*, 0xC);
        var_v0_2 += 0x10;
        var_v1_2 += 0x10;
    } while (var_v0_2 != temp_a0);
    M2C_FIELD(var_v1_2, s32*, 0) = (s32)M2C_FIELD(var_v0_2, s32*, 0);
    M2C_FIELD(var_v1_2, s32*, 4) = (s32)M2C_FIELD(var_v0_2, s32*, 4);
    var_v1_3 = (u8*)&D_800D9170 + 0x28;
    var_v0_3 = (u8*)&D_800D9170 + 0x50;
    do
    {
        M2C_FIELD(var_v1_3, s32*, 0) = (s32)M2C_FIELD(var_v0_3, s32*, 0);
        M2C_FIELD(var_v1_3, s32*, 4) = (s32)M2C_FIELD(var_v0_3, s32*, 4);
        M2C_FIELD(var_v1_3, s32*, 8) = (s32)M2C_FIELD(var_v0_3, s32*, 8);
        M2C_FIELD(var_v1_3, s32*, 0xC) = (s32)M2C_FIELD(var_v0_3, s32*, 0xC);
        var_v0_3 += 0x10;
        var_v1_3 += 0x10;
    } while (var_v0_3 != ((u8*)&D_800D9170 + 0x70));
    temp_a3_2 = M2C_FIELD(var_v0_3, s32*, 0);
    M2C_FIELD(var_v1_3, s32*, 0) = temp_a3_2;
    M2C_FIELD(var_v1_3, s32*, 4) = (s32)M2C_FIELD(var_v0_3, s32*, 4);
    D_8013B274 = func_8005D494();
    func_80063F38();
    func_8005B548();
}

/** @brief Update the world-map display transition and append its packets.
 * @param initialize Sequence event selector, unused by this callback.
 * @return Callback status; interpretation remains under study.
 */
s32 func_80060F04(s32 initialize)
{
    s8 sp3;
    s8 sp2;
    s8 sp1;
    s8 sp0;
    void* var_a2;
    void* var_a2_2;
    void* var_v1_2;
    void* var_v1_3;
    void* var_v1_4;
    void* var_v1_5;
    s16 temp_a0_11;
    s16 temp_a0_2;
    s16 temp_a0_5;
    s16 temp_a0_8;
    s16 temp_v0;
    s16 temp_v0_2;
    s16 temp_v0_3;
    s16 temp_v0_4;
    s32* temp_a0_12;
    s32* temp_a0_3;
    s32* temp_a0_6;
    s32* temp_a0_9;
    s32* temp_a1;
    s32* temp_a1_2;
    s32* temp_a1_3;
    s32* temp_a1_4;
    s32* temp_a1_5;
    s32* var_a1_2;
    s32* var_v0_3;
    s32* var_v0_4;
    s32* var_v0_5;
    s32* var_v0_6;
    s32* var_v1_6;
    s32 temp_a0;
    s32 temp_a0_10;
    s32 temp_a0_4;
    s32 temp_a0_7;
    s32 var_a0;
    s32 var_a1;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    u32 temp_v0_10;
    u32 temp_v0_11;
    u32 temp_v0_12;
    u32 temp_v0_13;
    u32 temp_v0_14;
    u32 temp_v0_15;
    u32 temp_v0_16;
    u32 temp_v0_5;
    u32 temp_v0_6;
    u32 temp_v0_7;
    u32 temp_v0_8;
    u32 temp_v0_9;
    u8 var_v0_10;
    u8 var_v0_11;
    u8 var_v0_12;
    u8 var_v0_13;
    u8 var_v0_14;
    u8 var_v0_15;
    u8 var_v0_16;
    u8 var_v0_17;
    u8 var_v0_18;
    u8 var_v0_7;
    u8 var_v0_8;
    u8 var_v0_9;

    if (D_801398F0 != D_801ADAF4)
    {
        var_v0 = D_801398F0 - 1;
        if (D_801ADAF4 >= D_801398F0)
        {
            var_v0 = D_801398F0 + 1;
        }
        D_801398F0 = var_v0;
    }
    if (D_801398F0 != 0)
    {
        if (D_801398D4 != 0)
        {
            var_a0 = D_801398F0 * M2C_FIELD(&D_801ADB04, u8*, 0);
            if (var_a0 < 0)
            {
                var_a0 += 0xF;
            }
            var_a1 = D_801398F0 * M2C_FIELD(&D_801ADB04, u8*, 1);
            sp0 = (s8)(var_a0 >> 4);
            if (var_a1 < 0)
            {
                var_a1 += 0xF;
            }
            var_v1 = D_801398F0 * M2C_FIELD(&D_801ADB04, u8*, 2);
            sp1 = (s8)(var_a1 >> 4);
            if (var_v1 < 0)
            {
                var_v1 += 0xF;
            }
            var_v0_2 = var_v1 >> 4;
        }
        else
        {
            sp0 = (D_801398F0 * M2C_FIELD(&D_801ADB04, u8*, 0)) / 24;
            sp1 = (D_801398F0 * M2C_FIELD(&D_801ADB04, u8*, 1)) / 24;
            var_v0_2 = (D_801398F0 * M2C_FIELD(&D_801ADB04, u8*, 2)) / 24;
        }
        sp2 = (s8)var_v0_2;
        sp3 = 0x2C;
        if ((D_8011CF44 == 0) && (D_801398D4 != 0))
        {
            var_a2 = &D_800D0554;
            do
            {
                var_v1_2 = var_a2;
                temp_a1 = M2C_FIELD(D_801398EC, s32**, 0x33C);
                var_v0_3 = temp_a1;
            loop_18:
                M2C_FIELD(var_v0_3, s32*, 0) = M2C_FIELD(var_v1_2, s32*, 0);
                M2C_FIELD(var_v0_3, s32*, 4) = (s32)M2C_FIELD(var_v1_2, s32*, 4);
                M2C_FIELD(var_v0_3, s32*, 8) = (s32)M2C_FIELD(var_v1_2, s32*, 8);
                M2C_FIELD(var_v0_3, s32*, 0xC) = (s32)M2C_FIELD(var_v1_2, s32*, 0xC);
                var_v1_2 += 0x10;
                var_v0_3 = (s32*)((u8*)var_v0_3 + 0x10);
                if (var_v1_2 != (var_a2 + 0x20))
                {
                    goto loop_18;
                }
                M2C_FIELD(var_v0_3, s32*, 0) = M2C_FIELD(var_v1_2, s32*, 0);
                M2C_FIELD(var_v0_3, s32*, 4) = (s32)M2C_FIELD(var_v1_2, s32*, 4);
                temp_a0 = (M2C_FIELD(temp_a1, s16*, 8) + 0x140 + D_800D06E0) % 640;
                temp_v0 = temp_a0 - 0x140;
                temp_a0_2 = temp_a0 - 0xA0;
                M2C_FIELD(temp_a1, s16*, 0x18) = temp_v0;
                M2C_FIELD(temp_a1, s16*, 8) = temp_v0;
                M2C_FIELD(temp_a1, s16*, 0x20) = temp_a0_2;
                M2C_FIELD(temp_a1, s16*, 0x10) = temp_a0_2;
                M2C_FIELD(temp_a1, s32*, 4) = (s32)sp0;
                M2C_FIELD(temp_a1, u8*, 7) = (u8)(M2C_FIELD(temp_a1, u8*, 7) | 2);
                temp_a0_3 = M2C_FIELD(D_801398EC, s32**, 0x33C);
                *temp_a0_3 = (*temp_a0_3 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFFFFFF);
                M2C_FIELD(D_801398EC, s32*, 0x334) =
                    (s32)((M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFF000000) | ((s32)M2C_FIELD(D_801398EC, s32**, 0x33C) & 0xFFFFFF));
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += 0x28;
                    M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x28);
                }
                var_v1_3 = var_a2;
                temp_a1_2 = M2C_FIELD(D_801398EC, s32**, 0x33C);
                var_v0_4 = temp_a1_2;
            loop_22:
                M2C_FIELD(var_v0_4, s32*, 0) = M2C_FIELD(var_v1_3, s32*, 0);
                M2C_FIELD(var_v0_4, s32*, 4) = (s32)M2C_FIELD(var_v1_3, s32*, 4);
                M2C_FIELD(var_v0_4, s32*, 8) = (s32)M2C_FIELD(var_v1_3, s32*, 8);
                M2C_FIELD(var_v0_4, s32*, 0xC) = (s32)M2C_FIELD(var_v1_3, s32*, 0xC);
                var_v1_3 += 0x10;
                var_v0_4 = (s32*)((u8*)var_v0_4 + 0x10);
                if (var_v1_3 != (var_a2 + 0x20))
                {
                    goto loop_22;
                }
                M2C_FIELD(var_v0_4, s32*, 0) = M2C_FIELD(var_v1_3, s32*, 0);
                M2C_FIELD(var_v0_4, s32*, 4) = (s32)M2C_FIELD(var_v1_3, s32*, 4);
                temp_a0_4 = (M2C_FIELD(temp_a1_2, s16*, 8) + D_800D06E0) % 640;
                temp_v0_2 = temp_a0_4 - 0x140;
                temp_a0_5 = temp_a0_4 - 0xA0;
                M2C_FIELD(temp_a1_2, s16*, 0x18) = temp_v0_2;
                M2C_FIELD(temp_a1_2, s16*, 8) = temp_v0_2;
                M2C_FIELD(temp_a1_2, s16*, 0x20) = temp_a0_5;
                M2C_FIELD(temp_a1_2, s16*, 0x10) = temp_a0_5;
                M2C_FIELD(temp_a1_2, s32*, 4) = (s32)sp0;
                M2C_FIELD(temp_a1_2, u8*, 7) = (u8)(M2C_FIELD(temp_a1_2, u8*, 7) | 2);
                temp_a0_6 = M2C_FIELD(D_801398EC, s32**, 0x33C);
                *temp_a0_6 = (*temp_a0_6 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFFFFFF);
                M2C_FIELD(D_801398EC, s32*, 0x334) =
                    (s32)((M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFF000000) | ((s32)M2C_FIELD(D_801398EC, s32**, 0x33C) & 0xFFFFFF));
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += 0x28;
                    M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x28);
                }
                var_a2 += 0x28;
            } while ((s32)var_a2 < (s32)((u8*)&D_800D0554 + 0xA0));
            if (!(D_8011CF74 & 3))
            {
                D_800D06E0 = (D_800D06E0 + 1) & 0x7FFF;
            }
        }
        var_a2_2 = &D_800D05F4;
        do
        {
            var_v1_4 = var_a2_2;
            temp_a1_3 = M2C_FIELD(D_801398EC, s32**, 0x33C);
            var_v0_5 = temp_a1_3;
        loop_31:
            M2C_FIELD(var_v0_5, s32*, 0) = M2C_FIELD(var_v1_4, s32*, 0);
            M2C_FIELD(var_v0_5, s32*, 4) = (s32)M2C_FIELD(var_v1_4, s32*, 4);
            M2C_FIELD(var_v0_5, s32*, 8) = (s32)M2C_FIELD(var_v1_4, s32*, 8);
            M2C_FIELD(var_v0_5, s32*, 0xC) = (s32)M2C_FIELD(var_v1_4, s32*, 0xC);
            var_v1_4 += 0x10;
            var_v0_5 = (s32*)((u8*)var_v0_5 + 0x10);
            if (var_v1_4 != (var_a2_2 + 0x20))
            {
                goto loop_31;
            }
            M2C_FIELD(var_v0_5, s32*, 0) = M2C_FIELD(var_v1_4, s32*, 0);
            M2C_FIELD(var_v0_5, s32*, 4) = (s32)M2C_FIELD(var_v1_4, s32*, 4);
            temp_a0_7 = ((M2C_FIELD(temp_a1_3, s16*, 8) + 0x8140) - D_800D06E0) % 640;
            temp_v0_3 = temp_a0_7 - 0x140;
            temp_a0_8 = temp_a0_7 - 0xA0;
            M2C_FIELD(temp_a1_3, s16*, 0x18) = temp_v0_3;
            M2C_FIELD(temp_a1_3, s16*, 8) = temp_v0_3;
            M2C_FIELD(temp_a1_3, s16*, 0x20) = temp_a0_8;
            M2C_FIELD(temp_a1_3, s16*, 0x10) = temp_a0_8;
            M2C_FIELD(temp_a1_3, s32*, 4) = (s32)sp0;
            temp_a0_9 = M2C_FIELD(D_801398EC, s32**, 0x33C);
            *temp_a0_9 = (*temp_a0_9 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFFFFFF);
            M2C_FIELD(D_801398EC, s32*, 0x334) =
                (s32)((M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFF000000) | ((s32)M2C_FIELD(D_801398EC, s32**, 0x33C) & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x28;
                M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x28);
            }
            var_v1_5 = var_a2_2;
            temp_a1_4 = M2C_FIELD(D_801398EC, s32**, 0x33C);
            var_v0_6 = temp_a1_4;
        loop_35:
            M2C_FIELD(var_v0_6, s32*, 0) = M2C_FIELD(var_v1_5, s32*, 0);
            M2C_FIELD(var_v0_6, s32*, 4) = (s32)M2C_FIELD(var_v1_5, s32*, 4);
            M2C_FIELD(var_v0_6, s32*, 8) = (s32)M2C_FIELD(var_v1_5, s32*, 8);
            M2C_FIELD(var_v0_6, s32*, 0xC) = (s32)M2C_FIELD(var_v1_5, s32*, 0xC);
            var_v1_5 += 0x10;
            var_v0_6 = (s32*)((u8*)var_v0_6 + 0x10);
            if (var_v1_5 != (var_a2_2 + 0x20))
            {
                goto loop_35;
            }
            M2C_FIELD(var_v0_6, s32*, 0) = M2C_FIELD(var_v1_5, s32*, 0);
            M2C_FIELD(var_v0_6, s32*, 4) = (s32)M2C_FIELD(var_v1_5, s32*, 4);
            temp_a0_10 = ((M2C_FIELD(temp_a1_4, s16*, 8) + 0x8000) - D_800D06E0) % 640;
            temp_v0_4 = temp_a0_10 - 0x140;
            temp_a0_11 = temp_a0_10 - 0xA0;
            M2C_FIELD(temp_a1_4, s16*, 0x18) = temp_v0_4;
            M2C_FIELD(temp_a1_4, s16*, 8) = temp_v0_4;
            M2C_FIELD(temp_a1_4, s16*, 0x20) = temp_a0_11;
            M2C_FIELD(temp_a1_4, s16*, 0x10) = temp_a0_11;
            M2C_FIELD(temp_a1_4, s32*, 4) = (s32)sp0;
            temp_a0_12 = M2C_FIELD(D_801398EC, s32**, 0x33C);
            *temp_a0_12 = (*temp_a0_12 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFFFFFF);
            M2C_FIELD(D_801398EC, s32*, 0x334) =
                (s32)((M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFF000000) | ((s32)M2C_FIELD(D_801398EC, s32**, 0x33C) & 0xFFFFFF));
            if (D_800D921C < 0x7D00)
            {
                D_800D921C += 0x28;
                M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x28);
            }
            var_a2_2 += 0x28;
        } while ((s32)var_a2_2 < (s32)((u8*)&D_800D05F4 + 0xA0));
        return 1;
    }
    temp_v0_5 = M2C_FIELD(&D_800D9240, u8*, 4) & 0xFF;
    if (D_80182D74 != temp_v0_5)
    {
        var_v0_7 = M2C_FIELD(&D_800D9240, u8*, 4) + 8;
        if (temp_v0_5 >= (u8)D_80182D74)
        {
            var_v0_7 = M2C_FIELD(&D_800D9240, u8*, 4) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 4) = var_v0_7;
    }
    temp_v0_6 = M2C_FIELD(&D_800D9240, u8*, 5) & 0xFF;
    if (D_80182D75 != temp_v0_6)
    {
        var_v0_8 = M2C_FIELD(&D_800D9240, u8*, 5) + 8;
        if (temp_v0_6 >= (u8)D_80182D75)
        {
            var_v0_8 = M2C_FIELD(&D_800D9240, u8*, 5) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 5) = var_v0_8;
    }
    temp_v0_7 = M2C_FIELD(&D_800D9240, u8*, 6) & 0xFF;
    if (D_80182D76 != temp_v0_7)
    {
        var_v0_9 = M2C_FIELD(&D_800D9240, u8*, 6) + 8;
        if (temp_v0_7 >= (u8)D_80182D76)
        {
            var_v0_9 = M2C_FIELD(&D_800D9240, u8*, 6) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 6) = var_v0_9;
    }
    temp_v0_8 = M2C_FIELD(&D_800D9240, u8*, 0xC) & 0xFF;
    if (D_80182D80 != temp_v0_8)
    {
        var_v0_10 = M2C_FIELD(&D_800D9240, u8*, 0xC) + 8;
        if (temp_v0_8 >= (u8)D_80182D80)
        {
            var_v0_10 = M2C_FIELD(&D_800D9240, u8*, 0xC) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0xC) = var_v0_10;
    }
    temp_v0_9 = M2C_FIELD(&D_800D9240, u8*, 0xD) & 0xFF;
    if (D_80182D81 != temp_v0_9)
    {
        var_v0_11 = M2C_FIELD(&D_800D9240, u8*, 0xD) + 8;
        if (temp_v0_9 >= (u8)D_80182D81)
        {
            var_v0_11 = M2C_FIELD(&D_800D9240, u8*, 0xD) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0xD) = var_v0_11;
    }
    temp_v0_10 = M2C_FIELD(&D_800D9240, u8*, 0xE) & 0xFF;
    if (D_80182D82 != temp_v0_10)
    {
        var_v0_12 = M2C_FIELD(&D_800D9240, u8*, 0xE) + 8;
        if (temp_v0_10 >= (u8)D_80182D82)
        {
            var_v0_12 = M2C_FIELD(&D_800D9240, u8*, 0xE) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0xE) = var_v0_12;
    }
    temp_v0_11 = M2C_FIELD(&D_800D9240, u8*, 0x14) & 0xFF;
    if (D_80182D8C != temp_v0_11)
    {
        var_v0_13 = M2C_FIELD(&D_800D9240, u8*, 0x14) + 8;
        if (temp_v0_11 >= (u8)D_80182D8C)
        {
            var_v0_13 = M2C_FIELD(&D_800D9240, u8*, 0x14) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0x14) = var_v0_13;
    }
    temp_v0_12 = M2C_FIELD(&D_800D9240, u8*, 0x15) & 0xFF;
    if (D_80182D8D != temp_v0_12)
    {
        var_v0_14 = M2C_FIELD(&D_800D9240, u8*, 0x15) + 8;
        if (temp_v0_12 >= (u8)D_80182D8D)
        {
            var_v0_14 = M2C_FIELD(&D_800D9240, u8*, 0x15) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0x15) = var_v0_14;
    }
    temp_v0_13 = M2C_FIELD(&D_800D9240, u8*, 0x16) & 0xFF;
    if (D_80182D8E != temp_v0_13)
    {
        var_v0_15 = M2C_FIELD(&D_800D9240, u8*, 0x16) + 8;
        if (temp_v0_13 >= (u8)D_80182D8E)
        {
            var_v0_15 = M2C_FIELD(&D_800D9240, u8*, 0x16) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0x16) = var_v0_15;
    }
    temp_v0_14 = M2C_FIELD(&D_800D9240, u8*, 0x1C) & 0xFF;
    if (D_80182D94 != temp_v0_14)
    {
        var_v0_16 = M2C_FIELD(&D_800D9240, u8*, 0x1C) + 8;
        if (temp_v0_14 >= (u8)D_80182D94)
        {
            var_v0_16 = M2C_FIELD(&D_800D9240, u8*, 0x1C) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0x1C) = var_v0_16;
    }
    temp_v0_15 = M2C_FIELD(&D_800D9240, u8*, 0x1D) & 0xFF;
    if (D_80182D95 != temp_v0_15)
    {
        var_v0_17 = M2C_FIELD(&D_800D9240, u8*, 0x1D) + 8;
        if (temp_v0_15 >= (u8)D_80182D95)
        {
            var_v0_17 = M2C_FIELD(&D_800D9240, u8*, 0x1D) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0x1D) = var_v0_17;
    }
    temp_v0_16 = M2C_FIELD(&D_800D9240, u8*, 0x1E) & 0xFF;
    if (D_80182D96 != temp_v0_16)
    {
        var_v0_18 = M2C_FIELD(&D_800D9240, u8*, 0x1E) + 8;
        if (temp_v0_16 >= (u8)D_80182D96)
        {
            var_v0_18 = M2C_FIELD(&D_800D9240, u8*, 0x1E) - 8;
        }
        M2C_FIELD(&D_800D9240, u8*, 0x1E) = var_v0_18;
    }
    var_a1_2 = &D_800D9240;
    var_v1_6 = M2C_FIELD(D_801398EC, s32**, 0x33C);
    do
    {
        M2C_FIELD(var_v1_6, s32*, 0) = M2C_FIELD(var_a1_2, s32*, 0);
        M2C_FIELD(var_v1_6, s32*, 4) = (s32)M2C_FIELD(var_a1_2, s32*, 4);
        M2C_FIELD(var_v1_6, s32*, 8) = (s32)M2C_FIELD(var_a1_2, s32*, 8);
        M2C_FIELD(var_v1_6, s32*, 0xC) = (s32)M2C_FIELD(var_a1_2, s32*, 0xC);
        var_a1_2 = (s32*)((u8*)var_a1_2 + 0x10);
        var_v1_6 = (s32*)((u8*)var_v1_6 + 0x10);
    } while (var_a1_2 != ((u8*)&D_800D9240 + 0x20));
    *var_v1_6 = *var_a1_2;
    temp_a1_5 = M2C_FIELD(D_801398EC, s32**, 0x33C);
    *temp_a1_5 = (*temp_a1_5 & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFFFFFF);
    M2C_FIELD(D_801398EC, s32*, 0x334) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x334) & 0xFF000000) | ((s32)M2C_FIELD(D_801398EC, s32**, 0x33C) & 0xFFFFFF));
    if (D_800D921C < 0x7D00)
    {
        D_800D921C += 0x24;
        M2C_FIELD(D_801398EC, s32**, 0x33C) = (s32*)(M2C_FIELD(D_801398EC, s32**, 0x33C) + 0x24);
    }
    return 1;
}

/** @brief Update the overlay fade and append its polygon to the ordering table.
 * @return Zero while disabled, otherwise one.
 * @param initialize Sequence event selector, unused by this callback.
 */
s32 func_80061878(s32 initialize)
{
    POLY_FT4* packet;
    s32* tag_packet;
    u8 intensity;

    switch (D_8013B254)
    {
    case 2:
        intensity = D_800D0694.b0 - 4;
        D_800D0694.b0 = intensity;
        D_800D0694.g0 = intensity;
        D_800D0694.r0 = intensity;
        if (intensity == 0)
        {
            D_8013B254 = 0;
        }
        else
        {
            goto draw;
        }
        break;
    case 1:
        intensity = D_800D0694.b0 + 4;
        D_800D0694.b0 = intensity;
        D_800D0694.g0 = intensity;
        D_800D0694.r0 = intensity;
        if (intensity == 128)
        {
            D_8013B254 = 0;
        }
        break;
    case 3:
        return 0;
    }
    if (D_800D0698 != 0)
    {
    draw:
        packet = (POLY_FT4*)D_801398EC->prim_cursor;
        *packet = D_800D0694;
        if (D_800D0698 != 128)
        {
            packet->code |= 2;
        }
        tag_packet = (s32*)D_801398EC->prim_cursor;
        *tag_packet = (*tag_packet & 0xFF000000) | (D_801398EC->ot[176] & 0xFFFFFF);
        D_801398EC->ot[176] = (D_801398EC->ot[176] & 0xFF000000) | ((u32)D_801398EC->prim_cursor & 0xFFFFFF);
        if (D_800D921C < 32000)
        {
            D_800D921C += 40;
            D_801398EC->prim_cursor += sizeof(POLY_FT4);
        }
    }
    return 1;
}

/** @brief Process the map input state and append its display packets.
 * @param value Caller context, unused by this routine.
 */
void func_80061A2C(s32 value)
{
    RECT sp10;
    void* var_a3;
    s32 temp_a0;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v0_3;
    s32 var_a0;
    s32 var_t1;
    void* temp_a0_2;
    void* temp_a0_3;

    if (D_8013922C & 0x100)
    {
        if (D_801398FC == 0)
        {
            temp_v0 = D_801398D4 == 0;
            D_801398D4 = temp_v0;
            if (temp_v0 != 0)
            {
                var_a0 = D_800CB224;
            }
            else
            {
                var_a0 = D_800CB23C;
            }
        }
        else
        {
            var_a0 = D_800CB224;
            D_801398FC = 0;
        }
        akao_play_sfx_from_buffer(var_a0, 0, 0x80, 0x7F);
        D_801398C0 = 0;
        D_8013922C = 0;
    }
    if (D_801398D4 == 0)
    {
        temp_a0 = D_800D9210 - 1;
        temp_v0_2 = temp_a0 * 0x10;
        if (temp_v0_2 != D_801398C4)
        {
            if (temp_v0_2 < D_801398C4)
            {
                D_801398C4 -= 4;
            }
            else
            {
                D_801398C4 += 4;
            }
            goto block_30;
        }
        if (D_801398FC == 0)
        {
            if (D_8013922C & 0x1000)
            {
                D_800D9210 = temp_a0;
                if (temp_a0 <= 0)
                {
                    D_800D9210 = 6;
                }
                akao_play_sfx_from_buffer(D_800CB204, 0, 0x80, 0x7F);
            }
            if (D_8013922C & 0x4000)
            {
                temp_v0_3 = D_800D9210 + 1;
                D_800D9210 = temp_v0_3;
                if (temp_v0_3 >= 7)
                {
                    D_800D9210 = 1;
                }
                akao_play_sfx_from_buffer(D_800CB204, 0, 0x80, 0x7F);
            }
        }
        if (D_8013922C & 0x40)
        {
            if (D_801398FC == 0)
            {
                D_801398FC = D_800D9210;
                akao_play_sfx_from_buffer(D_800CB254, 0, 0x80, 0x7F);
            }
        }
        if (D_8013922C & 0x20)
        {
            if (D_801398FC == 0)
            {
                D_801398D4 = D_801398D4 == 0;
                akao_play_sfx_from_buffer(D_800CB224, 0, 0x80, 0x7F);
                D_801398C0 = 0;
                D_8013922C = 0;
                return;
            }
            D_801398FC = 0;
            akao_play_sfx_from_buffer(D_800CB224, 0, 0x80, 0x7F);
            goto block_30;
        }
    block_30:
        if (D_800DCEA0 != D_801398FC)
        {
            D_800DCEA0 = D_801398FC;
            cdrom_queue_read(((u16)D_801398FC + 0x114B) & 0xFFFF, &D_8019D6E0);
            cdrom_wait_queue_empty();
            sp10 = *(RECT*)((u8*)&D_8019D6E0 + 12);
            LoadImage(&sp10, (u8*)&D_8019D6E0 + 0x14);
            sp10 = *(RECT*)((u8*)&D_8019D6E0 + 12 + M2C_FIELD(&D_8019D6E0, s32*, 8));
            LoadImage(&sp10, M2C_FIELD(&D_8019D6E0, s32*, 8) + ((u8*)&D_8019D6E0 + 8) + 0xC);
            DrawSync(0);
            D_801ADAFC = 1;
        }
        if (D_801398FC == 0)
        {
            var_t1 = 0;
            var_a3 = &D_800D06F8;
            do
            {
                temp_a0_2 = M2C_FIELD(D_801398EC, void**, 0x33C);
                M2C_FIELD(temp_a0_2, s32*, 0) = (s32)M2C_FIELD(var_a3, s32*, 0);
                M2C_FIELD(temp_a0_2, s32*, 4) = (s32)M2C_FIELD(var_a3, s32*, 4);
                M2C_FIELD(temp_a0_2, s32*, 8) = (s32)M2C_FIELD(var_a3, s32*, 8);
                M2C_FIELD(temp_a0_2, s32*, 0xC) = (s32)M2C_FIELD(var_a3, s32*, 0xC);
                M2C_FIELD(temp_a0_2, s32*, 0x10) = (s32)M2C_FIELD(var_a3, s32*, 0x10);
                M2C_FIELD(temp_a0_2, s32*, 0x14) = (s32)M2C_FIELD(var_a3, s32*, 0x14);
                M2C_FIELD(temp_a0_2, s32*, 0x18) = (s32)M2C_FIELD(var_a3, s32*, 0x18);
                M2C_FIELD(temp_a0_2, u16*, 0xA) = (u16)(M2C_FIELD(temp_a0_2, u16*, 0xA) + (u16)D_801398C4);
                M2C_FIELD(temp_a0_2, u16*, 0x1A) = (u16)(M2C_FIELD(temp_a0_2, u16*, 0x1A) + (u16)D_801398C4);
                M2C_FIELD(temp_a0_2, u16*, 0x12) = (u16)(M2C_FIELD(temp_a0_2, u16*, 0x12) + (u16)D_801398C4);
                M2C_FIELD(temp_a0_2, s32*, 0) = (s32)((M2C_FIELD(temp_a0_2, s32*, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x74) & 0xFFFFFF));
                M2C_FIELD(D_801398EC, s32*, 0x74) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x74) & 0xFF000000) | ((s32)temp_a0_2 & 0xFFFFFF));
                if (D_800D921C < 0x7D00)
                {
                    D_800D921C += 0x1C;
                    M2C_FIELD(D_801398EC, void**, 0x33C) = (void*)(M2C_FIELD(D_801398EC, void**, 0x33C) + 0x1C);
                }
                var_t1 += 1;
                var_a3 += 0x1C;
            } while (var_t1 < 0x1C);
        }
        func_8006534C(0xB5, 1);
        temp_a0_3 = M2C_FIELD(D_801398EC, void**, 0x33C);
        M2C_FIELD(temp_a0_3, s32*, 0) = (s32)M2C_FIELD(&D_800D06E4, s32*, 0);
        M2C_FIELD(temp_a0_3, s32*, 4) = (s32)M2C_FIELD(&D_800D06E4, s32*, 4);
        M2C_FIELD(temp_a0_3, s32*, 8) = (s32)M2C_FIELD(&D_800D06E4, s32*, 8);
        M2C_FIELD(temp_a0_3, s32*, 0xC) = (s32)M2C_FIELD(&D_800D06E4, s32*, 0xC);
        M2C_FIELD(temp_a0_3, s32*, 0x10) = (s32)M2C_FIELD(&D_800D06E4, s32*, 0x10);
        M2C_FIELD(temp_a0_3, s32*, 0) = (s32)((M2C_FIELD(temp_a0_3, s32*, 0) & 0xFF000000) | (M2C_FIELD(D_801398EC, s32*, 0x74) & 0xFFFFFF));
        M2C_FIELD(D_801398EC, s32*, 0x74) = (s32)((M2C_FIELD(D_801398EC, s32*, 0x74) & 0xFF000000) | ((s32)temp_a0_3 & 0xFFFFFF));
        if (D_800D921C < 0x7D00)
        {
            D_800D921C += 0x14;
            M2C_FIELD(D_801398EC, void**, 0x33C) = (void*)(M2C_FIELD(D_801398EC, void**, 0x33C) + 0x14);
        }
        func_8006534C(0xD5, 1);
        D_801398C0 = 0;
        D_8013922C = 0;
    }
}

/** @brief Clear completed wait states or continue the pending world-map action.
 * @param state Caller context, unused by this routine.
 */
void func_80061F18(s32* state)
{
    switch (D_80182DB4)
    {
    case 3:
        if (D_8011D4FC != -1)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    case 4:
        if (D_80182DE0 != 0)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    case 5:
        if (D_8011CF44 == 0)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    case 6:
        if (D_80139230 != 0)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    case 7:
        if (D_8011CF18 == 3)
        {
            D_80182DB4 = 0;
        }
        else
        {
            func_80063BE0();
        }
        break;
    }
}

/** @brief Resolve a pending map selection and queue its resource read.
 * @param state Caller context, unused by this routine.
 */
void func_80061FF8(s32* state)
{
    RECT sp10;
    s16 temp_v1;
    s16 temp_v1_2;
    s16 temp_v1_4;
    s16 temp_v1_5;
    s16 var_a1;
    s16 var_a2;
    s16 var_a3;
    s32 temp_a0;
    s32 temp_a0_2;
    s32 temp_v0;
    s32 temp_v0_2;
    s32 temp_v1_3;
    s32 var_t1;
    s32 var_v0;
    s32 var_v0_2;
    s32 var_v1;
    void* temp_s1;
    void* temp_s1_2;

    temp_s1 = *(((D_8011D0DC - 1) * 4) + (u8*)&D_800D0544) + (D_801398A8 * 2);
    temp_v1 = M2C_FIELD(temp_s1, s16*, 0);
    temp_s1_2 = temp_s1 + 2;
    D_801398B4 = (s32)temp_v1;
    if (temp_v1 == -1)
    {
        D_8011CF50 = 0;
        goto block_39;
    }
    if (temp_v1 == -2)
    {
        temp_v1_2 = M2C_FIELD(temp_s1, s16*, 2);
        switch (temp_v1_2)
        {
        case 0:
            if (M2C_FIELD(temp_s1_2, s16*, 2) > 0)
            {
                cdrom_queue_read((u16)M2C_FIELD(temp_s1_2, s16*, 2), &D_8019D6E0);
                cdrom_wait_queue_empty();
                sp10 = *(RECT*)((u8*)&D_8019D6E0 + 12);
                LoadImage(&sp10, (u8*)&D_8019D6E0 + 0x14);
                sp10 = *(RECT*)((u8*)&D_8019D6E0 + 12 + M2C_FIELD(&D_8019D6E0, s32*, 8));
                LoadImage(&sp10, M2C_FIELD(&D_8019D6E0, s32*, 8) + ((u8*)&D_8019D6E0 + 8) + 0xC);
                DrawSync(0);
                D_801ADAFC = 1;
                D_800D916C = 1;
            }
            else
            {
                D_800D916C = 0;
            }
            var_v0 = D_801398A8 + 3;
        block_27:
            D_801398A8 = var_v0;
            break;
        case 1:
            D_80139868 = (s32)M2C_FIELD(temp_s1_2, s16*, 2);
            D_801398A8 += 3;
            break;
        case 2:
            if (M2C_FIELD(temp_s1_2, s16*, 2) != 0)
            {
                D_8013B258 = 0;
            }
            else
            {
                D_8013B258 = 1;
            }
            var_v0 = D_801398A8 + 3;
            goto block_27;
        case 3:
            var_v1 = 3;
        block_19:
            D_80182DB4 = var_v1;
            D_801398A8 += 2;
            break;
        case 4:
            var_v1 = 4;
            goto block_19;
        case 5:
            var_v1 = 5;
            goto block_19;
        case 6:
            var_v1 = 6;
            goto block_19;
        case 7:
            var_v1 = 7;
            goto block_19;
        case 8:
            var_a1 = 0;
            var_a2 = 0;
            do
            {
                var_a3 = 0;
                var_t1 = 0;
            loop_22:
                temp_v0 = var_a2 << 0x10;
                if (*((var_a1 * 0x28) + var_t1 + (u8*)&D_80139290) == 0)
                {
                    M2C_FIELD(&D_8019D248, s16*, 0x10) = var_a3;
                    M2C_FIELD(&D_8019D248, s16*, 0xC) = var_a3;
                    var_a3 = 0x120;
                    var_t1 = 0x5A0;
                    M2C_FIELD(&D_8019D248, s16*, 8) = var_a2;
                    var_a2 = 6;
                    temp_v1_3 = var_a1 * 3;
                    temp_a0 = var_a1 << 0x10;
                    M2C_FIELD(&D_8019D248, s16*, 0xA) = var_a1;
                    var_a1 = 6;
                    temp_v1_4 = temp_v1_3 * 0x10;
                    temp_v0_2 = temp_v0 >> 0x10;
                    temp_a0_2 = temp_a0 >> 0x10;
                    M2C_FIELD(&D_8019D248, s16*, 0x12) = temp_v1_4;
                    M2C_FIELD(&D_8019D248, s16*, 0xE) = temp_v1_4;
                    M2C_FIELD(&D_8019D248, s32*, 0x18) = temp_v0_2;
                    M2C_FIELD(&D_8019D248, s32*, 0) = temp_v0_2;
                    M2C_FIELD(&D_8019D248, s32*, 0x1C) = temp_a0_2;
                    M2C_FIELD(&D_8019D248, s32*, 4) = temp_a0_2;
                    M2C_FIELD(&D_8019D248, s32*, 0x14) = 0;
                }
                var_a3 += 0x30;
                var_a2 += 1;
                var_t1 += 0xF0;
                if (var_a2 < 6)
                {
                    goto loop_22;
                }
                var_a1 += 1;
                var_a2 = 0;
            } while (var_a1 < 6);
            var_v0 = D_801398A8 + 2;
            goto block_27;
        }
        D_801398B4 = 1;
        return;
    }
    if (temp_v1 != 0)
    {
        temp_v1_5 = M2C_FIELD(temp_s1, s16*, 2);
        D_801398A8 += 2;
        D_801398C0 = (s32)temp_v1_5;
        D_8013922C = (s32)temp_v1_5;
        return;
    }
    if (M2C_FIELD(temp_s1, s16*, 2) == 0)
    {
        var_v0_2 = func_80065428(D_801398A8, &D_8011D0DC);
        goto block_35;
    }
    if ((u16)M2C_FIELD(temp_s1, s16*, 2) & 0x800)
    {
        var_v0_2 = func_80065428(D_801398A8, &D_8011D0DC) & (s16)((u16)M2C_FIELD(temp_s1, s16*, 2) & 0xF7FF);
    block_35:
        if (var_v0_2 != 0)
        {
            D_801398A8 += 2;
        }
        D_801398B4 = 1;
        D_801398C0 = 0;
        D_8013922C = 0;
        return;
    }
block_39:
    D_8011D0DC = 0;
    D_801398B4 = 1;
}

/** @brief Rectangle template and trailing zero used during display transfers. */
const s16 D_80051ACC[6] = {320, 0, 320, 240, 0, 0};

/**
 * @brief Run the world-map resource and rendering loop.
 * @return World-map loop status; value meanings are not yet fully identified.
 * @note Project object comparison: 91.866320% matching.
 * @note In-progress import; source filename reports 91.890980% matching.
 */
s32 func_800623CC(void)
{
    WmapRect rects[4];
    WmapRenderContext* render_base;
    WmapRenderContext* render_alt;
    u8* base_800e_late;
    u8* base_8014_late;
    u8* var_v1;
    s16 temp_v0_4;
    s16 temp_v0_5;
    s32 temp_a0_2;
    s32 temp_a0_4;
    s32 temp_a1;
    s32 temp_s0_7;
    s32 temp_v0;
    s32 temp_v0_6;
    s32 temp_v0_7;
    s32 temp_v1;
    s32 temp_v1_2;
    s32 var_a0;
    s32 var_a1_2;
    s32 var_s5;
    s32 one;
    s32 three;
    s8* event_flags;
    u8* base_8012;
    u8* base_8014a;
    u8* base_801b;
    u8* base_8014b;
    s32 var_t0;
    s32 var_t1;
    s32 var_v0;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s8 temp_v0_8;
    s8 temp_v0_9;
    u8* temp_s0;
    u8* temp_s0_3;
    u8* temp_s0_5;
    u8* var_a1;
    u8* var_a1_4;
    u16* var_s0;
    u16* var_s0_2;
    u8* var_s1;
    u8* var_s1_2;
    u8* var_s2;
    u16 temp_a0;
    u16 temp_v0_3;
    u32 temp_v0_2;
    u16* var_a0_2;
    SPRT* temp_a0_3;
    POLY_FT4* temp_a2;
    u8* temp_s0_2;
    void* var_sp;

    var_sp = 0;
    temp_v0 = func_8005C6B4();
    var_s5 = 1;
    D_800DBE68 = temp_v0;
    D_8013B234 = temp_v0;
    D_801ADB08 = -1;
    func_80060918();
    func_8006D520();
    one = var_s5;
    base_8012 = (u8*)0x80120000;
    base_8014a = (u8*)0x80140000;
    three = 3;
    event_flags = (s8*)&D_80129538;
    base_801b = (u8*)0x801B0000;
    base_8014b = (u8*)0x80140000;
loop_1:
    do
    {
        temp_v0_2 = func_8005C878();
        D_801398F8 = temp_v0_2;
        if (temp_v0_2 != -1U)
        {
            D_800D9224 += 1;
            if ((u32)(temp_v0_2 - 4) < 5U)
            {
                D_80182DD8 = temp_v0_2;
                D_8011CF50 = one;
                D_801398C0 = 0;
                D_8013922C = 0;
                D_800D9224 = one;
                {
                    s32 drain_event;

                drain_events:
                    drain_event = func_8005C878();
                    if (drain_event != -1)
                    {
                        goto drain_events;
                    }
                }
                func_8006CBD8(&func_800BFD18);
            }
            else
            {
                switch (temp_v0_2) /* switch 1 */
                {
                case 0: /* switch 1 */
                    *(s32*)(base_8012 - 0x2F24) = one;
                block_11:
                    *(s32*)(base_8014a - 0x4D84) = 0;
                    D_80182228 = one;
                    break;
                case 1: /* switch 1 */
                    *(s32*)(base_8012 - 0x2F24) = 2;
                    break;
                case 28: /* switch 1 */
                    *(s32*)(base_8012 - 0x2F24) = three;
                    goto block_11;
                case 25: /* switch 1 */
                    event_flags[0] = one;
                    break;
                case 17: /* switch 1 */
                    event_flags[1] = one;
                    break;
                case 18: /* switch 1 */
                    event_flags[2] = one;
                    break;
                case 19: /* switch 1 */
                    event_flags[3] = one;
                    break;
                case 20: /* switch 1 */
                    event_flags[4] = one;
                    break;
                case 22: /* switch 1 */
                    D_80182E1C = one;
                    break;
                case 23: /* switch 1 */
                    D_801398B8 = one;
                    break;
                case 21: /* switch 1 */
                    D_80129540 = one;
                    break;
                case 16: /* switch 1 */
                    D_80139248 = one;
                    break;
                case 11: /* switch 1 */
                    var_s5 = 0;
                    *(s32*)(base_801b - 0x2470) = 0;
                    D_8011D4F8 = one;
                    break;
                case 15: /* switch 1 */
                    D_80139900 = one;
                    break;
                case 12: /* switch 1 */
                    var_s5 = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_80182E34 = three;
                    D_800DBE78 = three;
                    D_8013B254 = three;
                    D_801ADAEC = 0;
                    *(s32*)(base_801b - 0x2470) = 0;
                    *(s32*)(base_8014a - 0x4D84) = 0;
                    D_8013B268 = 0;
                    *(s32*)(base_8014b - 0x4DF8) = one;
                    *(s32*)(base_801b - 0x2470) = 0;
                    D_80182D5C = func_8005D850(D_8019D248, D_8019D248 + 4);
                    D_80139834 = one;
                    *(s32*)(base_8014b - 0x4DF8) = one;
                    break;
                case 13: /* switch 1 */
                    var_s5 = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_80182E34 = three;
                    D_800DBE78 = three;
                    D_8013B254 = three;
                    D_801ADAEC = 0;
                    *(s32*)(base_801b - 0x2470) = 0;
                    *(s32*)(base_8014a - 0x4D84) = 0;
                    D_8013B268 = 0;
                    *(s32*)(base_8014b - 0x4DF8) = one;
                    D_80182D5C = func_8005D850(D_8019D248, D_8019D248 + 4);
                    D_80139238 = one;
                    *(s32*)(base_8014b - 0x4DF8) = one;
                    break;
                case 9: /* switch 1 */
                    var_s5 = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_8011D500 = 0xFF;
                    D_800D9228 = 0xFF;
                    D_80182E38 = one;
                    *(s32*)(base_801b - 0x2470) = 0;
                    D_80182E00 = 0;
                    func_8006CBD8(&func_80064D64);
                    D_80139244 = one;
                    D_800DBE78 = three;
                    D_80182E34 = three;
                    D_801ADAEC = 0;
                    D_8013B268 = 0;
                    D_8013B254 = three;
                    *(s32*)(base_8014b - 0x4DF8) = one;
                    *(s32*)(base_8014a - 0x4D84) = 0;
                    D_8012954C = one;
                    break;
                case 10: /* switch 1 */
                    var_s5 = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_8011D500 = 0xFF;
                    D_800D9228 = 0xFF;
                    D_80182E38 = one;
                    *(s32*)(base_801b - 0x2470) = 0;
                    *(s32*)(base_8014a - 0x4D84) = 0;
                    *(s32*)(base_8014b - 0x4DF8) = one;
                    D_80182E00 = 0;
                    func_8006CBD8(&func_80064D64);
                    D_80139244 = one;
                    D_80182E34 = three;
                    D_800DBE78 = three;
                    D_801ADAEC = 0;
                    D_8013B268 = 0;
                    D_8013B254 = three;
                    D_801ADAF0 = one;
                    break;
                case 24: /* switch 1 */
                    var_s5 = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_80182E34 = three;
                    D_800DBE78 = three;
                    D_8013B254 = three;
                    D_801ADAEC = 0;
                    D_8013B268 = 0;
                    *(s32*)(base_801b - 0x2470) = 0;
                    *(s32*)(base_8014a - 0x4D84) = 0;
                    *(s32*)(base_8014b - 0x4DF8) = one;
                    D_8013997C = one;
                    break;
                case 27: /* switch 1 */
                    var_s5 = 0;
                    func_8006D8F0(1);
                    func_8006D870(1);
                    D_8011D500 = 0;
                    D_800D9228 = 0;
                    func_8006CBD8(&func_80064D64);
                    D_80182DD4 = one;
                    D_80182E34 = three;
                    D_800DBE78 = three;
                    D_8013B254 = three;
                    D_801ADAEC = 0;
                    D_8013B268 = 0;
                    *(s32*)(base_801b - 0x2470) = 0;
                    *(s32*)(base_8014a - 0x4D84) = 0;
                    *(s32*)(base_8014b - 0x4DF8) = one;
                    D_80182240 = one;
                    break;
                }
                goto loop_1;
            }
        }
    } while (0);
    if (var_s5 != 0)
    {
        var_s1 = D_800DCF18;
        cdrom_queue_read((func_8005D4A4() + 0x1453) & 0xFFFF, var_s1);
        cdrom_wait_queue_empty();
        temp_s0 = var_s1;
        var_s1 += 8;
        if (M2C_FIELD(temp_s0, u8*, 4) & 8)
        {
            rects[0] = *(WmapRect*)((u8*)var_s1 + 4);
            LoadImage(&rects[0], var_s1 + 0xC);
            var_s1 += M2C_FIELD(temp_s0, s32*, 8);
        }
        rects[0] = *(WmapRect*)((u8*)var_s1 + 4);
        if (rects[0].x != -1)
        {
            LoadImage(&rects[0], var_s1 + 0xC);
            DrawSync(0);
            D_801ADAFC = 1;
        }
        D_800D0A08[6] = 0x20;
        D_800D0A08[5] = 0x20;
        D_800D0A08[4] = 0x20;
        PutDispEnv(D_801295BC);
        PutDrawEnv(D_801295BC - 0x5C);
        SetDispMask(1);
        DrawPrim(D_800D0A08);
        PutDispEnv(D_801295BC + 0x7E40);
        PutDrawEnv(D_801295BC + 0x7DE4);
        DrawPrim(D_800D0A08);
    }
    var_a0 = 0x145E;
    if (D_8011D4F8 != 0)
    {
        var_a0 = 0x145F;
    }
    cdrom_queue_read(var_a0, D_800DCF18);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking(D_800DCF18, 1);
    cdrom_queue_read(0x1460, D_800DCF18);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking(D_800DCF18, 1);
    cdrom_queue_read(0x1461, &D_8013B2A0);
    cdrom_wait_queue_empty();
    if (var_s5 != 0)
    {
        D_800D0A08[6] = 0x40;
        D_800D0A08[5] = 0x40;
        D_800D0A08[4] = 0x40;
        PutDispEnv(D_801295BC);
        PutDrawEnv(D_801295BC - 0x5C);
        DrawPrim(D_800D0A08);
        PutDispEnv(D_801295BC + 0x7E40);
        PutDrawEnv(D_801295BC + 0x7DE4);
        DrawPrim(D_800D0A08);
    }
    if (D_801ADB90 != 0)
    {
        akao_play_song(&D_8013B2A0);
        akao_cmd_d0(0);
        akao_cmd_c2(0, 0x1E, 1, 0x7F);
    }
    cdrom_stream((D_800DBE68 + 0x14DE) & 0xFFFF, D_800DCF18);
    temp_s0 = D_800DCF18 + 8;
    cdrom_wait_queue_empty();
    if (M2C_FIELD(D_800DCF18, u8*, 4) & 8)
    {
        rects[0] = *(WmapRect*)((u8*)temp_s0 + 4);
        LoadImage(&rects[0], D_800DCF18 + 0x14);
        temp_s0_2 = M2C_FIELD(D_800DCF18, s32*, 8) + temp_s0;
        rects[0] = *(WmapRect*)((u8*)temp_s0_2 + 4);
        if (rects[0].x != -1)
        {
            var_a1 = temp_s0_2 + 0xC;
            goto block_45;
        }
    }
    else
    {
        rects[0] = *(WmapRect*)((u8*)D_800DCF18 + 0xC);
        var_a1 = D_800DCF18 + 0x14;
        if (rects[0].x != -1)
        {
        block_45:
            LoadImage(&rects[0], var_a1);
            DrawSync(0);
            D_801ADAFC = 1;
        }
    }
    var_a1_2 = 0;
    var_a0_2 = D_800DCF2C;
    var_v1 = D_80139908;
    do
    {
        M2C_FIELD(var_v1, s8*, 0) = (s8)(*var_a0_2 & 0x1F);
        var_a1_2 += 1;
        M2C_FIELD(var_v1, s8*, 1) = (s8)(((u16)*var_a0_2 >> 5) & 0x1F);
        temp_v0_3 = (u16)*var_a0_2;
        var_a0_2++;
        M2C_FIELD(var_v1, s8*, 2) = (s8)((temp_v0_3 >> 0xA) & 0x1F);
        var_v1 += 4;
    } while (var_a1_2 < 0x10);
    rects[0].x = 0x278;
    rects[0].y = 0;
    rects[0].w = 0x10;
    rects[0].h = 0x180;
    MoveImage(&rects[0], 0x2B0, 0);
    rects[0].x = 0x240;
    rects[0].y = 0xE0;
    rects[0].w = 0x60;
    rects[0].h = 0x40;
    MoveImage(&rects[0], 0x240, 0x1C0);
    rects[0].x = 0x278;
    rects[0].y = 0xE0;
    rects[0].w = 0x10;
    rects[0].h = 0x40;
    MoveImage(&rects[0], 0x2B0, 0x1C0);
    cdrom_stream(0x10C4, D_800DCF18);
    cdrom_wait_queue_empty();
    var_s1_2 = D_800DCF18 + 8;
    if (M2C_FIELD(D_800DCF18, u8*, 4) & 8)
    {
        rects[1] = *(WmapRect*)((u8*)var_s1_2 + 4);
        LoadImage(&rects[1], var_s1_2 + 0xC);
        var_s1_2 += M2C_FIELD(D_800DCF18, s32*, 8);
    }
    rects[1] = *(WmapRect*)((u8*)var_s1_2 + 4);
    if (rects[1].x != -1)
    {
        LoadImage(&rects[1], var_s1_2 + 0xC);
        DrawSync(0);
        D_801ADAFC = 1;
    }
    D_801ADAF4 = 8;
    func_8006CBD8(&func_80060F04);
    cdrom_stream(0x10C5, &D_8013C628);
    if (var_s5 != 0)
    {
        D_800D0A08[6] = 0x80;
        D_800D0A08[5] = 0x80;
        D_800D0A08[4] = 0x80;
        PutDispEnv(D_801295BC);
        PutDrawEnv(D_801295BC - 0x5C);
        DrawPrim(D_800D0A08);
        PutDispEnv(D_801295BC + 0x7E40);
        PutDrawEnv(D_801295BC + 0x7DE4);
        DrawPrim(D_800D0A08);
    }
    func_8006CBD8(&func_8006579C);
    D_801398B0 = 1;
    D_8013B24C = 0x10;
    func_8006683C(0x808080);
    cdrom_stream(0x10C6, D_800DCF18);
    cdrom_wait_queue_empty();
    temp_s0_3 = D_800DCF18 + 8;
    if (M2C_FIELD(D_800DCF18, u8*, 4) & 8)
    {
        rects[1] = *(WmapRect*)((u8*)temp_s0_3 + 4);
        LoadImage(&rects[1], D_800DCF18 + 0x14);
        temp_s0_3 += M2C_FIELD(D_800DCF18, s32*, 8);
        rects[1] = *(WmapRect*)((u8*)temp_s0_3 + 4);
        if (rects[1].x != -1)
        {
            LoadImage(&rects[1], temp_s0_3 + 0xC);
            DrawSync(0);
        }
    }
    else
    {
        rects[1] = *(WmapRect*)((u8*)D_800DCF18 + 0xC);
        if (rects[1].x != -1)
        {
            LoadImage(&rects[1], D_800DCF18 + 0x14);
            DrawSync(0);
        }
    }
    cdrom_queue_read(0x10C7, &D_801ADBA0);
    D_8011CF88[16].resource_id = 0x1F;
    D_8011CF88[16].image = (u8*)&D_801ADBA0;
    D_8011CF88[16].unk0 = 0x10;
    D_8011CF88[16].age = 0xFFFF;
    D_8011CF88[16].busy = 0;
    var_s2 = D_800DCF18;
    cdrom_queue_read(0x10C8, var_s2, 0x10);
    cdrom_wait_queue_empty();
    var_s1 = var_s2;
    var_s2 += 8;
    if (M2C_FIELD(var_s1, u8*, 4) & 8)
    {
        rects[1] = *(WmapRect*)((u8*)var_s2 + 4);
        LoadImage(&rects[1], var_s2 + 0xC);
        var_s2 += M2C_FIELD(var_s1, s32*, 8);
    }
    rects[1] = *(WmapRect*)((u8*)var_s2 + 4);
    if (rects[1].x != -1)
    {
        LoadImage(&rects[1], var_s2 + 0xC);
        DrawSync(0);
        D_801ADAFC = 1;
    }
    D_80182248[31].slot = -1;
    D_80182248[31].unkE = 0;
    func_80058D9C();
    var_t0 = M2C_FIELD(&D_8019D248, s32*, 0) - 1;
    var_t1 = M2C_FIELD(&D_8019D248, s32*, 4) - 1;
    if (var_t0 < 0)
    {
        var_t0 = 0;
        D_800DCEEC = 0;
    }
    else if (var_t0 >= 4)
    {
        var_t0 = 3;
        D_800DCEEC = 2;
    }
    if (var_t1 < 0)
    {
        var_t1 = 0;
        D_800DCEF0 = 0;
    }
    else if (var_t1 >= 4)
    {
        var_t1 = 3;
        D_800DCEF0 = 2;
    }
    M2C_FIELD(&D_80139950, s32*, 0) = (s32)(var_t0 * 0x30);
    M2C_FIELD(&D_80139950, s32*, 4) = (s32)(var_t1 * 0x30);
    rects[1] = *(WmapRect*)D_80051A88;
    temp_v0_4 = (M2C_FIELD(&D_8019D248, s32*, 0) - 1) * 0xA0;
    M2C_FIELD(&D_8019D248, u16*, 8) = (u16)M2C_FIELD(&D_8019D248, s32*, 0);
    M2C_FIELD(&D_8019D248, u16*, 0xA) = (u16)M2C_FIELD(&D_8019D248, s32*, 4);
    M2C_FIELD(&D_8019D248, s16*, 0xC) = temp_v0_4;
    M2C_FIELD(&D_8019D248, s16*, 0x10) = temp_v0_4;
    temp_v0_5 = (M2C_FIELD(&D_8019D248, s32*, 4) - 1) * 0xA0;
    M2C_FIELD(&D_8019D248, s16*, 0xE) = temp_v0_5;
    M2C_FIELD(&D_8019D248, s16*, 0x12) = temp_v0_5;
    ClearImage(&rects[1], 0, 0, 0);
    rects[1].x = 0x2C0;
    rects[1].y = 0x1FF;
    rects[1].w = 0x100;
    rects[1].h = 1;
    StoreImage(&rects[1], D_800DCF18);
    DrawSync(0);
    var_v1_2 = 1;
    var_s0 = (u16*)(D_800DCF18 + 2);
    do
    {
        var_v1_2 += 1;
        *var_s0 |= ~0x7FFF;
        var_s0++;
    } while (var_v1_2 < 0x100);
    rects[1].x = 0;
    rects[1].y = 0x1FF;
    LoadImage(&rects[1], D_800DCF18);
    DrawSync(0);
    temp_s0_5 = D_800DCF18 + 8;
    cdrom_queue_read((func_8005D4A4() + 0x10CE) & 0xFFFF, D_800DCF18);
    cdrom_wait_queue_empty();
    if (M2C_FIELD(D_800DCF18, u8*, 4) & 8)
    {
        rects[2] = *(WmapRect*)((u8*)temp_s0_5 + 4);
        LoadImage(&rects[2], D_800DCF18 + 0x14);
        temp_s0_5 += M2C_FIELD(D_800DCF18, s32*, 8);
        rects[2] = *(WmapRect*)((u8*)temp_s0_5 + 4);
        if (rects[2].x != -1)
        {
            LoadImage(&rects[2], temp_s0_5 + 0xC);
            DrawSync(0);
            D_801ADAFC = 1;
        }
    }
    else
    {
        rects[2] = *(WmapRect*)((u8*)D_800DCF18 + 0xC);
        if (rects[2].x != -1)
        {
            LoadImage(&rects[2], D_800DCF18 + 0x14);
            DrawSync(0);
            D_801ADAFC = 1;
        }
    }
    func_8006CBD8(&func_80061878);
    D_801ADAF4 = 0x10;
    D_801ADAE8 = 1;
    if (D_800D9224 == 0)
    {
        D_8011CF50 = 0;
    }
    update_controllers(&D_801ADAE8);
    D_8011CF74 = 0;
    render_base = (WmapRenderContext*)D_80129560;
    render_alt = (WmapRenderContext*)(D_80129560 + 0x7E40);
    base_800e_late = (u8*)0x800E0000;
    base_8014_late = (u8*)0x80140000;
    while (1)
    {
        if (D_8011CF74 & 1)
        {
            render_base->prim_cursor = (u8*)&D_8010CF18;
            D_801398EC = render_base;
        }
        else
        {
            M2C_FIELD(render_base, u8**, 0x817C) = (u8*)&D_80114F18;
            D_801398EC = render_alt;
        }
        M2C_FIELD(base_800e_late, s32*, -0x6DE4) = 0;
        ClearOTagR(D_801398EC->ot, 0xB3);
        D_800DBE7C = 0;
        D_8011CF74 = (s32)(D_8011CF74 + 1);
        if ((D_8011CF50 == 0) && (D_8011CF18 < 3))
        {
            if ((D_80139960 == 0) && (D_801398D0 == 0))
            {
                if (D_8011CF44 != 0)
                {
                    goto block_92;
                }
                func_80063BE0();
            }
            else
            {
                goto block_93;
            }
        }
        else
        {
        block_92:
        block_93:
            D_8013922C = 0;
            D_801398C0 = 0;
        }
        if (D_8011CF74 < 0x28)
        {
            D_8013922C &= 0xF000;
            D_801398C0 &= 0xF000;
        }
        if (D_8011D0DC != 0)
        {
            if (D_80182DB4 != 0)
            {
                func_80061F18(&D_8013922C);
            }
            else
            {
                temp_v0_6 = D_801398B4 - 1;
                D_801398B4 = temp_v0_6;
                if (temp_v0_6 != 0)
                {
                    D_801398C0 = 0;
                    D_8013922C = 0;
                }
                else
                {
                    func_80061FF8(&D_8013922C);
                }
            }
        }
        if (((s32)M2C_FIELD(D_800D0454, u16*, 0xB4) >> 8) & 1)
        {
            D_8011D0E4 = 1;
        }
        else
        {
            D_8011D0E4 = 0;
        }
        temp_a0 = M2C_FIELD(D_800D0454, u16*, 2);
        temp_a0_2 = ((s32)temp_a0 >> 8) | (temp_a0 << 8);
        if ((temp_a0_2 & 0x90F) != 0x90F)
        {
            if (temp_a0_2 & 4)
            {
                D_8011D0E0 = 1;
            }
            else
            {
                D_8011D0E0 = 0;
            }
            temp_v1 = M2C_FIELD(base_8014_late, s32*, -0x4D6C);
            if (temp_v1 != 0)
            {
                D_8011CF50 = 1;
                D_801398C0 = 0;
                D_8013922C = 0;
                if ((temp_v1 != 1) || (var_v0 = 2, ((D_8011CF74 & 1) == 0)))
                {
                    var_v0 = M2C_FIELD(base_8014_late, s32*, -0x4D6C) + 1;
                }
                M2C_FIELD(base_8014_late, s32*, -0x4D6C) = var_v0;
            }
            if (M2C_FIELD(base_8014_late, s32*, -0x4D6C) >= 0xB)
            {
                if (M2C_FIELD(base_8014_late, s32*, -0x4D6C) == 0xB)
                {
                    akao_cmd_c2(0, 0x5A, 0x7F, 0);
                    if (D_80139228 == 0)
                    {
                        akao_play_sfx_from_buffer(D_800CB248, 0, 0x80, 0x7F);
                    }
                    func_8005DF50(M2C_FIELD(&D_8019D248, s32*, 0), M2C_FIELD(&D_8019D248, s32*, 4));
                    DrawSync(0);
                    var_s0_2 = (u16*)D_800DCF18;
                    StoreImage(&D_801398EC->disp_env, D_800DCF18);
                    MoveImage(&D_801398EC->disp_env, D_801398EC->draw_env.tw_x, D_801398EC->draw_env.tw_y);
                    DrawSync(0);
                    var_v1_3 = 0;
                    do
                    {
                        var_v1_3 += 1;
                        *var_s0_2 |= 0x8000;
                        var_s0_2++;
                    } while (var_v1_3 <= 0x12BFF);
                    rects[2] = *(const WmapRect*)D_80051ACC;
                    LoadImage(&rects[2], D_800DCF18);
                    M2C_FIELD(&D_8013B238, s16*, 0) = 0;
                    M2C_FIELD(&D_8013B238, s16*, 2) = 0;
                    M2C_FIELD(&D_8013B238, s16*, 4) = 0;
                    M2C_FIELD(&D_80139870, s32*, 0) = 0xA0;
                    M2C_FIELD(&D_80139870, s32*, 4) = 0x78;
                    M2C_FIELD(&D_80139870, s32*, 8) = 0;
                    M2C_FIELD(&D_8013B240, s16*, 0) = 0;
                    M2C_FIELD(&D_8013B240, s16*, 2) = 0;
                    M2C_FIELD(&D_8013B240, s16*, 4) = 0;
                    M2C_FIELD(&D_80139888, s32*, 0) = 0xA0;
                    M2C_FIELD(&D_80139888, s32*, 4) = 0x78;
                    M2C_FIELD(&D_80139888, s32*, 8) = 0;
                    M2C_FIELD(render_base, s8*, 0x7E56) = 0;
                    M2C_FIELD(render_base, s8*, 0x16) = 0;
                    M2C_FIELD(&D_801B2478, s32*, 0) = 3;
                    M2C_FIELD(&D_801B2478, s32*, 4) = 0;
                    M2C_FIELD(&D_801B2478, s32*, 8) = 0;
                    M2C_FIELD(&D_801B24A8, s16*, 0) = 8;
                    M2C_FIELD(&D_801B24A8, s16*, 2) = 0xC;
                    M2C_FIELD(&D_801B24A8, s16*, 4) = 0;
                    temp_v1_2 = rand(0xA0) & 3;
                    switch (temp_v1_2) /* switch 2; irregular */
                    {
                    case 0: /* switch 2 */
                        var_v1_4 = 0x201010;
                    block_133:
                        D_800D9238 = var_v1_4;
                        break;
                    case 1: /* switch 2 */
                        var_v1_4 = 0x101810;
                        goto block_133;
                    case 2: /* switch 2 */
                        var_v1_4 = 0x201020;
                        goto block_133;
                    case 3: /* switch 2 */
                        var_v1_4 = 0x202010;
                        goto block_133;
                    }
                    DrawSync(0);
                }
                if (M2C_FIELD(base_8014_late, s32*, -0x4D6C) == 0x78)
                {
                    return 0;
                }
                switch (D_80139228)
                {
                case 1:
                    func_800641DC();
                    goto block_168;
                case 0:
                    func_8006454C();
                    goto block_168;
                case 2:
                    return 0;
                default:
                    goto block_168;
                }
            }
            else
            {
                func_80061A2C(temp_a0_2);
                if (D_800D916C != 0)
                {
                    temp_a0_3 = (SPRT*)D_801398EC->prim_cursor;
                    *temp_a0_3 = D_800D06E4;
                    addPrim(&D_801398EC->ot[1], temp_a0_3);
                    temp_a1 = M2C_FIELD(base_800e_late, s32*, -0x6DE4);
                    if (temp_a1 < 0x7D00)
                    {
                        M2C_FIELD(base_800e_late, s32*, -0x6DE4) = temp_a1 + sizeof(SPRT);
                        D_801398EC->prim_cursor += sizeof(SPRT);
                    }
                    func_8006534C(0xD5, 1, 0xFF000000, D_801398EC);
                }
                if (D_8013986C == 0)
                {
                    if (D_8011D4FC != -1)
                    {
                        D_801398BC = 3;
                    }
                    else if (D_8011CF18 == 2)
                    {
                        D_801398BC = D_8011CF18;
                    }
                    else
                    {
                        D_801398BC = 0;
                    }
                }
                else if (D_8013986C == 1)
                {
                    D_801398BC = 1;
                }
                if ((D_8013986C == 0) && (D_8013B25C != 0))
                {
                    func_800594D8();
                }
                func_800664B8();
                func_8006CB60();
                func_8006CA28();
                func_80054A2C();
                if (D_8013B25C != 0)
                {
                    func_80056824();
                }
                if (D_800D9224 != 0)
                {
                    D_8011CF50 = 1;
                    if (D_8011CF74 >= 0x33)
                    {
                        func_800A5DFC();
                    }
                }
                if (D_8013986C != 0)
                {
                    func_8005FF88(-1);
                }
                func_8005F9BC();
                func_8005880C();
                func_80057C14();
                func_8005A318();
                func_80059C78();
                func_8006D674();
                var_sp = 0;
            block_168:
                func_8005D46C();
                D_801ADAF8 = 0;
                temp_s0_7 = VSync(1);
                DrawSync(0);
                set_controller_vsync_interval(2);
                VSync(2);
                if (temp_s0_7 >= 0x20E)
                {
                    D_801ADAFC = 0;
                }
                if (!(D_8011CF74 & 0x1F))
                {
                    D_801ADAFC = 1;
                }
                PutDispEnv(&D_801398EC->disp_env);
                PutDrawEnv(&D_801398EC->draw_env);
                if (D_8013B210 != 0)
                {
                    rects[2].x = 0x240;
                    rects[2].y = 0x151;
                    D_8013B210 = 0;
                    rects[2].w = 0x10;
                    rects[2].h = 1;
                    LoadImage(&rects[2], &D_8013B210);
                }
                func_80064BF8();
                DrawOTag(&D_801398EC->ot[0xB2]);
                update_controllers();
                cdrom_process_state();
                temp_v0_7 = cdrom_get_error_status();
                D_8013B26C = temp_v0_7;
                if (temp_v0_7 != 0)
                {
                    temp_a2 = (POLY_FT4*)D_801398EC->prim_cursor;
                    if (temp_v0_7 < 0)
                    {
                        D_8013B26C = 1;
                    }
                    if (D_8013B26C >= 6)
                    {
                        D_8013B26C = 5;
                    }
                    *(u32*)&temp_a2->r0 = 0x808080;
                    temp_a2->x2 = 0x64;
                    temp_a2->x0 = 0x64;
                    temp_a2->x3 = 0xE4;
                    temp_a2->x1 = 0xE4;
                    temp_a2->y1 = 0x50;
                    temp_a2->y0 = 0x50;
                    temp_a2->y3 = 0x70;
                    temp_a2->y2 = 0x70;
                    temp_a2->u2 = 0x80;
                    temp_a2->u0 = 0x80;
                    temp_a2->u3 = 0xFF;
                    temp_a2->u1 = 0xFF;
                    temp_a2->tpage = 0x5E;
                    temp_a2->clut = 0x7F70;
                    setlen(temp_a2, 9);
                    setcode(temp_a2, 0x2E);
                    temp_v0_8 = (D_8013B26C - 1) << 5;
                    temp_a2->v1 = temp_v0_8;
                    temp_a2->v0 = temp_v0_8;
                    temp_v0_9 = temp_v0_8 + 0x20;
                    temp_a2->v3 = temp_v0_9;
                    temp_a2->v2 = temp_v0_9;
                    setaddr(temp_a2, getaddr(&D_801398EC->ot[1]));
                    temp_a0_4 = M2C_FIELD(base_800e_late, s32*, -0x6DE4);
                    setaddr(&D_801398EC->ot[1], temp_a2);
                    if (temp_a0_4 < 0x7D00)
                    {
                        M2C_FIELD(base_800e_late, s32*, -0x6DE4) = temp_a0_4 + 0x28;
                        D_801398EC->prim_cursor += sizeof(POLY_FT4);
                    }
                }
                continue;
            }
        }
        else
        {
            return 2;
        }
    }
}

/** @brief Advance the pending world-map action. */
void func_80063BE0(void)
{
    s16 temp_v1_2;
    s16 temp_v1_3;
    s16 var_a0;
    s16 var_a0_2;
    s32 temp_a0_2;
    s32 temp_a1;
    s32 temp_a1_2;
    s32 var_v1;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    u16 temp_a0;
    u16 temp_v1;
    u8 temp_v0;

    if ((D_8013B26C != 0) || (D_8011CF50 != 0))
    {
        D_8013922C = 0;
        D_801398C0 = 0;
        return;
    }
    temp_v1 = M2C_FIELD(D_800D0454, u16*, 2);
    temp_a0 = M2C_FIELD(D_800D0454, u16*, 6);
    D_801398C0 = (s32)temp_v1;
    D_801398C0 = (temp_v1 >> 8) | (temp_v1 << 8);
    D_8013922C = (s32)temp_a0;
    temp_a1 = (temp_a0 >> 8) | (temp_a0 << 8);
    D_8013922C = temp_a1;
    temp_v0 = M2C_FIELD(D_800D0454, u8*, 0);
    if (temp_v0 == 0)
    {
        D_800DCE9C = 0;
        D_800DCE98 = 0;
    }
    else if ((s32)temp_v0 >= 0)
    {
        if ((s32)temp_v0 < 3)
        {
            temp_v1_2 = M2C_FIELD(D_800D0454, s16*, 0xC);
            var_a0 = temp_v1_2;
            if (temp_v1_2 < 0)
            {
                var_a0 = -var_a0;
            }
            if (var_a0 >= 8)
            {
                var_a0 = 7;
            }
            if (temp_v1_2 < 0)
            {
                var_v1 = temp_a1;
                if (((s32)D_8011CF74 % (s32)(0x200 >> var_a0)) == 0)
                {
                    var_v1 |= 0x8000;
                }
                D_8013922C = var_v1;
            }
            if (M2C_FIELD(D_800D0454, s16*, 0xC) > 0)
            {
                var_v1_2 = D_8013922C;
                if (((s32)D_8011CF74 % (s32)(0x200 >> var_a0)) == 0)
                {
                    var_v1_2 |= 0x2000;
                }
                D_8013922C = var_v1_2;
            }
            temp_v1_3 = M2C_FIELD(D_800D0454, s16*, 0xE);
            var_a0_2 = temp_v1_3;
            if (temp_v1_3 < 0)
            {
                var_a0_2 = -var_a0_2;
            }
            if (var_a0_2 >= 8)
            {
                var_a0_2 = 7;
            }
            if (temp_v1_3 < 0)
            {
                var_v1_3 = D_8013922C;
                if (((s32)D_8011CF74 % (s32)(0x200 >> var_a0_2)) == 0)
                {
                    var_v1_3 |= 0x1000;
                }
                D_8013922C = var_v1_3;
            }
            if (M2C_FIELD(D_800D0454, s16*, 0xE) > 0)
            {
                var_v1_4 = D_8013922C;
                if (((s32)D_8011CF74 % (s32)(0x200 >> var_a0_2)) == 0)
                {
                    var_v1_4 |= 0x4000;
                }
                D_8013922C = var_v1_4;
            }
        }
        else
        {
            goto block_32;
        }
    }
    else
    {
    block_32:
        D_8013922C = 0;
        D_801398C0 = 0;
    }
    if (D_801398C0 & 0x200)
    {
        D_801398C0 |= 0x40;
    }
    if (D_8013922C & 0x200)
    {
        D_8013922C |= 0x40;
    }
    temp_a1_2 = D_801398C0 & D_80139868;
    D_801398C0 = temp_a1_2;
    temp_a0_2 = D_8013922C & D_80139868;
    D_8013922C = temp_a0_2;
    if (D_801398D4 != 0)
    {
        D_801398C0 = temp_a1_2 & D_800D9220;
        D_8013922C = temp_a0_2 & D_800D9220;
    }
    M2C_FIELD(D_800D0454, u8*, 0x91) = (u8)D_800DCE98;
    M2C_FIELD(D_800D0454, u8*, 0x92) = (u8)D_800DCE9C;
    D_8019D6D8 = 0;
}

/** @brief Rebuild the tile layout and reset each tile's display state. */
void func_80063F38(void)
{
    s32 x;
    s32 y;
    s32 tile;

    for (y = 0; y < 6; y++)
    {
        for (x = 0; x < 6; x++)
        {
            tile = func_8005D670(x, y);
            if (D_80139830 != 0)
            {
                if (tile == 5)
                {
                    tile = 35;
                }
            }
            if (D_80182E20 != 0)
            {
                if (tile == 1)
                {
                    tile = 36;
                }
            }
            D_80139290[x][y].tile = tile;
            D_80139290[x][y].field_06 = func_8005D554(x, y);
            func_8005D6B8(x, y, D_80139290[x][y].neighbors);
            D_80139290[x][y].field_04 = func_8005B8C8(x, y, D_8011D4FC);
            D_8011D108[x][y].field_02 = 0;
        }
    }
}

/** @brief Reset map actors, layout, effect selection, and display state. */
void func_80064094(void)
{
    s32 i;

    D_80139978 = -1;
    for (i = 0; i < 256; i++)
    {
        D_800D9268[i].display_mode = 16;
        D_801AFBD0[i].state = 0;
    }
    func_80063F38();
    D_8011CF50 = 0;
    D_8013B258 = 0;
    D_80182E24 = 0;
    D_80129550 = 0;
    D_8011D4FC = -1;
    D_80182E30 = 0;
    D_800DCEE0 = 0;
    func_8005909C();
    D_8011D52C = 0;
    func_8006D870(0);
    D_800DBE70 = 2;
    D_801ADAE0 = 0;
    D_801ADAEC = 0x80;
    D_801ADAF4 = 0x10;
    D_80182DE0 = 0;
    D_801398FC = 0;
    D_801398C4 = 0;
    D_800D9210 = 1;
    D_800DCEA0 = -1;
    D_800D9220 = -1;
    akao_cmd_c2(0, 0x1E, 1, 0x7F);
    D_801ADAFC = 1;
    D_8013B24C = 4;
    D_80182E3C = -1;
}
