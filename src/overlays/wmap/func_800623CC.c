/* Partial WMAP decompilation: 91.872810% (gcc280_g0). */
#include "common.h"
#include "sdk/libgpu.h"

typedef s32 M2C_UNK;
#define M2C_FIELD(expr, type_ptr, offset) (*(type_ptr)((s8 *)(expr) + (offset)))
#define M2C_UNALIGNED32(expr) (expr)

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
    u8 *image;
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
    u8 *prim_cursor;
} WmapRenderContext;

extern M2C_UNK func_80064D64;
extern s32 D_800CB248;
extern void *D_800D0454;
extern SPRT D_800D06E4;
extern u8 D_800D0A08[];
extern s32 D_800D916C;
extern s32 D_800D921C;
extern s32 D_800D9224;
extern s32 D_800D9228;
extern s32 D_800D9238;
extern s32 D_800DBE68;
extern s32 D_800DBE78;
extern s32 D_800DBE7C;
extern s32 D_800DCEEC;
extern s32 D_800DCEF0;
extern u8 D_800DCF18[];
extern u16 D_800DCF2C[];
extern M2C_UNK D_8010CF18;
extern M2C_UNK D_80114F18;
extern s32 D_8011CF18;
extern s32 D_8011CF44;
extern s32 D_8011CF50;
extern M2C_UNK D_8011CF74;
extern WmapCacheEntry D_8011CF88[];
extern s32 D_8011D0DC;
extern s32 D_8011D0E0;
extern s32 D_8011D0E4;
extern s32 D_8011D4F8;
extern s32 D_8011D4FC;
extern s32 D_8011D500;
extern M2C_UNK D_80129538;
extern s32 D_80129540;
extern s32 D_8012954C;
extern u8 D_80129560[];
extern u8 D_801295BC[];
extern s32 D_80139228;
extern s32 D_8013922C;
extern s32 D_80139238;
extern s32 D_80139244;
extern s32 D_80139248;
extern s32 D_80139834;
extern s32 D_8013986C;
extern M2C_UNK D_80139870;
extern M2C_UNK D_80139888;
extern s32 D_801398B0;
extern s32 D_801398B4;
extern s32 D_801398B8;
extern s32 D_801398BC;
extern s32 D_801398C0;
extern s32 D_801398D0;
extern WmapRenderContext *D_801398EC;
extern u32 D_801398F8;
extern s32 D_80139900;
extern u8 D_80139908[];
extern M2C_UNK D_80139950;
extern s32 D_80139960;
extern s32 D_8013997C;
extern u16 D_8013B210;
extern s32 D_8013B234;
extern M2C_UNK D_8013B238;
extern M2C_UNK D_8013B240;
extern s32 D_8013B24C;
extern s32 D_8013B254;
extern s32 D_8013B25C;
extern s32 D_8013B268;
extern s32 D_8013B26C;
extern u16 D_8013B2A0;
extern u16 D_8013C628;
extern s32 D_80182228;
extern s32 D_80182240;
extern WmapResource D_80182248[];
extern s32 D_80182D5C;
extern s32 D_80182DB4;
extern s32 D_80182DD4;
extern u32 D_80182DD8;
extern s32 D_80182E00;
extern s32 D_80182E1C;
extern s32 D_80182E34;
extern s32 D_80182E38;
extern u8 D_8019D248[];
extern s32 D_801ADAE8;
extern s32 D_801ADAEC;
extern s32 D_801ADAF0;
extern s32 D_801ADAF4;
extern s32 D_801ADAF8;
extern s32 D_801ADAFC;
extern s32 D_801ADB08;
extern s32 D_801ADB90;
extern u16 D_801ADBA0;
extern M2C_UNK D_801B2478;
extern M2C_UNK D_801B24A8;
extern M2C_UNK func_80060F04;
extern M2C_UNK func_80061878;
extern M2C_UNK func_8006579C;
extern M2C_UNK func_800BFD18;
extern u8 D_80051A88[];
extern u8 D_80051ACC[];

/**
 * @brief Run the world-map resource and rendering loop.
 * @return World-map loop status; value meanings are not yet fully identified.
 * @note Project object comparison: 91.866320% matching.
 * @note In-progress import; source filename reports 91.890980% matching.
 */
s32 func_800623CC(void)
{
    WmapRect rects[4];
    WmapRenderContext *render_base;
    WmapRenderContext *render_alt;
    u8 *base_800e_late;
    u8 *base_8014_late;
    u8 *var_v1;
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
    s8 *event_flags;
    u8 *base_8012;
    u8 *base_8014a;
    u8 *base_801b;
    u8 *base_8014b;
    s32 var_t0;
    s32 var_t1;
    s32 var_v0;
    s32 var_v1_2;
    s32 var_v1_3;
    s32 var_v1_4;
    s8 temp_v0_8;
    s8 temp_v0_9;
    u8 *temp_s0;
    u8 *temp_s0_3;
    u8 *temp_s0_5;
    u8 *var_a1;
    u8 *var_a1_4;
    u16 *var_s0;
    u16 *var_s0_2;
    u8 *var_s1;
    u8 *var_s1_2;
    u8 *var_s2;
    u16 temp_a0;
    u16 temp_v0_3;
    u32 temp_v0_2;
    u16 *var_a0_2;
    SPRT *temp_a0_3;
    POLY_FT4 *temp_a2;
    u8 *temp_s0_2;
    void *var_sp;

    var_sp = 0;
    temp_v0 = func_8005C6B4();
    var_s5 = 1;
    D_800DBE68 = temp_v0;
    D_8013B234 = temp_v0;
    D_801ADB08 = -1;
    func_80060918();
    func_8006D520();
    one = var_s5;
    base_8012 = (u8 *)0x80120000;
    base_8014a = (u8 *)0x80140000;
    three = 3;
    event_flags = (s8 *)&D_80129538;
    base_801b = (u8 *)0x801B0000;
    base_8014b = (u8 *)0x80140000;
loop_1:
    do
    {
        temp_v0_2 = func_8005C878();
    D_801398F8 = temp_v0_2;
    if (temp_v0_2 != -1U)
    {
        D_800D9224 += 1;
        if ((u32) (temp_v0_2 - 4) < 5U)
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
            switch (temp_v0_2)                      /* switch 1 */
            {
            case 0:                                 /* switch 1 */
                *(s32 *)(base_8012 - 0x2F24) = one;
block_11:
                *(s32 *)(base_8014a - 0x4D84) = 0;
                D_80182228 = one;
                break;
            case 1:                                 /* switch 1 */
                *(s32 *)(base_8012 - 0x2F24) = 2;
                break;
            case 28:                                /* switch 1 */
                *(s32 *)(base_8012 - 0x2F24) = three;
                goto block_11;
            case 25:                                /* switch 1 */
                event_flags[0] = one;
                break;
            case 17:                                /* switch 1 */
                event_flags[1] = one;
                break;
            case 18:                                /* switch 1 */
                event_flags[2] = one;
                break;
            case 19:                                /* switch 1 */
                event_flags[3] = one;
                break;
            case 20:                                /* switch 1 */
                event_flags[4] = one;
                break;
            case 22:                                /* switch 1 */
                D_80182E1C = one;
                break;
            case 23:                                /* switch 1 */
                D_801398B8 = one;
                break;
            case 21:                                /* switch 1 */
                D_80129540 = one;
                break;
            case 16:                                /* switch 1 */
                D_80139248 = one;
                break;
            case 11:                                /* switch 1 */
                var_s5 = 0;
                *(s32 *)(base_801b - 0x2470) = 0;
                D_8011D4F8 = one;
                break;
            case 15:                                /* switch 1 */
                D_80139900 = one;
                break;
            case 12:                                /* switch 1 */
                var_s5 = 0;
                func_8006D8F0(1);
                func_8006D870(1);
                D_80182E34 = three;
                D_800DBE78 = three;
                D_8013B254 = three;
                D_801ADAEC = 0;
                *(s32 *)(base_801b - 0x2470) = 0;
                *(s32 *)(base_8014a - 0x4D84) = 0;
                D_8013B268 = 0;
                *(s32 *)(base_8014b - 0x4DF8) = one;
                *(s32 *)(base_801b - 0x2470) = 0;
                D_80182D5C = func_8005D850(D_8019D248, D_8019D248 + 4);
                D_80139834 = one;
                *(s32 *)(base_8014b - 0x4DF8) = one;
                break;
            case 13:                                /* switch 1 */
                var_s5 = 0;
                func_8006D8F0(1);
                func_8006D870(1);
                D_80182E34 = three;
                D_800DBE78 = three;
                D_8013B254 = three;
                D_801ADAEC = 0;
                *(s32 *)(base_801b - 0x2470) = 0;
                *(s32 *)(base_8014a - 0x4D84) = 0;
                D_8013B268 = 0;
                *(s32 *)(base_8014b - 0x4DF8) = one;
                D_80182D5C = func_8005D850(D_8019D248, D_8019D248 + 4);
                D_80139238 = one;
                *(s32 *)(base_8014b - 0x4DF8) = one;
                break;
            case 9:                                 /* switch 1 */
                var_s5 = 0;
                func_8006D8F0(1);
                func_8006D870(1);
                D_8011D500 = 0xFF;
                D_800D9228 = 0xFF;
                D_80182E38 = one;
                *(s32 *)(base_801b - 0x2470) = 0;
                D_80182E00 = 0;
                func_8006CBD8(&func_80064D64);
                D_80139244 = one;
                D_800DBE78 = three;
                D_80182E34 = three;
                D_801ADAEC = 0;
                D_8013B268 = 0;
                D_8013B254 = three;
                *(s32 *)(base_8014b - 0x4DF8) = one;
                *(s32 *)(base_8014a - 0x4D84) = 0;
                D_8012954C = one;
                break;
            case 10:                                /* switch 1 */
                var_s5 = 0;
                func_8006D8F0(1);
                func_8006D870(1);
                D_8011D500 = 0xFF;
                D_800D9228 = 0xFF;
                D_80182E38 = one;
                *(s32 *)(base_801b - 0x2470) = 0;
                *(s32 *)(base_8014a - 0x4D84) = 0;
                *(s32 *)(base_8014b - 0x4DF8) = one;
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
            case 24:                                /* switch 1 */
                var_s5 = 0;
                func_8006D8F0(1);
                func_8006D870(1);
                D_80182E34 = three;
                D_800DBE78 = three;
                D_8013B254 = three;
                D_801ADAEC = 0;
                D_8013B268 = 0;
                *(s32 *)(base_801b - 0x2470) = 0;
                *(s32 *)(base_8014a - 0x4D84) = 0;
                *(s32 *)(base_8014b - 0x4DF8) = one;
                D_8013997C = one;
                break;
            case 27:                                /* switch 1 */
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
                *(s32 *)(base_801b - 0x2470) = 0;
                *(s32 *)(base_8014a - 0x4D84) = 0;
                *(s32 *)(base_8014b - 0x4DF8) = one;
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
        if (M2C_FIELD(temp_s0, u8 *, 4) & 8)
        {
            rects[0] = *(WmapRect *)((u8 *)var_s1 + 4);
            LoadImage(&rects[0], var_s1 + 0xC);
            var_s1 += M2C_FIELD(temp_s0, s32 *, 8);
        }
        rects[0] = *(WmapRect *)((u8 *)var_s1 + 4);
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
    if (M2C_FIELD(D_800DCF18, u8 *, 4) & 8)
    {
        rects[0] = *(WmapRect *)((u8 *)temp_s0 + 4);
        LoadImage(&rects[0], D_800DCF18 + 0x14);
        temp_s0_2 = M2C_FIELD(D_800DCF18, s32 *, 8) + temp_s0;
        rects[0] = *(WmapRect *)((u8 *)temp_s0_2 + 4);
        if (rects[0].x != -1)
        {
            var_a1 = temp_s0_2 + 0xC;
            goto block_45;
        }
    }
    else
    {
        rects[0] = *(WmapRect *)((u8 *)D_800DCF18 + 0xC);
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
        M2C_FIELD(var_v1, s8 *, 0) = (s8) (*var_a0_2 & 0x1F);
        var_a1_2 += 1;
        M2C_FIELD(var_v1, s8 *, 1) = (s8) (((u16) *var_a0_2 >> 5) & 0x1F);
        temp_v0_3 = (u16) *var_a0_2;
        var_a0_2++;
        M2C_FIELD(var_v1, s8 *, 2) = (s8) ((temp_v0_3 >> 0xA) & 0x1F);
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
    if (M2C_FIELD(D_800DCF18, u8 *, 4) & 8)
    {
        rects[1] = *(WmapRect *)((u8 *)var_s1_2 + 4);
        LoadImage(&rects[1], var_s1_2 + 0xC);
        var_s1_2 += M2C_FIELD(D_800DCF18, s32 *, 8);
    }
    rects[1] = *(WmapRect *)((u8 *)var_s1_2 + 4);
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
    if (M2C_FIELD(D_800DCF18, u8 *, 4) & 8)
    {
        rects[1] = *(WmapRect *)((u8 *)temp_s0_3 + 4);
        LoadImage(&rects[1], D_800DCF18 + 0x14);
        temp_s0_3 += M2C_FIELD(D_800DCF18, s32 *, 8);
        rects[1] = *(WmapRect *)((u8 *)temp_s0_3 + 4);
        if (rects[1].x != -1)
        {
            LoadImage(&rects[1], temp_s0_3 + 0xC);
            DrawSync(0);
        }
    }
    else
    {
        rects[1] = *(WmapRect *)((u8 *)D_800DCF18 + 0xC);
        if (rects[1].x != -1)
        {
            LoadImage(&rects[1], D_800DCF18 + 0x14);
            DrawSync(0);
        }
    }
    cdrom_queue_read(0x10C7, &D_801ADBA0);
    D_8011CF88[16].resource_id = 0x1F;
    D_8011CF88[16].image = (u8 *)&D_801ADBA0;
    D_8011CF88[16].unk0 = 0x10;
    D_8011CF88[16].age = 0xFFFF;
    D_8011CF88[16].busy = 0;
    var_s2 = D_800DCF18;
    cdrom_queue_read(0x10C8, var_s2, 0x10);
    cdrom_wait_queue_empty();
    var_s1 = var_s2;
    var_s2 += 8;
    if (M2C_FIELD(var_s1, u8 *, 4) & 8)
    {
        rects[1] = *(WmapRect *)((u8 *)var_s2 + 4);
        LoadImage(&rects[1], var_s2 + 0xC);
        var_s2 += M2C_FIELD(var_s1, s32 *, 8);
    }
    rects[1] = *(WmapRect *)((u8 *)var_s2 + 4);
    if (rects[1].x != -1)
    {
        LoadImage(&rects[1], var_s2 + 0xC);
        DrawSync(0);
        D_801ADAFC = 1;
    }
    D_80182248[31].slot = -1;
    D_80182248[31].unkE = 0;
    func_80058D9C();
    var_t0 = M2C_FIELD(&D_8019D248, s32 *, 0) - 1;
    var_t1 = M2C_FIELD(&D_8019D248, s32 *, 4) - 1;
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
    M2C_FIELD(&D_80139950, s32 *, 0) = (s32) (var_t0 * 0x30);
    M2C_FIELD(&D_80139950, s32 *, 4) = (s32) (var_t1 * 0x30);
    rects[1] = *(WmapRect *)D_80051A88;
    temp_v0_4 = (M2C_FIELD(&D_8019D248, s32 *, 0) - 1) * 0xA0;
    M2C_FIELD(&D_8019D248, u16 *, 8) = (u16) M2C_FIELD(&D_8019D248, s32 *, 0);
    M2C_FIELD(&D_8019D248, u16 *, 0xA) = (u16) M2C_FIELD(&D_8019D248, s32 *, 4);
    M2C_FIELD(&D_8019D248, s16 *, 0xC) = temp_v0_4;
    M2C_FIELD(&D_8019D248, s16 *, 0x10) = temp_v0_4;
    temp_v0_5 = (M2C_FIELD(&D_8019D248, s32 *, 4) - 1) * 0xA0;
    M2C_FIELD(&D_8019D248, s16 *, 0xE) = temp_v0_5;
    M2C_FIELD(&D_8019D248, s16 *, 0x12) = temp_v0_5;
    ClearImage(&rects[1], 0, 0, 0);
    rects[1].x = 0x2C0;
    rects[1].y = 0x1FF;
    rects[1].w = 0x100;
    rects[1].h = 1;
    StoreImage(&rects[1], D_800DCF18);
    DrawSync(0);
    var_v1_2 = 1;
    var_s0 = (u16 *)(D_800DCF18 + 2);
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
    if (M2C_FIELD(D_800DCF18, u8 *, 4) & 8)
    {
        rects[2] = *(WmapRect *)((u8 *)temp_s0_5 + 4);
        LoadImage(&rects[2], D_800DCF18 + 0x14);
        temp_s0_5 += M2C_FIELD(D_800DCF18, s32 *, 8);
        rects[2] = *(WmapRect *)((u8 *)temp_s0_5 + 4);
        if (rects[2].x != -1)
        {
            LoadImage(&rects[2], temp_s0_5 + 0xC);
            DrawSync(0);
            D_801ADAFC = 1;
        }
    }
    else
    {
        rects[2] = *(WmapRect *)((u8 *)D_800DCF18 + 0xC);
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
    render_base = (WmapRenderContext *)D_80129560;
    render_alt = (WmapRenderContext *)(D_80129560 + 0x7E40);
    base_800e_late = (u8 *)0x800E0000;
    base_8014_late = (u8 *)0x80140000;
    while (1)
    {
    if (D_8011CF74 & 1)
    {
        render_base->prim_cursor = (u8 *)&D_8010CF18;
        D_801398EC = render_base;
    }
    else
    {
        M2C_FIELD(render_base, u8 **, 0x817C) = (u8 *)&D_80114F18;
        D_801398EC = render_alt;
    }
    M2C_FIELD(base_800e_late, s32 *, -0x6DE4) = 0;
    ClearOTagR(D_801398EC->ot, 0xB3);
    D_800DBE7C = 0;
    D_8011CF74 = (s32) (D_8011CF74 + 1);
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
    if (((s32) M2C_FIELD(D_800D0454, u16 *, 0xB4) >> 8) & 1)
    {
        D_8011D0E4 = 1;
    }
    else
    {
        D_8011D0E4 = 0;
    }
    temp_a0 = M2C_FIELD(D_800D0454, u16 *, 2);
    temp_a0_2 = ((s32) temp_a0 >> 8) | (temp_a0 << 8);
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
        temp_v1 = M2C_FIELD(base_8014_late, s32 *, -0x4D6C);
        if (temp_v1 != 0)
        {
            D_8011CF50 = 1;
            D_801398C0 = 0;
            D_8013922C = 0;
            if ((temp_v1 != 1) || (var_v0 = 2, ((D_8011CF74 & 1) == 0)))
            {
                var_v0 = M2C_FIELD(base_8014_late, s32 *, -0x4D6C) + 1;
            }
            M2C_FIELD(base_8014_late, s32 *, -0x4D6C) = var_v0;
        }
        if (M2C_FIELD(base_8014_late, s32 *, -0x4D6C) >= 0xB)
        {
            if (M2C_FIELD(base_8014_late, s32 *, -0x4D6C) == 0xB)
            {
                akao_cmd_c2(0, 0x5A, 0x7F, 0);
                if (D_80139228 == 0)
                {
                    akao_play_sfx_from_buffer(D_800CB248, 0, 0x80, 0x7F);
                }
                func_8005DF50(M2C_FIELD(&D_8019D248, s32 *, 0), M2C_FIELD(&D_8019D248, s32 *, 4));
                DrawSync(0);
                var_s0_2 = (u16 *)D_800DCF18;
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
                rects[2] = *(WmapRect *)D_80051ACC;
                LoadImage(&rects[2], D_800DCF18);
                M2C_FIELD(&D_8013B238, s16 *, 0) = 0;
                M2C_FIELD(&D_8013B238, s16 *, 2) = 0;
                M2C_FIELD(&D_8013B238, s16 *, 4) = 0;
                M2C_FIELD(&D_80139870, s32 *, 0) = 0xA0;
                M2C_FIELD(&D_80139870, s32 *, 4) = 0x78;
                M2C_FIELD(&D_80139870, s32 *, 8) = 0;
                M2C_FIELD(&D_8013B240, s16 *, 0) = 0;
                M2C_FIELD(&D_8013B240, s16 *, 2) = 0;
                M2C_FIELD(&D_8013B240, s16 *, 4) = 0;
                M2C_FIELD(&D_80139888, s32 *, 0) = 0xA0;
                M2C_FIELD(&D_80139888, s32 *, 4) = 0x78;
                M2C_FIELD(&D_80139888, s32 *, 8) = 0;
                M2C_FIELD(render_base, s8 *, 0x7E56) = 0;
                M2C_FIELD(render_base, s8 *, 0x16) = 0;
                M2C_FIELD(&D_801B2478, s32 *, 0) = 3;
                M2C_FIELD(&D_801B2478, s32 *, 4) = 0;
                M2C_FIELD(&D_801B2478, s32 *, 8) = 0;
                M2C_FIELD(&D_801B24A8, s16 *, 0) = 8;
                M2C_FIELD(&D_801B24A8, s16 *, 2) = 0xC;
                M2C_FIELD(&D_801B24A8, s16 *, 4) = 0;
                temp_v1_2 = rand(0xA0) & 3;
                switch (temp_v1_2)                  /* switch 2; irregular */
                {
                case 0:                             /* switch 2 */
                    var_v1_4 = 0x201010;
block_133:
                    D_800D9238 = var_v1_4;
                    break;
                case 1:                             /* switch 2 */
                    var_v1_4 = 0x101810;
                    goto block_133;
                case 2:                             /* switch 2 */
                    var_v1_4 = 0x201020;
                    goto block_133;
                case 3:                             /* switch 2 */
                    var_v1_4 = 0x202010;
                    goto block_133;
                }
                DrawSync(0);
            }
            if (M2C_FIELD(base_8014_late, s32 *, -0x4D6C) == 0x78)
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
                temp_a0_3 = (SPRT *)D_801398EC->prim_cursor;
                *temp_a0_3 = D_800D06E4;
                addPrim(&D_801398EC->ot[1], temp_a0_3);
                temp_a1 = M2C_FIELD(base_800e_late, s32 *, -0x6DE4);
                if (temp_a1 < 0x7D00)
                {
                    M2C_FIELD(base_800e_late, s32 *, -0x6DE4) = temp_a1 + sizeof(SPRT);
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
                temp_a2 = (POLY_FT4 *)D_801398EC->prim_cursor;
                if (temp_v0_7 < 0)
                {
                    D_8013B26C = 1;
                }
                if (D_8013B26C >= 6)
                {
                    D_8013B26C = 5;
                }
                *(u32 *)&temp_a2->r0 = 0x808080;
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
                temp_a0_4 = M2C_FIELD(base_800e_late, s32 *, -0x6DE4);
                setaddr(&D_801398EC->ot[1], temp_a2);
                if (temp_a0_4 < 0x7D00)
                {
                    M2C_FIELD(base_800e_late, s32 *, -0x6DE4) = temp_a0_4 + 0x28;
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
