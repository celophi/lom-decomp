#include "common.h"

/** @brief Compact per-frame config block populated before a sequence flush. */
typedef struct
{
    s32 unk0;   /* 0x00 owning object pointer */
    u8 unk4;    /* 0x04 mode selector */
    u8 unk5;    /* 0x05 primary record index */
    u8 unk6;    /* 0x06 secondary record index */
    u8 unk7;    /* 0x07 tertiary record index (biased by 0x40) */
} Cfg;

/** @brief 0xC-stride record view into the D_80123FC0 table. */
typedef struct
{
    u8 pad0[4];
    u16 unk4;   /* 0x04 */
    u16 unk6;   /* 0x06 */
    u8 pad8[0xBC];
    u16 unkC4;  /* 0xC4 */
    u16 unkC6;  /* 0xC6 */
} Rec0C;

/** @brief 0x14-stride record view into the D_80123FC0 table. */
typedef struct
{
    u8 pad0[0x184];
    u16 unk184; /* 0x184 */
} Rec14;

/** @brief 4-stride record view into the D_80123FC0 table. */
typedef struct
{
    u8 pad0[0x686];
    u16 unk686; /* 0x686 */
} Rec4;

extern s32 D_80122B78;
extern s32 D_80123FB8;
extern u8 *D_80123FC0;
extern Cfg *D_80123FC4;

/**
 * @brief Flush the staged config block through the sequence emitter chain.
 *
 * Redirects the active sequence buffer @c D_80123FB8 to the scratch region at
 * @c D_80122B78 + 0xD98, emits the note/param records selected by the config
 * indices (using @c unk4 to pick the base vs. alternate field), restores the
 * buffer, and finally dispatches the mode-0/mode-1 finaliser.
 *
 * @see decomp.me (100%) TODO
 */
void func_800BEF74(void)
{
    s32 temp_s2;
    Rec0C *new_var;
    Rec0C *new_var2;
    u16 var_a0;
    u16 var_a0_2;

    temp_s2 = D_80123FB8;
    D_80123FB8 = D_80122B78 + 0xD98;
    func_800BF158();
    if (D_80123FC4->unk4 == 0)
    {
        new_var = (Rec0C *)(D_80123FC0 + (D_80123FC4->unk5 * 3 << 2));
        var_a0 = new_var->unk4;
    }
    else
    {
        var_a0 = (new_var2 = (Rec0C *)(D_80123FC0 + (D_80123FC4->unk5 * 3 << 2)))->unkC4;
    }
    func_800BF2F0(var_a0);
    {
        Rec14 *r14 = (Rec14 *)(D_80123FC0 + (D_80123FC4->unk6 * 5 << 2));
        func_800BF2F0(r14->unk184);
    }
    func_800BF2F0(((Rec4 *)(D_80123FC0 + ((D_80123FC4->unk7 - 0x40) << 2)))->unk686);
    func_800BF3D8();
    func_800BF800();
    if (D_80123FC4->unk4 == 0)
    {
        var_a0_2 = ((Rec0C *)(D_80123FC0 + (D_80123FC4->unk5 * 3 << 2)))->unk6;
    }
    else
    {
        var_a0_2 = ((Rec0C *)(D_80123FC0 + (D_80123FC4->unk5 * 3 << 2)))->unkC6;
    }
    func_800BF2F0(var_a0_2);
    func_800BF700();
    D_80123FB8 = temp_s2;
    func_800BFA34();
    switch (D_80123FC4->unk4)
    {
    case 0:
        func_800BFF90(D_80123FC4->unk0);
        return;
    case 1:
        func_800C015C(D_80123FC4->unk0);
        return;
    }
}
