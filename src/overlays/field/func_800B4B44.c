#include "common.h"

extern u8 *D_80123FB0;

u8 *func_800A2E34(void);
u32 func_800B4CE4(void *arg0, s32 arg1);
void func_800B28E0(s32 arg0, s32 arg1, s32 arg2);
void akao_set_song_params();

/**
 * @brief Rebuild indexed field-state byte mappings and trigger dependent handlers.
 */
void func_800B4B44(void)
{
    s32 i;
    s32 j;
    u8 *list;
    u8 *other;
    s32 first;
    s32 second;
    s32 current;
    s32 entry_index;

    i = 0;
    do
    {
        j = 0;
        do
        {
            u8 *entry;
            entry_index = j + i * 0x68;
            j++;
            entry = D_80123FB0 + entry_index;
            entry[0x75] = 0;
        } while (j < 3);
        i++;
    } while (i < 3);

    i = 0;
    list = func_800A2E34();
    while (*list != 0xFF)
    {
        other = list + 1;
        current = *list;
        {
            s32 dst_offset;
            s32 src_offset;
            u8 *base;
            u8 *src;
            u8 *dst;
            u8 value;

            first = *other;
            dst_offset = i + current * 0x68;
            src_offset = first * 0x68;
            base = D_80123FB0;
            src = base + src_offset;
            value = src[0x74];
            dst = base + dst_offset;
            dst[0x75] = value;
        }
        second = *other;
        other += 2;
        first = *list;
        list += 2;
        {
            s32 dst_offset;
            s32 src_offset;
            u8 *base;
            u8 *src;
            u8 *dst;
            u8 value;

            dst_offset = i + second * 0x68;
            src_offset = first * 0x68;
            base = D_80123FB0;
            src = base + src_offset;
            value = src[0x74];
            dst = base + dst_offset;
            dst[0x75] = value;
        }
        i++;
    }

    if (func_800B4CE4(D_80123FB0 + 0x90, 0) < 3)
    {
        func_800B28E0(1, 0xC, 6);
    }
    if (func_800B4CE4(D_80123FB0 + 0xF8, 0) < 3)
    {
        func_800B28E0(2, 0xC, 6);
    }
    if (i >= 4)
    {
        akao_set_song_params(0x8001, 0x6F, i, 0);
    }
}
