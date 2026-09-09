#include "common.h"

/** @brief Byte fields used to derive the result stats. */
typedef struct
{
    u8 pad[5];
    u8 type, level;
    u8 pad7[6];
    u8 bonus;
    u8 pad_e[14];
    u8 value;
    u8 pad_1d[17];
    u8 stats[6];
    u8 extra;
    u8 pad35[7];
    u8 weights[4];
    u8 multipliers[4];
} Source;
/** @brief Access view for type weights and level divisors. */
typedef struct
{
    u8 pad[8];
    u8 weight;
    u8 pad9[3];
    u8 factor;
    u8 pad_d[0x179];
    u16 divisor;
} Table;
/** @brief Destination fields populated from the selected source. */
typedef struct
{
    u8 pad[0x24];
    u16 power;
    u8 stats[6];
    u8 extra;
    u8 pad_2d;
    u16 value;
    u8 scaled[4];
} Result;
extern Source *D_80123FC4;
extern Table *D_80123FC0;
/**
 * @brief Compute capped power, copy stats, and scale four source multipliers.
 * @param result Destination for the derived fields.
 */
void func_800BFF90(Result *result)
{
    s32 i, sum, bonus, product, power;
    Table *level;
    u16 divisor;
    Source *source;
    i = 0;
    sum = 0;
    do
    {
        sum += ((Table *)((u8 *)D_80123FC0 + (D_80123FC4->type * 12 + i)))->weight *
               D_80123FC4->weights[i];
        i++;
    } while (i < 4);
    i = 0;
    bonus = 0;
    source = D_80123FC4;
    do
    {
        bonus += ((Source *)((u8 *)source + i * 2))->bonus;
        i++;
    } while (i < 8);
    level = (Table *)((u8 *)D_80123FC0 + D_80123FC4->level * 20);
    divisor = level->divisor;
    power = (sum * (bonus + divisor) / divisor) >> 7;
    result->power = power;
    i = 0;
    if ((u32)(power & 0xFFFF) >= 1000)
    {
        result->power = 999;
    }
    do
    {
        result->stats[i] = D_80123FC4->stats[i];
        i++;
    } while (i < 6);
    i = 0;
    result->extra = D_80123FC4->extra;
    result->value = D_80123FC4->value;
    do
    {
        level = (Table *)((u8 *)D_80123FC0 + (D_80123FC4->type * 12 + i));
        result->scaled[i] = (level->factor * D_80123FC4->multipliers[i]) >> 6;
        i++;
    } while (i < 4);
}
