#include "common.h"

/** @brief Packed prompt flags accessed as both a word and individual fields. */
typedef union
{
    u32 word;
    struct
    {
        u32 low : 3;
        u32 type : 4;
        u32 priority : 9;
        u32 value : 8;
        u32 high : 8;
    } bits;
} FieldPromptFlags;

/** @brief Packed prompt state with an enable bit and an eight-bit value. */
typedef union
{
    u32 word;
    struct
    {
        u32 enabled : 1;
        u32 value : 8;
        u32 high : 23;
    } bits;
} FieldPromptState;

/** @brief Callback record returned by func_800ADF84. */
typedef struct
{
    FieldPromptFlags flags;
    FieldPromptState state;
    u8 pad8[8];
    void (*callback)(void);
} FieldADF84Rec;

extern void func_800A3938(s32 sound_id, s32 pan);
extern void func_800ADF34(void);
extern FieldADF84Rec *func_800ADF84(void);
extern void func_800A7FB4(void);
extern s32 D_800F229C;
extern s32 D_80122908;

/**
 * @brief Configure a field prompt record for the current selection state.
 */
void func_800A764C(void)
{
    FieldADF84Rec *rec;
    u32 state;

    D_800F229C = 2;
    func_800A3938(0xB9, 0x80);
    func_800ADF34();

    rec = func_800ADF84();
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x40;

    state = rec->state.word & ~0x1FE;
    state |= (((D_80122908 << 4) + 0x10) & 0xFF) << 1;
    rec->flags.bits.value = 0x70 - ((state >> 2) & 0x78);

    rec->callback = func_800A7FB4;
    rec->state.word = state;
    rec->flags.word = (rec->flags.word & 0xFFFFFF) | 0xC0000000;
    rec->state.bits.enabled = 0;
}
