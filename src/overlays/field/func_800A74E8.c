#include "common.h"

/** @brief Packed flags accessed as both a word and individual fields. */
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

/** @brief Partial slot record exposing the halfword cleared by this routine. */
typedef struct
{
    u8 pad0[0x260];
    s16 unk260;
    u8 pad262[0x268 - 0x262];
} RecFD818;

extern void func_800ADEB0(void);
extern void func_800A3938(s32 sound_id, s32 pan);
extern FieldADF84Rec *func_800ADF84(void);
extern void func_800AE8A8(void);
extern void func_800AED20(void);
extern void func_800B661C(s32 arg0, FieldADF84Rec *arg1);
extern void akao_cmd_f1(void);
extern void func_800AE9E0(void);
extern s32 g_field_return_to_title_prompt_delay;
extern s32 g_field_return_to_title_prompt_state;
extern s32 D_801226D8;
extern RecFD818 D_800FD818[];

/**
 * @brief Configure the return-to-title prompt and reset its three slot values.
 * @note The mixed word and bitfield updates preserve the original store widths.
 */
void func_800A74E8(void)
{
    FieldADF84Rec *rec;
    u32 state;
    s32 i;

    func_800ADEB0();
    func_800A3938(0xB9, 0x80);

    rec = func_800ADF84();
    rec->callback = func_800AE8A8;
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x20;
    state = (rec->state.word | 1) & ~0x1FE;
    rec->flags.bits.value = 0x30;
    state |= 0x60;
    rec->state.word = state;
    rec->flags.word &= 0xFFFFFF;

    rec = func_800ADF84();
    rec->callback = func_800AED20;
    rec->flags.bits.type = 1;
    rec->flags.bits.priority = 0x50;
    rec->state.bits.enabled = 0;
    rec->state.bits.value = 0x22;
    rec->flags.bits.value = 0x80;

    rec->flags.word = (rec->flags.word & 0xFFFFFF) | 0xA0000000;

    /* Keep the shared -2 value in the target's argument register. */
    do
    {
        func_800B661C(-2, rec);
    } while (0);
    akao_cmd_f1();

    g_field_return_to_title_prompt_delay = 0x3C;
    g_field_return_to_title_prompt_state = 3;
    D_801226D8 = 0;
    func_800AE9E0();

    for (i = 2; i >= 0; i--)
    {
        D_800FD818[i].unk260 = 0;
    }
}
