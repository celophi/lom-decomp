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
void func_80140114(s32*); /* extern */
void func_80140C18();    /* extern */
void func_80140DDC();    /* extern */
void func_80141024();    /* extern */
void func_801411E8();    /* extern */
void func_801413C0();    /* extern */
void func_80141580();    /* extern */
void func_801416E0();    /* extern */
void func_80141808();    /* extern */
void func_8014191C();    /* extern */
void func_80141A14();    /* extern */
void func_80141B28();    /* extern */
void func_80141C3C();    /* extern */
void func_80141ED8();    /* extern */
void func_8014229C();    /* extern */
void func_80142398();    /* extern */
void func_80142400();    /* extern */
void func_80142460();    /* extern */
void func_80142610();    /* extern */
void func_80142820();    /* extern */
void func_8014289C();    /* extern */
void func_80142B98();    /* extern */
void func_80142C64();    /* extern */
void func_80143054();    /* extern */
void func_80143B64();    /* extern */
void func_80145CEC();    /* extern */
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
extern u8 D_80142B18;
extern u8* D_8012271C;
extern s32 D_8014F29C;
extern s32 D_8016B8D4;
extern s32 D_8016B8D8;
extern s32 D_8016B8E4;
extern s32 D_8016B8EC;
extern s32 D_8016B8F0;
extern void* D_8016B8F8;
extern u8 D_8016B8FC;
extern u8 D_8016B8FD;
extern s32 D_8016B900;
extern s32 D_8016B950;
extern void* D_8016B954;
extern s32 D_8016B958;
extern s32 D_8017097C;
extern s32 D_80170980;

/**
 * @brief Resolve the message-archive entry at @p off and hand it to func_80145CEC.
 *
 * D_8014F29C is a self-relative offset word inside the archive header that
 * starts 0x20 bytes earlier; adding it to the header base yields the block
 * whose u16 at @p off is the entry's offset from that same base.
 *
 * @param off Byte offset of the u16 entry index within the resolved block.
 */
#define GOSUB_MSG(off) \
    func_80145CEC((u8*)&D_8014F29C - 0x20 + D_8014F29C + *(u16*)((u8*)&D_8014F29C + D_8014F29C + (off)))

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

/**
 * @brief Enter one of the 20 gosub sub-screens: reset the shared state, install
 *        the screen's update/draw handlers, and queue its intro message.
 *
 * Every arm zeroes the common state block, runs the screen's own setup helper,
 * publishes an update handler in D_8016B954 and a draw handler in D_8016B8F8,
 * then calls the screen's enter routine. Unless that routine reported a failure
 * (D_8016B8D4, or the save-slot count at D_8012271C[0x29D6] for arms 10 and 11)
 * the arm finishes by submitting its message through GOSUB_MSG.
 *
 * @param arg0 Screen id, 0..19; anything else returns without touching state.
 * @param arg1 Unused by this function; passed by func_80140114.
 */
void func_8014020C(s32 arg0, s32 arg1)
{
    D_8016B8E0 = 0;
    D_80170990 = 0;
    D_80170988 = 0;
    D_8016B8D0 = 0;
    D_8016B8E4 = 0;
    D_8017097C = 0;
    D_8016B8EC = 0;
    D_8016B8F0 = 0;
    D_8016B900 = 0;

    switch (arg0)
    {
    case 0:
        func_80143B64();
        func_80141B28();
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_801411E8(1);
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(8);
        }
        break;

    case 1:
        func_80143B64();
        func_80141A14();
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_801411E8(1);
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0xA);
        }
        break;

    case 2:
        func_80143B64();
        func_80142C64(0);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141024();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(2);
        }
        break;

    case 3:
        func_80143B64();
        func_80142C64(1);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141024();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(4);
        }
        break;

    case 4:
        func_80143B64();
        func_80142C64(2);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141024();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(6);
        }
        break;

    case 5:
        func_80143B64();
        func_80142C64(3);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141024();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0);
        }
        break;

    case 6:
    case 7:
    case 8:
        func_80143B64();
        func_80143054(arg0 - 6);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_801411E8(1);
        break;

    case 9:
        func_80143B64();
        func_80142C64(4);
        D_8016B8FD = 4;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142610;
        D_8016B8F8 = (void*)&D_80142B18;
        func_80140C18();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(-2);
        }
        break;

    case 10:
        func_80143B64();
        func_80142C64(3);
        D_8016B900 = 1;
        D_8016B8D8 = 6;
        D_80170980 = 0x10;
        D_8016B958 = 0xE8;
        D_8016B950 = 0x64;
        D_8016B8E4 = 0;
        D_8017097C = 0;
        D_8016B8EC = 0;
        D_8016B8FD = 2;
        D_8016B8FC = 2;
        D_8016B954 = (void*)func_80142400;
        D_8016B8F8 = (void*)func_80142820;
        D_80174A58 = (void*)func_8014289C;
        func_80140DDC();
        if (D_8012271C[0x29D6] >= 0x28)
        {
            func_80143B64();
            GOSUB_MSG(-4);
        }
        break;

    case 11:
        func_80143B64();
        func_80141C3C();
        D_8016B8FD = 2;
        D_8016B8FC = 2;
        D_8016B954 = (void*)func_80142460;
        D_8016B8F8 = (void*)func_80142B98;
        D_8016B8F0 = 1;
        func_801413C0();
        if (D_8012271C[0x29D6] == 0)
        {
            func_80143B64();
            GOSUB_MSG(-6);
        }
        break;

    case 12:
        func_80143B64();
        func_80141ED8(0);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_8014229C;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141580();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x32);
        }
        break;

    case 13:
        func_80143B64();
        func_80141ED8(1);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_8014229C;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141580();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x34);
        }
        break;

    case 14:
        func_80143B64();
        func_80141ED8(2);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141580();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x36);
        }
        break;

    case 15:
        func_80143B64();
        func_801416E0();
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_801411E8(1);
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x3E);
        }
        break;

    case 16:
        func_80143B64();
        func_8014191C();
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_801411E8(0);
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x3C);
        }
        break;

    case 17:
        func_80143B64();
        func_80141ED8(0);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141580();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x32);
        }
        break;

    case 18:
        func_80143B64();
        func_80141ED8(1);
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_80141580();
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x34);
        }
        break;

    case 19:
        func_80143B64();
        func_80141808();
        D_8016B8FD = 1;
        D_8016B8FC = 1;
        D_8016B954 = (void*)func_80142398;
        D_8016B8F8 = (void*)func_80142B98;
        func_801411E8(1);
        if (D_8016B8D4 == 0)
        {
            func_80143B64();
            GOSUB_MSG(0x4C);
        }
        break;
    }
}
