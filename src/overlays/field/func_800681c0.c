#include "common.h"

void field_open_return_to_title_prompt(void);

/**
 * @brief Open the return-to-title confirmation prompt if arg0 is zero.
 * @param arg0 When 0, opens the prompt; otherwise returns immediately.
 * @see decomp.me (100%) TODO
 */
void func_800681C0(s32 arg0) {
    if (arg0 == 0) {
        field_open_return_to_title_prompt();
    }
}

typedef struct
{
    s16 red;                // 0x00
    s16 green;              // 0x02
    s16 blue;               // 0x04
    s16 duration;           // 0x06
} FieldFadeTarget;

typedef struct
{
    s16 red;                // 0x00
    s16 green;              // 0x02
    s16 blue;               // 0x04
    s16 unk6;               // 0x06
} FieldFadeRestoreColor;

extern FieldFadeTarget g_field_fade_target;
extern FieldFadeRestoreColor g_field_fade_restore_color;
extern s32 D_800F22B8;
extern s32 D_800F22BC;
extern s32 D_800F22C0;
extern s32 D_800F22C4;

void func_80084240(void);
void cdrom_queue_seek(s32);
void akao_cmd_c1(s32, s32, s32);
void akao_cmd_a9(s32, s32);

/**
 * @see decomp.me (100%) TODO
 */
void func_800681E4(s32 arg0, s32 arg1, s32 arg2)
{
    s32 fade_time;

    if (D_800F22C0 == 0)
    {
        D_800F22B8 = arg0;
        D_800F22BC = arg1;
        D_800F22C4 = arg2;
        func_80084240();
        g_field_fade_target.red = 0;
        g_field_fade_restore_color.red = 0;
        g_field_fade_target.green = 0;
        g_field_fade_restore_color.green = 0;
        g_field_fade_target.blue = 0;
        g_field_fade_restore_color.blue = 0;
        fade_time = 0x3C;
        g_field_fade_target.duration = fade_time;
        cdrom_queue_seek(0xA);
        D_800F22C0 = fade_time;
        akao_cmd_c1(0, 0x78, 0);
        akao_cmd_a9(0x78, 0);
    }
}
