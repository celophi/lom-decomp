#include "common.h"

typedef struct Inner {
    u32 unk0;
    s32 unk4;
    u8 pad8[4];
    u32 unkC;
} Inner;

typedef struct Other {
    u8 pad0[0x3F];
    u8 unk3F;
} Other;

typedef struct Rec {
    u8 pad0[4];
    u8 unk4;
    u8 pad5[0xB];
    Inner *unk10;
    Other *unk14;
} Rec;

/**
 * @see decomp.me (100%)
 */
void func_800B4E60(Rec *arg0)
{
    s32 var_a2;
    u32 var_v0;
    s32 var_v1;
    Inner *var_a0;
    Inner *var_a1;

    var_v0 = arg0->unk4;
    var_a1 = arg0->unk10;
    var_v0 = var_v0 < 3U;
    var_a2 = var_a1->unk4;
    if (var_v0 != 0) {
        var_v0 = var_a1->unkC;
        var_v0 &= 0x190;
        if (var_v0 != 0) {
            var_v0 = var_a1->unk0;
            var_v0 >>= 5;
            do {
                var_v1 = 1;
                if (var_v0 != 0) {
                    var_v1 = var_v0;
                }
                var_a2 -= var_v1;
            } while (0);
            var_v0 = 1;
            if (var_a2 > 0) {
                var_a1->unk4 = var_a2;
                return;
            }
            var_a1->unk4 = var_v0;
        }
    } else {
        var_v0 = var_a1->unkC;
        var_v0 &= 0x191;
        if (var_v0 != 0) {
            var_v0 = arg0->unk14->unk3F;
            var_v0 &= 0x80;
            do {
                var_v1 = 1;
                if (var_v0 != 0) {
                    var_v0 = var_a1->unk0;
                    var_v0 >>= 8;
                } else {
                    var_v0 = var_a1->unk0;
                    var_v0 >>= 5;
                }
                if (var_v0 != 0) {
                    var_v1 = var_v0;
                }
                var_a2 -= var_v1;
            } while (0);
            var_a0 = arg0->unk10;
            var_v0 = 1;
            if (var_a2 > 0) {
                var_a0->unk4 = var_a2;
                return;
            }
            var_a0->unk4 = var_v0;
        }
    }
}
