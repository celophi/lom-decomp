#include "common.h"

/** @brief World-map resource record with its initialization field at offset 0x26. */
typedef struct
{
    u8 unknown_0[0x26];
    s16 initial_value;
    u8 unknown_28[4];
} WmapInitResource;

/** @brief World-map display record with a leading state field. */
typedef struct
{
    s16 state;
    u8 unknown_2[0x12];
} WmapInitDisplay;

extern WmapInitResource D_800D9268[];
extern WmapInitDisplay D_801AFBD0[];

/** @brief Initialize the 256 resource and display records. */
void func_800653EC(void)
{
    s32 index;
    for (index = 0; index < 256; index++)
    {
        D_800D9268[index].initial_value = 16;
        D_801AFBD0[index].state = 0;
    }
}
