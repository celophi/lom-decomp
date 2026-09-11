typedef signed int s32; typedef unsigned int u32; typedef unsigned char u8;
typedef struct { char name[20]; s32 attr; s32 size; void *next; s32 head; char system[4]; } CardaDirEntry;
extern s32 D_80165FEC;
extern s32 D_801660A0;
extern CardaDirEntry D_80166440[][20];
extern s32 D_80166AE8[];
extern s32 D_80166A80[];
extern char D_800ECF7C[];
s32 func_80147490(u8 *);
s32 func_8001714C(void *, void *, s32);

/**
 * @brief Scan directory entries for records whose name matches D_800ECF7C, parse a
 *        hex rank suffix from each, store it, and return the maximum rank seen.
 * @return Highest rank value produced by func_80147490 across matching entries.
 * @see decomp.me (100%)
 */
s32 func_80147588(void)
{
    s32 entry_index;
    s32 max_suffix;
    u8 *cursor;
    u8 *suffix;
    s32 digits_left;
    s32 value;
    u32 decimal_base;
    u32 uppercase_base;
    u32 lowercase_base;
    s32 suffix_value;

    entry_index = 0;
    max_suffix = entry_index;
    while (entry_index < D_80165FEC)
    {
        u8 *pattern;

        pattern = (u8 *)&D_800ECF7C;
        if (func_8001714C(pattern, (u8 *)&D_80166440[D_801660A0][entry_index], 0xC) == 0)
        {
            digits_left = 5;
            cursor = (u8 *)(D_801660A0 * 0x320 + entry_index * 0x28 + (s32)D_80166440 + 0xC);
            value = 0;
            while (((u8)(*cursor - '0') < 10) || ((u8)(*cursor - 'a') < 6) || ((u8)(*cursor - 'A') < 6))
            {
                if (digits_left == 0)
                {
                    break;
                }
                value <<= 4;
                if ((u8)(*cursor - '0') < 10)
                {
                    decimal_base = value - '0';
                    value = decimal_base + *cursor;
                }
                else if ((u8)(*cursor - 'A') < 6)
                {
                    uppercase_base = value - ('A' - 10);
                    value = uppercase_base + *cursor;
                }
                else if ((u8)(*cursor - 'a') < 6)
                {
                    lowercase_base = value - ('a' - 10);
                    value = lowercase_base + *cursor;
                }
                cursor++;
                digits_left--;
            }
            suffix = &D_80166440[D_801660A0][entry_index].name[0xC];
            {
                s32 *fields = &D_80166AE8[D_801660A0 * 20];

                fields[entry_index] = value;
            }
            suffix_value = func_80147490(suffix);
            D_80166A80[entry_index] = suffix_value;
            if (max_suffix < suffix_value)
            {
                max_suffix = suffix_value;
            }
        }
        else
        {
            s32 *fields = &D_80166AE8[D_801660A0 * 20];

            fields[entry_index] = -1;
            D_80166A80[entry_index] = 0;
        }
        entry_index++;
    }
    return max_suffix;
}
