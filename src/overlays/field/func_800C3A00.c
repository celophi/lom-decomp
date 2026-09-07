#include "common.h"

typedef struct
{
    u8 pad0[0x29D4];
    s32 unk29D4;
} Rec29D4;

typedef struct
{
    u8 pad0[0x29D7];
    s8 unk29D7;
} Rec29D7;

extern u8 g_menuLayoutBuffer[];
extern s32 D_80122C00;
extern s16 D_80122C06;
extern s16 D_80122C1A;
extern u8 D_800459AF;

/**
 * @brief Update or remap the active menu-layout slot state.
 * @param arg0 Dispatch selector; 0x92BC selects the slot-swap path.
 */
void func_800C3A00(s32 arg0)
{
    s32 i;
    s32 found;
    s8 ref;
    s32 idx;
    u8* p;
    u8* q;
    u8* slot_found;
    u8* slot_d;
    u8 v;
    s32 refv;

    i = 0;
    if (arg0 == 0x92BC)
    {
        idx = D_80122C00;
        if ((u32)idx < 3)
        {
            p = g_menuLayoutBuffer;
            ref = p[0x29D7];
            do
            {
                if (p[i + 0x29D8] == ref)
                {
                    found = i;
                }
                i++;
            } while (i < 3);

            q = g_menuLayoutBuffer;
            slot_d = idx + q;
            v = slot_d[0x29D8];
            slot_found = found + q;
            slot_found[0x29D8] = v;
            slot_d[0x29D8] = q[0x29D7];
            if (slot_found[0x29D8] == ((Rec29D7*)q)->unk29D7)
            {
                D_80122C06 = 3;
            }
            else
            {
                D_80122C06 = slot_found[0x29D8];
            }
            D_80122C1A = (s8)D_800459AF;
        }
    }
    else
    {
        for (; i < 0x15; i++)
        {
            g_menuLayoutBuffer[i + ((Rec29D7*)g_menuLayoutBuffer)->unk29D7 * 0x14C + 0x2B0C] = g_menuLayoutBuffer[i + 0xA90];
        }
        refv = ((Rec29D7*)g_menuLayoutBuffer)->unk29D7;
        if (refv != 3)
        {
            ((Rec29D4*)g_menuLayoutBuffer)->unk29D4 = (((Rec29D4*)g_menuLayoutBuffer)->unk29D4 & ~0xF0) | ((refv & 0xF) * 0x10);
        }
        g_menuLayoutBuffer[0x29D7] = 3;
    }
}
