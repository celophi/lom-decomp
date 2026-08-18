/*
 * akao_driver_init_state is isolated in its own translation unit on purpose.
 *
 * The function pokes the AKAO driver-state globals purely by byte offset. When
 * it is compiled against the real aggregate types from akao_driver.h/akao.h
 * (AkaoChannelState g_akao_seq_master_state, u8 g_akao_seq_channels[], ...),
 * the aggregate alias info shifts the register coloring of the two entry
 * address materializations and the match tops out at 99.88% (the %hi temp lands
 * in a scratch instead of the destination). Declaring the two entry symbols as
 * scalar u8 here restores the original coloring and the function matches 100%.
 *
 * These extern declarations therefore INTENTIONALLY differ in C type from the
 * canonical ones in akao_driver.h; only the byte addresses matter. This file
 * must not include akao.h / akao_driver.h, or the types would clash.
 */

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef signed char s8;
typedef short s16;
typedef int s32;

void SpuSetReverb(int);
void akao_apply_reverb_type();

extern u8 D_8003EC30[];
extern u8 g_akao_bank_slot_keys[];
extern u8 g_akao_driver_flags[];
extern u8 g_akao_sfx_control[];
extern u8 g_akao_seq_master_state;
extern u8 D_8004C2D0[];
extern u8 g_akao_xa_tracker[];
extern u8 D_8004F830[];
extern u8 g_akao_seq_channels;
extern u8 g_sfx_channels[];
extern s32 g_akao_pending_channels;
extern s32 g_akao_seq_channel1;
extern s16 g_akao_mastervol_fade_ticks;
extern s16 g_akao_masterpan_fade_ticks;
extern s32 g_akao_seq_pending_ticks;
extern s16 g_akao_cdvol_fade_ticks;
extern s32 g_akao_cdvol_acc;
extern s32 D_8003EC6C;
extern s32 g_akao_cdvol_tick;
extern s32 g_akao_mastervol_acc;
extern s32 g_akao_masterpan_acc;
extern s32 g_akao_driver_mode_flags;
extern void* D_8003EC58;
extern void* g_akao_seq_channel0;

inline static u8* off(u8* p, int o) { return p + o; }

/** @brief View of the master channel-state block used to reach its 0x40 field. */
typedef struct { u8 pad40[0x40]; u32 f40; } A0S40;

/**
 * @brief Zeroes and primes the AKAO driver's runtime state.
 *
 * Touched in akao_driver_init after the SPU is brought up. Clears the music
 * channel state for 0x20 sequence channels (each 0x118 bytes wide, indexed via
 * @c g_akao_seq_channels) and 0x18 SFX channels (also 0x118-byte stride, in
 * @c g_sfx_channels). Pokes the SPU master/reverb registers
 * (@c 0x1F801D80..1F801DB2, @c 0x1F801DAA control). Calls back into the
 * higher-level @c akao_apply_reverb_type / @c SpuSetReverb to install the
 * channel state pointer and reverb mode.
 *
 * @see https://decomp.me/scratch/9R0Vj (100%)
 */
void akao_driver_init_state(void)
{
    u32 hw = 0x1F801DAA;
    u32 t0 = 0x18;
    u8** new_var4;
    u32* new_var3;
    u32* new_var2;
    u32* new_var;
    int new_var7;
    int new_var5;
    u8* state;
    u8* a0;
    u8* a2;
    int new_var6;
    u8* new_var9;
    u32 loop_one;
    u32 tail_value;
    u32 a3;


    state = &g_akao_seq_master_state;
    state = (u8 *)((u32)state ^ 1);
    state = (u8 *)((u32)state ^ 1);

    a2 = &g_akao_seq_channels;

    new_var3 = (u32*)off(D_8003EC30, 4);
    *new_var3 = 0;
    *((u32*)off(D_8003EC30, 0)) = 0;
    *((u32*)off(g_akao_bank_slot_keys, 0x14)) = 0;
    *((u32*)off(g_akao_bank_slot_keys, 0x10)) = 0;
    *((u32*)off(g_akao_bank_slot_keys, 0x0C)) = 0;
    *((u32*)off(g_akao_bank_slot_keys, 0x08)) = 0;
    *((u32*)off(g_akao_bank_slot_keys, 0x04)) = 0;
    *((u32*)off(g_akao_bank_slot_keys, 0x00)) = 0;
    *((u32*)off(g_akao_driver_flags, 0x00)) = 0;
    *((u32*)off(g_akao_driver_flags, 0x04)) = 1;
    new_var = (u32*)off(D_8004F830, 0x00);

    *((u32*)off(g_akao_sfx_control, 0x00)) = 0;
    *((u32*)off(state, 0x04)) = 0;
    *((u32*)off(state, 0x08)) = 0;
    *((u16*)off(state, 0x5E)) = 0;
    *((u32*)off(g_akao_sfx_control, 0x10)) = 0;
    *((u32*)off(state, 0x1C)) = 0;
    *((u16*)off(D_8004C2D0, 0x5E)) = 0;
    *((u32*)off(D_8004C2D0, 0x04)) = 0;
    *((u32*)off(state, 0x50)) = 0x7F0000;

    D_8003EC58 = a2;
    a2 = (u8 *)((u32)a2 ^ 1);
    a2 = (u8 *)((u32)a2 ^ 1);
    a2 += 0x58;
    g_akao_seq_channel0 = state;
    g_akao_seq_channel1 = 0;
    g_akao_pending_channels = 0;
    g_akao_cdvol_tick = 0;
    *((u16*)off(state, 0x58)) = 0;
    g_akao_cdvol_acc = 0x7FFF0000;
    g_akao_mastervol_fade_ticks = 0;
    g_akao_mastervol_acc = 0;
    g_akao_masterpan_fade_ticks = 0;
    g_akao_masterpan_acc = 0;
    g_akao_cdvol_fade_ticks = 0;
    *((u32*)off(g_akao_sfx_control, 0x1C)) = 0;
    *((u32*)off(state, 0x3C)) = 0;
    *((u32*)off(g_akao_sfx_control, 0x20)) = 0;

    a3 = *(u16*)hw;
    ((A0S40*)state)->f40 = 0;
    *((s16*)0x1F801D80) = 0x3FFF;
    *((s16*)0x1F801D82) = 0x3FFF;
    *((s16*)0x1F801DB0) = 0x7FFF;
    *((s16*)0x1F801DB2) = 0x7FFF;
    *((u32*)off(g_akao_sfx_control, 0x24)) = 0;
    *((u32*)off(state, 0x44)) = 0;
    *((u16*)off(state, 0x68)) = 0;
    *((u16*)off(state, 0x66)) = 0;
    *((u16*)off(state, 0x64)) = 0;
    *((u16*)off(state, 0x6C)) = 0;
    *((u32*)off(g_akao_xa_tracker, 0x40)) = 0x7F00;
    *((u32*)off(g_akao_xa_tracker, 0x48)) = 0;
    g_akao_seq_pending_ticks = 0;
    D_8003EC6C = 0;
    g_akao_driver_mode_flags = 0;
    *((u32*)off(D_8004F830, 0x08)) = 0;
    *((u32*)off(D_8004F830, 0x04)) = 0;
    *new_var = 0;
    *(u16*)hw = (a3 & 0xFFFA) | 1;
    a3 = 0;
    do {
        a3++;
        *((volatile u32*)(a2 - 0x24)) = 0;
        *((volatile u32*)(a2 + 0xA4)) = t0;
        *((volatile u16*)(a2 + 0x0C)) = 0;
        *((volatile u32*)a2) = 0;
        a2 += 0x100;
        a2 += 0x10;
        a2 += 0x8;
    } while ((a3 & 0xFFFF) < 0x20);

    a3 = 0xC;
    loop_one = 1;
    new_var7 = 0x7F00;
    new_var5 = 0x8C;
    {
        u8* v1 = g_sfx_channels;
        do {
            u32 tmp = a3 & 0xFFFF;
            a3++;
            *((u32*)(v1 + 0x34)) = 0;
            *((u32*)(v1 + 0xFC)) = tmp;
            *((u16*)(v1 + 0x64)) = (u16)loop_one;
            *((u32*)(v1 + 0x58)) = 0;
            *((u16*)(v1 + 0xE4)) = new_var7;
            *((u16*)(v1 + 0x8E)) = 0;
            *((u16*)(v1 + 0x88)) = 0;
            *((u32*)(v1 + 0x40)) = 0;
            *((u16*)(v1 + 0x8C)) = 0;
            v1 += 0x118;
        } while ((a3 & 0xFFFF) < 0x18);
    }

    {
        u8* a0_ptr = g_akao_seq_channel0;
        u8* v0_ptr = g_akao_sfx_control;
        u8* v1_ptr = g_akao_driver_flags;
        a0 = a0_ptr;
        *((u32*)off(a0, 0x18)) = 0;
        *((u32*)off(a0, 0x14)) = 0;
        new_var2 = (u32*)off(v0_ptr, 0x18);
        *((u32*)off(a0, 0x10)) = 0;
        *new_var2 = 1;
        *((u32*)off(v0_ptr, 0x14)) = 0x66A80000;
        *((u32*)off(v0_ptr, 0x0C)) = 0;
        *((u32*)off(v0_ptr, 0x08)) = 0;
        *((u32*)off(v0_ptr, 0x04)) = 0;
        tail_value = 0x03FFF000;
        *((u32*)off(a0, 0x48)) = tail_value;
        *((u32*)off(a0, 0x4C)) = 0;
        *((u16*)off(a0, 0x5A)) = 0;
        new_var9 = v1_ptr;
        *((u32*)off(new_var9, 0x08)) = (*((u32*)off(new_var9, 0x08))) | 0x80;
    }

    akao_apply_reverb_type(4);
    SpuSetReverb(1);
}
