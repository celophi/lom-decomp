#include "common.h"

/** @brief One 0x94-byte element in the D_80122B78 table; unk0 is its id. */
typedef struct Elem
{
    u8 unk0;    /* 0x00 */
    u8 pad[0x93];
} Elem;

/** @brief D_80122B78 table: count at 0x400 (u16/u32 union), elements at 0x430. */
typedef struct Foo
{
    u8 pad0[0x400];
    union
    {
        u16 count;  /* 0x400 as lhu */
        u32 flags;  /* 0x400 as lw */
    } f400;
    u8 pad1[0x430 - 0x404];
    Elem elem[1];   /* 0x430 */
} Foo;

extern Foo *D_80122B78;
extern s32 D_8010AE78;
void func_800B177C(void);
void func_800B286C(u8 arg0, s32 arg1, s32 arg2);

/**
 * @brief Re-issue every active table element and clear the batch-dirty flags.
 *
 * After func_800B177C, walks the @c count live elements of @c D_80122B78,
 * dispatching func_800B286C for each element's id, then clears the 0x60000 bits
 * of the flag word at 0x400 and resets @c D_8010AE78.
 *
 * @see decomp.me (100%) TODO
 */
void func_800BD99C(void)
{
    s32 i;

    func_800B177C();
    do
    {
        i = 0;
    } while (0);
    if (D_80122B78->f400.count != 0)
    {
        do
        {
            func_800B286C(D_80122B78->elem[i].unk0, 0xD, 0x82);
            i += 1;
        } while (i < (s32)D_80122B78->f400.count);
    }
    D_80122B78->f400.flags &= 0xFFF9FFFF;
    D_8010AE78 = 0;
}
