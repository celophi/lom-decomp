#include "common.h"

typedef struct StateB9C30
{
    u8 pad0[4];
    s32 index;
} StateB9C30;

typedef struct EntryB9C30
{
    u8 pad0[8];
    u8 *pc;
} EntryB9C30;

extern StateB9C30 *g_field_script;
void func_8009AFBC(s32 arg0);

/**
 * @brief Execute one 16-bit operand opcode from the active script entry.
 *
 * Resolves the active entry (state at @c g_field_script, index at 0x4, stride 0xC)
 * and reads a big-endian 16-bit operand from its program counter (offset 0x8),
 * passing the low 15 bits to func_8009AFBC. Then advances the program counter
 * by three bytes.
 *
 * @note gcc280_g0, 100% match.
 */
void func_800B9C30(void)
{
    EntryB9C30 *entry;
    s32 idx;
    s32 note;

    idx = g_field_script->index;
    entry = (EntryB9C30 *)((u8 *)g_field_script + idx * 0xC);
    note = entry->pc[1] + (entry->pc[2] << 8);
    func_8009AFBC(note & 0x7FFF);
    idx = g_field_script->index;
    entry = (EntryB9C30 *)((u8 *)g_field_script + idx * 0xC);
    entry->pc += 3;
}
