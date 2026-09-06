#include "field_script.h"

/** @brief One entry of the shop item list built on the stack. */
typedef struct
{
    s16 price;
    s16 pad2;
    s32 scaled;
} ShopItemEntry;

u8 *field_script_read_operand(u32 type, u8 *data, s32 *value);
u8 *func_800C1E40(s32 arg0);
void field_open_shop_mode_1(s32 arg0, s32 arg1, s32 arg2, s32 arg3);

/**
 * @brief Build the current shop item list from field-script operands and open the shop interface.
 */
void func_800BB3D8(void)
{
    u8 *operands;
    u8 descriptor;
    s32 shop_id;
    s32 scale;
    u8 *list_base;
    u8 *header;
    ShopItemEntry local_buf[32];
    u8 *resource;
    s32 i;
    s32 kind;
    u32 lo;
    u32 word;

    operands = FIELD_SCRIPT_ACTIVE_RECORD()->pc;
    descriptor = operands[1];
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_0(descriptor), operands + 2, &shop_id);
    FIELD_SCRIPT_ACTIVE_RECORD()->pc = field_script_read_operand(OPERAND_TYPE_1(descriptor), FIELD_SCRIPT_ACTIVE_RECORD()->pc, &scale);

    list_base = func_800C1E40(0xA);
    header = list_base + *(s32 *)(list_base + shop_id * 4 + 4);

    resource = func_800C1E40(5);
    i = 0;

    if (*(s32 *)header != 0)
    {
        do
        {
            kind = *(header + i * 4 + 4);
            word = *(u32 *)(header + i * 4 + 4);
            local_buf[i].pad2 = 0;
            local_buf[i].price = (s16)(kind + ((word << 7) & 0x8000));
            lo = (*(u32 *)(header + i * 4 + 4) >> 9) * scale;
            local_buf[i].scaled = (s32)(lo >> 4);
            i++;
        } while ((u32)i < *(s32 *)header);
    }

    field_open_shop_mode_1(*(s32 *)header, (s32)local_buf, (s32)(resource + 4), 2);
}
