#include "common.h"

/** @brief Party slot flags and action state at the original 0x268-byte stride. */
typedef struct
{
    union
    {
        u16 flags;
        struct
        {
            u16 active : 1;
            u16 selected : 1;
            u16 rest : 14;
        } bits;
    } status;
    u8 pad2;
    u8 unk3;
    u8 pad4[0x250];
    u16 unk254;
    u8 unk256;
    u8 pad257[0x11];
} Slot;
/** @brief Saved scene parameters and party-selection bit in the pad context. */
typedef struct
{
    u8 pad0[0x18];
    u32 unk18;
    s16 unk1C;
    s8 unk1E;
    u8 pad1F;
    s32 unk20;
    u16 unk24;
    u8 unk26;
    u8 unk27;
    u8 pad28[0x830];
    u8 unk858;
} PadContext;
s32 DrawSync(s32);                                             /* extern */
void field_restore_fade_target(void);                          /* extern */
void field_set_fade_target_only(s16, s16, s16, s16);           /* extern */
void field_set_scene_parameters(s32, s32, u32, s32, s32, s32); /* extern */
void field_text_reset_scratch(void);                           /* extern */
void field_text_reset_windows(void);                           /* extern */
void func_80063194(void);                                      /* extern */
void func_8006AB38(s32);                                       /* extern */
s32 func_8006AD04(s32, s32, s32);                              /* extern */
void func_80084240(void);                                      /* extern */
void func_800A39A8(s32, s32, s32, s32);                        /* extern */
void func_800A7384(void);                                      /* extern */
s32 func_800A9D70(s32);                                        /* extern */
void func_800AA90C(s32);                                       /* extern */
void func_800AB690(void *);                                    /* extern */
s32 func_800AB86C(void *);                                     /* extern */
s32 func_800AC768(void *);                                     /* extern */
s32 func_800B0888(void);                                       /* extern */
s32 func_801400C4(s32);                                        /* extern */
s32 func_801400D4(s32);                                        /* extern */
s32 func_801401F0(s32);                                        /* extern */
s32 func_801401F8(s32);                                        /* extern */
s32 func_80140370(s32);                                        /* extern */
extern Slot D_800FD818[];
extern s32 D_8011F41C;
extern u32 D_8012269C;
extern s32 D_801227BC;
extern s32 D_801227C0;
extern s32 D_801227D8;
extern s32 D_801227E4;
extern s32 D_801227F0;
extern s32 D_80122994;
extern s32 D_8012299C;
extern s32 D_801229AC;
extern s32 D_801229F8;
extern s32 g_gosub_result_count;
extern s32 g_gosub_result_values;
extern s16 g_music_track_index;
extern PadContext *g_pad_ctx;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

/**
 * @brief Advance the active field modal state and restore field input on completion.
 * @param context_or_delay Opaque context forwarded to the active modal handler.
 * @note The argument carrier is reused for the two fifteen-frame input delays.
 * @note Party slot and scene data remain owned by their existing external tables.
 */
void func_800AB214(s32 context_or_delay)
{
    s32 slot_address;
    s32 result;
    s32 *request;
    s32 index_or_zero;
    u16 temp_v1;

    switch (D_8012269C)
    {
    case 1:
        if ((D_801229AC != 0) && (func_801400D4(context_or_delay) != 0))
        {
            D_801229AC = 0;
            D_8012269C = 0;
            func_80084240();
            return;
        }
    case 0:
        return;
    case 2:
        if (D_8011F41C != 0)
        {
            if (D_8011F41C >= 2)
            {
                if (func_801400C4(context_or_delay) != 0)
                {
                    DrawSync(0);
                    D_8011F41C = 1;
                    func_80084240();
                    return;
                }
            }
            else
            {
                DrawSync(0);
                field_text_reset_windows();
                D_801227F0 = 2;
                D_8012269C = 0;
                D_8011F41C = 0;
                return;
            }
        }
        break;
    case 3:
        if ((D_8012299C != 0) && (func_80140370(context_or_delay) != 0))
        {
            func_80084240();
            switch (D_8012299C)
            {
            case 2:
                func_800AA90C(0);
                index_or_zero = 0;
                slot_address = (s32)D_800FD818;
                do
                {
                    slot_address = (s32)&D_800FD818[index_or_zero];
                    *(u16 *)(slot_address + 0x254) = 0;
                    *(u8 *)(slot_address + 0x256) = 0xFF;
                    index_or_zero += 1;
                } while (index_or_zero < 3);
                g_music_track_index = (s16) * (volatile s32 *)&g_pad_ctx->unk20;
                field_set_scene_parameters(g_pad_ctx->unk24, g_pad_ctx->unk26, g_pad_ctx->unk18 & 0x01FFFFFF,
                                           g_pad_ctx->unk27, (s32)g_pad_ctx->unk1C, (s32)g_pad_ctx->unk1E);
                field_set_fade_target_only(0x100, 0x100, 0x100, 8);
                break;
            case 6:
            case 7:
                g_gosub_result_values = 6;
                /* fall through */
            default:
            case 3:
            case 4:
            case 5:
                break;
            }
            g_gosub_result_count = 1;
            D_801227F0 = 2;
            D_8012299C = 0;
            D_8012269C = 0;
            return;
        }
        break;
    case 4:
        request = &D_80122994;
        if ((*request != 0) && (func_801401F0(context_or_delay) != 0))
        {
            goto block_32;
        }
        break;
    case 5:
        request = &D_80122994;
        if (*request != 0)
        {
            result = func_801401F8(context_or_delay);
            switch (result)
            {
            case 1:
                D_800FD818[1].status.bits.active = 0;
                D_800FD818[1].status.bits.selected = g_pad_ctx->unk858 & 1;
                D_800FD818[1].unk3 = 0;
                func_8006AD04(-2, 0, 0);
                goto block_32;
            case 3:
                goto block_32;
            case 2:
                func_8006AB38(0);
            block_32:
                func_80084240();
                *request = 0;
                D_8012269C = 0;
                return;
            }
        }
        break;
    case 6:
        if (g_pad_input & 0x220)
        {
            field_text_reset_windows();
            index_or_zero = 0;
            D_8012269C = 0;
            g_pad_input = 0;
            goto block_42;
        }
        else
        {
            field_text_reset_scratch();
            func_800AB690((void *)context_or_delay);
            func_80063194();
            return;
        }
        break;
    case 7:
        if (func_800AB86C((void *)context_or_delay) != 0)
        {
            func_800A39A8(0, 0x80, 0, 3);
            goto block_41;
        }
        break;
    case 8:
        if ((func_800AC768((void *)context_or_delay) != 0) && (func_800B0888() == 0))
        {
            func_800A7384();
        block_41:
            field_text_reset_windows();
            index_or_zero = 0;
            D_8012269C = 0;
            g_pad_input = 0;
        block_42:
            D_801227BC = func_800A9D70(index_or_zero);
            context_or_delay = 0xF;
            D_801227C0 = context_or_delay;
            g_pad_input_inject = 0;
            D_801227D8 = func_800A9D70(1);
            D_801227E4 = context_or_delay;
            D_801229F8 = 0;
            field_restore_fade_target();
        }
        break;
    }
}
