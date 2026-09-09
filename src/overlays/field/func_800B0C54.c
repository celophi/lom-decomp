#include "common.h"

typedef struct
{
    u8 unk0;
    u8 _pad1[3];
    u8* unk4;
} FieldTextMacro;

extern FieldTextMacro D_80122B80[];
extern u8* D_80122B74;
extern s16 D_800EF600;

s32 func_800BD414(s32 arg0, s32 arg1);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);

/**
 * @brief Initialize the reserved field text macros and their default selection state.
 */
void func_800B0C54(void)
{
    s32 slot;
    s32 last_slot;
    FieldTextMacro* macros;
    FieldTextMacro* macro;

    slot = 0;
    macros = D_80122B80;
    last_slot = 0xF;
    do
    {
        macro = (FieldTextMacro*)(((last_slot - slot) * sizeof(*macro)) + (u32)macros);
        macro->unk0 = 0x15;
        macro->unk4 = D_80122B74 + 0x5F0 + slot * 0x250;
        slot++;
    } while (slot < 3);

    D_80122B80[0xC].unk0 = 0xFF;
    D_80122B80[0xC].unk4 = (u8*)&D_800EF600 + *(s16*)((u8*)&D_800EF600 + ((*(u16*)(D_80122B74 + 0x2E6) & 0x7F) * 2));

    if ((func_800BD414(0, 0xA02) != 0) || ((*(s32*)(D_80122B74 + 0x858) & 0x80) != 0))
    {
        func_800BD520(0, 0xA03, 1);
    }
    else
    {
        func_800BD520(0, 0xA03, 0);
    }
}
