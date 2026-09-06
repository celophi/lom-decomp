#include "common.h"

void field_set_fade_target_only(s16 red, s16 green, s16 blue, s16 duration);
void func_800A3938(s32 sound_id, s32 pan);
void func_800AE9E0(void);

extern s32 D_8011F3D4;
extern s32 D_80122820;
extern s32 D_80122990;
extern s32 D_80122B08;
extern s32 D_8012269C;
extern u8 D_800FDF79;
extern u8 *g_pad_ctx;

/**
 * @brief Enter modal state 8, arm the 500-frame slide-out counter, and bump
 *        the visit counter for whichever pad-context record the "hidden
 *        controller test" byte selects.
 *
 * D_800FDF79 (masked to 7 bits) selects record 1 when it equals 0x1D,
 * otherwise record 0; D_80122820 is left pointing at the chosen record.
 * The chosen record's own-position visit counter is always incremented; its
 * paired counter is incremented too while the record's mode byte (masked to
 * 7 bits) is below 2.
 */
void func_800AB774(void)
{
    D_8012269C = 8;
    field_set_fade_target_only(0xC0, 0xC0, 0xC0, 8);
    func_800AE9E0();
    func_800A3938(0x126, 0x80);
    D_8011F3D4 = 0;
    D_80122B08 = 0;
    D_80122990 = 0x1F4;

    if ((D_800FDF79 & 0x7F) == 0x1D)
    {
        D_80122820 = 1;
        *(u16 *)(g_pad_ctx + 0x636) = *(u16 *)(g_pad_ctx + 0x636) + 1;
        if ((u32)(g_pad_ctx[0x858] & 0x7F) < 2)
        {
            *(u16 *)(g_pad_ctx + 0x884) = *(u16 *)(g_pad_ctx + 0x884) + 1;
        }
    }
    else
    {
        D_80122820 = 0;
        *(u16 *)(g_pad_ctx + 0x634) = *(u16 *)(g_pad_ctx + 0x634) + 1;
        if ((u32)(g_pad_ctx[0x858] & 0x7F) < 2)
        {
            *(u16 *)(g_pad_ctx + 0x886) = *(u16 *)(g_pad_ctx + 0x886) + 1;
        }
    }
}
