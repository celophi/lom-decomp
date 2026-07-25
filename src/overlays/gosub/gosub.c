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

void func_80067F8C();    /* extern */
void func_800AA02C();    /* extern */
void func_80140114(s32); /* extern */
extern s32 D_8016B8C8;
extern s32 D_8016B8CC;
extern s32 D_8016B8E0;
extern s32 D_80170990;
extern s32 D_80170988;
extern s32 D_8016B8D0;
extern s32 D_8017098C;
extern u8 D_8016B8DC;
extern s32 D_801228F0;
extern u8 D_80145744;  
extern void* D_80174A58;    
extern s32 D_8016B948;
extern u8 D_80170968[];    

/**
 * decomp.me (100%) https://decomp.me/scratch/qM81L
 */
void func_80140080(s32 arg1, s32 arg2)
{
    func_80067F8C();
    D_8016B8C8 = 0;
    D_8016B8CC = 0;
    func_800AA02C();
    func_80140114(arg2);
}

/**
 * decomp.me (100%) https://decomp.me/scratch/ykfW4
 */
s32 func_801400C4(s32 arg0)
{
    s32* ptr;
    s32 ret;
    func_8006441C();
    func_80143258(arg0);
    func_80063194();
    ptr = &D_8016B8C8;
    ret = D_8016B8CC;
    *ptr ^= 1;
    return ret;
}

/**
 * decomp.me (100%) https://decomp.me/scratch/weBhP
 */
void func_80140114(s32* arg0)
{
    u8* var_a0;
    s32 var_a1;
    u8 temp_v0;
    s32 pad[2];

    func_801465BC();
    D_8016B8E0 = 0;
    D_80170990 = 0;
    D_80170988 = 0;
    D_8016B8D0 = 0;
    func_80143BD0();
    D_8017098C = 0;
    D_8016B8DC = 0;
    D_801228F0 = 0;
    D_80174A58 = (void*)&D_80145744;
    var_a1 = 0;
    if (*arg0 != 0xFE)
    {
        u8* arr = D_80170968;
        s32 sentinel = 0xFE;
        var_a0 = (u8*)arg0;
        do
        {
            temp_v0 = *var_a0;
            var_a0 += 4;
            *((u8*)(var_a1 + (u32)arr)) = temp_v0;
            var_a1++;
        } while (*(s32*)var_a0 != sentinel);
    }
    D_80170968[var_a1] = ((u8*)arg0)[var_a1 * 4];
    D_8016B948 = 0;
    func_8014020C(D_80170968[D_8016B8DC], var_a1);
    func_80146DA8();
}