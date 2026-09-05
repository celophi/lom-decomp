#include "common.h"

/*
 * D_801227C8 is the active flag of the field text session that these three
 * functions open, reset and advance.
 */

/** @brief Block at 0x801ED600; only bytes 0 and 0xAE are copied here. */
typedef struct
{
    u8 unk0;
    u8 pad1[0xAE - 1];
    u8 unkAE;
} Struct_801ED600_2;

typedef struct
{
    u8 unk0;
    u8 unk1;
} D_801227B8_t;

s32 func_800A9D70(s32);
void akao_cmd_98_9a_9c_9e(s32 arg0);
void field_text_reset_scratch(void);
void field_text_reset_windows(void);
void func_80063194(void);
void func_800A3904(s32 arg0, s32 arg1, s32 arg2);
void func_800A92CC(s32 arg0);
void func_800A939C(s32 arg0);
void func_800A9B88(void);

extern D_801227B8_t D_801227B8;
extern s32 D_801227BC;
extern s32 D_801227C0;
extern s32 D_801227C8;
extern s32 D_801227D8;
extern s32 D_801227E4;
extern s32 D_8012291C;
extern s32 D_80122984;
extern s32 D_801229F8;
extern s32 g_pad_input;
extern s32 g_pad_input_inject;

/**
 * @brief Open the text session: set the active flag, start the CD-error fade, clear pad input, seed the two func_800A9D70 handles with 0xF counters, and run func_800A9198.
 * @see decomp.me (100%) TODO
 */
void func_800AA7A4(void)
{
    D_801227C8 = 1;
    field_set_cd_error_fade_target();
    g_pad_input = 0;
    D_801227BC = func_800A9D70(0);
    D_801227C0 = 0xF;
    g_pad_input_inject = 0;
    D_801227D8 = func_800A9D70(1);
    D_801227E4 = 0xF;
    D_801229F8 = 0;
    func_800A9198();
}

/**
 * @brief Clear the session flags and copy bytes 0 and 0xAE of the 0x801ED600 block into D_801227B8.
 */
void func_800AA824(void)
{
    Struct_801ED600_2 *ptr = (Struct_801ED600_2 *)0x801ED600;

    D_801227C8 = 0;
    D_8012291C = 0;
    D_801227B8.unk0 = ptr->unk0;
    D_801227B8.unk1 = ptr->unkAE;
}

/**
 * @brief Tear down or advance the active field text window on a gated event.
 *
 * When the text subsystem is active (@c D_801227C8), runs func_800A9B88 and, if
 * still active, resets the scratch buffer and dispatches the per-mode advance
 * (@c func_800A92CC / @c func_800A939C selected by @c D_80122984). If the pass
 * cleared the active flag it instead resets the windows, stops the sequence
 * (@c akao_cmd_98_9a_9c_9e), and kicks off the close fade.
 *
 * @param arg0 Mode parameter forwarded to the advance dispatch.
 * @see decomp.me (100%) TODO
 */
void func_800AA858(s32 arg0)
{
    if (D_801227C8 != 0)
    {
        func_800A9B88();
        if (D_801227C8 != 0)
        {
            field_text_reset_scratch();
            if (D_80122984 != 0)
            {
                func_800A92CC(arg0);
            }
            else
            {
                func_800A939C(arg0);
            }
            func_80063194();
            return;
        }
        field_text_reset_windows();
        akao_cmd_98_9a_9c_9e(2);
        func_800A3904(0, 0x3C, 0x7F);
    }
}
