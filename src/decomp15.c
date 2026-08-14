#include "common.h"
#include "psyq/libspu.h"

typedef struct
{
    u8  *buf;        /* 0x00 */
    u8   pad0[0x08]; /* 0x04 - 0x0B */
    s32  unkC;       /* 0x0C */
    u8   pad1[0x04]; /* 0x10 - 0x13 */
    s32  unk14;      /* 0x14 */
} AkaoXaTracker;

extern AkaoXaTracker g_akao_xa_tracker;
extern s32 g_akao_spu_xfer_pending;
extern u32 D_8004F76C[];

void spu_set_key_on(u32 voice_mask);
void func_8002D140(void);

/**
 * @see decomp.me (100%)
 */
void func_8002D694(int a0, int a1, void (*cb)())
{
    if (g_akao_xa_tracker.unkC != 0)
    {
        SpuSetTransferCallback(0);
        g_akao_spu_xfer_pending = 0;
        if ((u32)g_akao_xa_tracker.unk14 > 0x1000)
        {
            g_akao_xa_tracker.unk14 -= 0x1000;
            g_akao_xa_tracker.buf += a0;
            SpuSetIRQCallback(cb);
        }
        else
        {
            SpuSetIRQCallback(func_8002D140);
            a1 = 0x1030;
            g_akao_xa_tracker.unk14 = 0;
        }
        SpuSetIRQAddr(a1 + 8);
        spu_set_key_on(D_8004F76C[0]);
        SpuSetIRQ(1);
    }
}
