#include "common.h"

typedef struct
{
    s32 unk0;
    s32 unk4;
    s32 unk8;
    u32 unkC;
    s16 unk10;
    s16 unk12;
    s16 unk14;
    s16 unk16;
    u8 unk18;
    u8 unk19;
    u8 unk1A;
    u8 unk1B;
    s32 unk1C;
    u8 pad20[0x21 - 0x20];
    u8 unk21;
    u8 unk22;
    u8 unk23;
    u8 unk24;
    u8 unk25;
    u8 pad26[0x27 - 0x26];
    u8 unk27;
    u8 unk28;
    u8 pad29[0x2A - 0x29];
    s16 unk2A;
    s16 unk2C;
    u16 unk2E;
    s16 unk30;
    u8 unk32;
    u8 unk33;
    u8 unk34;
    u8 unk35;
    u8 unk36;
    u8 unk37;
    u8 unk38;
    u8 pad39[0x3A - 0x39];
    u8 unk3A;
    u8 unk3B;
    u32 unk3C;
    s32 unk40;
    u32 unk44;
    u32 unk48;
    u32 unk4C;
    u8 pad50[0x54 - 0x50];
} Struct_D800FDF58;

typedef struct
{
    s16 unk0;
    s16 unk2;
    s16 unk4;
    s16 unk6;
} Rec80122B28;

Struct_D800FDF58 *func_80087C9C(s32 arg0);

extern s32 D_80122B10;
extern Rec80122B28 D_80122B28[];

/**
 * @brief Update an existing pending actor entry or append a new one.
 * @param arg0 Actor identifier used to resolve the source record.
 * @param arg1 Value stored in the entry's second field.
 * @param arg2 Value stored in the entry's third field and used to select its reset value.
 * @param arg3 Value stored in the entry's fourth field.
 * @return Zero on success, or -1 when no entry can be created.
 */
s32 func_800B0710(s32 arg0, s32 arg1, s32 arg2, s32 arg3)
{
    Struct_D800FDF58 *rec;
    s32 i;
    Rec80122B28 *p;

    if (D_80122B10 == 8)
    {
        return -1;
    }
    rec = func_80087C9C(arg0);
    if (rec == (Struct_D800FDF58 *)-1)
    {
        return -1;
    }
    if (rec->unk25 == 0xFF)
    {
        return -1;
    }
    i = 0;
    if (D_80122B10 > 0)
    {
        do
        {
            p = &D_80122B28[i];
            if (p->unk0 == rec->unk3A)
            {
                p->unk0 = (s16)rec->unk3A;
                p->unk2 = arg1;
                p->unk4 = arg2;
                if (arg2 != -1)
                {
                    p->unk2 = 0;
                }
                p->unk6 = arg3;
                return 0;
            }
            i += 1;
        } while (i < D_80122B10);
    }
    {
        Rec80122B28 *base = D_80122B28;
        s32 idx = D_80122B10;

        p = &base[idx];
    }
    p->unk0 = (s16)rec->unk3A;
    p->unk2 = arg1;
    p->unk4 = arg2;
    if (arg2 != -1)
    {
        p->unk2 = 0xA;
    }
    D_80122B28[D_80122B10].unk6 = arg3;
    D_80122B10 += 1;
    return 0;
}
