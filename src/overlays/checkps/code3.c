#include "checkps.h"

/**
 * decomp.me link (100%) https://decomp.me/scratch/cjji6
 * PsyQ 4.3 / gcc 2.8.0
 */
void func_80051830(u32 arg0, void* arg1, s32 arg2)
{
    u32 end;
    int unk0;
    unsigned int new_var2;
    u32 new_var;
    s32 local_arg2 = arg2;
    end = arg0 + strlen((char*)arg0);
    unk0 = ((arg1struct*)arg1)->unk0;
    if (arg0 < end)
    {
        u8 nl = 0x0A;
        (void)nl;

        new_var = end;
        do
        {
            if ((*((u8*)arg0)) == nl)
            {
                ((arg1struct*)arg1)->unk0 = unk0;
                ((arg1struct*)arg1)->unk2 += 0x12;
            }
            else
            {
                int temp_a0 = *((u8*)arg0);
                arg0++;
                func_80051908(arg1, (u8*)Krom2RawAdd((*((u8*)arg0)) | (temp_a0 << 8)), local_arg2);
                ((arg1struct*)arg1)->unk0 =
                    (end = (new_var2 = (s16)(((0x11 * 0, (u16)((arg1struct*)arg1)->unk0)) + 0x11)));
            }
            arg0++;
        } while (arg0 < new_var);
    }
}

/**
 * decomp.me link (100%) https://decomp.me/scratch/PIDIi
 * matches under Psy-Q 4.3 / gcc 2.8.0
 */
void func_80051908(void* arg0, u8* arg1, s32 arg2)
{
    /* No intermediate pointer — reference arg0 directly so it gets s1 first */
    u8* var_s2 = arg1;     /* s2 */
    s16 local_arg2 = arg2; /* s4 */

    /* Force exact stack offsets: tag=0x10(sp), code=0x14(sp),
       xy=0x18(sp), wh=0x1C(sp), pixels=0x20(sp) */
    struct
    {
        s32 tag;
        s32 code;
        s32 xy;
        s32 wh;
        s16 pixels[16];
    } packet;

    s32 temp_s5; /* sign-extended, kept in s5 */
    s32 temp_s6; /* sign-extended, kept in s6 */
    s32 var_s3;  /* kept in s3, zeroed after loading arg->unk4 */
    s32 var_s0;
    s16* writePtr;
    s16* pixelPtr;
    int bit;
    s16 pixelValue;

    /* Exact instruction order from target */
    temp_s5 = ((Arg0Struct*)arg0)->unk0; /* first use of arg0 → allocates s1 */
    temp_s6 = ((Arg0Struct*)arg0)->unk2;

    packet.tag = 0x0B000000;  // packet is 11 words long
    packet.code = 0xA0000000; /* lui/sw -> 0x14(sp) */

    {
        s32 tmp = ((Arg0Struct*)arg0)->unk4; /* lw v0,4(s1) */
        packet.wh = tmp;                     /* sw v0,0x1c(sp) */
    }

    /* The two nested loops – identical to target */
    for (var_s3 = 0; var_s3 < 15; var_s3++)
    {
        writePtr = packet.pixels; /* addiu a2,sp,0x20 */

        for (var_s0 = 0; var_s0 < 2; var_s0++)
        {
            for (bit = 7; bit >= 0; bit--)
            {
                pixelPtr = writePtr;     /* move a1,a2 */
                writePtr = pixelPtr + 1; /* addiu a2,a1,2 */
                pixelValue = 0;

                if (((s32)(*var_s2) >> bit) & 1)
                {
                    pixelValue = local_arg2;
                }

                *pixelPtr = pixelValue; /* sh a0,0(a1) */
            }

            var_s2++;
        }

        for (var_s0 = 0; var_s0 < 2; var_s0++)
        {
            packet.xy = *(s32*)&((Arg0Struct*)arg0)->unk0; /* lw v0,0(s1); sw v0,0x18(sp) */
            DrawPrim(&packet);

            ((Arg0Struct*)arg0)->unk0 = (s16)((u16)((Arg0Struct*)arg0)->unk0 + 1);
        }

        ((Arg0Struct*)arg0)->unk0 = (s16)temp_s5;
        ((Arg0Struct*)arg0)->unk2 = (s16)((u16)((Arg0Struct*)arg0)->unk2 + 1);
    }

    ((Arg0Struct*)arg0)->unk2 = (s16)temp_s6;
}