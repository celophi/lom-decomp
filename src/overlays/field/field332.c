#include "common.h"

typedef struct {
    u8 pad0[0x2A];
    s16 unk2A;   /* 0x2A */
    u8 pad2C[0x3A - 0x2C];
    u8 unk3A;    /* 0x3A */
    u8 unk3B;    /* 0x3B */
    u8 pad3C[0x178 - 0x3C];
    u8 unk178;   /* 0x178 */
    u8 pad179[0x23C - 0x179];
} ActorRec;

typedef struct {
    u8 pad0[0xE];
    u16 unkE;    /* 0x0E */
    u8 pad10[0x14 - 0x10];
} ResEntry;

extern ActorRec D_80105AE0[];
extern ResEntry g_field_resource_entries[];


/**
 * @note NOT YET MATCHED (94.87%). residue is 2 branch-delay slots the target leaves as nop but gcc's dbr fills; not source-fixable.
 * @see decomp.me (94.87%) TODO
 */
void func_80094EA4(ActorRec *arg0)
{
    if ((D_80105AE0[arg0->unk3A].unk178 & 1) == 0)
    {
        if (func_8009104C(arg0->unk3A, 0, 0, g_field_resource_entries[arg0->unk3B].unkE) != 0)
        {
            arg0->unk2A = 0x8E;
        }
    }
}

/**
 * @note NOT YET MATCHED (94.87%). twin of func_80094EA4 (success const 0x94 vs 0x8E); same dbr residue.
 * @see decomp.me (94.87%) TODO
 */
void func_80094F40(ActorRec *arg0)
{
    if ((D_80105AE0[arg0->unk3A].unk178 & 1) == 0)
    {
        if (func_8009104C(arg0->unk3A, 0, 0, g_field_resource_entries[arg0->unk3B].unkE) != 0)
        {
            arg0->unk2A = 0x94;
        }
    }
}
