#include "common.h"

typedef struct
{
    s16 red;
    s16 green;
    s16 blue;
    s16 duration;
} FieldFade;

extern s32 D_801178B8;
extern FieldFade g_field_fade_target;
extern u16 D_800EB2D4[];

void func_8009C56C(void)
{
    s32 i;
    u16 *p;

    D_801178B8 = 0;
    if (g_field_fade_target.red == 0x1FF &&
        g_field_fade_target.green == g_field_fade_target.red &&
        g_field_fade_target.blue == g_field_fade_target.green)
    {
        p = D_800EB2D4;
        for (i = 0; i < 0x800; i++, p++)
        {
            if ((*p & 0x7FFF) == 0)
                *p = 0xFFFF;
        }
    }
    else
    {
        p = D_800EB2D4;
        for (i = 0; i < 0x800; i++, p++)
        {
            if (*p == 0xFFFF)
                *p = 0;
        }
    }
}
