#include "common.h"
typedef struct
{
    u8 pad[0x14];
    u32 unk14;
    u8 *unk18;
    s32 *unk1C;
    s32 unk20;
    s32 unk24;
} FieldB78C0State;
extern FieldB78C0State *D_80123FB0;
extern void akao_set_song_params(s32, s32, s32, s32);
extern void field_clear_record_state(s32, s32);
extern void func_800B30B8(s32, s32);
extern s32 func_800B4CE4(s32, s32);
extern s32 func_800B76F8(s32);
extern s32 func_800B788C(s32);
extern s32 func_800BD414(s32, s32);
extern void saturating_counter_add(s32, s32);


extern void func_800B70F4(s32, s32 *);
extern void func_800B7164(s32, s32 *);
extern void func_800B729C(s32, s32, s32 *, s32 *);
extern void func_800B78C0(void);
extern void func_800B2B54(s32, s32, s32, s32, s32, s32);

extern void func_800BD520(s32, s32, s32);

#define FB0_BYTES ((u8 *)D_80123FB0)

/** @brief Sub-state block pointed to by FieldStateBlockView::unk18. */
typedef struct
{
    u8 pad0[4];
    s32 unk4;
} SubState;

/**
 * @brief Entry pointed to by FieldStateBlockView::unk1C. Byte 3 selects a D_800F0B98
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
} FieldStateBlockView;

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
 * func_800B7020 and func_800B70EC. All eight handlers are defined below.
 */
typedef s32 (*Handler)(void);


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


extern s32 D_80122698;
extern u8 *D_801228F8[];
extern u8 *D_80122B74;
extern Handler D_800F0B98[];


extern void func_800B28E0();


void func_800B61C4(s32 arg0)
{
    func_800BD520(-1, 0x428C, arg0);
}



void func_800B61EC(void)
{
    s32 i;

    i = 0;
    do
    {
        func_800B28E0(i, 0xC, 3);
        i++;
    } while (i < 3);
}


typedef struct B
{
    u16 unk0;
    u16 unk2;
    union
    {
        u32 unk4;
        struct
        {
            u16 lo;
            u16 unk6;
        } h;
    } u;
} B;

typedef struct Node
{
    u32 unk0;
    u32 unk4;
} Node;

typedef struct A
{
    u8 pad0[0x1C];
    u32 *unk1C;
    Node *unk20;
    B *unk24;
} A;



/**
 * @brief Compute the active record status from its parity and selector bits.
 * @return -1 for selectors 2-3, or for selectors 0-1 when parity is set; otherwise 0.
 */
s32 func_800B622C(void)
{
    s32 selector;
    s32 parity;
    s32 case_one;
    u32 record_bit;
    u32 node_bit;
    u8 parity_byte;
    u32 selector_word;
    u32 record_flags;
    B *record;
    Node *node;
    u32 *selector_ptr;

    record = ((A *)D_80123FB0)->unk24;
    if (record->u.h.unk6 & 0x4000)
    {
        return 0;
    }
    node = ((A *)D_80123FB0)->unk20;
    record_flags = record->u.unk4;
    selector_ptr = ((A *)D_80123FB0)->unk1C;
    do
    {
        record_bit = (record_flags >> 9) & 1;
        node_bit = ((u32)node->unk4 >> 9) & 1;
        parity_byte = node_bit ^ record_bit;
    } while (0);
    selector_word = *selector_ptr;
    parity = parity_byte;
    case_one = 1;
    selector = (selector_word >> 4) & 3;
    if (selector == case_one)
    {
        goto common;
    }
    switch (selector)
    {
    case 0:
        if (parity != 0)
        {
            return -1;
        }
        return 0;
    case 1:
common:
        if (parity != 0)
        {
            return -1;
        }
        return 0;
    case 2:
    case 3:
        return -1;
    }
}

typedef struct
{
    s32 unk0;
    s32 unk4;
} UnkStruct800B62D8;

extern s32 func_800BD414(s32 arg0, s32 arg1);

s32 func_800B62D8(UnkStruct800B62D8 *arg0)
{
    if (arg0->unk4 & 0x200)
    {
        if (func_800BD414(0, 0x4280) == 0)
        {
            return 1;
        }
    }
    else
    {
        if (func_800BD414(0, 0x4284) == 0)
        {
            return 2;
        }
    }

    return 0;
}

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} FieldCounter6334;

extern void func_80089BE8(s32, s32, s32, s32, s32);
extern void func_8008AE14(s32, s32);
extern void func_8008B500(s32, s32);
extern s32 func_800C0A38(u8 *);
extern void func_800C2848(s32, s32);
extern void func_800B6744(ActorB4934 *);

/** @brief Update actor depletion state and return the remaining-side result. */
s32 func_800B6334(s32 value)
{
    u8 *arg0;
    s32 var_a1;
    s32 var_a1_2;
    s32 *temp_v0;
    s32 var_a2;
    s32 var_s0;
    s32 var_s2;
    FieldCounter6334 *temp_a0;
    FieldCounter6334 *temp_a0_2;
    FieldCounter6334 *temp_v1;
    FieldCounter6334 *temp_v1_2;
    FieldCounter6334 *temp_v1_3;

    arg0 = (u8 *)value;

    if ((u8) arg0[4] < 3U)

    {
        if (func_800BD414(0, 0x428C) == arg0[4])
        {
            func_800BD520(0, 0x428C, -1);
        }
        if (*(s32 *)(arg0 + 4) & 0x200)
        {
            var_a1 = 0x4280;
        }
        else
        {
            var_a1 = 0x4284;
        }
        var_s0 = func_800BD414(0, var_a1) - 1;
        if ((var_s0 == 0) && (*(u16 *)(arg0 + 0xA) & 2))
        {
            func_8008B500(arg0[4], 0x2C);
            temp_v1 = *(FieldCounter6334 **)(arg0 + 0x10);
            var_s0 = 1;
            temp_v1->unk4 = (s32) temp_v1->unk0;
            func_800B6744((ActorB4934 *)arg0);
        }
        else
        {
            temp_a0 = *(FieldCounter6334 **)(arg0 + 0x10);
            temp_a0->unkC = (s32) (temp_a0->unkC & ~0x7FF);
            temp_v1_2 = *(FieldCounter6334 **)(arg0 + 0x10);
            temp_v1_2->unkC = (s32) (temp_v1_2->unkC | 0x200);
            func_8008AE14(arg0[4], -1);
            var_s2 = 0x384;
            if (*(u16 *)(arg0 + 0xA) & 0x20)
            {
                var_s2 = 0x1C2;
            }
            if (func_800B4CE4((s32)arg0, 0xC) != 0)
            {
                temp_v0 = D_80123FB0->unk1C;
                if ((temp_v0 != NULL) && ((u32) (*temp_v0 & 0xF) < 2U))
                {
                    var_s2 = 0xF;
                }
            }
            if (!(*(u16 *)(arg0 + 0xA) & 0x40))
            {
                func_80089BE8(*(s32 *)(D_80123FB0->unk18 + 0xC), 0x1E, 0x2C, -1, var_s2);
            }
        }
        var_a1_2 = 0x4280;
        if (*(s32 *)(arg0 + 4) & 0x200)
        {
            var_a2 = var_s0;
        }
        else
        {
            var_a1_2 = 0x4284;
            var_a2 = var_s0;
        }
    }
        else
    {
        temp_a0_2 = *(FieldCounter6334 **)(arg0 + 0x10);
        temp_a0_2->unkC = (s32) (temp_a0_2->unkC & ~0x7FF);
        if ((*(u16 *)(arg0 + 6) & 1) || ((*(u8 **)(arg0 + 0x14))[0x3F] & 1))
        {
            temp_v1_3 = *(FieldCounter6334 **)(arg0 + 0x10);
            temp_v1_3->unkC = (s32) (temp_v1_3->unkC | 0x200);
        }
        func_800C2848(arg0[4], 0);
        *(s32 *)(arg0 + 4) = (s32) (*(s32 *)(arg0 + 4) & ~0x100);
        func_8008AE14(arg0[4], func_800C0A38(arg0));
        var_a1_2 = 0x4284;
        var_a2 = func_800BD414(0, 0x4284) - 1;
    }
    func_800BD520(0, var_a1_2, var_a2);
    return func_800B62D8((UnkStruct800B62D8 *)arg0);
}

/**
 * @brief Write script variable 0x4288, set the block's 0x80000000 flag, and call func_800B28E0 with the block.
 *
 * func_800B28E0 is called with four arguments here and three elsewhere, so it
 * is declared without a prototype.
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
    ((FieldStateBlockView *)D_80123FB0)->unk20 = value;
    ((FieldStateBlockView *)D_80123FB0)->unk24 = value;
    ((FieldStateBlockView *)D_80123FB0)->unk1C = 0;
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

    e = ((FieldStateBlockView *)D_80123FB0)->unk1C;
    if (e != NULL)
    {
        if (e->unk3 < 8)
        {
            return D_800F0B98[e->unk3]();
        }
        akao_set_song_params(0x8001, 0x65, e->unk3, ((FieldStateBlockView *)D_80123FB0)->unk18->unk4);
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
            *(s32 *)(FB0_BYTES + 0x24),
            0,
            (packed_tail >> 0x14) & 0xF,
            (((packed_tail >> 0x10) & 0xF) + 1) * 0x10,
            (packed_tail >> 0x18) * 0x10);
    }
    func_800B78C0();
    return ret;
}




/**
 * @brief Resolve the active packed field operation and update its saturating counter.
 * @return Result returned by func_800B742C, or zero when the active state block is unavailable.
 */
s32 func_800B69B0(void)
{
    s32 sp10;
    s32 sp14;
    u32 packed;
    s32 byte8;
    s32 ret;
    s32 remainder;
    s32 idx;
    s32 product;
    s32 *out1;
    s32 *out2;

    packed = *(u32 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x1C) + 4);
    out1 = &sp10;
    func_800B70F4(packed & 0xF, out1);
    out2 = &sp14;
    func_800B7164((packed >> 4) & 0xF, out2);
    func_800B729C(0, 0, out1, out2);
    ret = func_800B742C(sp10, sp14);

    if (*(s32 *)((u8 *)(*(void **)(*(u8 **)(((u8 *)D_80123FB0) + 0x20) + 0x10)) + 4) == 0)
    {
        return 0;
    }

    byte8 = (packed >> 8) & 0xFF;
    if ((*(u8 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x24) + 0x39) & byte8) ||
        ((*(u32 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x1C)) & 0xF) != 2))
    {
        if ((packed >> 24) == 0)
        {
            packed &= 0xFFFFFF;
            packed |= 0x1000000;
        }

        remainder = rand() % (s32)(packed >> 24);
        idx = ((packed >> 16) & 0xFF) + remainder;
        product = ret * idx;
        saturating_counter_add(*(s32 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x20) + 0x10), (u32)product >> 7);
    }

    func_800B78C0();
    return ret;
}



extern void func_800B2D64(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Decode the current packed field command and dispatch its two effects.
 *
 * The source shape mirrors func_800B6890: the two stack outputs remain ordinary
 * locals while a copy of the packed command keeps the long-lived value web used
 * by the later command dispatches.
 *
 * @return The result produced by func_800B742C.
 */
s32 func_800B6B28(void)
{
    s32 sp10;
    s32 sp14;
    u32 packed;
    u32 packed_tail;
    s32 byte8;
    s32 ret;

    packed = *(u32 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x1C) + 4);
    packed_tail = packed;
    func_800B70F4(packed & 0xF, &sp10);
    func_800B7164((packed >> 4) & 0xF, &sp14);
    byte8 = (packed >> 8) & 0xFF;
    func_800B729C(0, byte8, &sp10, &sp14);
    ret = func_800B742C(sp10, sp14);
    if ((*(u8 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x24) + 0x39) & byte8) ||
        ((**(u32 **)(((u8 *)D_80123FB0) + 0x1C) & 0xF) != 2))
    {
        func_800B2D64(
            *(s32 *)(((u8 *)D_80123FB0) + 0x20),
            (packed_tail >> 20) & 0xF,
            (packed_tail >> 16) & 0xF,
            -1);
        func_800B2D64(
            *(s32 *)(((u8 *)D_80123FB0) + 0x24),
            packed_tail >> 28,
            (packed_tail >> 24) & 0xF,
            -1);
    }
    func_800B78C0();
    return ret;
}




/**
 * @brief Handler 3 of D_800F0B98: roll a random offset into the entry's byte-2
 *        index, scale it by a nested resource factor, and run the
 *        func_800B729C .. func_800B742C chain.
 *
 * Near-clone of func_800B6890 and func_800B6B28 (the other decoded handlers in
 * this table): the entry's packed word at +4 supplies byte 2 as a base index
 * and byte 3 as a modulus for rand(). When byte 3 is zero it is forced to 1
 * (with the packed word retagged into the 0x1000000 range) before the modulo,
 * avoiding a divide-by-zero.
 *
 * @return func_800B742C's result.
 */
s32 func_800B6C48(void)
{
    s32 sp10;
    s32 sp14;
    u32 packed;
    s32 remainder;
    s32 factor;
    s32 idx;
    s32 product;
    s32 ret;

    packed = *(u32 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x1C) + 4);
    if ((packed >> 24) == 0)
    {
        packed &= 0xFFFFFF;
        packed |= 0x1000000;
    }

    remainder = rand() % (s32)(packed >> 24);

    factor = *(s32 *)(*(s32 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x20) + 0x10) + 4);
    idx = ((packed >> 16) & 0xFF) + remainder;
    product = factor * idx;

    sp14 = 0;
    sp10 = (u32)product >> 4;
    func_800B729C(0, 0, &sp10, &sp14);
    ret = func_800B742C(sp10, sp14);
    func_800B78C0();
    return ret;
}


/** @brief Packed field command record consumed by func_800B6D3C. */
typedef struct
{
    u8 pad0[3];
    u8 selector;
    u32 packed;
} FieldCommandB6D3C;

/** @brief Minimal view of the active FIELD state used by func_800B6D3C. */
typedef struct
{
    u8 pad0[0x1C];
    FieldCommandB6D3C *command;
    s32 unk20;
    u8 *target;
} FieldStateB6D3C;



/**
 * @brief Dispatch the active packed field command through its selected processing path.
 * @return Result produced by func_800B742C.
 */
s32 func_800B6D3C(void)
{
    u32 packed;
    s32 first_value;
    s32 second_value;
    s32 arg4_value;
    s32 arg5_value;
    s32 selector;
    s32 packed_selector;
    s32 result;
    u32 packed_tail;

    packed = ((FieldStateB6D3C *)D_80123FB0)->command->packed;
    selector = ((FieldStateB6D3C *)D_80123FB0)->target[3];
    packed_selector = (packed >> 8) & 0xF;
    packed_tail = packed;

    if (selector == packed_selector)
    {
        akao_set_song_params(0x8003, selector, selector, 1);

        packed++;
        packed--;
        packed_tail--;
        packed_tail++;
        func_800B70F4(packed & 0xF, &first_value);
        if (((packed >> 12) & 0xF) == 1)
        {
            first_value <<= 1;
        }
        func_800B7164((packed >> 4) & 0xF, &second_value);
        func_800B729C(0, 0, &first_value, &second_value);
        result = func_800B742C(first_value, second_value);

        arg4_value = (((packed >> 16) & 0xF) + 1) << 4;
        arg5_value = (packed >> 24) << 4;
        func_800B2B54(((FieldStateB6D3C *)D_80123FB0)->unk20, (s32)((FieldStateB6D3C *)D_80123FB0)->target, 0, (packed >> 20) & 0xF, arg4_value, arg5_value);
    }
    else
    {
        packed_tail = ((FieldStateB6D3C *)D_80123FB0)->command->packed;
        akao_set_song_params(0x8003, packed_selector, selector, 0);

        func_800B70F4(packed_tail & 0xF, &first_value);
        func_800B7164((packed_tail >> 4) & 0xF, &second_value);
        func_800B729C(0, 0, &first_value, &second_value);
        result = func_800B742C(first_value, second_value);
    }

    func_800B78C0();
    return result;
}





/**
 * @brief Apply the packed field-state operation selected by the active state block.
 * @return Result returned by func_800B742C.
 */
s32 func_800B6EC0(void)
{
    s32 sp10;
    s32 sp14;
    s32 *p10;
    s32 *p14;
    u32 packed;
    u32 packed_tail;
    s32 byte8;
    s32 ret;
    u8 *p;

    p10 = &sp10;
    p14 = &sp14;
    packed = *(u32 *)(*(u8 **)(FB0_BYTES + 0x1C) + 4);
    packed_tail = packed;
    func_800B70F4(packed & 0xF, p10);
    func_800B7164((packed >> 4) & 0xF, p14);
    func_800B729C(0, 0, p10, p14);
    ret = func_800B742C(sp10, sp14);
    func_800B78C0();
    byte8 = (packed >> 8) & 0xFF;
    switch (byte8)
    {
    case 0:
        *(s16 *)(*(u8 **)(*(u8 **)(FB0_BYTES + 0x24) + 0x10) + 0x48) = 0;
        break;
    case 1:
        p = *(u8 **)(FB0_BYTES + 0x24);
        *(s32 *)(p + 0xC) |= (packed_tail >> 0x10) << 0x10;
        break;
    case 2:
        p = *(u8 **)(FB0_BYTES + 0x24);
        *(s32 *)(p + 0xC) |= packed_tail >> 0x10;
        break;
    case 3:
        p = *(u8 **)(FB0_BYTES + 0x24);
        *(u8 *)(p + 0x39) |= packed_tail >> 0x10;
        break;
    default:
        return ret;
    }
    return ret;
}




/**
 * @brief Roll a randomized scaled value into the 4-byte slot of the active resource.
 * @note Reads the packed word at *(((u8 *)D_80123FB0)+0x1C)+4; if its top byte is zero it is
 *       forced to 1. A random roll modulo that byte is added to the middle byte, then
 *       multiplied by the slot's current value at *(((u8 *)D_80123FB0)+0x24)+0x10, shifted right
 *       by 4, floored to 1, and written back.
 * @see decomp.me (100.00%)
 */
void func_800B7020(void)
{
    u32 packed;
    u8 *slot;
    u32 value;
    s32 roll;
    u32 scratch;

    packed = *(u32 *)(*(u8 **)(((u8 *)D_80123FB0) + 0x1C) + 4);
    if ((packed >> 24) == 0)
    {
        packed &= 0xFFFFFF;
        packed |= 0x1000000;
    }
    roll = rand();
    scratch = packed >> 24;
    roll %= (s32)scratch;
    scratch = *(u32 *)(((u8 *)D_80123FB0) + 0x24);
    slot = *(u8 **)(scratch + 0x10);
    value = ((packed >> 16) & 0xFF) + roll;
    {
        s32 scale;
        scale = *(s32 *)(slot + 4);
        scale *= value;
        value = (u32)scale >> 4;
    }
    if (value == 0)
    {
        value = 1;
    }
    *(s32 *)(slot + 4) = value;
}


s32 func_800B70EC(void)
{
    return 0;
}



extern s32 func_800B2D34(u8 *arg0, s32 arg1);

/**
 * @brief Scales a per-field chance value into an output slot.
 *
 * Rolls func_800B2D34 with the field's 0x20 pointer, biases the result by 50,
 * multiplies by the field's 0x4A0 half-word, divides by 50, and stores the
 * quotient through @p out.
 *
 * 100% match with the FIELD GCC 2.8.0 G0 toolchain. The former 93.93%
 * result was a compiler-routing mismatch; GCC 2.8.0 reproduces the target
 * allocation and epilogue exactly.
 */
void func_800B70F4(s32 arg0, s32 *out)
{
    s32 v;
    u32 prod;

    v = func_800B2D34(*(u8 **)(((u8 *)D_80123FB0) + 0x20), arg0);
    prod = *(u16 *)(((u8 *)D_80123FB0) + 0x4A0) * (v + 0x32);
    *out = prod / 50;
}


typedef struct
{
    u8 pad0[0xC];
    s32 unkC;
} Obj2;

typedef struct
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0x10 - 5];
    Obj2 *unk10;
    u8 pad14[0x1C - 0x14];
    u16 arr1C[4];
    u8 arr24[4];
} Obj;

typedef struct
{
    u8 pad0[0x14];
    s32 unk14;
    u8 pad18[0x1C - 0x18];
    u8 *unk1C;
    u8 pad20[0x24 - 0x20];
    Obj *unk24;
} FieldState;



s32 func_8008ADB4(s32 arg0);
s32 func_800B2D34(u8 *arg0, s32 arg1);

/**
 * @brief Resolve and scale an actor state value selected by the caller.
 * @param arg0 Value selector passed to the actor-state lookup.
 * @param arg1 Output location that receives the resolved value.
 */
void func_800B7164(s32 arg0, s32 *arg1)
{
    Obj *obj;
    Obj *obj3;
    s32 status;
    s32 mult_val;
    u8 idx;

    status = func_8008ADB4(((FieldState *)D_80123FB0)->unk24->unk4);
    mult_val = func_800B2D34((u8 *)((FieldState *)D_80123FB0)->unk24, arg0);
    obj = ((FieldState *)D_80123FB0)->unk24;
    if ((obj->unk10->unkC & 2) || status == 0x31)
    {
        *arg1 = 0;
    }
    else if ((u32)(status - 0xA) < 2)
    {
        ((FieldState *)D_80123FB0)->unk14 |= 1;
        idx = *((FieldState *)D_80123FB0)->unk1C >> 6;
        obj3 = ((FieldState *)D_80123FB0)->unk24;
        *arg1 = obj3->arr1C[idx] + obj3->arr24[idx];
    }
    else
    {
        idx = *((FieldState *)D_80123FB0)->unk1C >> 6;
        *arg1 = obj->arr1C[idx];
    }
    *arg1 = (u32)(*arg1 * (mult_val + 0x32)) / 50;
}

typedef struct
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0x39 - 5];
    u8 unk39;
    u8 unk3A;
    u8 pad3B[0x44 - 0x3B];
    u8 unk44;
} SubStruct24;

typedef struct
{
    u8 pad0[0x20];
    u8 *unk20;
    SubStruct24 *unk24;
    u8 pad28[0x4A2 - 0x28];
    u8 unk4A2;
} FieldStateView729C;


extern u8 D_800F0BB8[];


void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Apply active field-state modifiers to the supplied value pair.
 * @param arg0 Unused operation selector.
 * @param arg1 Additional modifier mask.
 * @param arg2 Primary value adjusted by active modifiers.
 * @param arg3 Secondary value passed through unchanged.
 */
void func_800B729C(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3)
{
    s32 combined;
    s32 mask;
    s32 sum;
    s32 i;
    u8 *base24;

    combined = arg1 | ((FieldStateView729C *)D_80123FB0)->unk4A2;
    mask = combined & ((FieldStateView729C *)D_80123FB0)->unk24->unk39;

    if (mask != 0)
    {
        i = 0;
        sum = 0;
        do
        {
            if (mask & 1)
            {
                sum += *(((FieldStateView729C *)D_80123FB0)->unk20 + i + 0x3C);
            }
            i++;
            mask >>= 1;
        } while (i < 8);
        *arg2 = (u32)(*arg2 * (sum + 5)) >> 2;
    }

    if (func_800B4CE4((s32)((FieldStateView729C *)D_80123FB0)->unk24, 8) == 0)
    {
        mask = combined & ((FieldStateView729C *)D_80123FB0)->unk24->unk3A;
    }
    else
    {
        mask = combined;
    }

    i = 0;
    if (mask != 0)
    {
        sum = 0;
        do
        {
            if (mask & 1)
            {
                base24 = (u8 *)((FieldStateView729C *)D_80123FB0)->unk24;
                sum += *(base24 + D_800F0BB8[i] + 0x44);
            }
            i++;
            mask >>= 1;
        } while (i < 8);

        if (sum >= 9)
        {
            *arg2 = (u32)*arg2 >> 2;
        }
        else
        {
            *arg2 = (u32)*arg2 >> 1;
        }
    }

    if (((FieldStateView729C *)D_80123FB0)->unk24->unk4 != 0)
    {
        func_800BD520(((FieldStateView729C *)D_80123FB0)->unk24->unk4, 0xD008, combined);
    }
}

/** @brief Calculate and apply an action value to the current target. */
s32 func_800B742C(u32 arg0, u32 arg1)
{
    s32 temp_s1;
    s32 temp_s1_2;
    s32 temp_v0;
    s32 var_a0_2;
    s32 var_a1;
    s32 var_s1_2;
    s32 var_v0;
    s32 var_v0_3;
    u32 var_a0;
    u32 var_s1;
    u32 var_v0_2;

    var_v0 = 0;
    if (func_800B4CE4(D_80123FB0->unk24, 0xB) == 0)
    {
        if (*((u8 *)D_80123FB0->unk1C + 2) == 0)
        {
            return 0;
        }
        var_v0_2 = arg1 >> 1;
        if ((arg1 < arg0) || (var_v0_2 = arg1 >> 1, ((D_80123FB0->unk14 & 1) != 0)))
        {
            temp_s1 = arg0 - var_v0_2;
            var_v0_3 = temp_s1;
            if (temp_s1 < 0)
            {
                var_v0_3 = 0;
            }
            var_s1 = (u32) var_v0_3;
        }
        else
        {
            var_a0 = arg1 >> 1;
            if (var_a0 == 0)
            {
                var_a0 = 1;
            }
            temp_v0 = (arg0 >> 1) & 0xFFFF;
            var_s1 = (u32) (temp_v0 * temp_v0) / var_a0;
        }
        var_a0_2 = 0;
        if ((*(u32 *)((u8 *)D_80123FB0->unk20 + 0x4) & 0xFC00) == 0x1000)
        {
            var_a0_2 = func_800BD414(2, 0xD038) * 4;
        }
        temp_s1_2 = func_800B76F8(((u32) ((*((u8 *)D_80123FB0->unk1C + 2) + var_a0_2) * var_s1) >> 4) * *(s32 *)((u8 *)D_80123FB0->unk18 + 0x18));
        if ((func_800B4CE4(D_80123FB0->unk20, 0xA) != 0) && ((u32) (*(u32 *)D_80123FB0->unk1C & 0xF) < 2U))
        {
            var_a1 = temp_s1_2;
            if (temp_s1_2 < 0)
            {
                var_a1 = temp_s1_2 + 3;
            }
            saturating_counter_add(*(s32 *)((u8 *)D_80123FB0->unk20 + 0x10), var_a1 >> 2);
        }
        var_s1_2 = func_800B788C(temp_s1_2);
        if (!(D_80123FB0->unk14 & 1))
        {
            if (var_s1_2 <= 0)
            {
                var_s1_2 = 1;
            }
        }
        if (func_800BD414(0, 0xFFC) != 0)
        {
            akao_set_song_params(0x8002, *(u8 *)((u8 *)D_80123FB0->unk20 + 4), *(u8 *)((u8 *)D_80123FB0->unk24 + 4), var_s1_2);
        }
        if (((func_800BD414(0, 0xFFA) == 0) || (var_v0 = var_s1_2, ((*(u8 *)((u8 *)D_80123FB0->unk24 + 4) < 3U) != 0))) && ((func_800BD414(0, 0xFFB) == 0) || (var_v0 = var_s1_2, ((*(u8 *)((u8 *)D_80123FB0->unk24 + 4) < 3U) == 0))))
        {
            field_clear_record_state(D_80123FB0->unk24, 6);
            func_800B30B8(*(s32 *)((u8 *)D_80123FB0->unk24 + 0x10), var_s1_2);
            var_v0 = var_s1_2;
        }
        return var_v0;
    }
    return var_v0;
}

/**
 * @brief Apply field-state modifiers to an input value.
 * @param arg0 Base value to modify.
 * @return Value after applying the active field-state modifiers.
 */
s32 func_800B76F8(s32 arg0)
{
    s32 s0;
    s32 count;
    s32 value;

    s0 = arg0;

    count = func_800B4CE4(D_80123FB0->unk20, (*(u8 *)D_80123FB0->unk1C >> 6) | 0x40);
    if (count != 0)
    {
        do
        {
            s0 = (s0 * 3) / 2;
        } while (--count != 0);
    }

    value = *D_80123FB0->unk1C & 0xF;
    count = func_800B4CE4(D_80123FB0->unk20, value + 0x38);
    if (count != 0)
    {
        do
        {
            s0 = (s0 * 3) / 2;
        } while (--count != 0);
    }

    value = *D_80123FB0->unk1C & 0xF;
    count = func_800B4CE4(D_80123FB0->unk24, value + 0x30);
    if (count != 0)
    {
        do
        {
            s0 = s0 / 2;
        } while (--count != 0);
    }

    count = func_800B4CE4(D_80123FB0->unk20, *(u8 *)(D_80123FB0->unk24 + 3) + 0x10);
    if (count != 0)
    {
        do
        {
            s0 = (s0 * 3) / 2;
        } while (--count != 0);
    }

    count = func_800B4CE4(D_80123FB0->unk24, *(u8 *)(D_80123FB0->unk20 + 3) + 0x20);
    if (count != 0)
    {
        do
        {
            s0 = s0 / 2;
        } while (--count != 0);
    }

    return s0;
}

typedef struct
{
    s8 pad[0xC];
    s32 flags;
} InnerStruct80123FB0;

typedef struct
{
    s8 pad[0x20];
    InnerStruct80123FB0* inner;
} OuterStruct80123FB0;



s32 func_800B788C(s32 arg0)
{
    s32 result;

    result = arg0;
    if (((OuterStruct80123FB0 *)D_80123FB0)->inner->flags & 1)
    {
        result *= 2;
    }
    return result;
}

typedef struct
{
    u8 unk0;
    u8 unk1;
} FieldB78C0Rec;



extern void func_800B2B54(s32 a, s32 b, s32 c, s32 d, s32 e, s32 f);
extern s32 func_800B4CE4(s32 a, s32 b);
extern FieldB78C0Rec D_800F0BC0;
extern FieldB78C0State *D_80123FB0;

void func_800B78C0(void)
{
    s32 var_s0;
    FieldB78C0Rec *new_var;
    FieldB78C0Rec *temp_v1;

    var_s0 = 0x50;
    if ((*D_80123FB0->unk1C & 0xF) != 2)
    {
        do
        {
            if (func_800B4CE4(D_80123FB0->unk20, var_s0) != 0)
            {
                new_var = &D_800F0BC0;
                temp_v1 = &new_var[var_s0 - 0x50];
                func_800B2B54(D_80123FB0->unk20, D_80123FB0->unk24, 0, var_s0 - 0x50,
                              (s32)temp_v1->unk0, temp_v1->unk1 * 0x10);
            }
            var_s0 += 1;
        } while (var_s0 < 0x60);
    }
}
