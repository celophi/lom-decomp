typedef int s32;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
void akao_set_song_params(s32, s32, s32, s32); /* extern */
s32 func_800C19D0(s32, s32, s32);              /* extern */
/** Packed state accessed at byte, halfword, and word widths by the game. */
typedef union PackedWord
{
    u32 word;
    u8 bytes[4];
    u16 halves[2];
} PackedWord;
/** One 0x60-byte character record containing level and stat-growth state. */
typedef struct Record
{
    u8 active;
    u8 pad01[0x17];
    PackedWord progress;
    u16 hp, mp;
    u16 values[4];
    u16 stats[8];
    u8 pad38[12];
    PackedWord status;
    u8 pad48[4];
    u8 growth[8];
    u8 value_growth[4];
    PackedWord extra;
    u8 pad5c[4];
} Record;
extern u8 *D_80122B74;

/**
 * Apply every pending level increase for an active character slot.
 *
 * Updates packed stat growth, fractional carries, and derived values, then clears
 * the three cached status bytes. The level is capped at 99.
 * @param slot Character slot; values at least five take the original error-sound path.
 * @note Partial assembly match; retained probes document the remaining scheduling differences.
 */
void func_800C1230(s32 slot)
{
    s32 previous_level;
    s32 scaled_level;
    s32 mask;
    s32 threshold;
    s32 updated_extra;
    s32 growth_nibble;
    s32 growth_high;
    s32 stat_index;
    s32 index;
    s32 pending;
    u16 stat;
    u32 extra_word;
    u32 experience;
    u32 growth_value;
    u32 carry;
    u32 base_stat;
    s32 level;
    u32 next_level;
    u8 *value_growth_cursor;
    u8 *growth_cursor;
    u8 *record;
    u8 *stat_cursor;
    u8 *value_cursor;
    u8 *status_cursor;

    if (slot >= 5)
    {
        akao_set_song_params(0x8001, 0x1F3, slot, 0);
        return;
    }
    record = D_80122B74 + ((slot * 0x60) + 0x2EF4);
    pending = -1;
    if (((Record *)record)->active != 0)
    {
        mask = -0xF1;
        do
        {
            level = ((Record *)record)->progress.bytes[0];
            previous_level = level - 1;
            scaled_level = level * 5;
            threshold = previous_level * (scaled_level << 2) + (scaled_level << 1);
            experience = ((Record *)record)->progress.word >> 8;
            if ((experience != 0) && (experience >= (u32)threshold))
            {
                next_level = level + 1;
                ((Record *)record)->progress.bytes[0] = next_level;
                stat_index = 0;
                if ((u32)(next_level & 0xFF) >= 0x64U)
                {
                    ((Record *)record)->progress.bytes[0] = 0x63U;
                    goto block_17;
                }
                stat_cursor = record;
            stat_loop:
            {
                growth_cursor = record + stat_index;
                stat = ((Record *)stat_cursor)->stats[0];
                stat =
                    ((((u8)((Record *)growth_cursor)->growth[0] >> 4) + (stat & 0x1FF)) & 0x1FF) |
                    (stat & 0xFE00);
                ((Record *)stat_cursor)->stats[0] = stat;
                if ((u32)(stat & 0x1FF) >= 0x18DU)
                {
                    ((Record *)stat_cursor)->stats[0] = (u16)((stat & 0xFE00) | 0x18C);
                }
                stat_index += 1;
                base_stat = ((Record *)stat_cursor)->stats[0] & 0x1FF;
                ((Record *)stat_cursor)->stats[0] = (u16)(base_stat | ((base_stat >> 2) << 9));
                growth_nibble = ((Record *)growth_cursor)->growth[0] & 0xF;
                ((Record *)growth_cursor)->growth[0] = (u8)(growth_nibble | (growth_nibble * 0x10));
                stat_cursor += 2;
            }
                if (stat_index < 8)
                {
                    goto stat_loop;
                }
                index = 0;
                value_cursor = record;
            value_loop:
            {
                value_growth_cursor = record + index;
                index += 1;
                ((Record *)value_cursor)->values[0] =
                    (u16)(((Record *)value_cursor)->values[0] +
                          ((u8)((Record *)value_growth_cursor)->value_growth[0] >> 7));
                growth_value = ((Record *)value_growth_cursor)->value_growth[0] & 0x7F;
                growth_high = growth_value >> 4;
                growth_value &= 15;
                ((Record *)value_growth_cursor)->value_growth[0] =
                    growth_value | ((growth_high + growth_value) << 4);
                value_cursor += 2;
            }
                if (index < 4)
                {
                    goto value_loop;
                }
                carry = ((Record *)record)->extra.bytes[0] >> 7;
                extra_word = (mask | 0x70) & (s32)((Record *)record)->extra.word;
                ((Record *)record)->extra.word = extra_word;
                ((Record *)record)->mp = (u16)(((Record *)record)->mp + carry);
                updated_extra =
                    (extra_word & mask) |
                    (((((extra_word >> 4) & 0xF) + (((Record *)record)->extra.bytes[0] & 0xF)) &
                      0xF) *
                     0x10);
                ((Record *)record)->extra.word = updated_extra;
                ((Record *)record)->hp = func_800C19D0(
                    ((Record *)record)->hp, (u32)(((Record *)record)->stats[4] & 0x1FF) >> 2, 3);
                index = 2;
                status_cursor = record + index;
                ((Record *)record)->extra.halves[1] =
                    (u16)(((Record *)record)->extra.halves[1] + ((Record *)record)->extra.bytes[1]);
                ((Record *)record)->status.word =
                    (s32)(((Record *)record)->status.word & 0xF8FFFFFF);
                do
                {
                    ((Record *)status_cursor)->status.bytes[0] = 0xFF;
                    index -= 1;
                    status_cursor -= 1;
                } while (index >= 0);
            }
            else
            {
            block_17:
                pending = 0;
            }
        } while (pending != 0);
    }
}
