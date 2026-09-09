#include "common.h"

/** @brief Partial FIELD state used by the frame/update dispatcher (see func_800B19FC.c). */
typedef struct
{
    u8 pad0[0xBC];
    s32 unkBC;
    u8 padC0[0x400 - 0xC0];
    s32 unk400;
    u8 pad404[0x418 - 0x404];
    s32 unk418;
} FieldStateB19FC;

extern s32 *D_80123FB0;
extern FieldStateB19FC *D_80122B78;

void func_800B4B44(void);
void func_800B4D1C(void *arg0);
void func_800B4DF0(void *arg0);
void func_800B4E60(void *arg0);
void func_800B4F38(void *arg0);
void func_800B4F80(void *arg0);
s32 func_800BD414(s32 arg0, s32 arg1);

/**
 * @brief Run the two update passes over eleven flagged field records.
 * @note The first three records receive additional updates in the first pass.
 * @note WIP: address recomputation and temporary-register differences remain.
 */
void func_800B49C0(void)
{
    s32 var_s0;
    s32 var_s1;
    s32 var_s2;
    s32 keep;

    if ((D_80123FB0 != NULL) && (*D_80123FB0 >= 0))
    {
        var_s1 = 0;
        func_800B4B44();
        var_s0 = 0x28;
        var_s2 = 0;
        do
        {
            if ((*(u32 *)((u8 *)D_80123FB0 + var_s2 + 0x2C) >> 8) & 1)
            {
                if (func_800BD414(0, 0xFFD) == 0)
                {
                    keep = var_s1 < 3;
                }
                else
                {
                    if (*((u8 *)D_80123FB0 + var_s2 + 0x2C) < 2)
                    {
                        *(u16 *)(*(u8 **)((u8 *)D_80123FB0 + var_s2 + 0x38) + 0x48) = 0xFF;
                    }
                    keep = var_s1 < 3;
                }
                if (keep != 0)
                {
                    func_800B4D1C((u8 *)D_80123FB0 + var_s0);
                    func_800B4F80((u8 *)D_80123FB0 + var_s0);
                }
                func_800B4DF0((u8 *)D_80123FB0 + var_s0);
            }
            var_s0 += 0x68;
            var_s1 += 1;
            var_s2 += 0x68;
        } while (var_s1 < 0xB);

        var_s1 = 0;
        if (!(D_80122B78->unkBC & 0xF))
        {
            var_s0 = 0x28;
            var_s2 = 0;
            do
            {
                if ((*(u32 *)((u8 *)D_80123FB0 + var_s2 + 0x2C) >> 8) & 1)
                {
                    func_800B4E60((u8 *)D_80123FB0 + var_s0);
                    func_800B4F38((u8 *)D_80123FB0 + var_s0);
                }
                var_s0 += 0x68;
                var_s1 += 1;
                var_s2 += 0x68;
            } while (var_s1 < 0xB);
        }
    }
}
