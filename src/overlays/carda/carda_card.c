#include "carda_internal.h"

s8 *func_801471E4(s8 *out, s32 value)
{
    struct Copy7 { s8 data[7]; };
    extern s8 D_801401B8[];
    s32 digit;
    s32 divisor;
    s32 started;
    s8 *p;

    p = out;
    divisor = 100000;
    if (value < divisor * 10)
    {
        goto format;
    }
    *(struct Copy7 *)p = *(struct Copy7 *)D_801401B8;
    return p + 6;

format:
    started = 0;
    do
    {
        digit = value / divisor;
        if (digit != 0 || started != 0)
        {
            *p++ = (digit + 0x824F) >> 8;
            *p++ = digit + 0x4F;
            started = 1;
        }
        if (divisor == 1)
        {
            break;
        }
        if (divisor == 10)
        {
            started = 1;
        }
        value -= digit * divisor;
        divisor /= 10;
    } while (1);
    *p = 0;
    return p;
}

void func_801472F8(s8 *out, s32 value, s32 max_chars)
{
    s32 nibble;
    s32 shift_index;
    s32 remaining_chars;
    s32 remaining_value;
    s32 started;
    s8 *cursor;
    s32 end_index;

    cursor = out;
    remaining_value = value;
    remaining_chars = max_chars;
    shift_index = 7;
    started = 0;
    if (remaining_chars != 0)
    {
        end_index = -1;
loop_2:
        nibble = (remaining_value >> (shift_index * 4)) & 0xF;
        do
        {
            if ((nibble != 0) || (started != 0))
            {
                func_801473C0(cursor, nibble);
                cursor += 1;
                remaining_chars -= 1;
                started = 1;
                remaining_value -= nibble << (shift_index * 4);
            }
        } while (0);
        do
        {
            shift_index -= 1;
        } while (0);
        if (shift_index != end_index)
        {
            if (shift_index == 0)
            {
                started = 1;
            }
            do
            {
                if (remaining_chars != 0)
                {
                    goto loop_2;
                }
            } while (0);
        }
    }
    *cursor = 0;
}

void func_801473C0(s8 *out, s32 value)
{
    if (value < 10)
    {
        *out = value + 0x30;
    }
    else if (value < 16)
    {
        *out = value + 0x37;
    }
    else
    {
        *out = 0x5F;
    }
}

u32 func_801473E4(u8 *s, s32 len)
{
    u32 result;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;

    result = 0;
    while (((u8)(*s - '0') < 10) || ((u8)(*s - 'a') < 6) || ((u8)(*s - 'A') < 6))
    {
        if (len == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*s - '0') < 10)
        {
            tmp0 = result - 0x30;
            result = tmp0 + *s;
        }
        else if ((u8)(*s - 'A') < 6)
        {
            tmp1 = result - 0x37;
            result = tmp1 + *s;
        }
        else if ((u8)(*s - 'a') < 6)
        {
            tmp2 = result - 0x57;
            result = tmp2 + *s;
        }
        s++;
        len--;
    }
    return result;
}

s32 func_80147490(u8 *text)
{
    u32 c;
    s32 count;
    u32 result;
    u32 tmp0;
    u32 tmp1;
    u32 tmp2;

    while (1)
    {
        c = *text;
        text++;
        if ((u32)(c - '0') < 10)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }

        text++;
        if ((u32)(c - 'a') < 6)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }

        text++;
        if ((u32)(c - 'A') < 6)
        {
            continue;
        }
        text--;
        if (text)
        {
            text++;
            text--;
        }
        break;
    }

    text++;
    count = 2;
    result = 0;
    while (((u8)(*text - '0') < 10) || ((u8)(*text - 'a') < 6) || ((u8)(*text - 'A') < 6))
    {
        if (count == 0)
        {
            break;
        }
        result <<= 4;
        if ((u8)(*text - '0') < 10)
        {
            tmp0 = result - 0x30;
            result = tmp0 + *text;
        }
        else if ((u8)(*text - 'A') < 6)
        {
            tmp1 = result - 0x37;
            result = tmp1 + *text;
        }
        else if ((u8)(*text - 'a') < 6)
        {
            tmp2 = result - 0x57;
            result = tmp2 + *text;
        }
        text++;
        count--;
    }
    return result;
}

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

/**
 * @see decomp.me (100%)
 * @brief Rebuild card directory ranking and slot metadata.
 * @return Index associated with the highest directory field value.
 */
s32 func_801477CC(void)
{
    s32 used[10];
    s32 *zero_ptr;
    s32 *scan_ptr;
    s32 *rank_ptr;
    s32 *base_rank;
    s32 *row;
    s32 *elem;
    s32 *cmp_ptr;
    s32 *inc_ptr;
    s32 *ecopy;
    s32 *max_ptr;
    s32 *field_base;
    s32 *field1;
    s32 slot;
    s32 count;
    s32 max_count;
    s32 final_count;
    s32 less_count;
    s32 j;
    s32 t0v;
    s32 i;
    s32 shared3;
    s32 shared4;
    s32 shared1;
    s32 shared2;
    u8 *suffix;
    u8 *p;
    s32 n;
    s32 acc;
    u32 decimal_base, uppercase_base, lowercase_base;

    func_80147588();
    i = 9;
    func_8014A1C4();
    zero_ptr = &used[9];
    do
    {
        *zero_ptr = 0;
        i--;
        zero_ptr--;
    } while (i >= 0);

    shared4 = 0;
    i = shared4;
    while (i < D_80165FEC)
    {
        u8 *pattern;

        pattern = (u8 *)&D_800ECF7C;
        if (func_8001714C(pattern, (u8 *)&D_80166440[D_801660A0][i], 0xC) == 0)
        {
            n = 5;
            p = (u8 *)(D_801660A0 * 0x320 + i * 0x28 + (s32)D_80166440 + 0xC);
            acc = 0;
            while (((u8)(*p - '0') < 10) || ((u8)(*p - 'a') < 6) || ((u8)(*p - 'A') < 6))
            {
                if (n == 0)
                {
                    break;
                }
                acc <<= 4;
                if ((u8)(*p - '0') < 10)
                {
                    decimal_base = acc - '0';
                    acc = decimal_base + *p;
                }
                else if ((u8)(*p - 'A') < 6)
                {
                    uppercase_base = acc - ('A' - 10);
                    acc = uppercase_base + *p;
                }
                else if ((u8)(*p - 'a') < 6)
                {
                    lowercase_base = acc - ('a' - 10);
                    acc = lowercase_base + *p;
                }
                p++;
                n--;
            }
            suffix = &D_80166440[D_801660A0][i].name[0xC];
            {
                s32 *loop_fields = &D_80166AE8[D_801660A0 * 20];
                loop_fields[i] = acc;
            }
            D_80166A80[i] = func_80147490(suffix);
            used[D_80166A80[i]] = 1;
            if (shared4 < D_80166A80[i])
            {
                shared4 = D_80166A80[i];
            }
        }
        else
        {
            s32 *loop_fields = &D_80166AE8[D_801660A0 * 20];
            loop_fields[i] = -1;
            D_80166A80[i] = 0;
        }
        i++;
    }

    do
    {
        shared3 = -1;
    } while (0);
    func_80147C5C();
    t0v = 1;
    i = 0;
    if (D_80165FEC > 0)
    {
        count = D_80165FEC;
        base_rank = &D_801663A8[0];
        rank_ptr = base_rank;
        slot = D_801660A0;
        field1 = D_80166AE8;
        row = field1 + slot * 20;
        elem = row;
        do
        {
            if (*elem >= 0)
            {
                j = 0;
                if (i > 0)
                {
                    j++;
                    j--;
                }
                if (*elem >= shared3)
                {
                    *rank_ptr = t0v;
                    shared3 = *elem;
                    t0v += 1;
                }
                else
                {
                    less_count = j;
                    if (i > 0)
                    {
                        ecopy = elem;
                        inc_ptr = base_rank;
                        cmp_ptr = row;
                        do
                        {
                            if (*ecopy < *cmp_ptr)
                            {
                                less_count += 1;
                                *inc_ptr += 1;
                            }
                            inc_ptr += 1;
                            j += 1;
                            cmp_ptr += 1;
                        } while (j < i);
                    }
                    {
                        s32 rank_value;
                        do
                        {
                            do
                            {
                                do
                                {
                                    rank_value = t0v - less_count;
                                } while (0);
                            } while (0);
                        } while (0);
                        *rank_ptr = rank_value;
                    }
                    t0v += 1;
                }
            }
            rank_ptr += 1;
            i += 1;
            elem += 1;
        } while (i < count);
    }
    cmp_ptr = base_rank;
    inc_ptr = row;
    D_80166438 = t0v;
    t0v = -1;
    i = 0;
    shared3 = 0;
    if (D_80165FEC > 0)
    {
        max_count = D_80165FEC;
        slot = D_801660A0;
        field_base = D_80166AE8;
        max_ptr = (s32 *)((slot * 0x50) + (s32)field_base);
        do
        {
            if (t0v < *max_ptr)
            {
                t0v = *max_ptr;
                shared3 = i;
            }
            i += 1;
            max_ptr += 1;
        } while (i < max_count);
    }
    D_80166AD8 = t0v + 1;

    shared4 = 1;
    scan_ptr = &used[1];
scan_used:
    if (*scan_ptr != 0)
    {
        shared4++;
        scan_ptr++;
        if (shared4 < 9)
        {
            goto scan_used;
        }
    }

    i = 0;
    if (D_80165FEC > 0)
    {
        shared2 = (s32)D_80166A80;
        shared1 = (s32)D_80166440;
loop_prefix:
        if (func_8001714C(D_800ECFC4, (void *)(D_801660A0 * 0x320 + shared1), 8) == 0)
        {
            *(s32 *)shared2 = shared4;
        }
        else
        {
            shared2 += 4;
            final_count = D_80165FEC;
            i++;
            do
            {
                shared1 += 0x28;
            } while (0);
            if (i < final_count)
            {
                goto loop_prefix;
            }
        }
    }
    return shared3;
}

void func_80147C5C(void)
{
    s32 i;
    s32 val;

    D_80166438 = 0x28;
    val = -1;
    for (i = 16; i >= 0; i--)
    {
        D_801663A8[i] = val;
    }
}

s32 func_80147C94(void)
{
    s32 i;
    u8 *entry;

    i = 0;
    if (D_80165FEC > 0)
    {
        do
        {
            entry = (u8 *)D_80166440 + i * 0x28;
            if (D_80166078 != 2)
            {
                if (func_8001714C(&D_800ECF7C, (void *)(D_801660A0 * 0x320 + (s32)entry), 0xC) == 0 ||
                    func_8001714C(&D_800ECF8C, (void *)(D_801660A0 * 0x320 + (s32)entry), 0xC) == 0)
                {
                    return 1;
                }
            }
            if (D_80166078 == 2 &&
                func_8001714C(&D_800ECF8C, (void *)(D_801660A0 * 0x320 + (s32)entry), 0xC) == 0)
            {
                return 1;
            }
            i++;
        } while (i < D_80165FEC);
    }
    return 0;
}

s32 func_80147DCC(void)
{
    s32 i;
    s32 sum;
    s32 offset;

    i = 0;
    sum = 0;
    if (D_80165FEC > 0)
    {
        offset = D_801660A0 * 0x320;
        do
        {
            do
            {
                sum += ((CardaDirEntry *)((u8 *)D_80166440 + offset))->size / 8192;
            } while (0);
            i++;
            offset += 0x28;
        } while (i < D_80165FEC);
    }
    if (sum >= 14 || D_80166078 == 2)
    {
        if (sum < 10)
        {
            if (D_80166078 != 2)
            {
                goto one;
            }
            goto zero;
        }
        goto one;
    }
zero:
    return 0;
one:
    return 1;
}

void func_80147E88(void)
{
    CardaLoadScratch buf;

    memcpy(&buf, &D_801401C0, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_801660A0;
    func_80016F9C(&buf, &D_800ECF9C);
    func_8001686C(&buf);

    memcpy(&buf, &D_801401C0, 6);
    ((u8 *)&buf)[2] += *(u8 *)&D_801660A0;
    func_80016F9C(&buf, &D_800ECFB0);
    func_8001686C(&buf);
    func_80149FEC();
    func_8014A044();
}

#define CARD_DIR_BYTES 0x320
#define CARD_ENTRY_BYTES 0x28
#define CARD_ENTRY_PTR() ((void*)((s32)D_80166440 + D_801660A0 * CARD_DIR_BYTES + D_80165FF4 * CARD_ENTRY_BYTES))

static inline void carda_prepare_fixed_names(void)
{
    CardaLoadScratch namebuf;
    memcpy(&namebuf, &D_801401C0, 6);
    ((u8*)&namebuf)[2] += (u8)D_801660A0;
    func_80016F9C(&namebuf, &D_800ECF9C);
    func_8001686C(&namebuf);
    memcpy(&namebuf, &D_801401C0, 6);
    ((u8*)&namebuf)[2] += (u8)D_801660A0;
    func_80016F9C(&namebuf, &D_800ECFB0);
    func_8001686C(&namebuf);
    func_80149FEC();
    func_8014A044();
}

static inline void carda_append_suffix(CardaLoadScratch* buf, CardaLoadScratch* strbuf)
{
    ((u8*)strbuf)[1] = ((u8 *)D_80166A80)[D_80165FF4 * 4] + 0x30;
    ((u8*)strbuf)[2] = 0;
    func_80016F9C(buf, strbuf);
}

static inline void carda_abort_write(void)
{
    func_80149FEC();
    func_8014A044();
    func_801447DC(0);
}

/** @see decomp.me (100%) */
s32 func_80147F4C(void)
{
    CardaLoadScratch buf;
    CardaLoadScratch strbuf;
    CardaLoadScratch base;
    u8 entryrec[0x80];
    u8 readbuf[0x28];
    s32 wait_attempts;
    s32 poll_shared;
    s32 phase_result;
    s32 status0;
    s32 status1;
    s32 nibble;
    s32 keep_going;
    s32 remaining;
    s8* glyph_out;
    u8* tmpl;
    u8* dst;

    memcpy(&buf, &D_801401C0, 6);
    ((u8*)&buf)[2] += (u8)D_801660A0;
    func_800170BC(&base, &buf);
    phase_result = 1;
    if ((u32)(D_80166078 - 2) < 2U)
    {
        if (D_801663A0 == D_80165B70)
        {
            D_801663A0 = D_80165BAC;
        }
    }
    if (D_801663A0 == NULL)
    {
        return phase_result;
    }

    switch (*D_801663A0)
    {
    default:
        goto block_return;
    case 1:
    {
        phase_result = 3;
        func_8001729C(D_801660A0);
        func_8001724C(D_801660A0 * 0x10);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 2:
    {
        poll_shared = func_8014A09C();
        switch (poll_shared)
        {
        case 0:
            D_801663A0++;
            goto block_return;
        case 1:
        case 2:
            goto c2_pos;
        case 3:
            goto c2_eq3;
        default:
            goto block_return;
        }
    c2_pos:
        phase_result = 4;
        D_801660FC = 0;
        D_80165FEC = 0xFD;
        D_801663A0 = D_801663A0 + 1;
        func_80142CA4();
        goto block_return;
    c2_eq3:
        D_80166438 = 0x28;
        {
            s32 index;
            s32 value = -1;
            for (index = 16; index >= 0; index--)
            {
                D_801663A8[index] = value;
            }
        }
        D_80165FEC = 0xFF;
        D_801663A0 = D_80165B70;
        goto block_return;
    }
    case 25:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll == 0)
        {
            D_801663A0 = D_801663A0 + 1;
            goto block_return;
        }
        if (poll < 0)
        {
            goto block_return;
        }
        if (poll >= 4)
        {
            goto block_return;
        }
        func_801447DC(5);
        goto block_return;
    }
    case 3:
    {
        func_80149FEC();
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 4:
    {
        do
        {
            poll_shared = func_8014A130();
        } while (poll_shared == -1);
        if (poll_shared == 0)
        {
            D_801663A0 = D_801663A0 + 1;
            goto block_return;
        }
        if (poll_shared < 0)
        {
            goto block_return;
        }
        if (poll_shared >= 4)
        {
            goto block_return;
        }
        phase_result = 4;
        goto block_status_fd;
    }
    case 5:
    {
        func_8014A044();
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 6:
    {
        carda_prepare_fixed_names();
        D_80166AE0 = 1;
        if (func_8014991C(D_801660A0) == 0)
        {
            phase_result = 2;
            D_80165FEC = 0xF8;
            D_801663A0 = NULL;
            D_80166AE0 = 0;
            goto block_return;
        }
        D_801663A0 = D_801663A0 + 1;
        wait_attempts = 0;
        do
        {
            if (func_80149A4C(D_801660A0) == 0)
            {
                D_80166AE0 = 0;
                if (D_80165FEC != 0xF8 && D_80165FEC != 0xFA)
                {
                    if (D_80165FEC != 0xF7)
                    {
                        func_80149DF4();
                    }
                    goto block_return;
                }
                goto block_return;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        goto block_return;
    }
    case 8:
    {
        phase_result = 3;
        func_8001729C(D_801660A0);
        func_800172AC(D_801660A0 * 0x10);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 9:
    {
        phase_result = 3;
        func_8001729C(D_801660A0);
        func_8001725C(D_801660A0 * 0x10);
        D_80166AD4 = 0x10;
        D_80166B90 = 0x10;
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 0:
    {
        phase_result = 2;
        D_80166118 = 0;
        goto block_return;
    }
    case 10:
    {
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 11:
    {
        if (D_80166B88 == 0)
        {
            func_80016F9C(&buf, CARD_ENTRY_PTR());
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&buf) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            func_80149FEC();
            func_8014A044();
            func_800170BC(&buf, &base);
        }
        func_800170BC(&strbuf, &D_800ECF9C);
        func_80016F9C(&buf, &strbuf);
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&buf, 0x20200);
        if (D_801663A4 == -1)
        {
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                carda_abort_write();
                return phase_result;
            }
            goto block_return;
        }
        func_8001683C(D_801663A4);
        func_800170BC(&D_80166BA8, &buf);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 12:
    {
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_80166BA8, 0x8002);
        func_80149FEC();
        D_80166ADC = 1;
        D_80166B8C = func_8002054C(-1);
        func_8001729C(D_801660A0);
        if (func_8001682C(D_801663A4, D_80165FF0, 0x4000) == -1)
        {
            func_8001683C(D_801663A4);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&D_80166BA8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                carda_abort_write();
                return phase_result;
            }
            goto block_decrement_step;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 13:
    {
        s32 poll;
        s32 outer_count;
        poll = func_8014A09C();
        if (poll != 0)
        {
            if (poll < 0)
            {
                goto block_return;
            }
            if (poll >= 4)
            {
                goto block_return;
            }
            goto c13_nonzero;
        }
        if (D_80166B88 != 0)
        {
            func_800170BC(&buf, &base);
            func_80016F9C(&buf, CARD_ENTRY_PTR());
            wait_attempts = 0;
            func_8001729C(D_801660A0);
            do
            {
                s32 result = func_8001686C(&buf);
                wait_attempts++;
                if (result != 0)
                {
                    break;
                }
            } while (wait_attempts < 0x14);
            func_80149FEC();
            func_8014A044();
        }
        func_800170BC(&buf, &base);
        func_800170BC(&strbuf, &D_800ECF7C);
        glyph_out = (s8*)&strbuf + 0xC;
        remaining = D_80166AD8;
        outer_count = 5;
        wait_attempts = 7;
        keep_going = 0;
        while (outer_count != 0)
        {
            nibble = (remaining >> (wait_attempts * 4)) & 0xF;
            if (nibble != 0 || keep_going != 0)
            {
                func_801473C0(glyph_out, nibble);
                glyph_out += 1;
                outer_count -= 1;
                keep_going = 1;
                remaining -= nibble << (wait_attempts * 4);
            }
            wait_attempts--;
            if (wait_attempts == -1)
            {
                goto hex_done;
            }
            if (wait_attempts == 0)
            {
                keep_going = 1;
            }
        }
    hex_done:
        *glyph_out = 0;
        func_80016F9C(&buf, &strbuf);
        if (func_80143380() != 0)
        {
            *(s8*)&strbuf = 0x2B;
        }
        else
        {
            *(s8*)&strbuf = 0x2D;
        }
        carda_append_suffix(&buf, &strbuf);
        wait_attempts = 0;
        func_8001729C(D_801660A0);
        do
        {
            s32 result = func_8001685C(&D_80166BA8, &buf);
            wait_attempts++;
            if (result != 0)
            {
                break;
            }
        } while (wait_attempts < 0x14);
        wait_attempts = 0;
        do
        {
            if (func_80016BCC(&buf, &readbuf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        if (wait_attempts != 0x14)
        {
            func_8001729C(D_801660A0);
            func_8001727C(D_801660A0 * 0x10, *(s32*)&readbuf[0x20], &entryrec);
            func_8001729C(D_801660A0);
            dst = &entryrec[4];
            tmpl = &D_8014BECC[0];
            wait_attempts = 0;
            do
            {
                wait_attempts = wait_attempts + 1;
                *dst = *tmpl;
                tmpl += 1;
                dst += 1;
            } while (wait_attempts < 0x12);
            if (func_80143380() != 0)
            {
                entryrec[0xC] = 0x81;
                entryrec[0xD] = 0xF4;
            }
            func_8001729C(D_801660A0);
            func_8001726C(D_801660A0 * 0x10, *(s32*)&readbuf[0x20], &entryrec);
            func_8001729C(D_801660A0);
        }
        D_80166118 = 0;
        D_801663A0 = D_801663A0 + 1;
        func_8001683C(D_801663A4);
        D_80165FEC = 0xFF;
        D_801663A0 = D_80165B70;
        goto block_return;
    c13_nonzero:
        D_8016643C = D_8016643C - 1;
        if (D_8016643C == 0)
        {
            goto c13_loop103;
        }
        func_8001683C(D_801663A4);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_801663A0 = D_801663A0 - 2;
        goto block_return;
    c13_loop103:
        func_8001683C(D_801663A4);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80166ADC = 0;
        D_80166B8C = func_8002054C(-1);
        carda_abort_write();
        return phase_result;
    }
    case 21:
    {
        D_80166ADC = 1;
        D_80166B8C = func_8002054C(-1);
        if (func_8001714C(&D_800ECFC4, CARD_ENTRY_PTR(), 8) != 0)
        {
            func_80016F9C(&buf, CARD_ENTRY_PTR());
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&buf) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            func_80149FEC();
            func_8014A044();
        }
        func_800170BC(&buf, &base);
        func_80016F9C(&buf, &D_800ECF8C);
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&buf, 0x60200);
        if (D_801663A4 == -1)
        {
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                func_8001683C(-1);
                func_801447DC(0);
                return phase_result;
            }
            goto block_return;
        }
        func_8001683C(D_801663A4);
        func_800170BC(&D_80166BA8, &buf);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 22:
    {
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_80166BA8, 0x8002);
        func_8001729C(D_801660A0);
        if (func_8001682C(D_801663A4, D_80165FF0, 0xC000) == -1)
        {
            wait_attempts = 0;
            func_80033E7C(D_801660A0 * 0x10);
            func_8001683C(D_801663A4);
            do
            {
                if (func_8001686C(&D_80166BA8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                carda_abort_write();
                return phase_result;
            }
            goto block_decrement_step;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 23:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll != 0)
        {
            if (poll < 0)
            {
                goto block_return;
            }
            if (poll >= 4)
            {
                goto block_return;
            }
            goto c23_nonzero;
        }
        wait_attempts = 0;
        D_80166118 = 0;
        D_801663A0 = D_801663A0 + 1;
        func_8001683C(D_801663A4);
        D_801663A0 = D_80165B70;
        do
        {
            if (func_80016BCC(&D_80166BA8, &readbuf) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        if (wait_attempts == 0x14)
        {
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&D_80166BA8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            carda_abort_write();
            return phase_result;
        }
        wait_attempts = 0;
        do
        {
            if (func_80034648(D_801660A0 * 0x10, *(s32*)&readbuf[0x20] / 64, 1) != 0)
            {
                break;
            }
            wait_attempts++;
        } while (wait_attempts < 0x14);
        if (wait_attempts != 0x14)
        {
            return phase_result;
        }
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        carda_abort_write();
        return phase_result;
    c23_nonzero:
        D_8016643C = D_8016643C - 1;
        if (D_8016643C == 0)
        {
            goto c23_loop151;
        }
        func_8001683C(D_801663A4);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_801663A0 = D_801663A0 - 2;
        goto block_return;
    c23_loop151:
        func_8001683C(D_801663A4);
        wait_attempts = 0;
        do
        {
            if (func_8001686C(&D_80166BA8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        func_80149FEC();
        func_8014A044();
        func_801447DC(0);
        D_80166ADC = 0;
        D_80166B8C = func_8002054C(-1);
        goto block_return;
    }
    case 15:
    {
        s32 poll;
        poll = func_8014A09C();
        switch (poll)
        {
        case 0:
            D_801663A0++;
            goto block_return;
        case 1:
        case 2:
            goto c15_pos;
        case 3:
            goto c15_eq3;
        default:
            goto block_return;
        }
    c15_pos:
        D_80166B90 = D_80166B90 - 1;
        if (D_80166B90 != 0)
        {
            goto block_reissue;
        }
        phase_result = 4;
    block_status_fd:
        D_801660FC = 0;
        D_80165FEC = 0xFD;
        goto block_return;
    c15_eq3:
        D_80166AD4 = D_80166AD4 - 1;
        if (D_80166AD4 == 0)
        {
            goto c15_done;
        }
    block_reissue:
        func_8001729C(D_801660A0);
        func_800172AC(D_801660A0 * 0x10);
        func_8001729C(D_801660A0);
        func_80149FEC();
        func_8001725C(D_801660A0 * 0x10);
        goto block_return;
    c15_done:
        phase_result = 5;
        D_80165FEC = 0xFC;
        D_801663A0 = D_80165B78;
        goto block_return;
    }
    case 16:
    {
        func_8001729C(D_801660A0);
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 17:
    {
        D_80166000 = 1;
        D_801660FC = 0;
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_801663F8, 0x8001);
        if (D_801663A4 != -1)
        {
            func_80149FEC();
            func_8001729C(D_801660A0);
            if (func_8001681C(D_801663A4, &D_80166120, D_80166AD0 != 0 ? 0x280 : 0x80) == -1)
            {
                func_8001683C(D_801663A4);
                return phase_result;
            }
            D_801663A0 = D_801663A0 + 1;
            goto block_return;
        }
        goto block_return;
    }
    case 18:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll == 0)
        {
            D_80166000 = 0;
            D_801660FC = 1;
            D_801663A0 = D_801663A0 + 1;
            func_8001683C(D_801663A4);
            goto block_return;
        }
        if (poll != -1)
        {
            D_80165FEC = 0xFF;
            D_80166000 = 0;
            func_8001683C(D_801663A4);
            D_801663A0 = D_80165B70;
            goto block_return;
        }
        goto block_return;
    }
    case 19:
    {
        D_80166070 = 1;
        D_80166B8C = func_8002054C(-1);
        D_80166ADC = 1;
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_801663F8, 0x8001);
        func_80149FEC();
        func_8001729C(D_801660A0);
        if (func_8001681C(D_801663A4, D_80165FF0, 0x4000) == -1)
        {
            func_801447DC(1);
            return phase_result;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 20:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll == 0)
        {
            D_80166070 = 0;
            D_801663A0 = D_801663A0 + 1;
            func_8001683C(D_801663A4);
            goto block_return;
        }
        if (poll < 0)
        {
            goto block_return;
        }
        if (poll < 4)
        {
            D_80166B8C = func_8002054C(-1);
            D_80166ADC = 0;
            func_801447DC(1);
            goto block_return;
        }
        goto block_return;
    }
    case 24:
    {
        s32 outer_count = 0;
        do
        {
            wait_attempts = 0;
            do
            {
                if (func_800342CC(D_801660A0 * 0x10) == 1)
                {
                    break;
                }
                func_8002054C(0);
                wait_attempts++;
            } while (wait_attempts < 0x78);
            func_8002054C(0);
            if (wait_attempts != 0x14)
            {
                func_80032174(0, &status0, &status1);
                switch (status1)
                {
                case 0:
                    D_801663A0++;
                    return phase_result;
                case 1:
                    D_80165FEC = 0xFD;
                    D_801663A0 = NULL;
                    return phase_result;
                }
            }
            outer_count++;
        } while (outer_count < 0x14);
        if (D_80165FEC != 0xF6)
        {
            D_80165FEC = 0xF6;
        }
        D_801663A0 = NULL;
        return phase_result;
    }
    case 28:
    {
        D_80166070 = 1;
        D_80166B8C = func_8002054C(-1);
        D_80166ADC = 1;
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_801663F8, 0x8001);
        func_80149FEC();
        func_8001729C(D_801660A0);
        if (func_8001681C(D_801663A4, D_80165FF0, 0x400) == -1)
        {
        block_200:
            func_8014697C(1);
            return phase_result;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 29:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll != 0)
        {
            goto c29_nonzero;
        }
        D_80166070 = 0;
        D_801663A0 = D_801663A0 + 1;
        func_8001683C(D_801663A4);
        goto block_return;
    c29_nonzero:
        if (poll < 0)
        {
            goto block_return;
        }
        if (poll >= 4)
        {
            goto block_return;
        }
        D_80166ADC = 0;
        goto block_200;
    }
    case 30:
    {
        D_8016643C = 5;
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 26:
    {
        if (D_80166B88 == 0)
        {
            wait_attempts = 0;
            do
            {
                s32 result = func_8001686C(&D_801663F8);
                wait_attempts++;
                if (result != 0)
                {
                    break;
                }
            } while (wait_attempts < 0x14);
        }
        func_80016F9C(&buf, &D_800ECFB0);
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&buf, 0x60200);
        if (D_801663A4 == -1)
        {
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                func_8001683C(-1);
                wait_attempts = 0;
                do
                {
                    s32 result = func_8001686C(&buf);
                    wait_attempts++;
                    if (result != 0)
                    {
                        break;
                    }
                } while (wait_attempts < 0x14);
                func_8014697C(0);
                return phase_result;
            }
            goto block_return;
        }
        func_8001683C(D_801663A4);
        func_800170BC(&D_80166BA8, &buf);
        func_8001729C(D_801660A0);
        D_801663A4 = func_8001680C(&D_80166BA8, 0x8002);
        func_80149FEC();
        D_80166B8C = func_8002054C(-1);
        D_80166ADC = 1;
        func_8001729C(D_801660A0);
        if (func_8001682C(D_801663A4, D_80165FF0, 0xC000) == -1)
        {
            func_8001683C(D_801663A4);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&D_80166BA8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
            D_8016643C = D_8016643C - 1;
            if (D_8016643C == 0)
            {
                func_8014697C(0);
                return phase_result;
            }
            goto block_return;
        }
        D_801663A0 = D_801663A0 + 1;
        goto block_return;
    }
    case 27:
    {
        s32 poll;
        poll = func_8014A09C();
        if (poll != 0)
        {
            if (poll < 0)
            {
                goto block_return;
            }
            if (poll >= 4)
            {
                goto block_return;
            }
            goto c27_nonzero;
        }
        if (D_80166B88 != 0)
        {
            func_8001729C(D_801660A0);
            wait_attempts = 0;
            do
            {
                if (func_8001686C(&D_801663F8) != 0)
                {
                    break;
                }
                wait_attempts = wait_attempts + 1;
            } while (wait_attempts < 0x14);
        }
        func_8001729C(D_801660A0);
        wait_attempts = 0;
        do
        {
            if (func_8001685C(&D_80166BA8, &D_801663F8) != 0)
            {
                break;
            }
            wait_attempts = wait_attempts + 1;
        } while (wait_attempts < 0x14);
        D_80166118 = 0;
        D_801663A0 = D_801663A0 + 1;
        func_8001683C(D_801663A4);
        D_801663A0 = D_80165B70;
        goto block_return;
    c27_nonzero:
        D_8016643C = D_8016643C - 1;
        if (D_8016643C == 0)
        {
            func_8001683C(D_801663A4);
            D_80166ADC = 0;
            func_8014697C(0);
            return phase_result;
        }
        func_8001683C(D_801663A4);
        goto block_decrement_step;
    }
    }

block_decrement_step:
    D_801663A0--;
block_return:
    return phase_result;
}

void func_80149554(void)
{
    s32 i;
    u8 scratch[16];

    i = 0;
    do
    {
        if (func_80017B3C(D_801660A0 << 4) != 0)
        {
            break;
        }
        i++;
    } while (i < 0x14);

    D_80166B88 = 0;
    D_80165FF4 = 0;
    func_800170BC((u8 *)D_80166440 + D_801660A0 * 0x320, D_800ECFC4);
}

/** @see decomp.me (100.00%) */
void func_801495E4(void)
{
    func_8001729C(D_801660A0);
    func_80149FEC();
    func_8001724C(D_801660A0 * 0x10);
    D_801663A0 = D_80165B78;
}

/** @see decomp.me (100.00%) */
s32 func_80149638(void)
{
    s32 busy_slot;

    busy_slot = func_8014A09C();
    if (busy_slot != -1)
    {
        func_8001729C(D_801660A0);
        func_8001724C(D_801660A0 * 0x10);
    }
    return busy_slot;
}

/** @see decomp.me (100.00%) */
void func_80149690(void)
{
    func_800158E0();
    func_800167EC();
    D_80166B94 = func_800167AC(0xF4000001, 4, 0x2000, 0);
    D_80166B98 = func_800167AC(0xF4000001, 0x8000, 0x2000, 0);
    D_80166B9C = func_800167AC(0xF4000001, 0x100, 0x2000, 0);
    D_80166BA0 = func_800167AC(0xF4000001, 0x2000, 0x2000, 0);
    D_80166108 = func_800167AC(0xF0000011, 4, 0x2000, 0);
    D_8016610C = func_800167AC(0xF0000011, 0x8000, 0x2000, 0);
    D_80166110 = func_800167AC(0xF0000011, 0x100, 0x2000, 0);
    D_80166114 = func_800167AC(0xF0000011, 0x2000, 0x2000, 0);
    func_800167DC(D_80166B94);
    func_800167DC(D_80166B98);
    func_800167DC(D_80166B9C);
    func_800167DC(D_80166BA0);
    func_800167DC(D_80166108);
    func_800167DC(D_8016610C);
    func_800167DC(D_80166110);
    func_800167DC(D_80166114);
    func_800167FC();
    D_80166B8C = func_8002054C(-1);
    D_80166ADC = 0;
    D_80166AE0 = 0;
}

/** @see decomp.me (100.00%) */
void func_8014986C(void)
{
    func_800158E0();
    func_800167EC();
    func_800167BC(D_80166B94);
    func_800167BC(D_80166B98);
    func_800167BC(D_80166B9C);
    func_800167BC(D_80166BA0);
    func_800167BC(D_80166108);
    func_800167BC(D_8016610C);
    func_800167BC(D_80166110);
    func_800167BC(D_80166114);
    func_800167FC();
}

s32 func_8014991C(s32 page)
{
    EntryHeader7 buf;
    s32 i;

    memcpy(&buf, &D_80140244, 7);
    D_80165FFC = 0;
    D_80165F38 = 0;
    D_80166104 = 0;
    D_80165FF4 = 0;
    ((u8 *)&buf)[2] += page;
    func_8001729C(page);
    D_80165FEC = 0;
    i = 0;
    do
    {
        if (func_80016BCC(&buf, (u8 *)D_80166440 + page * 0x320) != 0)
        {
            func_800B0170((u8 *)D_80166440 + page * 0x320 + D_80165FEC * 0x28);
            D_80165FEC += 1;
            return 1;
        }
        i++;
    } while (i < 20);
    if (D_80166078 == 1 || D_80166078 == 3)
    {
        return 0;
    }
    return 1;
}

s32 func_80149A4C(s32 page)
{
    s32 scan_i;
    s32 i;
    s32 sum;
    s32 selected;
    s32 count;
    s32 offset;
    s32 cond;
    CardaDirEntry *entries;

    func_8001729C(page);
    scan_i = 0;
    do {
        if (func_8001684C(&D_80166440[page][D_80165FEC]) != 0) {
            func_800B0170(&D_80166440[page][D_80165FEC]);
            D_80165FEC += 1;
            return 1;
        }
        scan_i++;
    } while (scan_i < 20);

    field_reset_input_repeat();
    if (D_80166078 == 1 && func_80147C94() == 0) {
        D_80165FEC = 0xF8;
    } else {
        i = 0;
        sum = 0;
        D_80166B88 = 0;
        count = D_80165FEC;
        if (count > 0) {
            do { entries = (CardaDirEntry *)D_80166440; } while (0);
            offset = D_801660A0 * 0x320;
            do {
                do {
                    sum += ((CardaDirEntry *)((u32)offset + (u32)entries))->size / 8192;
                } while (0);
                i++;
                offset += 0x28;
            } while (i < count);
        }
        if (sum >= 14 || D_80166078 == 2) {
            if (sum < 10) {
                if (D_80166078 != 2)
                {
                    goto cond_one;
                }
                goto cond_zero;
            }
            goto cond_one;
        }
cond_zero:
        cond = 0;
        goto cond_done;
cond_one:
        cond = 1;
cond_done:
        if (cond != 0) {
            if (D_80166078 == 0 || D_80166078 == 2) {
                func_800170BC(&D_80166440[page][D_80165FEC], D_800ECFD0);
                D_80166440[page][D_80165FEC].size = 0;
                D_80165FEC += 1;
            }
            selected = func_801477CC();
            if (func_80147C94() == 0) {
                if ((u32)(D_80166078 - 2) < 2U) {
                    D_80165FEC = 0xF7;
                } else {
                    D_80165FEC = 0xFA;
                }
                D_80166AD8 = 0;
            } else {
                D_80165FF4 = selected;
                func_801411CC();
            }
        } else {
            D_80166B88 = 1;
            if ((u32)(D_80166078 - 2) < 2U) {
                func_800170BC(&D_80166440[page][D_80165FEC], D_800ECFC4);
                D_80166440[page][D_80165FEC].size = 0xC000;
                D_80165FEC += 1;
            } else if (D_80166078 == 0) {
                func_800170BC(&D_80166440[page][D_80165FEC], D_800ECFC4);
                D_80166440[page][D_80165FEC].size = 0x4000;
                D_80165FEC += 1;
            }
            selected = func_801477CC();
            if (func_80147C94() == 0) {
                D_80165FF4 = 0;
                func_801411CC();
                D_80166AD8 = 0;
            } else {
                D_80165FF4 = selected;
                func_801411CC();
            }
        }
    }
    return 0;
}

typedef struct FileHeader100 {
    s32 unk0;
    s16 unk4;
    u8 pad[0xFA];
} FileHeader100;

void func_80149DF4(void)
{
    FileHeader100 local;
    u8 *p;

    if (D_80165FEC == 0)
    {
        D_801660FC = 3;
        return;
    }
    {
        s32 term1;
        s32 term2;
        term1 = D_801660A0 * 0x320;
        term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
        if (func_8001714C(&D_800ECFC4[0], (void *)(term1 + term2), 8) == 0)
        {
            D_801660FC = 2;
            return;
        }
    }
    {
        s32 term1;
        s32 term2;
        term1 = D_801660A0 * 0x320;
        term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
        if (func_8001714C(&D_800ECFD0[0], (void *)(term1 + term2), 9) == 0)
        {
            D_801660FC = 4;
            return;
        }
    }
    memcpy(&local, &D_801401C0, 6);
    p = (u8 *)&local;
    {
        s32 term1;
        s32 term2;
        term1 = D_801660A0 * 0x320;
        term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
        func_80016F9C(p, (void *)(term1 + term2));
    }
    {
        s32 slot;
        s32 value;
        value = *((u8 *)&local + 2);
        slot = (u8)D_801660A0;
        D_801660FC = 0;
        value += slot;
        *((u8 *)&local + 2) = value;
        func_800170BC(D_801663F8.raw, p, slot);
    }
    D_801663A0 = &D_80165BA0[0];
    D_80166000 = 1;
    {
        s32 term1;
        s32 term2;
        term1 = D_801660A0 * 0x320;
        term2 = (D_80165FF4 * 0x28) + (s32)D_80166440;
        if (func_8001714C(&D_800ECF7C[0], (void *)(term1 + term2), 0xC) == 0)
        {
            D_80166AD0 = 1;
        }
        else
        {
            D_80166AD0 = 0;
        }
    }
}

/** @see decomp.me (100.00%) */
void func_80149FEC(void)
{
    func_800167CC(D_80166B94);
    func_800167CC(D_80166B98);
    func_800167CC(D_80166B9C);
    func_800167CC(D_80166BA0);
}

/** @see decomp.me (100.00%) */
void func_8014A044(void)
{
    func_800167CC(D_80166108);
    func_800167CC(D_8016610C);
    func_800167CC(D_80166110);
    func_800167CC(D_80166114);
}

/** @see decomp.me (100.00%) */
s32 func_8014A09C(void)
{
    if (func_800167CC(D_80166B94) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_80166B98) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80166B9C) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80166BA0) == 1)
    {
        return 3;
    }
    return -1;
}

/** @see decomp.me (100.00%) */
s32 func_8014A130(void)
{
    if (func_800167CC(D_80166108) == 1)
    {
        return 0;
    }
    if (func_800167CC(D_8016610C) == 1)
    {
        return 1;
    }
    if (func_800167CC(D_80166110) == 1)
    {
        return 2;
    }
    if (func_800167CC(D_80166114) == 1)
    {
        return 3;
    }
    return -1;
}

#define NENT 20
typedef struct
{
    u8 data[0x28];
} CardaEntry28;

void func_8014A1C4(void)
{
    CardaEntry28 sorted[NENT];
    s32 out;
    s32 group;
    s32 i;

    out = 0;
    group = 0;
    do
    {
        do
        {
            i = 0;
        } while (0);
        if (D_80165FEC > 0)
        {
            do
            {
                if (D_80166A80[i] == group &&
                    func_8001714C(D_800ECF7C, &D_80166440[D_801660A0][i], 0xC) == 0)
                {
                    func_80016E7C(&D_80166440[D_801660A0][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_80165FEC);
        }
        group++;
    } while (group < 8);

    group = 0;
    do
    {
        do
        {
            i = 0;
        } while (0);
        if (D_80165FEC > 0)
        {
            do
            {
                if (D_80166A80[i] == group &&
                    func_8001714C(D_800ECF8C, &D_80166440[D_801660A0][i], 0xC) == 0)
                {
                    func_80016E7C(&D_80166440[D_801660A0][i], &sorted[out], 0x28);
                    out++;
                }
                i++;
            } while (i < D_80165FEC);
        }
        group++;
    } while (group < 8);

    do
    {
        i = 0;
    } while (0);
    if (D_80165FEC > 0)
    {
        do
        {
            if (func_8001714C(D_800ECFC4, &D_80166440[D_801660A0][i], 8) == 0 ||
                func_8001714C(D_800ECFD0, &D_80166440[D_801660A0][i], 9) == 0)
            {
                func_80016E7C(&D_80166440[D_801660A0][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_80165FEC);
    }

    if (*(volatile s32 *)&D_80165FEC > 0)
    {
        do
        {
            i = 0;
        } while (0);
        do
        {
            if (func_8001714C(D_800ECF7C, &D_80166440[D_801660A0][i], 0xC) != 0 &&
                func_8001714C(D_800ECF8C, &D_80166440[D_801660A0][i], 0xC) != 0 &&
                func_8001714C(D_800ECFC4, &D_80166440[D_801660A0][i], 8) != 0 &&
                func_8001714C(D_800ECFD0, &D_80166440[D_801660A0][i], 9) != 0)
            {
                func_80016E7C(&D_80166440[D_801660A0][i], &sorted[out], 0x28);
                out++;
            }
            i++;
        } while (i < D_80165FEC);
    }

    do
    {
        i = 0;
    } while (0);
    if (D_80165FEC > 0)
    {
        do
        {
            func_80016E7C(&sorted[i], &D_80166440[D_801660A0][i], 0x28);
            i++;
        } while (i < D_80165FEC);
    }
}
