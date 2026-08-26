#include "common.h"

typedef struct
{
    u8 pad0[0xA0];
    u8 *unkA0;   /* 0xA0 */
    u8 *unkA4;   /* 0xA4 */
    u8 unkA8;    /* 0xA8 */
    u8 unkA9;    /* 0xA9 */
    s16 unkAA;   /* 0xAA */
    u8 padAC[2]; /* 0xAC */
    s16 unkAE;   /* 0xAE */
    s32 unkB0;   /* 0xB0 */
} ResEntry;

extern ResEntry g_field_resource_entries;
extern u8 D_800EB274[];

void func_8009BCAC(void)
{
    s32 tmp;

    g_field_resource_entries.unkA9 = 7;
    g_field_resource_entries.unkA0 = D_800EB274;
    g_field_resource_entries.unkA4 = D_800EB274 + 0x3C;
    tmp = g_field_resource_entries.unkB0 & ~1;
    g_field_resource_entries.unkB0 = tmp;
    g_field_resource_entries.unkAA = 0;
    g_field_resource_entries.unkA8 = 0;
    g_field_resource_entries.unkAE = 0;
    g_field_resource_entries.unkB0 = tmp | 2;
}
