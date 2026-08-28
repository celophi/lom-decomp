#include "common.h"

/** @brief 0xC-stride field record; only the status byte at 0x2F0 is read. */
typedef struct
{
    u8 pad0[0x2F0];
    u8 unk2F0;  /* 0x2F0 packed status bits */
} FieldStatusRec;

extern u8 *D_80122B74;

/**
 * @brief Classify the packed status byte of a field record.
 *
 * For an in-range @p arg0 (< 0x40), reads the record's status byte. When its
 * "valid" bit (0x08) is set, returns a code from the low bits: 1 if bit 0 is
 * clear, 2 if bit 1 is clear, 4 if bit 2 is set, otherwise 3. Returns 0 when
 * the record is not valid; for out-of-range @p arg0 it instead notifies the
 * audio driver and returns 0.
 *
 * @param arg0 Record index; >= 0x40 triggers the audio notification path.
 * @return Status code 1-4, or 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800C35E4(s32 arg0)
{
    u8 temp_a0;
    u32 temp_v1;
    u8 *b;

    if (arg0 < 0x40)
    {
        b = D_80122B74;
        temp_a0 = ((FieldStatusRec *)(b + (arg0 * 3 << 2)))->unk2F0;
        temp_v1 = temp_a0 & 0xFF;
        if ((temp_v1 >> 3) & 1)
        {
            if (!(temp_a0 & 1))
            {
                return 1;
            }
            if (!((temp_v1 >> 1) & 1))
            {
                return 2;
            }
            if (((temp_v1 >> 2) & 1) == 0)
            {
                return 3;
            }
            return 4;
        }
    }
    else
    {
        akao_set_song_params(0x8001, 0x73, arg0, 0);
    }
    return 0;
}
