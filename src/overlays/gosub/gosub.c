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

void field_set_default_fade_target(); /* extern */
void func_800A8B90();    /* extern */
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
s32 func_8014229C();    /* extern */
s32 func_80142398();    /* extern */
s32 func_80142400();    /* extern */
s32 func_80142460();    /* extern */
s32 func_80142610();    /* extern */
s32 func_80142820();    /* extern */
void func_8014289C();    /* extern */
void func_80142B98();    /* extern */
void func_80142C64();    /* extern */
void func_80143054();    /* extern */
void func_80143B64();    /* extern */
void func_80145CEC();    /* extern */
void func_80146468();    /* extern */
void func_80146538();    /* extern */
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
extern s32 D_8016B95C;
extern u8 D_8016B94C[2];
extern s32 D_8016B908[];
extern s32 D_801229B0[];
extern s32 D_8017097C;
extern s32 D_80170980;
extern u8 D_801448EC;
extern u8 D_801452F0;
extern u8 D_80145DF8;
extern u8 D_80145EA4;
extern u8 D_801460D0;
extern u8 D_80145F80;
extern u8 D_80146418;
extern u8 D_80170960;
extern s32 D_8016B8E8;
extern u8 D_800EC3DA[];
extern u32 D_8014F27C[];
extern u32 D_8014F280[];
extern u32 D_8014F294[];
extern u8 D_8016B960[];
extern u8 D_8016B5AC[];
extern u8* D_80170994;

/**
 * @brief One entry in the gosub screen's element list, as handed out by
 *        func_80143C04.
 *
 * The first word packs several bitfields plus a top byte that the code always
 * rewrites through the whole word (see SET_ELEM_CODE), so it is exposed as a
 * union. The second word carries a flag and the y coordinate.
 *
 * @note Field meanings beyond x/y are unconfirmed; the `_N` suffixes record the
 *       bit position each one starts at.
 */
typedef struct
{
    union
    {
        u32 word;
        struct
        {
            u32 unk0_0 : 3;
            u32 unk0_3 : 4;
            u32 x : 9;
            u32 unk0_16 : 8;
        } f;
    } attr;
    u32 unk4_0 : 1;
    u32 y : 8;
    u32 unk4_9 : 23;
    void* handler;
} GosubElement;

/**
 * @brief Set the top byte of an element's attr word.
 *
 * Must go through the whole word rather than an 8-bit bitfield: a bitfield
 * assignment narrows to `sb` at offset 3, which is not what the game does.
 *
 * @param e Element to update.
 * @param c New top-byte value. TODO: meaning unknown; observed 0xE8 and 0x08.
 */
#define SET_ELEM_CODE(e, c) ((e)->attr.word = ((e)->attr.word & 0x00FFFFFF) | ((u32)(c) << 24))

extern GosubElement D_80170998;

GosubElement* func_80143C04();

/**
 * @brief One row of the item list built by the gosub screen builders.
 *
 * The word at 0xC packs three signed bytes below a 4-bit row kind. The simple
 * slot builders only ever write @c kind, but func_80141C3C uses all three
 * bytes, so they are declared as byte-aligned 8-bit bitfields: writes narrow
 * to @c sb and reads to @c lb, which is what the target does. The top nibble
 * @c unkC_28 is unsigned: func_80142610 reads it with a plain @c srl, where a
 * signed field would sign-extend.
 *
 * The word at 0x1C is a flag set. Most builders write single bits through the
 * @c f bitfields, but func_80141ED8 tests bit 0 through the @c half alias and
 * func_80141C3C rewrites bit 2 through @c word, so all three views are exposed
 * as a union; each spelling is the access width the game's code uses.
 *
 * @note Only the fields the builders touch are known; 0x10..0x1B is a block of
 *       u16s copied verbatim out of the source record.
 */
typedef struct
{
    u8* name;
    u8* desc;
    s16 value;
    s16 index;
    s32 unkC : 8;
    s32 unkD : 8;
    s32 unkE : 8;
    s32 kind : 4;
    u32 unkC_28 : 4;
    u16 unk10;
    u16 unk12[4];
    u16 unk1A;
    union
    {
        struct
        {
            u32 flag0 : 1;
            u32 flag1 : 1;
            u32 flag2 : 1;
            u32 unk1C_3 : 29;
        } f;
        u16 half;
        u32 word;
    } flags;
} GosubListEntry;

extern GosubListEntry D_80170A58[];

/**
 * @brief Resolve one entry of a text archive block.
 *
 * D_8014F27C heads a 13-word table of block offsets, each relative to the
 * table itself. A block begins with a u16 per entry giving that entry's offset
 * from the same table base, so a lookup is base + block + block[idx].
 *
 * @param blk Block offset word, e.g. D_8014F280[0] or D_8014F27C[12].
 * @param idx Entry index within the block.
 * @return Pointer to the entry.
 * @note The term order matters: writing the base first is what produces the
 *       target's accumulate chain (see idioms.md [EXPAND-14]).
 */
#define ARCHIVE_ENTRY(blk, idx) \
    ((u8*)D_8014F27C + (blk) + *(u16*)((u8*)D_8014F27C + (blk) + (idx) * 2))

/**
 * @brief Resolve a message-archive entry at byte offset @p off.
 *
 * The same lookup as ARCHIVE_ENTRY against block 8, but spelled through
 * D_8014F29C (which is that block's offset word) because that is the
 * relocation the game's code carries; do not fold it into ARCHIVE_ENTRY.
 *
 * @param off Byte offset of the u16 entry index within the resolved block.
 * @return Pointer to the entry.
 */
#define GOSUB_MSG_PTR(off) \
    ((u8*)&D_8014F29C - 0x20 + D_8014F29C + *(u16*)((u8*)&D_8014F29C + D_8014F29C + (off)))

/**
 * @brief Resolve the message-archive entry at @p off and hand it to func_80145CEC.
 *
 * @param off Byte offset of the u16 entry index within the resolved block.
 */
#define GOSUB_MSG(off) func_80145CEC(GOSUB_MSG_PTR(off))

/**
 * decomp.me (100%) https://decomp.me/scratch/qM81L
 */
void func_80140080(s32 arg1, s32 arg2)
{
    field_set_default_fade_target();
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

/**
 * @brief Build the three elements of the gosub screen entered by arm 9.
 *
 * Each element is allocated by func_80143C04, given its draw handler, and
 * positioned. The first is centred horizontally against the current window
 * width in D_8016B958 and stacked vertically by D_80170980 * D_8016B8D8; the
 * other two sit at a fixed x with only their attr byte differing.
 */
void func_80140C18(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - D_8016B958 / 2;
    p->attr.f.unk0_16 = 0x38;
    p->unk4_0 = 0;
    p->y = D_80170980 * D_8016B8D8 + 4;
    SET_ELEM_CODE(p, 0xE8);
    D_80170960 = 0;

    p = func_80143C04();
    p->handler = (void*)&D_80145EA4;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->handler = (void*)&D_801460D0;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0xB0;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the three elements of the gosub screen entered by arm 10.
 *
 * Same layout as func_80140C18 - the first element is centred against the
 * window width in D_8016B958 and stacked by D_80170980 * D_8016B8D8, the other
 * two sit at a fixed x - but with a different set of draw handlers and attr
 * bytes.
 */
void func_80140DDC(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - D_8016B958 / 2;
    p->attr.f.unk0_16 = 0x48;
    p->unk4_0 = 0;
    p->y = D_80170980 * D_8016B8D8 + 4;
    SET_ELEM_CODE(p, 0xE8);
    D_80170960 = 0;

    p = func_80143C04();
    p->handler = (void*)&D_80145DF8;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0xB0;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->handler = (void*)&D_801452F0;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x20;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Configure the fixed element D_80170998 and clear D_8016B8E8.
 *
 * Unlike func_80140C18 / func_80140DDC this one does not allocate: it reuses a
 * single statically allocated element, so it needs no frame. Its attr top byte
 * is cleared rather than set to a handler code.
 */
void func_80140FA0(void)
{
    GosubElement* p;

    p = &D_80170998;
    p->handler = (void*)&D_80145F80;
    D_8016B8E8 = 0;
    p->attr.f.unk0_0 = 1;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x20;
    p->attr.f.unk0_16 = 0x70;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 0);
}

/**
 * @brief Build the three elements of the gosub screens entered by arms 2 to 5.
 *
 * Same layout as func_80140C18 and func_80140DDC, with its own handlers and
 * attr bytes; the third element also sits higher up the screen than in the
 * other two variants.
 */
void func_80141024(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - D_8016B958 / 2;
    p->attr.f.unk0_16 = 0x28;
    p->unk4_0 = 0;
    p->y = D_80170980 * D_8016B8D8 + 4;
    SET_ELEM_CODE(p, 0xE8);
    D_80170960 = 0;

    p = func_80143C04();
    p->handler = (void*)&D_801460D0;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0xB0;
    p->unk4_0 = 1;
    p->y = 0x24;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->handler = (void*)&D_80146418;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the elements of the gosub screens entered by arms 0, 1, 6-8,
 *        15, 16 and 19.
 *
 * Same layout as func_80141024, except the middle element is optional: callers
 * pass zero to get just the header and footer elements.
 *
 * @param arg0 Non-zero to include the middle element, zero to skip it.
 */
void func_801411E8(s32 arg0)
{
    GosubElement* p;

    p = func_80143C04();
    p->handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - D_8016B958 / 2;
    p->attr.f.unk0_16 = 0x28;
    p->unk4_0 = 0;
    p->y = D_80170980 * D_8016B8D8 + 4;
    SET_ELEM_CODE(p, 0xE8);
    D_80170960 = 0;

    if (arg0 != 0)
    {
        p = func_80143C04();
        p->handler = (void*)&D_801460D0;
        p->attr.f.unk0_3 = 1;
        p->attr.f.x = 0x1C;
        p->attr.f.unk0_16 = 0xB0;
        p->unk4_0 = 1;
        p->y = 0x14;
        SET_ELEM_CODE(p, 8);
    }

    p = func_80143C04();
    p->handler = (void*)&D_80146418;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the three elements of the gosub screen entered by arm 11.
 *
 * Same layout as func_80141024, but the first element has its unk4_0 flag set
 * rather than cleared and takes attr code 0x20 instead of 0xE8.
 */
void func_801413C0(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - D_8016B958 / 2;
    p->attr.f.unk0_16 = 0x28;
    p->unk4_0 = 1;
    p->y = D_80170980 * D_8016B8D8 + 4;
    SET_ELEM_CODE(p, 0x20);
    D_80170960 = 0;

    p = func_80143C04();
    p->handler = (void*)&D_801460D0;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0xB0;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);

    p = func_80143C04();
    p->handler = (void*)&D_80146418;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the two elements of the gosub screens entered by arms 12-14 and
 *        17-18.
 *
 * The shortest member of this family: no middle element, so only a header
 * centred against D_8016B958 and a footer at a fixed x.
 */
void func_80141580(void)
{
    GosubElement* p;

    p = func_80143C04();
    p->handler = (void*)&D_801448EC;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0xA0 - D_8016B958 / 2;
    p->attr.f.unk0_16 = 0x30;
    p->unk4_0 = 1;
    p->y = D_80170980 * D_8016B8D8 + 4;
    SET_ELEM_CODE(p, 0x18);
    D_80170960 = 0;

    p = func_80143C04();
    p->handler = (void*)&D_80146418;
    p->attr.f.unk0_3 = 1;
    p->attr.f.x = 0x1C;
    p->attr.f.unk0_16 = 0x10;
    p->unk4_0 = 1;
    p->y = 0x14;
    SET_ELEM_CODE(p, 8);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 15.
 *
 * Walks the player's item slots at D_8012271C[0x25E0 + 0x60 .. 0x84] and emits
 * one D_80170A58 row per non-empty slot, resolving the item's two archive
 * strings, recording the slot's count and index, and tagging the row kind. The
 * number of rows lands in D_8016B8D4, and the screen's title message pointer in
 * D_80170994.
 */
void func_801416E0(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x85; i++)
    {
        if (*(D_8012271C + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &D_80170A58[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->desc = ARCHIVE_ENTRY(D_8014F27C[12], D_8016B5AC[i]);
            e->value = *(D_8012271C + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    D_8016B8D4 = count;
    D_8016B8D8 = 8;
    D_80170980 = 0x10;
    D_8016B958 = 0xE8;
    D_8016B950 = 0x84;
    D_80170994 = GOSUB_MSG_PTR(0x3A);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 19.
 *
 * Same row layout as func_801416E0, over the adjacent slot range
 * D_8012271C[0x25E0 + 0x60 .. 0x8F]. Both archive strings for a row come from
 * blocks indexed directly by the slot number, so no D_8016B5AC indirection is
 * needed for the description. The row count lands in D_8016B8D4 and the
 * screen's title message pointer in D_80170994.
 */
void func_80141808(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x60; i < 0x90; i++)
    {
        if (*(D_8012271C + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &D_80170A58[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->desc = ARCHIVE_ENTRY(D_8014F27C[2], i);
            e->value = *(D_8012271C + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    D_8016B8D4 = count;
    D_8016B8D8 = 8;
    D_80170980 = 0x10;
    D_8016B958 = 0xE8;
    D_8016B950 = 0x84;
    D_80170994 = GOSUB_MSG_PTR(0x4A);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 16.
 *
 * Same row layout as func_801416E0 over slots D_8012271C[0x25E0 + 0x40 .. 0x4F],
 * but these rows carry no description string: only the name, the slot's count,
 * its index, and the row kind are written. The row count lands in D_8016B8D4
 * and the screen's title message pointer in D_80170994.
 */
void func_8014191C(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0x50; i++)
    {
        if (*(D_8012271C + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &D_80170A58[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->value = *(D_8012271C + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    D_8016B8D4 = count;
    D_8016B8D8 = 8;
    D_80170980 = 0x10;
    D_8016B958 = 0xE8;
    D_8016B950 = 0x84;
    D_80170994 = GOSUB_MSG_PTR(0x38);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 1.
 *
 * Identical row layout and archive blocks to func_80141808, over the widest
 * slot range in the family: D_8012271C[0x25E0 + 0x40 .. 0xFE]. The row count
 * lands in D_8016B8D4 and the screen's title message pointer in D_80170994.
 */
void func_80141A14(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0x40; i < 0xFF; i++)
    {
        if (*(D_8012271C + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &D_80170A58[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->desc = ARCHIVE_ENTRY(D_8014F27C[2], i);
            e->value = *(D_8012271C + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    D_8016B8D4 = count;
    D_8016B8D8 = 8;
    D_80170980 = 0x10;
    D_8016B958 = 0xE8;
    D_8016B950 = 0x84;
    D_80170994 = GOSUB_MSG_PTR(0x14);
}

/**
 * @brief Build the item list for the gosub screen entered by arm 0.
 *
 * Identical row layout and archive blocks to func_80141808, over the slot
 * range D_8012271C[0x25E0 + 0x00 .. 0x3F] -- the block immediately below the
 * one func_80141A14 walks. The row count lands in D_8016B8D4 and the screen's
 * title message pointer in D_80170994.
 */
void func_80141B28(void)
{
    s32 i;
    s32 count;

    count = 0;
    for (i = 0; i < 0x40; i++)
    {
        if (*(D_8012271C + i + 0x25E0) != 0)
        {
            GosubListEntry* e = &D_80170A58[count];
            e->name = ARCHIVE_ENTRY(D_8014F280[0], i);
            e->desc = ARCHIVE_ENTRY(D_8014F27C[2], i);
            e->value = *(D_8012271C + i + 0x25E0);
            e->kind = 4;
            e->index = i;
            count++;
        }
    }
    D_8016B8D4 = count;
    D_8016B8D8 = 8;
    D_80170980 = 0x10;
    D_8016B958 = 0xE8;
    D_8016B950 = 0x84;
    D_80170994 = GOSUB_MSG_PTR(0x12);
}

/**
 * @brief Build the equipment list for the gosub screen entered by arm 17.
 *
 * Unlike the slot builders above, every record produces a row: the count comes
 * from D_8012271C[0x29D6] and each record is a packed word at
 * D_8012271C[0x29DC + i * 4]. Three fields are unpacked out of it into the
 * row's byte fields, and the row's name is composed into its own 0x50-byte
 * scratch buffer in D_8016B960 -- the archive string, then optionally a
 * separator and the decimal form of the unkC field appended after it.
 *
 * @note Bit 2 of the flag word is cleared only for records that are both unflagged at
 *       bit 16 and have both low bits set; every other record sets it.
 */
void func_80141C3C(void)
{
    s32 i;
    u8* buf;
    u8 tmp[32];

    for (i = 0; i < *(D_8012271C + 0x29D6); i++)
    {
        D_80170A58[i].unkE = *(D_8012271C + (i << 2) + 0x29DC) >> 2;
        D_80170A58[i].unkC = (*(u32*)(D_8012271C + (i << 2) + 0x29DC) >> 8) & 0xF;
        buf = D_8016B960 + i * 0x50;
        func_80146538(buf, ARCHIVE_ENTRY(D_8014F294[0], D_80170A58[i].unkE));
        if (D_80170A58[i].unkC != 0)
        {
            func_80146468(buf, D_800EC3DA - 0x16 + D_800EC3DA[0] + (D_800EC3DA[1] << 8));
            func_800A8B90(tmp, D_80170A58[i].unkC, 1);
            func_80146468(buf, tmp);
        }
        D_80170A58[i].name = buf;
        D_80170A58[i].desc = ARCHIVE_ENTRY(D_8014F27C[7], D_80170A58[i].unkE);
        D_80170A58[i].value = -2;
        D_80170A58[i].unkD = (*(u32*)(D_8012271C + (i << 2) + 0x29DC) >> 12) & 0xF;
        if (((*(u32*)(D_8012271C + (i << 2) + 0x29DC) >> 16) & 1) != 0 ||
            (*(u32*)(D_8012271C + (i << 2) + 0x29DC) & 3) != 3)
        {
            D_80170A58[i].flags.word |= 4;
        }
        else
        {
            D_80170A58[i].flags.word &= ~4;
        }
        D_80170A58[i].index = i;
        D_80170A58[i].kind = 4;
    }
    D_8016B8D4 = *(D_8012271C + 0x29D6);
    D_8016B8D8 = 4;
    D_80170980 = 0x20;
    D_8016B958 = 0x120;
    D_8016B950 = 0x84;
    D_80170994 = GOSUB_MSG_PTR(0x18);
}

/**
 * @brief Build the party/character list for the gosub screen entered by arm 18.
 *
 * Two record sources feed the same D_80170A58 row array, and @p mode selects
 * which of them contribute: mode 1 suppresses the first block, mode 2
 * suppresses the second, and any other value emits both back to back.
 *
 * The first block walks the three kind bytes at D_8012271C[0x29D8]. A byte
 * below 3 selects a 332-byte record at D_8012271C[0x2B0C], whose name string,
 * two packed bytes and five u16s are copied into the row; the row is marked
 * current when the kind matches the selection byte at D_8012271C[0x29D7]. In
 * mode 0 the row's index keeps only bit 7 of the kind instead of the kind
 * itself.
 *
 * The second block walks five 0x60-byte slots at D_8012271C[0x2EF4], skipping
 * any whose first byte (the name string's first character) is zero. Each row
 * takes its name straight from the slot and its unkC/unkD from the slot's byte
 * fields, unless bit 31 of the slot word at +0x44 is set: those rows bias unkC
 * by 0x48 and derive unkD from three ranges of the u16 at +0x42. The row is
 * marked current when the slot index matches the selection word at
 * D_8012271C[0x2EF0].
 *
 * @param mode Which blocks to emit: 1 = second only, 2 = first only, otherwise
 *             both. Also picks the screen's title message.
 * @note @c tmp is reserved but never written; the sibling builders use their
 *       scratch buffer to compose names, and this one has nothing to compose.
 */
void func_80141ED8(s32 mode)
{
    s32 i;
    s32 j;
    s32 count;
    s32 kind;
    s32 off;
    s32 slot;
    s32 reload_off;
    u8 tmp[32];

    count = 0;
    if (mode != 1)
    {
        for (i = 0; i < 3; i++)
        {
            kind = *(D_8012271C + i + 0x29D8);
            if (kind < 3)
            {
                if (mode == 0)
                {
                    D_80170A58[count].index = kind & 0x80;
                }
                else
                {
                    D_80170A58[count].index = kind;
                }
                D_80170A58[count].value = -3;
                D_80170A58[count].flags.f.flag2 = 0;
                if (*(s8*)(D_8012271C + 0x29D7) == kind)
                {
                    D_80170A58[count].unkE = 1;
                }
                else
                {
                    D_80170A58[count].unkE = 0;
                }
                D_80170A58[count].flags.f.flag0 = 0;
                D_80170A58[count].flags.f.flag1 = 0;
                D_80170A58[count].kind = 4;
                off = kind * 332;
                D_80170A58[count].name = D_8012271C + 0x2B0C + off;
                D_80170A58[count].unkC = *(D_8012271C + off + 0x2B50) & 0xF;
                D_80170A58[count].unkD = *(D_8012271C + off + 0x2B54);
                D_80170A58[count].unk10 = *(u16*)(D_8012271C + off + 0x2B24);
                for (j = 0; j < 4; j++)
                {
                    D_80170A58[count].unk12[j] = *(u16*)(D_8012271C + off + 0x2B26 + j * 2);
                }
                reload_off = kind * 332;
                D_80170A58[count].unk1A = *(u16*)(D_8012271C + reload_off + 0x2B22);
                count++;
            }
        }
    }
    if (mode != 2)
    {
        for (slot = 0; slot < 5; slot++)
        {
            off = slot * 0x60;
            if (*(D_8012271C + off + 0x2EF4) != 0)
            {
                D_80170A58[count].index = slot;
                D_80170A58[count].value = -3;
                D_80170A58[count].flags.f.flag2 = 1;
                if (*(s32*)(D_8012271C + 0x2EF0) == slot)
                {
                    D_80170A58[count].unkE = 1;
                }
                else
                {
                    D_80170A58[count].unkE = 0;
                }
                D_80170A58[count].kind = 4;
                D_80170A58[count].name = D_8012271C + 0x2EF4 + slot * 0x60;
                D_80170A58[count].unkC = *(D_8012271C + off + 0x2F09);
                D_80170A58[count].unk10 = *(u16*)(D_8012271C + off + 0x2F12);
                D_80170A58[count].flags.f.flag0 = *(u32*)(D_8012271C + off + 0x2F38) >> 31;
                D_80170A58[count].flags.f.flag1 = (*(u32*)(D_8012271C + off + 0x2F38) >> 30) & 1;
                D_80170A58[count].unkD = *(D_8012271C + off + 0x2F0C);
                if (D_80170A58[count].flags.half & 1)
                {
                    D_80170A58[count].unkC = *(D_8012271C + off + 0x2F0A) + 0x48;
                    if (*(u16*)(D_8012271C + off + 0x2F36) < 6)
                    {
                        D_80170A58[count].unkD = 0;
                    }
                    else if (*(u16*)(D_8012271C + off + 0x2F36) < 0x1F)
                    {
                        D_80170A58[count].unkD = 1;
                    }
                    else
                    {
                        D_80170A58[count].unkD = 2;
                    }
                }
                for (j = 0; j < 4; j++)
                {
                    D_80170A58[count].unk12[j] = *(u16*)(D_8012271C + off + 0x2F14 + j * 2);
                }
                reload_off = slot * 0x60;
                D_80170A58[count].unk1A = *(u16*)(D_8012271C + reload_off + 0x2F10);
                count++;
            }
        }
    }
    D_8016B8D4 = count;
    D_8016B8D8 = 3;
    D_80170980 = 0x30;
    D_8016B958 = 0x120;
    D_8016B950 = 0x94;
    D_80170994 = GOSUB_MSG_PTR(mode * 2 + 0x2C);
}

/**
 * @brief Confirm the currently highlighted row of the gosub list.
 *
 * Rows carrying either of the two low flag bits cannot be picked: each shows
 * its own refusal message, clears D_80170960 to close the picker and raises
 * D_8016B95C, then reports failure. Otherwise, and only while D_80170960 is
 * still set, the row is appended to the running selection: its index goes to
 * D_801229B0 and the row number itself to D_8016B908, both keyed by the shared
 * write cursor D_801228F0.
 *
 * @return 0 if the row was rejected by a flag, 1 otherwise. Note that 1 is also
 *         returned when D_80170960 is clear and nothing was appended.
 */
s32 func_8014229C(void)
{
    s32 row;
    GosubListEntry* list;
    GosubListEntry* e;

    list = D_80170A58;
    row = D_8016B8D0;
    e = &list[row];
    if (e->flags.half & 1)
    {
        GOSUB_MSG(0x42);
        D_80170960 = 0;
        D_8016B95C = 1;
        return 0;
    }
    if (e->flags.f.flag1)
    {
        GOSUB_MSG(0x50);
        D_80170960 = 0;
        D_8016B95C = 1;
        return 0;
    }
    if (D_80170960 != 0)
    {
        D_801229B0[D_801228F0] = e->index;
        D_8016B908[D_801228F0] = row;
        D_801228F0++;
    }
    return 1;
}

/**
 * @brief Append the highlighted row to the selection, with no flag checks.
 *
 * The unconditional counterpart to func_8014229C: the same append -- row index
 * to D_801229B0, row number to D_8016B908, both keyed by the shared write
 * cursor D_801228F0 -- but no refusal messages, so any row may be picked. It is
 * the handler D_8016B954 is pointed at for the plain list screens.
 *
 * @return Always 1. Nothing is appended while D_80170960 is clear.
 */
s32 func_80142398(void)
{
    s32 n;

    if (D_80170960 != 0)
    {
        n = D_801228F0;
        D_801228F0 = n + 1;
        D_801229B0[n] = D_80170A58[D_8016B8D0].index;
        D_8016B908[n] = D_8016B8D0;
    }
    return 1;
}

/**
 * @brief Close the picker, or hand off to func_80142820 while it is busy.
 *
 * Runs only while the picker is open (D_80170960 non-zero). If D_8017097C is
 * still set the work is delegated to func_80142820 and its result passed
 * straight back; otherwise the picker state is stepped from 2 down to 1 and the
 * caller is told nothing happened.
 *
 * @return func_80142820's result while D_8017097C is set, 0 on every other path.
 */
s32 func_80142400(void)
{
    if (D_80170960 != 0)
    {
        if (D_8017097C == 0)
        {
            if (D_80170960 == 2)
            {
                D_80170960 = 1;
            }
            return 0;
        }
        return func_80142820();
    }
    return 0;
}

/**
 * @brief Commit a pending row move by swapping the two marked rows.
 *
 * Runs only in picker state 2. D_8016B94C holds the pair being reordered --
 * [0] is the row the move started on and [1] is where it was dropped. If they
 * differ, three things are exchanged: the rows' 4-byte records in the
 * D_8012271C[0x29DC] table (keyed by each row's index), the D_80170A58 rows
 * themselves, and finally the index fields, which must stay with the slot
 * rather than travel with the row. The picker is then closed.
 *
 * Dropping a row onto itself is not a move: func_801458A4 is rung instead and
 * the picker falls back to state 1.
 *
 * @return Always 0.
 */
s32 func_80142460(void)
{
    GosubListEntry entry_tmp;
    u32 rec_tmp;
    s32 tmp;

    if (D_80170960 == 0)
    {
        return 0;
    }
    if (D_80170960 != 2)
    {
        return 0;
    }
    if (D_8016B94C[0] != D_8016B94C[1])
    {
        func_80146AA8(&rec_tmp, D_8012271C + (D_80170A58[D_8016B94C[0]].index * 4 + 0x29DC));
        func_80146AA8(D_8012271C + (D_80170A58[D_8016B94C[0]].index * 4 + 0x29DC),
                      D_8012271C + (D_80170A58[D_8016B94C[1]].index * 4 + 0x29DC));
        func_80146AA8(D_8012271C + (D_80170A58[D_8016B94C[1]].index * 4 + 0x29DC), &rec_tmp);
        func_80146AD0(&entry_tmp, &D_80170A58[D_8016B94C[0]]);
        func_80146AD0(&D_80170A58[D_8016B94C[0]], &D_80170A58[D_8016B94C[1]]);
        func_80146AD0(&D_80170A58[D_8016B94C[1]], &entry_tmp);
        tmp = D_80170A58[D_8016B94C[0]].index;
        D_80170A58[D_8016B94C[0]].index = D_80170A58[D_8016B94C[1]].index;
        D_80170A58[D_8016B94C[1]].index = tmp;
        D_80170960 = 0;
    }
    else
    {
        func_801458A4();
        D_80170960 = 1;
    }
    return 0;
}

/**
 * @brief Recompute every row's kind for the current pick, and report when the
 *        selection is complete.
 *
 * Runs in four passes. The first resets all D_8016B8D4 rows to kind 4. The
 * second walks the D_80170960 entries already picked in D_8016B94C and tallies
 * them by their rows' top nibble: unmarked rows into @c open_count, marked rows
 * into @c marked_count. The remaining two passes promote candidate rows to kind
 * 5 -- unmarked ones once anything unmarked has been picked, marked ones once
 * exactly three marked rows have been -- with func_80142C18 deciding whether an
 * individual row qualifies.
 *
 * @return 1 when both conditions hold, in which case func_80142B98 is rung to
 *         announce the completed selection; 0 otherwise.
 */
s32 func_80142610(void)
{
    s32 i;
    s32 open_count;
    s32 marked_count;

    for (i = 0; i < D_8016B8D4; i++)
    {
        D_80170A58[i].kind = 4;
    }
    open_count = 0;
    marked_count = 0;
    for (i = 0; i < D_80170960; i++)
    {
        if (D_80170A58[D_8016B94C[i]].unkC_28 == 0)
        {
            open_count++;
        }
        else
        {
            marked_count++;
        }
    }
    if (open_count != 0)
    {
        for (i = 0; i < D_8016B8D4; i++)
        {
            if (D_80170A58[i].unkC_28 == 0 && func_80142C18(i) != 0)
            {
                D_80170A58[i].kind = 5;
            }
        }
    }
    if (marked_count == 3)
    {
        for (i = 0; i < D_8016B8D4; i++)
        {
            if (D_80170A58[i].unkC_28 != 0 && func_80142C18(i) != 0)
            {
                D_80170A58[i].kind = 5;
            }
        }
    }
    if (open_count != 0)
    {
        if (marked_count == 3)
        {
            func_80142B98();
            return 1;
        }
        return 0;
    }
    return 0;
}

/**
 * @brief Publish the picked rows' indices as the screen's result.
 *
 * Only acts in picker state 2. The number of entries picked so far
 * (D_80170960) becomes the result count in D_801228F0, and each entry in
 * D_8016B94C is resolved through D_80170A58 so that D_801229B0 ends up holding
 * the rows' index fields rather than their row numbers.
 *
 * @return 1 if the result was published, 0 if the picker was not in state 2.
 */
s32 func_80142820(void)
{
    s32 i;

    if (D_80170960 == 2)
    {
        D_801228F0 = D_80170960;
        for (i = 0; i < D_80170960; i++)
        {
            D_801229B0[i] = D_80170A58[D_8016B94C[i]].index;
        }
        return 1;
    }
    return 0;
}
