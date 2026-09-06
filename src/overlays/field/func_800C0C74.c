#include "common.h"

typedef struct Quad
{
    u8 pad0[0x60];
    u8 unk60;
    u8 unk61;
    u8 unk62;
    u8 unk63;
} Quad;

typedef struct Flag
{
    u8 pad0[0x3F];
    u8 unk3F;
} Flag;

typedef struct Rec
{
    u8 pad0[0xC];
    s32 unkC;
    Quad *unk10;
    Flag *unk14;
} Rec;

/**
 * @brief Decode packed directional flags into the record output bytes.
 * @param arg0 Unused context pointer.
 * @param arg1 Packed directional value.
 * @param arg2 Record receiving the decoded values.
 * @return Constant command length value 0x1F.
 */
s32 func_800C0C74(void *arg0, s32 arg1, Rec *arg2)
{
    s32 temp_a0;
    s32 var_v1;

    var_v1 = arg1 >> 4;
    temp_a0 = arg2->unkC;
    arg1 = arg1 & 0xF;
    if (temp_a0 < 0)
    {
        arg1 += var_v1;
        var_v1 = 0;
    }
    if (temp_a0 & 0x20000000)
    {
        arg1 += 2;
    }
    if (temp_a0 & 0x10000000)
    {
        arg1 += 1;
    }
    if (temp_a0 & 0x02000000)
    {
        var_v1 += 2;
    }
    if (temp_a0 & 0x01000000)
    {
        var_v1 += 1;
    }
    if (arg2->unk14->unk3F & 0x80)
    {
        arg2->unk10->unk60 = (s8) var_v1;
        arg2->unk10->unk61 = 0;
        arg2->unk10->unk62 = (s8) arg1;
        arg2->unk10->unk63 = 0;
    }
    else
    {
        arg2->unk10->unk60 = 0;
        arg2->unk10->unk61 = (s8) var_v1;
        arg2->unk10->unk62 = 0;
        arg2->unk10->unk63 = (s8) arg1;
    }

    return 0x1F;
}
