#include "common.h"

typedef struct PadContext PadContext;

extern void *bcopy(const void *, void *, int);
extern PadContext *g_pad_ctx;
extern u8 D_8011F430[];

void func_800A6EEC(void)
{
    bcopy(g_pad_ctx, D_8011F430, 0x3268);
}
