
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned char undefined;
typedef unsigned char undefined1;
typedef unsigned short undefined2;
typedef unsigned int undefined4;
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef struct
{
    short x;
    short y;
    short w;
    short h;
} RECT;
typedef struct
{
    u_long tag;
    u_long code[15];
} DR_ENV;
typedef struct
{
    RECT clip;
    short ofs[2];
    RECT tw;
    u_short tpage;
    u_char dtd;
    u_char dfe;
    u_char isbg;
    u_char r0;
    u_char g0;
    u_char b0;
    DR_ENV dr_env;
} DRAWENV;
typedef struct
{
    RECT disp;
    RECT screen;
    u_char isinter;
    u_char isrgb24;
    u_char pad0;
    u_char pad1;
} DISPENV;
extern int ClearImage(RECT* rect, u_char r, u_char g, u_char b);
extern int DrawSync(int mode);
extern void VSync(int mode);
extern void FUN_8002279c(undefined4 param_1, u_int param_2);
extern void FUN_80022aa8(void);
extern void FUN_80022ac8(void);
extern DISPENV* SetDefDispEnv(DISPENV* env, int x, int y, int w, int h);
extern DRAWENV* SetDefDrawEnv(DRAWENV* env, int x, int y, int w, int h);
extern void func_800224D8(s32);
extern s32 func_800A368C(s32, s32);
extern s32 func_800A380C(void);
extern s32 func_800A39A8(s32, s32, s32, s32);
extern s32 func_801401F0(void);
extern s32 func_80140538(s32, s16*, s32);
extern s32 func_80140648(s32);
extern s32 D_8011588C;
extern s32 D_80140708;
extern s32 D_80141048;
extern u8 D_801407A0[];

/**
 * decomp.me link (100%) https://decomp.me/scratch/1qYnn
 */
void func_80140004(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    RECT rect;
    u8* base;
    u8* new_var;
    u16* second;
    u8(*new_var2)[];
    new_var2 = &D_801407A0;
    VSync(0);
    DrawSync(0);
    base = *new_var2;
    new_var = base + 0x49C;
    *((u16*)(base + 0)) = 0;
    *((u16*)(base + 2)) = 0;
    *((u16*)(base + 4)) = 0x140;
    *((u16*)(base + 6)) = 0xF0;
    second = (u16*)new_var;
    second[0] = 0;
    second[1] = 0xE8;
    second[2] = 0x140;
    second[3] = 0xF0;
    rect.x = 0;
    rect.y = 0;
    rect.w = 0x400;
    rect.h = 0x200;
    ClearImage(&rect, 0, 0, 0);
    SetDefDispEnv((DISPENV*)(base - 0x70), 0, 0, 0x140, 0xF0);
    SetDefDispEnv((DISPENV*)(base + 0x42C), 0, 0xE8, 0x140, 0xF0);
    SetDefDrawEnv((DRAWENV*)(base - 0x5C), 0, 0xF0, 0x140, 0xE0);
    SetDefDrawEnv((DRAWENV*)(base + 0x440), 0, 8, 0x140, 0xE0);
    base = base - 0x90;
    *((u8*)(base + 0x4E6)) = 0;
    *((u8*)(base + 0x4A)) = 0;
    rect.x = 0x140;
    rect.y = 0;
    rect.w = 0;
    rect.h = 0x1E0;
    base = base - 0x90;
    func_80140538(arg1 + 0xFFC, (s16*)(&rect), arg0);
    FUN_80022aa8();
    FUN_80022ac8();
    func_800224D8(0x7F);
    if (arg3 != (-1))
    {
        func_80140648(arg3);
        func_800A39A8(0, 0x80, 0, 0);
    }
    if (arg2 != (-1))
    {
        func_800A368C(arg2, 0);
        D_8011588C = 0x7F;
        func_800A380C();
        FUN_8002279c(0, 0x7F);
    }
    D_80141048 = 4;
    D_80140708 = 4;
    func_801401F0();
}