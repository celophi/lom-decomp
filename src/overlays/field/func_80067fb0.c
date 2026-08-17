#include "common.h"

void func_800A710C(void);
void func_8006441C(void);
void func_800A8880(s32);
void func_80063194(void);

extern s32 D_800F229C[];

/**
 * @see decomp.me (100%) TODO
 */
void func_80067FB0(s32 arg0)
{
    if (D_800F229C[0] != 0)
    {
        func_800A710C();
        if (D_800F229C[0] != 0)
        {
            func_8006441C();
            if (D_800F229C[0] != 0)
            {
                func_800A8880(arg0);
            }
            func_80063194();
        }
    }
}

typedef struct
{
    s16 r;                  // 0x00
    s16 g;                  // 0x02
    s16 b;                  // 0x04
    s16 steps;              // 0x06
} FieldFade;

void func_800AA02C(void);
void func_800A9A5C(void);
void func_800A68B4(void);
void func_800A7434(void);
void func_800A74B8(void);

extern FieldFade g_field_fade_target;
extern s32 D_8010D020[];

/**
 * @see decomp.me (100%) TODO
 */
void func_80068028(void)
{
    func_800AA02C();
    g_field_fade_target.r = 0xC0;
    g_field_fade_target.g = 0xC0;
    g_field_fade_target.b = 0xC0;
    g_field_fade_target.steps = 5;
    if (D_8010D020[0] == 0)
    {
        func_800A9A5C();
        func_800A68B4();
        func_800A7434();
    }
    else
    {
        func_800A74B8();
    }
}

typedef struct
{
    s16 red;                // 0x00
    s16 green;              // 0x02
    s16 blue;               // 0x04
    s16 unk6;               // 0x06
} FieldFadeRestoreColor;

typedef struct
{
    u8 pad0[0x1C];
    s32 unk1C;              // 0x1C
    u8 pad20[0x21 - 0x20];
    u8 unk21;               // 0x21
    u8 pad22[0x24 - 0x22];
    u8 unk24;               // 0x24
    u8 pad25[0x27 - 0x25];
    u8 unk27;               // 0x27
    u8 pad28[0x2A - 0x28];
    s16 unk2A;              // 0x2A
    u8 pad2C[0x2E - 0x2C];
    s16 unk2E;              // 0x2E
    u8 pad30[0x54 - 0x30];
} Struct_D800FDF58;

typedef struct
{
    u8 pad0[0x174];
    s32 unk174;             // 0x174
    u8 pad178[0x23C - 0x178];
} Struct_D80105AE0;

typedef struct
{
    u8 unk0;                // 0x00
    u8 pad1[0x268 - 1];
} D_800FD818_type;

extern FieldFadeRestoreColor g_field_fade_restore_color;
extern Struct_D800FDF58 D_800FDF58[];
extern Struct_D80105AE0 D_80105AE0[];
extern D_800FD818_type D_800FD818[];
extern s32 D_8012291C;

void func_800643E0(void);
void func_8006C3FC(Struct_D800FDF58*, void*);
void field_restore_default_action_animation_mappings(s32);
void func_80092124(void);

/**
 * @see decomp.me (100%) TODO
 */
void func_8006809C(void)
{
    s32 i;

    g_field_fade_target.r = g_field_fade_restore_color.red;
    g_field_fade_target.g = g_field_fade_restore_color.green;
    g_field_fade_target.b = g_field_fade_restore_color.blue;
    g_field_fade_target.steps = 5;
    func_800643E0();

    D_800F229C[0] = 0;

    for (i = 0; i < 2; i++)
    {
        if (D_800FD818[i].unk0 & 1)
        {
            D_800FDF58[i].unk2A = 0x9A;
            D_800FDF58[i].unk2E = 1;
            D_800FDF58[i].unk27 = 0;
            D_800FDF58[i].unk24 = 1;
            D_800FDF58[i].unk1C &= ~0x1FF;
            D_800FDF58[i].unk21 = (D_800FDF58[i].unk21 & 0x80) + 0x12;
            D_80105AE0[i].unk174 &= ~0x1800;
            func_8006C3FC(&D_800FDF58[i], (void*)~0x1FF);
        }
    }

    field_restore_default_action_animation_mappings(0);
    D_8012291C = 0;
    func_80092124();
}
