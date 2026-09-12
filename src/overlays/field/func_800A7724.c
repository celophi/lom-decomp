#include "common.h"

/** @brief Packed field-process flags. */
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

/** @brief Packed field-process state. */
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

/** @brief Field-process record returned by func_800ADF84. */
typedef struct
{
    FieldPromptFlags flags;
    FieldPromptState state;
    u8 pad8[8];
    void (*callback)(void);
} FieldADF84Rec;

/** @brief Actor record prefix containing the active flag. */
typedef struct
{
    u8 active;
    u8 pad1[0x268 - 1];
} FieldActorEntry;

extern FieldADF84Rec *func_800ADF84(void);
extern FieldActorEntry D_800FD818[];
extern s32 D_800F229C;

void func_800ADF34(void);
void func_800A3938(s32 sound_id, s32 pan);
void func_800A788C(void);
void func_800A7B54(void);

/**
 * @brief Initialize two field processes and position the second from the active actor count.
 */
void func_800A7724(void)
{
    FieldADF84Rec *record;
    s32 actor_index;
    s32 active_count;

    func_800ADF34();
    D_800F229C = 1;
    func_800A3938(0xB9, 0x80);

    active_count = 0;
    record = func_800ADF84();
    actor_index = active_count;
    record->callback = func_800A788C;
    record->flags.bits.type = 1;
    record->flags.bits.priority = 0x20;
    record->flags.bits.value = 0x30;
    record->state.bits.enabled = 1;
    record->state.bits.value = 0x20;
    record->flags.word &= 0xFFFFFF;

    for (actor_index = 0; actor_index < 3; actor_index++)
    {
        if (D_800FD818[actor_index].active & 1)
        {
            active_count++;
        }
    }

    record = func_800ADF84();
    record->callback = func_800A7B54;
    record->flags.bits.type = 1;
    record->flags.bits.priority = 0x20;
    record->flags.bits.value = 0x58;
    record->state.bits.enabled = 1;
    record->state.bits.value = ((active_count * 28) + 16) & 0xFF;
    record->flags.word &= 0xFFFFFF;
}
