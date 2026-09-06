#include "common.h"

typedef struct
{
    u8 id;
    u8 pad[0x93];
} FieldActorRec1F10;

typedef struct
{
    u8 pad0[0x400];
    union { u16 count; u32 flags; } actors;
    u8 pad404[0x418 - 0x404];
    u32 state418;
    u8 pad41C[0x430 - 0x41C];
    FieldActorRec1F10 rec[16];
    u8 padD70[0xE98 - (0x430 + 16 * 0x94)];
    u32 script_status;
    s32 script_depth;
    u8 *script_pc;
} FieldState1F10;

extern FieldState1F10 *D_80122B78;
extern s32 D_8010AE78;
extern void field_script_run(void *ctx);
extern void func_800B286C(u8 id, s32 arg1, s32 arg2);
extern s32 func_800BD414(s32 arg0, s32 arg1);
extern void func_800B177C(void);
extern s32 func_8006751C(s32 arg0);

/**
 * @brief Advance the active field script state and dispatch pending actor commands.
 */
void func_800B1F10(void)
{
    s32 i;

    {
        u8 *script_record;
        script_record = (u8 *)D_80122B78;
        script_record += D_80122B78->script_depth * 12;
        if (*(u32 *)(script_record + 0xEA0) != 0)
        {
            field_script_run((u8 *)D_80122B78 + 0xE98);
            return;
        }
    }

    if (D_8010AE78 != 0)
    {
        i = 0;
        if (D_80122B78->actors.count != 0)
        {
            do
            {
                func_800B286C(D_80122B78->rec[i].id, 0xD, 0x82);
                i++;
            } while (i < (s32)D_80122B78->actors.count);
        }
        if ((((D_80122B78->state418 >> 30) & 1) == 0) && (func_800BD414(0, 0xFE2) == 0))
        {
            func_800B177C();
        }
    }
    else if ((D_80122B78->actors.flags & 0x80000) && (func_8006751C(0) == -1))
    {
        i = 0;
        if (D_80122B78->actors.count != 0)
        {
            do
            {
                func_800B286C(D_80122B78->rec[i].id, 0xD, 0x85);
                i++;
            } while (i < (s32)D_80122B78->actors.count);
        }
        D_80122B78->actors.flags &= 0xFFF7FFFF;
    }
}
