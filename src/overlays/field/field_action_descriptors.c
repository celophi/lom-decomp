#include "common.h"
typedef struct
{
    u8 pad0[0x18];
    u8 *unk18;
    u8 *unk1C;
    u8 *unk20;
    u8 *unk24;
    u8 pad28[0x4A0 - 0x28];
    u16 unk4A0;
    u8 unk4A2;
} DescriptorContext;
extern DescriptorContext *D_80123FB0;
extern u8 *D_80123FAC;
extern u8 *D_80122B74;
extern void akao_set_song_params(s32, s32, s32, s32);
extern void *func_800B543C(s32);
extern void *func_800C2958(s32, u16);

/** @brief Select the active action descriptor and initialize its scaling fields. */
void *func_800B50B8(void)
{
    s32 temp_a0_3;
    s32 temp_a3;
    s32 temp_a3_2;
    s32 temp_v1;
    u32 temp_a0_2;
    u8 temp_a1;
    u8 *temp_a0;
    u8 *temp_a2;
    u8 *temp_t0;
    u8 *temp_v1_2;
    u8 *temp_v1_3;
    u8 *temp_v1_4;
    u8 *var_a1;

    D_80123FB0->unk4A2 = 0U;
    D_80123FB0->unk4A0 = *(u16 *)(D_80123FB0->unk20 + 0x18);
    temp_a0 = D_80123FB0->unk20;
    temp_v1 = *(u32 *)(temp_a0 + 0x4) & 0xFC00;
    if (temp_v1 == 0x1400)
    {
        temp_a2 = D_80123FB0->unk18;
        temp_v1_2 = *(u8 * *)(temp_a0 + 0x14);
        temp_a3 = *(s32 *)(temp_a2 + 0x4);
        if (temp_a3 < *(s32 *)(temp_v1_2 + 0x50))
        {
            var_a1 = temp_v1_2 + ((temp_a3 * 8) + 0x54);
        }
        else
        {
            var_a1 = NULL;
            if ((u32) (temp_a3 - 0x17) >= 2U)
            {
                akao_set_song_params(0x8001, 0x69, *(s32 *)(temp_a2 + 0x0), temp_a3);
                var_a1 = NULL;
            }
        }
    }
    else if ((temp_v1 == 0xC00) || (temp_v1 == 0x1000))
    {
        var_a1 = func_800C2958(2, *(u16 *)(D_80123FB0->unk18 + 4));
    }
    else
    {
        temp_t0 = (temp_a0[4] * 0x250) + D_80122B74 + 0x640;
        D_80123FB0->unk4A2 = (u8) *(u8 *)(temp_t0 + 0x2C);
        temp_a0_2 = *(s32 *)(D_80123FB0->unk18 + 0x4);
        switch (temp_a0_2)
        {
        case 0:
        case 1:
            var_a1 = func_800B543C(D_80122B74[*(s32 *)(D_80123FB0->unk18 + 4) + D_80123FB0->unk20[4] * 0x250 + 0x60A]);
            break;
        case 2:
            var_a1 = D_80123FAC + *(s32 *)(D_80123FAC + 0x0) + (*(u8 *)(temp_t0 + 0x26) * 8) + (*(s32 *)(D_80123FB0->unk18 + 0x8) * 8);
            break;
        case 3:
            var_a1 = D_80123FAC + *(s32 *)(D_80123FAC + 0x0) + (temp_t0[*(s32 *)(D_80123FB0->unk18 + 4) + 0x24] * 8);
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            temp_a3_2 = D_80123FB0->unk20[4] * 0x250;
            temp_a1 = D_80122B74[*(s32 *)(D_80123FB0->unk18 + 4) + (temp_a3_2 - 4) + 0x60C];
            if (temp_a1 < 0x80U)
            {
                var_a1 = D_80123FAC + *(s32 *)(D_80123FAC + 0x4) + ((((((u32) *(u32 *)(temp_t0 + 0x14) >> 0xA) & 0x3F) * 0x18) + temp_a1) * 8);
            }
            else
            {
                temp_v1_3 = D_80122B74 + (temp_a3_2 + 0x5F0) + ((((temp_a1 + 4) & 0x7F) << 6) + 0x50);
                D_80123FB0->unk4A2 = 0U;
                D_80123FB0->unk4A0 = (u16) *(u8 *)(temp_v1_3 + 0x26);
                var_a1 = D_80123FAC + *(s32 *)(D_80123FAC + 0x8) + (*(u8 *)(temp_v1_3 + 0x24) * 0x70) + (*(u8 *)(temp_v1_3 + 0x25) * 8);
            }
            break;
        case 8:
        case 9:
        case 10:
            temp_a0_3 = *(s32 *)(D_80123FB0->unk18 + 0x4);
            var_a1 = D_80123FAC + *(s32 *)(D_80123FAC + 0x0) + (temp_t0[temp_a0_3 + 0x20] * 8);
            if ((u32) (temp_a0_3 - 0x12) < 2U)
            {
                temp_v1_4 = D_80123FB0->unk20;
                *(u8 *)(temp_v1_4 + 0x8) = (u8) (*(u8 *)(temp_v1_4 + 0x8) - 1);
            }
            break;
        default:
            var_a1 = (D_80123FAC + *(s32 *)(D_80123FAC + 0xC) + (temp_a0_2 * 8)) - 0x38;
            break;
        }
    }
    return var_a1;
}


typedef struct
{
    u8 pad0[4];
    u8 unk4;
    u8 pad5[3];
    u8 unk8;
} AkaoChannelB543C;

typedef struct
{
    u8 pad0[0x20];
    AkaoChannelB543C* unk20;
} FieldStateB543C;

typedef struct
{
    u8 pad0[0xC];
    u32 unkC;
} TrackBaseB543C;




/**
 * @brief Compute the next command-stream pointer for a field audio opcode.
 * @param arg0 Audio command opcode.
 * @return Pointer to the next command, or NULL when processing does not continue.
 */
void* func_800B543C(s32 arg0)
{
    void* result;

    result = (u8*)D_80123FAC + ((TrackBaseB543C *)D_80123FAC)->unkC;
    switch (arg0)
    {
    case 0x33:
        func_800B61EC(((FieldStateB543C *)D_80123FB0)->unk20->unk4);
        result = (u8*)result + 0x80;
        break;
    case 0x34:
        result = (u8*)result + 0x78;
        ((FieldStateB543C *)D_80123FB0)->unk20->unk8 -= 1;
        break;
    case 0x32:
        break;
    case 0x36:
        result = (u8*)result + 8;
        break;
    case 0x37:
        result = NULL;
        break;
    case 0x3E:
        result = (u8*)result + 0x10;
        break;
    case 0x43:
        result = (u8*)result + 0x18;
        break;
    case 0x45:
        result = (u8*)result + 0x20;
        break;
    case 0x4F:
        result = (u8*)result + 0x70;
        break;
    default:
        akao_set_song_params(0x8001, 0x66, arg0, -1);
        return NULL;
    }
    return result;
}
