#include "common.h"
#include "field_script.h"

void field_script_op_00(void);

/**
 * @brief Apply a signed 16-bit relative jump to the active record's program counter.
 *
 * Reads a little-endian 16-bit delta from the active record's program counter
 * at offset @p delta_offset. A non-zero delta advances the pc by it
 * (sign-extended via the 0x8000 bit); a zero delta hands off to
 * field_script_op_00 to step the cursor.
 *
 * @param delta_offset Byte offset from the program counter holding the delta.
 * @see decomp.me (100%) TODO
 */
void field_script_branch(s32 delta_offset)
{
    FieldScriptRecord *rec;
    s32 pc;
    u8 *ptr;
    s32 val;
    s32 lo;

    rec = FIELD_SCRIPT_ACTIVE_RECORD();
    pc = (s32)rec->pc;
    ptr = (u8 *)(pc + delta_offset);
    val = ptr[0] + (ptr[1] << 8);
    lo = val & 0xFFFF;
    if (lo != 0)
    {
        if (val & 0x8000)
        {
            s32 t = pc + 0xFFFF0000;
            rec->pc = (u8 *)(t + lo);
            return;
        }
        rec->pc = (u8 *)(pc + lo);
        return;
    }
    field_script_op_00();
}

/**
 * @brief Read one operand, mapping the value 0xFF to the script owner's id.
 * @param type Operand type from the descriptor byte.
 * @param data Operand stream position.
 * @param value Receives the decoded value.
 * @return The advanced operand stream position.
 */
u8 *field_script_read_operand_or_owner(u32 type, u8 *data, s32 *value)
{
    u8 *result;

    result = field_script_read_operand(type, data, value);
    if (*value == 0xFF)
    {
        *value = g_field_script->status.owner_id;
    }
    return result;
}

u8 *field_script_read_u16(u8 *data, u16 *value);
s32 func_800BD3B0(s32 arg0, s32 arg1);

/**
 * @brief Decode one field script operand and advance the read pointer.
 * @param type Two-bit operand kind: 0 script variable, 1 byte, 2 halfword, 3 word.
 * @param data Operand byte stream to read from.
 * @param value Destination for the decoded 32-bit value.
 * @return @p data advanced past the bytes consumed for this operand.
 */
u8 *field_script_read_operand(u32 operand_type, u8 *data, s32 *value)
{
    s32 type = operand_type;

    type &= 3;
    switch (type)
    {
    case 0:
    {
        u16 var_index;

        data = field_script_read_u16(data, &var_index);
        *value = func_800BD3B0(g_field_script->status.owner_id, var_index << 16);
        return data;
    }
    case 1:
        *value = *data;
        return data + 1;
    case 2:
        *value = data[0] + (data[1] << 8);
        return data + 2;
    case 3:
        *value = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
        return data + 4;
    }
}

/**
 * @param arg0 Byte stream to read a little-endian 16-bit value from.
 * @param arg1 Destination for the unpacked 16-bit value.
 * @return arg0 advanced past the two bytes read.
 * @see decomp.me (100%) N/A -- trivial 7-instruction leaf function, no scratch needed.
 */
u8* field_script_read_u16(u8* data, u16* value)
{
    *value = data[0] + (data[1] << 8);
    return data + 2;
}

extern s32 D_80122B74;
extern s32 D_80122B78;

typedef struct
{
    u8 pad0[0x28];
    u32 unk28;
} StructC1B60;

StructC1B60 *func_800C1B60(s32 arg0);

/**
 * @brief Decode a packed slot value and emit its two sub-fields.
 *
 * Extracts the 16-bit field @c (arg1>>16), splits its low 12 bits into
 * @c v>>5 (written to @p arg2) and @c v&0x1F (written to @p arg3), then
 * selects a return record based on bits 12-14 of the field: for values below
 * 3 it returns @c D_80122B74+0xE4; otherwise @c D_80122B78, additionally
 * folding a per-slot adjustment into @p arg2 when bit 15 is set.
 *
 * @param arg0 Slot handle passed through to func_800C1B60.
 * @param arg1 Packed value; the slot descriptor is its high 16 bits.
 * @param arg2 Out: primary sub-field (updated again on the bit-15 path).
 * @param arg3 Out: secondary sub-field (low 5 bits).
 * @return Selected record pointer/value, or 0 when bit 14 of the field is set.
 * @note 84.21% match (gcc280_g0). Residue is a coupled sched1 emit-order tie:
 *       `arg1>>16` will not schedule ahead of the arg2->s1 parameter-save copy
 *       (a LUID tie-break). Same mechanism as sibling func_800BD3B0.
 */
s32 func_800BD318(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3)
{
    s32 a;
    u32 v;
    s32 result;

    a = arg1 >> 16;
    v = ((unsigned short)a) & 0xFFF;
    *arg2 = v >> 5;
    *arg3 = v & 0x1F;
    if ((((u32)a >> 12) & 7) < 3)
    {
        result = D_80122B74 + 0xE4;
    }
    else
    {
        result = D_80122B78;
        if (a & 0x8000)
        {
            *arg2 += (func_800C1B60(arg0)->unk28 >> 9) & 0x7F;
        }
    }
    return result;
}

extern u8 D_800F0E08[8];

s32 func_800BD318(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
s32 func_800BD650(s32 arg0, u8 *arg1, s32 arg2, s32 arg3, s32 arg4);

/**
 * @brief Adds a value into a per-slot record and forwards it through two calls.
 *
 * Extracts the high 16 bits of @p arg1 as a slot index, calls func_800BD318
 * with that index re-packed into the high half (@p arg0 is passed straight
 * through), then forwards the result plus the two out-params to func_800BD650,
 * indexing @c D_800F0E08 by bits 28-30 of @p arg1.
 *
 * @param arg0 Passed through unchanged to func_800BD318 (kept in a0).
 * @param arg1 Packed value; high 16 bits select the slot, bits 28-30 index
 *             D_800F0E08.
 * @note 68.0% match (gcc280_g0). The residue is a coupled scheduling/register
 *       decision: the target keeps @c arg1>>16 in caller-saved a1 with an s0
 *       copy, flipping both the sra-before-prologue placement and the jal
 *       delay-slot fill. The `result`/`index` split and the `do {} while (0)`
 *       wrapper are required to reproduce the target's separate copy insn
 *       (without them the match drops to 47.6%); do not remove them.
 */
s32 func_800BD3B0(s32 arg0, s32 arg1)
{
    s32 sp18;
    s32 sp1c;
    s32 index;
    s32 result;

    result = arg1 >> 0x10;
    index = result;
    do
    {
        result = func_800BD318(arg0, index << 0x10, &sp18, &sp1c);
        return func_800BD650(2, (u8 *)result, sp18, sp1c, D_800F0E08[((u32) index >> 0xC) & 7]);
    } while (0);
}

s32 func_800BD3B0(s32 arg0, s32 arg1);

void func_800BD414(s32 arg0, s32 arg1)
{
    func_800BD3B0(arg0, arg1 << 0x10);
}

extern u8 D_800F0E08[8];

s32 func_800BD318(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
void func_800BD55C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/**
 * @brief Resolves a packed slot and forwards the resulting field parameters.
 *
 * @param arg0 Passed through to func_800BD318.
 * @param arg1 Packed value whose high half selects the slot.
 * @param arg2 Final argument forwarded to func_800BD55C.
 * @note 72.034485% match with gcc280_g0. The remaining four-row mismatch is
 *       scheduling around the first call; all 29 target instructions and the
 *       0x30-byte stack frame are otherwise represented.
 */
void func_800BD434(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sp18;
    s32 sp1c;
    s32 index;
    s32 result;

    result = arg1 >> 0x10;
    index = result;
    /* Retain the original scheduler's saved-register ordering. */
    arg2++;
    arg2--;
    do
    {
        result = func_800BD318(arg0, index << 0x10, &sp18, &sp1c);
        func_800BD55C(2, result, sp18, sp1c,
                      D_800F0E08[((u32)index >> 0xC) & 7], arg2);
    } while (0);
}

extern u8 D_800F0E08[8];

s32 func_800BD318(s32 arg0, s32 arg1, s32 *arg2, s32 *arg3);
void func_800BD55C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5);

/**
 * @brief Resolves a packed slot and forwards decremented slot metadata.
 *
 * @param arg0 Passed through to func_800BD318.
 * @param arg1 Packed value whose high half selects the slot.
 * @param arg2 Final argument forwarded to func_800BD55C.
 * @note 72.966670% match with gcc280_g0. The remaining four-row mismatch is
 *       scheduling around the first call; all 30 target instructions and the
 *       0x30-byte stack frame are otherwise represented.
 */
void func_800BD4A8(s32 arg0, s32 arg1, s32 arg2)
{
    s32 sp18;
    s32 sp1c;
    s32 index;
    s32 result;

    result = arg1 >> 0x10;
    index = result;
    /* Retain the original scheduler's saved-register ordering. */
    arg2++;
    arg2--;
    do
    {
        result = func_800BD318(arg0, index << 0x10, &sp18, &sp1c);
        func_800BD55C(2, result, sp18, sp1c,
                      D_800F0E08[((u32)index >> 0xC) & 7] - 1, arg2);
    } while (0);
}

void func_800BD434(s32 arg0, s32 arg1, s32 arg2);
void func_800BD4A8(s32 arg0, s32 arg1, s32 arg2);

void func_800BD520(s32 arg0, u32 arg1, s32 arg2)
{
    if (arg1 <= 0xFFFFU)
    {
        func_800BD434(arg0, arg1 << 0x10, arg2);
        return;
    }
    func_800BD4A8(arg0, arg1 << 0x10, arg2);
}

void func_800BD55C(s32 arg0, s32 arg1, s32 arg2, s32 arg3, s32 arg4, s32 arg5)
{
    s32 clearMask;
    s32 valueMask;
    u8 *p8;
    u16 *p16;
    s32 *p32;

    if (arg1 == 0)
    {
        return;
    }
    if (arg1 == -1)
    {
        return;
    }

    if (arg4 < 0x20)
    {
        clearMask = ~(((1 << arg4) - 1) << arg3);
    }
    else
    {
        clearMask = 0;
    }

    if (arg4 < 0x20)
    {
        valueMask = ((1 << arg4) - 1) << arg3;
    }
    else
    {
        valueMask = -1;
    }

    switch (arg0)
    {
    case 0:
        p8 = (u8 *)(arg1 + arg2);
        *p8 = (*p8 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    case 1:
        p16 = (u16 *)((arg2 * 2) + arg1);
        *p16 = (*p16 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    case 2:
        p32 = (s32 *)((arg2 * 4) + arg1);
        *p32 = (*p32 & clearMask) | (valueMask & (arg5 << arg3));
        break;
    }
}

/**
 * @brief Extract a masked bitfield from an 8-, 16-, or 32-bit element.
 *
 * @param arg0 Element width selector: 0 = byte, 1 = halfword, 2 = word.
 * @param arg1 Base address of the element array.
 * @param arg2 Element index.
 * @param arg3 Right-shift amount.
 * @param arg4 Number of low bits to retain; values >= 32 retain all bits.
 * @return The selected element shifted right by @p arg3 and masked to @p arg4 bits.
 * @note 100% match with the FIELD GCC 2.8.0 G0 toolchain.
 */
s32 func_800BD650(s32 arg0, u8 *arg1, s32 arg2, s32 arg3, s32 arg4)
{
    s32 mask;
    s32 value;

    if (arg4 < 0x20)
    {
        mask = (1 << arg4) - 1;
    }
    else
    {
        mask = -1;
    }

    switch (arg0)
    {
    case 0:
        value = arg1[arg2] >> arg3;
        return value & mask;
    case 1:
        value = (*(u16 *)(arg1 + arg2 * 2)) >> arg3;
        return value & mask;
    case 2:
        return (*(u32 *)(arg1 + arg2 * 4) >> arg3) & mask;
    }
}
