#include "common.h"

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

extern TrackBaseB543C* D_80123FAC;
extern FieldStateB543C* D_80123FB0;

/**
 * @brief Compute the next command-stream pointer for a field audio opcode.
 * @param arg0 Audio command opcode.
 * @return Pointer to the next command, or NULL when processing does not continue.
 */
void* func_800B543C(s32 arg0)
{
    void* result;

    result = (u8*)D_80123FAC + D_80123FAC->unkC;
    switch (arg0)
    {
    case 0x33:
        func_800B61EC(D_80123FB0->unk20->unk4);
        result = (u8*)result + 0x80;
        break;
    case 0x34:
        result = (u8*)result + 0x78;
        D_80123FB0->unk20->unk8 -= 1;
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
