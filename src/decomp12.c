#include "common.h"
#include "decomp4.h"      /* AkaoChannelState + akao_driver.h (SfxControl, AkaoXaTracker, ...) */
#include "decomp9.h"      /* spu_set_key_off and the SPU register writers */
#include "psyq/libspu.h"  /* SpuSetIRQ, SpuWrite, SpuSetTransfer* */

void akao_release_channels(AkaoChannelState* channel, u32 release_mask);
void func_800266B0(s32 arg0, s32 arg1);
void func_8002D764(void);

/**
 * @see decomp.me (100%)
 */
void func_8002D0FC(void *p)
{
    *(u16 *)((char *)p + 0xDA) = *(*(u8 **)p)++;
}

/**
 * @see decomp.me (100%)
 */
void func_8002D118(void *p)
{
    *(u16 *)((char *)p + 0xDA) = 0;
}

/**
 * @see decomp.me (100%)
 */
void func_8002D120(AkaoChannelState* arg0, u32 arg1)
{
    akao_release_channels(arg0, arg1);
}

/**
 * @see decomp.me (100%)
 */
void func_8002D140(void)
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        SpuSetIRQ(0);
        SpuSetIRQCallback(0);
        spu_set_key_off(g_akao_xa_tracker.unkC);
        g_akao_sfx_control.noise_mask &= ~g_akao_xa_tracker.unkC;
        g_akao_xa_tracker.unkC = 0;
        g_akao_driver_flags.unk8 |= 0x100;
    }
}

/**
 * @see decomp.me (94.44%)
 */
s32 func_8002D1C4(void)
{
    s32 combined;
    s32 count;
    s32 result;
    u32 mask;

    for (mask = 0xC00000;; mask = 0xC00000)
    {
        count = 0xB;
        combined = g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10;
    loop_2:
        if (combined & mask)
        {
            count -= 1;
            mask >>= 1;
            if (count != 0)
            {
                goto loop_2;
            }
        }
        result = count + 0xB;
        if (count != 0)
        {
            return result;
        }
        func_800266B0(0, 0x40000000);
        if (combined == (s32)(g_akao_sfx_control.unk0 | g_akao_sfx_control.unk10))
        {
            return -1;
        }
    }
}

/**
 * @brief Private XA-tracker view exposing the SPU buffer base at offset 0.
 * @note Distinct name from akao_driver.h's AkaoXaTracker so the two views of
 *       the same g_akao_xa_tracker global coexist in one translation unit.
 */
typedef struct
{
    u8 *buf;         /* 0x00 - SPU buffer base */
    u8  pad[0x48];   /* 0x04 - 0x4B */
} XaTracker;

/**
 * @see decomp.me (100%)
 */
void func_8002D254(void)
{
    u8 *addr = ((XaTracker *)&g_akao_xa_tracker)->buf + 0x800;
    SpuSetTransferStartAddr(0x2100);
    SpuSetTransferCallback(func_8002D764);
    SpuWrite(addr, 0x800);
}
