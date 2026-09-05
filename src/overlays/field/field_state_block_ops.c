#include "common.h"

/** @brief Sub-state block pointed to by FieldState::unk18. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;
} SubState;

/**
 * @brief Entry pointed to by FieldState::unk1C. Byte 3 selects a D_800F0B98
 *        handler; the handlers also read the word at +0 (low nibble) and the
 *        packed word at +4.
 */
typedef struct
{
    u8 pad0[3];
    u8 unk3;
} Entry;

/**
 * @brief View of the 0x4A4-byte block at D_80123FB0 (D_80123B08, built by
 *        func_800B3580 in field_state_ops.c). Word 0 carries flag 0x80000000,
 *        which field309.c tests as the sign bit; the pointers at 0x18 and 0x1C
 *        select the active sub-state and entry.
 */
typedef struct
{
    u32 unk0;
    u8 pad4[0x18 - 4];
    SubState *unk18;
    Entry *unk1C;
    s32 unk20;
    s32 unk24;
} FieldState;

/** @brief Actor record; unk4 selects a 0x250-byte block in the layout buffer. */
typedef struct
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[5];
    u16 unkA;
} ActorB4934;

/*
 * D_800F0B98 holds eight handlers indexed by Entry::unk3: func_800B6890,
 * func_800B69B0, func_800B6B28, func_800B6C48, func_800B6D3C, func_800B6EC0,
 * func_800B7020 and func_800B70EC. Only entries 0, 2, 6 and 7 are decompiled.
 */
typedef s32 (*Handler)(void);

#define FB0_BYTES ((u8 *)D_80123FB0)

u8 *func_800C1E40(s32 arg0);
/*
 * Declared without a prototype: func_800B2A9C takes an id argument (see
 * func_800B2A9C.c), but func_800B66F0 calls it with none and lets the caller's
 * a0 flow through. A void prototype would misstate that; an s32 one would add
 * an argument load.
 */
s32 func_800B2A9C();
s32 func_800B6334(s32 value);
void func_800B65CC(s32 value);
void func_800B4934(ActorB4934 *arg0);
void akao_set_song_params(s32 flags, s32 duration, s32 field_id, s32 sub_id);
void func_800B2B54(s32 arg0, void *arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);
void func_800B70F4(s32 arg0, s32 *out);
void func_800B7164(s32 arg0, s32 *out);
void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
s32 func_800B742C(s32 arg0, s32 arg1);
void func_800B78C0(void);

extern FieldState *D_80123FB0;
extern s32 D_80122698;
extern u8 *D_801228F8[];
extern u8 *D_80122B74;
extern Handler D_800F0B98[];

/**
 * @brief Write script variable 0x4288, set the block's 0x80000000 flag, and call func_800B28E0 with the block.
 *
 * func_800B28E0 is called with four arguments here and three elsewhere, so it
 * is left implicitly declared.
 *
 * @param arg0 Value written to script variable 0x4288. Callers pass func_800B6334's result.
 */
void func_800B65CC(s32 arg0)
{
    u32 *p;
    func_800BD520(0, 0x4288, arg0);
    p = (u32 *)D_80123FB0;
    *p |= 0x80000000;
    func_800B28E0(0x80, 0xD, 1, p);
}

/**
 * @brief Resolve three entries of resource record 9 into D_801228F8, or clear them when the record is absent.
 *
 * When the layout buffer's 0x840 byte is set and the 0x858 word's low 7 bits
 * equal 2, D_80122698 is set and the entry base advances by (byte 0x859 + 1)
 * groups of three. func_800C31BC and script opcode 0x27 treat 0x840/0x858 as
 * one of two parallel slots (the other is 0xA90/0xAA8), and func_800C10F0 uses
 * byte 0x859 as a small per-member index.
 */
void func_800B661C(void)
{
    s32 off;
    s32 i;
    u8 *base;
    u8 *p;

    D_80122698 = 0;
    off = 0;
    if (D_80122B74[0x840] != 0 && ((*(u32 *)(D_80122B74 + 0x858) & 0x7F) == 2))
    {
        D_80122698 = 1;
        off = (D_80122B74[0x859] + 1) * 3;
    }

    base = func_800C1E40(9);
    i = 0;
    if (base != NULL)
    {
        u8 **out;
        out = D_801228F8;
        p = (u8 *)((off * 2) + (s32)base);
        do
        {
            *out = base + (*(u16 *)(p + 4) + 4);
            p += 2;
            i++;
            out++;
        } while ((u32)i < 3);
    }
    else
    {
        u8 **out;
        out = D_801228F8;
        do
        {
            *out = NULL;
            i++;
            out++;
        } while ((u32)i < 3);
    }
}

/**
 * @brief Store func_800B2A9C's result in unk20 and unk24, clear unk1C, and forward a nonzero func_800B6334 result to func_800B65CC.
 */
void func_800B66F0(void)
{
    s32 value;
    s32 result;

    value = func_800B2A9C();
    D_80123FB0->unk20 = value;
    D_80123FB0->unk24 = value;
    D_80123FB0->unk1C = 0;
    if (value != 0)
    {
        result = func_800B6334(value);
        if (result != 0)
        {
            func_800B65CC(result);
        }
    }
}

/**
 * @brief Find the first of the actor's four 0x40-byte sub-entries holding id 0x58 with bit 1 of its 0x2E flags set, replace the id with 0xFF, clear the flags, and rebuild the status mask via func_800B4934.
 *
 * The four sub-entries start at layout offset 0x5F0 + unk4 * 0x250 + 0x50,
 * the same walk func_800B4934 in field293.c performs.
 *
 * @param arg0 Actor whose unk4 selects the 0x250-byte block.
 */
void func_800B6744(ActorB4934 *arg0)
{
    s32 i;
    s32 off;
    s32 j;
    u8 *rec;
    u8 *p;

    for (i = 0; i < 4; i++)
    {
        off = 0x50 + i * 0x40;
        rec = D_80122B74 + (arg0->unk4 * 0x250 + 0x5F0) + off;
        if (rec[0] != 0 && (*(u16 *)(rec + 0x2E) & 2))
        {
            for (j = 0; j < 4; j++)
            {
                p = rec + j;
                if (p[0x20] == 0x58)
                {
                    p[0x20] = 0xFF;
                    *(u16 *)(rec + 0x2E) = 0;
                    func_800B4934(arg0);
                    return;
                }
            }
        }
    }
}

/**
 * @brief Dispatch the active entry's byte 3 through the D_800F0B98 handler table.
 *
 * Values below 8 run the table entry and return its result; higher values are
 * reported to akao_set_song_params with the sub-state's word at 0x4. Returns 0
 * when there is no entry or after the report.
 *
 * @return The dispatched handler's result, or 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800B6808(void)
{
    Entry *e;

    e = D_80123FB0->unk1C;
    if (e != NULL)
    {
        if (e->unk3 < 8)
        {
            return D_800F0B98[e->unk3]();
        }
        akao_set_song_params(0x8001, 0x65, e->unk3, D_80123FB0->unk18->unk4);
        return 0;
    }
    return 0;
}

/**
 * @brief Handler 0 of D_800F0B98: unpack the entry's packed word at +4 and run the func_800B70F4 .. func_800B742C chain.
 *
 * Nibble 0 goes to func_800B70F4, nibble 1 to func_800B7164, byte 1 to
 * func_800B729C, and the two results to func_800B742C. func_800B2B54 then runs
 * unless the entry's low nibble is 2 and byte 1 misses the mask at
 * (*unk24)[0x39]; func_800B78C0 applies the inverse guard. func_800B6B28 is a
 * near-clone of this function.
 *
 * @return func_800B742C's result.
 */
s32 func_800B6890(void)
{
    s32 sp18;
    s32 sp1C;
    u32 packed;
    u32 packed_tail;
    s32 byte8;
    s32 ret;

    packed = *(u32 *)(*(u8 **)(FB0_BYTES + 0x1C) + 4);
    packed_tail = packed;
    func_800B70F4(packed & 0xF, &sp18);
    func_800B7164((packed >> 4) & 0xF, &sp1C);
    byte8 = (packed >> 8) & 0xFF;
    func_800B729C(0, byte8, &sp18, &sp1C);
    ret = func_800B742C(sp18, sp1C);
    if ((*(u8 *)(*(u8 **)(FB0_BYTES + 0x24) + 0x39) & byte8) ||
        ((**(u32 **)(FB0_BYTES + 0x1C) & 0xF) != 2))
    {
        func_800B2B54(
            *(s32 *)(FB0_BYTES + 0x20),
            *(void **)(FB0_BYTES + 0x24),
            0,
            (packed_tail >> 0x14) & 0xF,
            (((packed_tail >> 0x10) & 0xF) + 1) * 0x10,
            (packed_tail >> 0x18) * 0x10);
    }
    func_800B78C0();
    return ret;
}
