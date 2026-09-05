#include "common.h"

/**
 * @brief Views of the block pointed to by D_80122B74. The block is a large
 *        game-state record; each view names only the fields a function here
 *        touches, so several partial layouts coexist over one pointer.
 */

/** @brief 0xC-stride record with a packed status byte and count. */
typedef struct
{
    u8 flags;
    u8 pad1[2];
    u8 count;
    u8 pad4[8];
} FieldRec;

/** @brief Header counter at 0x2E4 followed by the 64 FieldRec entries at 0x2F0. */
typedef struct
{
    u8 pad0[0x2E4];
    u8 counter;
    u8 pad2E5[11];
    FieldRec recs[64];
} FieldBig;

typedef struct
{
    s32 unk0;
    s32 unk4;
} PairC36F0;

typedef struct
{
    u8 unk0;
    u8 pad1[0x37];
    s32 unk38;
    s32 unk3C;
} RecC36F0;

typedef struct
{
    u8 pad0[0x640];
    RecC36F0 unk640[8];
    u8 pad840[0x4A0];
    RecC36F0 unkCE0[100];
} StructC36F0;

/** @brief 0xC-stride field record; only the status byte at 0x2F0 is read. */
typedef struct
{
    u8 pad0[0x2F0];
    u8 unk2F0;  /* 0x2F0 packed status bits */
} FieldStatusRec;

/** @brief 32-bit resource counter at 0x2C, saturated at 10,000,000. */
typedef struct
{
    u8 pad[0x2C];
    u32 unk2C;
} UnkStruct2C;

/** @brief 0x40-byte record in the FieldBlock80122B74 records[] array. */
typedef struct
{
    u8 flag;      /* 0x00 activation flag */
    u8 pad1[0x33];
    s32 result;   /* 0x34 cached result handle */
    u8 pad2[0x8];
} FieldRecord80122B74;

/** @brief Header then 100 records at 0xCE0. */
typedef struct
{
    u8 header[0xCE0];
    FieldRecord80122B74 records[100];
} FieldBlock80122B74;

#define FIELD_BIG ((FieldBig *)D_80122B74)
#define FIELD_C36F0 ((StructC36F0 *)D_80122B74)
#define FIELD_COUNTER ((UnkStruct2C *)D_80122B74)
#define FIELD_BLOCK ((FieldBlock80122B74 *)D_80122B74)

void func_800C3B50(void);
void func_800BD520(s32 arg0, s32 arg1, s32 arg2);
void func_800C3A00(s32 arg0);
void func_8006AB38(s32 arg0);
void func_800C1EC8(void *dst, void *src, s32 n);
void func_800C2138(s32 arg0);
u8 *func_800C1E40(s32 arg0);
void func_800C32C8(void);
s32 func_800C3518(s32 arg0);
s32 func_800C3688(s32 arg0);

extern u8 *D_80122B74;
extern u8 D_800F198C[];
extern u16 g_music_track_index;

/**
 * @brief Refresh the block and return its byte at 0xAA9 offset by 0x41.
 * @return The adjusted byte value.
 */
s32 func_800C318C(void)
{
    func_800C3B50();
    return D_80122B74[0xAA9] + 0x41;
}

/**
 * @brief Close either the primary (arg0 == 0) or secondary record and notify func_8006AB38.
 * @param arg0 Zero selects the record at 0x840, nonzero the record at 0xA90.
 */
void func_800C31BC(s32 arg0)
{
    if (arg0 == 0)
    {
        if ((*(s32 *)(D_80122B74 + 0x858) & 0x7F) == 2)
        {
            func_800BD520(0, (D_80122B74[0x859] << 3) + 0xF87, 0);
        }
        func_800BD520(0, 0x2F08, 0xFF);
        D_80122B74[0x840] = 0;
        *(s32 *)(D_80122B74 + 0x858) |= 0x7F;
    }
    else
    {
        if ((*(s32 *)(D_80122B74 + 0xAA8) & 0x7F) == 3)
        {
            func_800C32C8();
            *(s32 *)(D_80122B74 + 0x2EF0) = 5;
        }
        else
        {
            func_800C3A00(0);
        }
        D_80122B74[0xA90] = 0;
        *(s32 *)(D_80122B74 + 0xAA8) |= 0x7F;
        func_800BD520(0, 0x2F00, 0xFF);
    }
    func_8006AB38(arg0);
}

/**
 * @brief Copy the pending record at 0xA90 into the menu slot selected by the word at 0x2EF0.
 */
void func_800C32C8(void)
{
    s32 i;
    s32 dst_off;
    u8 *src;
    u8 *p;

    if ((u32)*(s32 *)(D_80122B74 + 0x2EF0) < 5)
    {
        i = 0;
        do
        {
            dst_off = i + *(s32 *)(D_80122B74 + 0x2EF0) * 0x60;
            src = D_80122B74 + i;
            i += 1;
            *(u8 *)(D_80122B74 + dst_off + 0x2EF4) = *(u8 *)(src + 0xA90);
        } while (i < 0x15);

        {
            u8 *b; s32 idx; s32 off; u32 val;
            b = D_80122B74;
            idx = *(s32 *)(b + 0x2EF0);
            off = idx * 0x60;
            val = *(u8 *)(b + 0xAB0);
            b += off;
            *(u8 *)(b + 0x2F0C) = val;
        }
        p = D_80122B74 + *(s32 *)(D_80122B74 + 0x2EF0) * 0x60;
        {
            u32 word = *(u32 *)(D_80122B74 + 0xAB0);
            u32 low = *(u8 *)(p + 0x2F0C);
            low |= (word >> 8) << 8;
            *(u32 *)(p + 0x2F0C) = low;
        }
        {
            u8 *b = D_80122B74;
            s32 off = *(s32 *)(b + 0x2EF0) * 0x60;
            u16 val = *(u16 *)(b + 0xAB4);
            b += off;
            *(u16 *)(b + 0x2F10) = val;
        }
        {
            u8 *b = D_80122B74;
            s32 off = *(s32 *)(b + 0x2EF0) * 0x60;
            off += (s32)b;
            func_800C1EC8(b + 0xAC0, (void *)(off + 0x2F1C), 0x10);
        }
    }
}

/**
 * @brief Try to claim up to two field records and write the claimed ids to a list.
 * @param arg0 First record index to claim.
 * @param arg1 Second record index to claim (only tried when fewer than 3 are active).
 * @param arg2 Output list; terminated with 0xFF.
 * @return Number of records claimed.
 */
s32 func_800C33E4(s32 arg0, s32 arg1, s32 *arg2)
{
    s32 *s0;
    s32 s1;
    s32 count;
    s32 i;
    u32 temp;

    s0 = arg2;
    count = 0;
    for (i = 0; i < 0x40; i++)
    {
        temp = D_80122B74[i * 0xC + 0x2F0];
        if ((temp & 1) && !((temp >> 1) & 1))
        {
            count += 1;
        }
    }

    s1 = 0;
    if (count < 3)
    {
        if (func_800C3518(arg0) >= 0)
        {
            s1 = 1;
            *s0 = arg0;
            s0++;
        }
        if (func_800C3518(arg1) >= 0)
        {
            s1 += 1;
            *s0 = arg1;
            goto block_12;
        }
    }
    else if (func_800C3518(arg0) >= 0)
    {
        s1 = 1;
        *s0 = arg0;
    block_12:
        s0++;
    }

    if (s1 == 0)
    {
        i = func_800C3688(g_music_track_index);
        temp = 0x1F;
        if (i < 0x20)
        {
            temp = i;
        }
        func_800C2138(D_800F198C[temp]);
    }
    *s0 = 0xFF;
    return s1;
}

/**
 * @brief Claim a field record if it is unclaimed and available.
 * @param arg0 Record index.
 * @return arg0 on success, -1 when out of range or unavailable.
 */
s32 func_800C3518(s32 arg0)
{
    u8 flags;

    if (arg0 < 0x40)
    {
        flags = FIELD_BIG->recs[arg0].flags;
        if ((flags & 1) == 0)
        {
            if ((flags >> 3) & 1)
            {
                goto do_stuff;
            }
        }
        return -1;
    do_stuff:
        FIELD_BIG->counter += 1;
        FIELD_BIG->recs[arg0].flags |= 1;
        FIELD_BIG->recs[arg0].count = FIELD_BIG->counter;
        return arg0;
    }
    return -1;
}

/**
 * @brief Mark a field record as available.
 * @param arg0 Record index; ignored when >= 0x40.
 */
void func_800C35AC(s32 arg0)
{
    u8 *temp_v1;

    if (arg0 < 0x40)
    {
        temp_v1 = D_80122B74 + arg0 * 0xC;
        temp_v1[0x2F0] |= 8;
    }
}

/**
 * @brief Classify the packed status byte of a field record.
 *
 * For an in-range @p arg0 (< 0x40), reads the record's status byte. When its
 * "valid" bit (0x08) is set, returns a code from the low bits: 1 if bit 0 is
 * clear, 2 if bit 1 is clear, 4 if bit 2 is set, otherwise 3. Returns 0 when
 * the record is not valid; for out-of-range @p arg0 it instead notifies the
 * audio driver and returns 0.
 *
 * @param arg0 Record index; >= 0x40 triggers the audio notification path.
 * @return Status code 1-4, or 0.
 * @see decomp.me (100%) TODO
 */
s32 func_800C35E4(s32 arg0)
{
    u8 temp_a0;
    u32 temp_v1;
    u8 *b;

    if (arg0 < 0x40)
    {
        b = D_80122B74;
        temp_a0 = ((FieldStatusRec *)(b + (arg0 * 3 << 2)))->unk2F0;
        temp_v1 = temp_a0 & 0xFF;
        if ((temp_v1 >> 3) & 1)
        {
            if (!(temp_a0 & 1))
            {
                return 1;
            }
            if (!((temp_v1 >> 1) & 1))
            {
                return 2;
            }
            if (((temp_v1 >> 2) & 1) == 0)
            {
                return 3;
            }
            return 4;
        }
    }
    else
    {
        akao_set_song_params(0x8001, 0x73, arg0, 0);
    }
    return 0;
}

/**
 * @brief Measures the packed coordinate distance for a field record.
 *
 * Compares the two coordinate nibbles in the selected 12-byte record against
 * the corresponding nibbles in the base record, sums their absolute
 * differences, and adds the selected record's byte at offset 0x2F2.
 *
 * @param arg0 Index of the 12-byte field record to measure.
 * @return The two-nibble Manhattan distance plus the record's extra byte.
 * @note 96.73% match (26/26 instructions, 10 register-allocation-only rows).
 *       Identical result under gcc272_cdk and gcc280_g0; see
 *       working/func_800C3688/STATUS.md.
 */
s32 func_800C3688(s32 arg0)
{
    s32 var_a2;
    s32 split_tmp;
    u32 temp_a0;
    u8 temp_v0;
    u8 temp_v1;
    u8 *new_var;
    u8 *base;
    u8 *temp_a1;
    s32 offset;

    base = D_80122B74;
    offset = arg0 * 3;
    offset *= 4;
    new_var = base + offset;
    temp_a1 = new_var;
    temp_v1 = temp_a1[0x2F1];
    temp_a0 = *(u32 *)(base + 0x2F0);
    split_tmp = temp_v1;
    split_tmp &= 0xF;
    split_tmp -= (temp_a0 >> 8) & 0xF;
    var_a2 = split_tmp;
    if (var_a2 < 0)
    {
        var_a2 = -var_a2;
    }
    arg0 = temp_a0 >> 12;
    arg0 = (temp_v1 >> 4) - (arg0 & 0xF);
    temp_v0 = temp_a1[0x2F2];
    if (arg0 < 0)
    {
        arg0 = -arg0;
    }
    ;
    return temp_v0 + var_a2 + arg0;
}

/**
 * @brief Check whether a key pair is already used by any active record.
 * @param key Pair to look up.
 * @return 1 when the pair is in use, 0 otherwise.
 * @see decomp.me (100%)
 */
s32 func_800C36F0(PairC36F0 *key)
{
    s32 i;
    s32 a;
    s32 b;

    a = key->unk0;
    b = key->unk4;

    for (i = 0; i < 100; i++)
    {
        if ((FIELD_C36F0->unkCE0[i].unk0 != 0) && (FIELD_C36F0->unkCE0[i].unk38 == a) && (FIELD_C36F0->unkCE0[i].unk3C == b))
        {
            return 1;
        }
    }

    for (i = 0; i < 8; i++)
    {
        if ((FIELD_C36F0->unk640[i].unk0 != 0) && (FIELD_C36F0->unk640[i].unk38 == a) && (FIELD_C36F0->unk640[i].unk3C == b))
        {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief Generate a random key pair, seeded by nibble masks, that no record uses yet.
 * @param seed Nibble pattern mixed into both halves of the key.
 * @param out Receives the unique pair.
 * @see decomp.me (100%)
 */
void func_800C37A8(u32 seed, PairC36F0 *out)
{
    PairC36F0 key;
    u32 value;
    u32 hi;
    u32 lo;
    u32 mask_hi;
    u32 mask_lo;

    mask_hi = 0xF0F0F0F0;
    hi = seed & mask_hi;
    mask_lo = 0x0F0F0F0F;
    lo = seed & mask_lo;

    do
    {
        value = rand();
        value += rand() << 16;
        key.unk0 = hi | (value & mask_lo);
        key.unk4 = lo | (value & mask_hi);
    } while (func_800C36F0(&key) != 0);

    out->unk0 = key.unk0;
    out->unk4 = key.unk4;
}

/**
 * @brief Add to the counter at 0x2C, saturating at 10,000,000.
 * @param arg0 Amount to add.
 * @return Always 1.
 */
s32 func_800C3860(s32 arg0)
{
    u32 temp_v0;

    temp_v0 = FIELD_COUNTER->unk2C + arg0;
    FIELD_COUNTER->unk2C = temp_v0;
    if (temp_v0 > 0x989680U)
    {
        FIELD_COUNTER->unk2C = 0x989680U;
    }
    return 1;
}

/**
 * @brief Subtract from the counter at 0x2C when enough is available.
 * @param arg0 Amount to remove.
 * @return 1 when the amount was removed, 0 when the counter was too small.
 */
s32 func_800C3894(u32 arg0)
{
    u32 temp_v1;

    temp_v1 = FIELD_COUNTER->unk2C;
    if (arg0 < temp_v1)
    {
        FIELD_COUNTER->unk2C = temp_v1 - arg0;
        return 1;
    }
    return 0;
}

/**
 * @brief Compute the accumulated lookup-table value for a field record.
 *
 * @param arg0 Record containing the packed lookup selector at 0x14 and four
 *             table-entry bytes beginning at 0x20.
 * @return Product of the two packed-selector table entries plus each valid
 *         per-record table contribution.
 */
s32 func_800C38C8(u8 *arg0)
{
    u8 *base;
    u32 packed;
    s32 sum;
    u32 i;
    u8 id;
    u8 *p;

    base = func_800C1E40(0x11);
    i = 0;
    packed = *(u32 *)(arg0 + 0x14);
    sum = *(u16 *)(base + ((((packed >> 4) & 0x30) + ((packed >> 10) & 0x3F)) << 1) + 4) *
          *(u16 *)(base + ((packed >> 15) & 0x7E) + 0x4C);
    do
    {
        p = arg0 + i;
        id = p[0x20];
        if (id != 0xFF)
        {
            sum += *(u16 *)(base + (p[0x20] << 1) + 0xCC);
        }
        i++;
    } while (i < 4);
    return sum;
}

/**
 * @brief Populate cached result handles for every active D_80122B74 record.
 *
 * Walks the 100 records at offset 0xCE0: for each one flagged active whose
 * cached result is still 0, calls func_800C38C8 on the record and stores the
 * returned handle back into the record.
 *
 * @see decomp.me (100%) TODO
 */
void func_800C396C(void)
{
    u32 i;

    for (i = 0; i < 0x64; i++)
    {
        if (FIELD_BLOCK->records[i].flag != 0 && FIELD_BLOCK->records[i].result == 0)
        {
            FIELD_BLOCK->records[i].result = func_800C38C8((u8 *)&FIELD_BLOCK->records[i]);
        }
    }
}
