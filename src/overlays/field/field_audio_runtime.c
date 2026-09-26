/**
 * @file field_audio_runtime.c
 * @brief FIELD music, instrument-bank and sound-effect helpers.
 *
 * Loads AKAO song containers and instrument banks from CD-ROM, starts, stops
 * and fades the field songs, plays sound effects from the loaded effect
 * tables, and runs the sector-by-sector music stream (song data into the
 * resident song area, instrument bank data into the SPU).
 *
 * @note GOVER calls field_load_song and field_play_song while FIELD is
 *       resident.
 */

#include "common.h"
#include "field_calls.h"
#include "akao.h"
#include "akao_cmd.h"
#include "cd_resources.h"
#include "cdrom.h"
#include "game_state.h"
#include "sdk/memory.h"

/*
 * AKAO driver entry points in the main executable without a shared header
 * (include/akao_cmd.h is used by every overlay, some with other local
 * declarations of these commands).
 */
s32 akao_cmd_14(u8 *sequence, s32 param1, s32 param2);
s32 akao_cmd_19_c0(s32 value0, s32 value1);
void akao_cmd_21(s32 value0, s32 value1);
s32 akao_cmd_c1(s32 song_handle, s32 frames, s32 volume);
s32 akao_cmd_d0(s32 value0);
s32 akao_cmd_d4(s32 value0);
s32 akao_is_sfx_playing(s32 voice_mask);
s32 akao_play_sfx_from_buffer(s32 buffer_address, s32 voice_mask, s32 pan, s32 volume);
s32 akao_get_xfer_state(void);
s32 akao_reset_xfer_state(void);
s32 func_80022ED8(void *bank, s32 slot, s32 wait_for_completion);
s32 func_80022EF8(void *bank, s32 slot, s32 wait_for_completion);

/** @brief Scratch buffer the CD layer loads song containers into. */
#define FIELD_AUDIO_LOAD_BUFFER 0x80180000

/** @brief Section offset table of the container in FIELD_AUDIO_LOAD_BUFFER. */
#define FIELD_AUDIO_LOAD_OFFSETS 0x80180004

/** @brief Resident address of the registered AKAO instrument bank. */
#define FIELD_INSTRUMENT_BANK_ADDRESS 0x8013C000

/** @brief Largest music-file index field_load_song accepts. */
#define FIELD_SONG_INDEX_MAX 256

/** @brief First CD resource of the instrument banks loaded by field_load_instrument_bank. */
#define FIELD_INSTRUMENT_BANK_RESOURCE_BASE 142

/** @brief CD resource of the instrument bank uploaded by field_upload_resource_22_bank. */
#define FIELD_UPLOAD_BANK_RESOURCE 22

/** @brief CD resource of the song container loaded by field_load_fixed_song. */
#define FIELD_FIXED_SONG_RESOURCE 146

/** @brief First CD resource of the effect-table sets loaded by field_load_sfx_tables. */
#define FIELD_SFX_SET_RESOURCE_BASE 81

/** @brief First CD resource of the weapon effect tables loaded by field_load_weapon_sfx_table. */
#define FIELD_WEAPON_SFX_RESOURCE_BASE 131

/** @brief Effect-set argument that keeps the loaded tables. */
#define FIELD_SFX_SET_KEEP (-2)

/** @brief Effect-set argument that only clears the loaded tables. */
#define FIELD_SFX_SET_NONE (-1)

/** @brief Size of one weapon effect table in g_field_sound_tables. */
#define FIELD_WEAPON_SFX_TABLE_SIZE 0x1A00

/** @brief Number of weapon effect tables in g_field_sound_tables. */
#define FIELD_WEAPON_SFX_TABLE_COUNT 2

/** @brief Number of three-voice sound-effect channel groups. */
#define FIELD_SFX_GROUP_COUNT 8

/** @brief Voices per sound-effect channel group. */
#define FIELD_SFX_GROUP_VOICES 3

/** @brief Previous game state 6 (no GAME_STATE_ name yet); CHECKPS also treats the bank as resident after it. */
#define FIELD_AUDIO_BANK_RESIDENT_STATE 6

/** @brief Size of one streamed CD sector. */
#define FIELD_STREAM_SECTOR_SIZE 0x800

/** @brief The two alternating sector buffers of the music stream. */
#define FIELD_STREAM_SECTOR_BUFFER 0x801DC000
#define FIELD_STREAM_SECTOR_BUFFER_UPPER 0x801DC800

/** @brief Staging buffer for the streamed instrument bank data. */
#define FIELD_STREAM_BANK_BUFFER 0x801DD000

/** @brief Bank data past this point is moved back to the buffer start after an upload tick. */
#define FIELD_STREAM_BANK_OVERFLOW 0x801DD800

/** @brief Number of saved ring selections cleared by field_reset_ring_selections. */
#define FIELD_RING_SELECTION_COUNT 30

/** @brief Indices into g_field_song_handles. */
#define FIELD_SONG_MAIN 0
#define FIELD_SONG_SECOND 1

/** @brief States of the music stream (g_field_stream_state). */
enum
{
    FIELD_STREAM_IDLE = 0,
    FIELD_STREAM_SECTOR = 1,
    FIELD_STREAM_FIRST_SECTOR = 2,
    FIELD_STREAM_WAIT_UPLOAD = 3,
    FIELD_STREAM_WAIT_LAST_UPLOAD = 4
};

/**
 * @brief Effect tables copied out of an effect-set resource.
 *
 * The tables are copied one after another into table_data; active_table_offset
 * is the byte offset (from the start of this buffer) of the one sounds play from.
 */
typedef struct
{
    /** @brief Byte offset of the active table, or 0 when no set is loaded. */
    s32 active_table_offset;
    /** @brief Cleared with the set; not read in FIELD. */
    s32 reserved_0;
    /** @brief Cleared with the set; not read in FIELD. */
    s32 reserved_1;
    /** @brief Copied effect tables. */
    u8 table_data[0x2000 - 12];
} FieldSfxTableBuffer;

/** @brief Header of the first sector of a streamed music container. */
typedef struct
{
    /** @brief Section count of the container. */
    s32 section_count;
    /** @brief Offset of the song data in the sector. */
    s32 song_offset;
    /** @brief Offset of the instrument bank (the end of the song data). */
    s32 bank_offset;
} FieldMusicStreamHeader;

/*
 * D_8003ECA0 is the main executable's resident song area (also used by TITLE).
 * It holds an AKAO container: the section count at D_8003ECA0 and the section
 * offsets at D_8003ECA4.
 */
extern u8 D_8003ECA0[];
extern s32 D_8003ECA4[];

extern u8 *g_field_cd_buffer;
extern s32 g_field_song_volume;

/** @brief Second song area (FIELD-resident), played through AKAO command 0x19. */
extern u8 g_field_second_song[];

/** @brief Per effect set: non-zero to upload each table's bank into its own slot. */
extern u8 g_field_sfx_set_uses_slots[];

extern FieldSfxTableBuffer g_field_sfx_tables;
extern u8 g_field_sound_tables[];

/** @brief Registered AKAO instrument bank. */
extern AkaoHeader *g_field_instrument_bank;

/** @brief Song handles, indexed by FIELD_SONG_MAIN / FIELD_SONG_SECOND. */
extern s32 g_field_song_handles[2];

extern u8 g_field_ring_saved_selections[];

/* Music stream state (field_start_music_stream .. field_stream_sector_callback). */
extern s32 g_field_stream_sector_ready;
extern s32 g_field_stream_resource;
extern s32 g_field_stream_song_bytes;
extern u8 *g_field_stream_sector;
extern s32 g_field_stream_read_handle;
extern s32 g_field_stream_song_remaining;
extern s32 g_field_stream_bank_pending;
extern u32 g_field_stream_state;
extern s32 g_field_stream_bank_bytes;
extern u32 g_field_stream_bytes_left;
extern s32 g_field_stream_bytes_done;

static void field_stream_copy_bytes(u8 *dst, u8 *src, s32 count);
static u8 *field_stream_sector_callback(s32 bytes_transferred, u32 bytes_remaining);

/**
 * @brief Reload the instrument bank from EFFECT.SET unless the previous state kept it resident.
 */
void field_restore_entry_music(void)
{
    AkaoContainerHeader *container;
    u32 *off;

    if ((g_previous_game_state != GAME_STATE_TITLE) && (g_previous_game_state != GAME_STATE_GNAME) &&
        (g_previous_game_state != GAME_STATE_FIELD) &&
        (g_previous_game_state != FIELD_AUDIO_BANK_RESIDENT_STATE) && (g_previous_game_state != GAME_STATE_MENU_LOAD) &&
        (g_previous_game_state != GAME_STATE_WORLD_SELECT))
    {
        g_field_instrument_bank = (AkaoHeader *)FIELD_INSTRUMENT_BANK_ADDRESS;
        cdrom_queue_read(CD_RES_SOUND_EFFECT_SET, (void *)FIELD_AUDIO_LOAD_BUFFER);
        cdrom_wait_queue_empty();

        container = (AkaoContainerHeader *)FIELD_AUDIO_LOAD_BUFFER;
        off = container->section_offsets;
        bcopy(AKAO_CONTAINER_DATA_AT(container, off[0]), (u8 *)g_field_instrument_bank, (s32)(off[1] - off[0]));
        akao_register_bank(g_field_instrument_bank);
        akao_upload_bank_blocking((AkaoBankHeader *)AKAO_CONTAINER_DATA_AT(container, off[1]), 1);
    }
}

/**
 * @brief Load an instrument bank to its resident address and register it.
 * @param bank_index Bank index, relative to FIELD_INSTRUMENT_BANK_RESOURCE_BASE.
 * @return The result of akao_register_bank.
 */
s32 field_load_instrument_bank(s32 bank_index)
{
    akao_cmd_f1();
    g_field_instrument_bank = (AkaoHeader *)FIELD_INSTRUMENT_BANK_ADDRESS;
    cdrom_queue_read((bank_index + FIELD_INSTRUMENT_BANK_RESOURCE_BASE) & 0xFFFF, (void *)FIELD_INSTRUMENT_BANK_ADDRESS);
    cdrom_wait_queue_empty();
    return akao_register_bank(g_field_instrument_bank);
}

/**
 * @brief Load CD resource 22 and upload it as an instrument bank.
 */
void field_upload_resource_22_bank(void)
{
    cdrom_queue_read(FIELD_UPLOAD_BANK_RESOURCE, (void *)FIELD_AUDIO_LOAD_BUFFER);
    cdrom_wait_queue_empty();
    akao_upload_bank_blocking((AkaoBankHeader *)FIELD_AUDIO_LOAD_BUFFER, 1);
}

/**
 * @brief Load a song container, copy its song to a song area and upload its bank.
 * @details Counterpart of TITLE's load_title_seq.
 * @param music_index Music-file index (0 selects MSC_DATA.DAT); indices above
 *        FIELD_SONG_INDEX_MAX are ignored.
 * @param second_song Non-zero to copy the song to g_field_second_song instead of D_8003ECA0.
 */
void field_load_song(s32 music_index, s32 second_song)
{
    u32 *off;
    u8 *src;
    s32 count;

    if (music_index < FIELD_SONG_INDEX_MAX + 1)
    {
        cdrom_queue_read(CD_RES_MUSIC_FILE(music_index), (void *)FIELD_AUDIO_LOAD_BUFFER);
        cdrom_wait_queue_empty();

        off = (u32 *)FIELD_AUDIO_LOAD_OFFSETS;

        count = off[1] - off[0];
        /* The offset is the first addu operand here, so the sum is written int first. */
        src = (u8 *)(off[0] + FIELD_AUDIO_LOAD_BUFFER);

        if (second_song != 0)
        {
            bcopy(src, g_field_second_song, count);
        }
        else
        {
            bcopy(src, D_8003ECA0, count);
        }

        akao_upload_bank_blocking((AkaoBankHeader *)(off[1] + FIELD_AUDIO_LOAD_BUFFER), 1);
    }
}

/**
 * @brief Load the fixed song container (CD resource 146) and upload its bank.
 * @details Unlike field_load_song, the whole container up to its last section
 *          is copied to D_8003ECA0, and the last section is the bank.
 */
void field_load_fixed_song(void)
{
    u8 *dst;
    u32 *off_end;
    u32 count;

    cdrom_queue_read(FIELD_FIXED_SONG_RESOURCE, (void *)FIELD_AUDIO_LOAD_BUFFER);
    cdrom_wait_queue_empty();

    dst = D_8003ECA0;
    /* count holds the container address before its section count; reading the count directly changes the code. */
    count = FIELD_AUDIO_LOAD_BUFFER;
    count = *(u32 *)count;
    off_end = (u32 *)FIELD_AUDIO_LOAD_OFFSETS + count;

    bcopy((u8 *)FIELD_AUDIO_LOAD_BUFFER, dst, off_end[-1]);
    akao_upload_bank_blocking((AkaoBankHeader *)(off_end[-1] + FIELD_AUDIO_LOAD_BUFFER), 1);
}

/**
 * @brief Stop the field song.
 * @details Counterpart of TITLE's stop_title_music.
 */
void field_stop_song(void)
{
    akao_stop_song(0);
}

/**
 * @brief Stop the field song by its handle.
 */
void field_stop_field_song(void)
{
    akao_stop_song(g_field_song_handles[FIELD_SONG_MAIN]);
}

/**
 * @brief Stop the second song by its handle.
 */
void field_stop_second_song(void)
{
    akao_stop_song(g_field_song_handles[FIELD_SONG_SECOND]);
}

/**
 * @brief Play the song in D_8003ECA0 at the field song volume.
 * @note GOVER calls this after staging its own song with field_load_song.
 * @note Declared inline so field_update_music_stream gets its own copy, as in the original.
 */
inline void field_play_song(void)
{
    s32 song_handle;

    song_handle = akao_play_song((AkaoHeader *)D_8003ECA0);
    g_field_song_handles[FIELD_SONG_MAIN] = song_handle;
    akao_set_song_volume(song_handle, g_field_song_volume);
    akao_cmd_d4(0);
    akao_cmd_d0(0);
}

/**
 * @brief Play one section of the container in D_8003ECA0 through AKAO command 0x14.
 * @param section_index Section of the resident container.
 * @param param1 TODO: forwarded to AKAO command 0x14 as its second value.
 */
void field_play_song_section(s32 section_index, s32 param1)
{
    s32 song_handle;

    /* D_8003ECA0 + offsets[section_index], with the base formed from the offset table address. */
    song_handle = akao_cmd_14((u8 *)&D_8003ECA4 - 4 + D_8003ECA4[section_index], param1, 0);
    g_field_song_handles[FIELD_SONG_MAIN] = song_handle;
    if (song_handle == -1)
    {
        g_field_song_handles[FIELD_SONG_MAIN] = 0;
    }
    else if (song_handle == 0)
    {
        g_field_song_handles[FIELD_SONG_MAIN] = 0;
    }
    else
    {
        akao_set_song_volume(0, g_field_song_volume);
        akao_cmd_d4(0);
    }
    akao_cmd_d0(0);
}

/**
 * @brief Play the song in g_field_second_song at the field song volume.
 */
void field_play_second_song(void)
{
    g_field_song_handles[FIELD_SONG_SECOND] = akao_cmd_19_c0((s32)g_field_second_song, g_field_song_volume);
}

/**
 * @brief Fade a song to a volume.
 * @param song FIELD_SONG_MAIN or FIELD_SONG_SECOND.
 * @param frames Length of the fade.
 * @param volume Target volume (0 to AKAO_VOLUME_MAX).
 */
void field_fade_song(s32 song, s32 frames, s32 volume)
{
    akao_cmd_c1(g_field_song_handles[song], frames, volume);
}

/**
 * @brief Play a sound effect at full volume.
 * @param sound_id Sound id.
 * @param pan Pan position.
 */
void field_play_sound(s32 sound_id, s32 pan)
{
    akao_play_sfx(sound_id, 0, pan, AKAO_VOLUME_MAX);
}

/**
 * @brief Play a sound effect at full volume with the pan scaled from 0-127 to 0-254.
 * @param sound_id Sound id.
 * @param pan Pan position in half steps.
 */
void field_play_sound_half_pan(s32 sound_id, s32 pan)
{
    akao_play_sfx(sound_id, 0, pan * 2, AKAO_VOLUME_MAX);
}

/**
 * @brief Play an effect of the active effect table in channel group 0.
 * @param sfx_index Effect index.
 * @param pan Pan position.
 * @param unused Forwarded to field_play_set_sfx, which ignores it.
 */
void field_play_set_sfx_group0(s32 sfx_index, s32 pan, s32 unused)
{
    field_play_set_sfx(sfx_index, pan, unused, 0);
}

/**
 * @brief Play an effect of the active effect table on a free voice of a channel group.
 * @param sfx_index Effect index.
 * @param pan Pan position.
 * @param unused Not used.
 * @param channel_group Channel group; clamped to the last group.
 */
void field_play_set_sfx(s32 sfx_index, s32 pan, s32 unused, s32 channel_group)
{
    s32 *table;
    s32 base;
    s32 i;
    s32 mask;
    s32 buf;

    if (g_field_sfx_tables.active_table_offset != 0)
    {
        if (channel_group >= FIELD_SFX_GROUP_COUNT)
        {
            channel_group = FIELD_SFX_GROUP_COUNT - 1;
        }
        table = (s32 *)((u8 *)&g_field_sfx_tables + g_field_sfx_tables.active_table_offset);
        if ((u32)sfx_index < (u32)(table[0] - 1))
        {
            buf = (s32)table;
            i = 0;
            base = channel_group * FIELD_SFX_GROUP_VOICES;
            buf += table[sfx_index + 1];
            for (; i < FIELD_SFX_GROUP_VOICES; i++)
            {
                mask = 1 << (base + i);
                if (!akao_is_sfx_playing(mask))
                {
                    akao_play_sfx_from_buffer(buf, mask, pan, AKAO_VOLUME_MAX);
                    break;
                }
            }
        }
    }
}

/**
 * @brief Play an effect of a weapon effect table on a free voice of that table's channel group.
 * @param sfx_index Effect index.
 * @param pan Pan position.
 * @param table_index Weapon table (and channel group) index.
 */
void field_play_weapon_sfx(s32 sfx_index, s32 pan, s32 table_index)
{
    s32 *table;
    s32 base;
    s32 i;
    s32 mask;
    s32 buf;
    u8 *p;

    if (table_index < FIELD_WEAPON_SFX_TABLE_COUNT)
    {
        /* Taking the table base into a local first keeps the original instruction order. */
        p = g_field_sound_tables;
        table = (s32 *)(p + table_index * FIELD_WEAPON_SFX_TABLE_SIZE);
        if (table[0] != 0)
        {
            if ((u32)sfx_index < (u32)table[0])
            {
                i = 0;
                base = table_index * FIELD_SFX_GROUP_VOICES;
                buf = (s32)table + table[sfx_index + 1];
                for (; i < FIELD_SFX_GROUP_VOICES; i++)
                {
                    mask = 1 << (base + i);
                    if (!akao_is_sfx_playing(mask))
                    {
                        akao_play_sfx_from_buffer(buf, mask, pan, AKAO_VOLUME_MAX);
                        break;
                    }
                }
            }
        }
    }
}

/**
 * @brief Send AKAO command 0x21 for each voice of a channel group.
 * @param channel_group Channel group; clamped to the last group.
 */
void field_release_sfx_group(s32 channel_group)
{
    s32 bit;

    if (channel_group >= FIELD_SFX_GROUP_COUNT)
    {
        channel_group = FIELD_SFX_GROUP_COUNT - 1;
    }
    bit = channel_group * FIELD_SFX_GROUP_VOICES;
    akao_cmd_21(0, 1 << bit);
    akao_cmd_21(0, 1 << (bit + 1));
    akao_cmd_21(0, 1 << (bit + 2));
}

/**
 * @brief Load an effect-table set, copy its tables and upload their banks.
 * @param set_id Effect set; FIELD_SFX_SET_KEEP keeps the loaded set and
 *        FIELD_SFX_SET_NONE only clears it.
 * @note The resource holds a section count and offsets; each section is an
 *       effect table (its last offset entry marks its end) followed by its bank.
 */
void field_load_sfx_tables(s32 set_id)
{
    /* A separate copy of the set id for the slot-flag lookup; using set_id there changes register allocation. */
    s32 set_index = set_id;
    FieldSfxTableBuffer *tables;
    u8 *base;
    u8 *cursor;
    u8 *table;
    u8 *table_end;
    u8 *buffer;
    s32 *blob;
    s32 *entry;
    s32 count;
    s32 i;

    if (set_id == FIELD_SFX_SET_KEEP)
    {
        return;
    }
    g_field_sfx_tables.reserved_1 = 0;
    g_field_sfx_tables.reserved_0 = 0;
    g_field_sfx_tables.active_table_offset = 0;
    if (set_id == FIELD_SFX_SET_NONE)
    {
        return;
    }
    buffer = g_field_cd_buffer;
    cdrom_queue_read((u16)(set_id + FIELD_SFX_SET_RESOURCE_BASE), buffer);
    cdrom_wait_queue_empty();
    blob = (s32 *)buffer;
    base = (u8 *)&g_field_sfx_tables;
    tables = (FieldSfxTableBuffer *)base;
    cursor = tables->table_data;
    entry = blob + 1;
    count = blob[0];
    for (i = 0; i < count; i++)
    {
        /* Word-aligned offset of the table about to be copied. */
        tables->active_table_offset = ((cursor - base) >> 2) * 4;
        /* The offset is loaded into table first; adding blob + table directly swaps the addu operands. */
        table = (u8 *)blob + (s32)(table = (u8 *)*entry);
        table_end = table + ((s32 *)table)[*(s32 *)table];
        {
            u8 *dst = cursor;

            while (table != table_end)
            {
                *dst++ = *table++;
            }
            cursor = dst;
        }
        if (g_field_sfx_set_uses_slots[set_index] == 0)
        {
            akao_upload_bank_blocking((AkaoBankHeader *)table_end, 1);
            return;
        }
        func_80022EF8(table_end, i, 1);
        entry++;
    }
}

/**
 * @brief Load a weapon effect table and upload its bank into the weapon's slot.
 * @param slot Weapon table slot (0 or 1).
 * @param weapon_type Weapon type; FIELD_SFX_SET_KEEP keeps the table and
 *        FIELD_SFX_SET_NONE only clears it.
 */
void field_load_weapon_sfx_table(s32 slot, s32 weapon_type)
{
    u8 *dst;
    u8 *base;
    u8 *src;
    u8 *end;
    u8 *dst_cursor;

    if (weapon_type != FIELD_SFX_SET_KEEP)
    {
        /* Taking the table base into a local first keeps the original instruction order. */
        base = g_field_sound_tables;
        dst = base + slot * FIELD_WEAPON_SFX_TABLE_SIZE;
        *(s32 *)dst = 0;
        if (weapon_type != FIELD_SFX_SET_NONE)
        {
            weapon_type += FIELD_WEAPON_SFX_RESOURCE_BASE;
            src = g_field_cd_buffer;
            cdrom_queue_read(weapon_type & 0xFFFF, src);
            cdrom_wait_queue_empty();
            end = src + ((s32 *)src)[*(s32 *)src];
            dst_cursor = dst;
            while (src != end)
            {
                *dst_cursor++ = *src++;
            }
            func_80022ED8(end, slot, 1);
        }
    }
}

/**
 * @brief Play an effect buffer on a free voice of a channel group.
 * @param buffer Effect buffer address.
 * @param pan Pan position.
 * @param channel_group Channel group; groups past the last one are ignored.
 * @return Nothing meaningful; the original declares an int return and never sets it.
 */
s32 field_play_sfx_buffer(s32 buffer, s32 pan, s32 channel_group)
{
    s32 base;
    s32 i;
    s32 mask;

    if (channel_group * FIELD_SFX_GROUP_VOICES < FIELD_SFX_GROUP_COUNT * FIELD_SFX_GROUP_VOICES)
    {
        i = 0;
        base = channel_group * FIELD_SFX_GROUP_VOICES;
        for (; i < FIELD_SFX_GROUP_VOICES; i++)
        {
            mask = 1 << (base + i);
            if (!akao_is_sfx_playing(mask))
            {
                akao_play_sfx_from_buffer(buffer, mask, pan, AKAO_VOLUME_MAX);
                break;
            }
        }
    }
}

/**
 * @brief Reset the music stream state.
 * @note Declared inline so field_start_music_stream gets its own copy, as in the original.
 */
inline void field_reset_music_stream(void)
{
    g_field_stream_sector_ready = 0;
    g_field_stream_sector = 0;
    g_field_stream_bytes_left = 0;
    g_field_stream_bytes_done = 0;
    g_field_stream_state = FIELD_STREAM_IDLE;
    g_field_stream_read_handle = 0;
    g_field_stream_song_remaining = 0;
    g_field_stream_song_bytes = 0;
    g_field_stream_bank_bytes = 0;
    g_field_stream_bank_pending = 0;
    g_field_stream_resource = 0;
}

/**
 * @brief Start streaming a music file unless a stream is already running.
 * @param music_index Music-file index (0 selects MSC_DATA.DAT).
 */
void field_start_music_stream(s32 music_index)
{
    if (g_field_stream_resource == 0)
    {
        field_reset_music_stream();
        g_field_stream_resource = music_index + CD_RES_MSC_DATA;
        g_field_stream_read_handle =
            cdrom_queue_read_with_callback((u16)g_field_stream_resource, field_stream_sector_callback);
    }
}

/** @brief The stream header in the current sector. */
#define FIELD_MUSIC_STREAM_HEADER ((FieldMusicStreamHeader *)g_field_stream_sector)

/**
 * @brief Process the last streamed sector: copy song data, stage and upload bank data, then play.
 * @note The song is assembled in D_8003ECA0 and the bank data is staged at
 *       FIELD_STREAM_BANK_BUFFER.
 */
void field_update_music_stream(void)
{
    s32 song_tail;

    if (g_field_stream_sector_ready != 0)
    {
        switch (g_field_stream_state)
        {
        case FIELD_STREAM_SECTOR:
            if (g_field_stream_song_remaining != 0)
            {
                if (g_field_stream_song_remaining < FIELD_STREAM_SECTOR_SIZE)
                {
                    /* The song ends in this sector; the rest is bank data. */
                    bcopy(g_field_stream_sector, g_field_stream_song_bytes + D_8003ECA0, g_field_stream_song_remaining);
                    bcopy(g_field_stream_sector + g_field_stream_song_remaining, (void *)FIELD_STREAM_BANK_BUFFER,
                          FIELD_STREAM_SECTOR_SIZE - g_field_stream_song_remaining);
                    song_tail = g_field_stream_song_remaining;
                    g_field_stream_bank_pending = 1;
                    g_field_stream_song_remaining = 0;
                    g_field_stream_bank_bytes = FIELD_STREAM_SECTOR_SIZE - song_tail;
                    g_field_stream_song_bytes += song_tail;
                }
                else
                {
                    bcopy(g_field_stream_sector, g_field_stream_song_bytes + D_8003ECA0, FIELD_STREAM_SECTOR_SIZE);
                    g_field_stream_song_bytes += FIELD_STREAM_SECTOR_SIZE;
                    g_field_stream_song_remaining -= FIELD_STREAM_SECTOR_SIZE;
                    if (g_field_stream_song_remaining == 0)
                    {
                        g_field_stream_bank_bytes = 0;
                        g_field_stream_bank_pending = 1;
                    }
                }
                g_field_stream_sector_ready = 0;
                break;
            }
            if (g_field_stream_bank_pending != 0)
            {
                /* First bank sector: restart the upload. */
                bcopy(g_field_stream_sector, (void *)(g_field_stream_bank_bytes + FIELD_STREAM_BANK_BUFFER),
                      FIELD_STREAM_SECTOR_SIZE);
                g_field_stream_bank_bytes += FIELD_STREAM_SECTOR_SIZE;
                akao_reset_xfer_state();
                akao_streaming_upload_tick((u8 *)FIELD_STREAM_BANK_BUFFER, FIELD_STREAM_SECTOR_SIZE, 1);
                g_field_stream_bank_bytes -= FIELD_STREAM_SECTOR_SIZE;
                field_stream_copy_bytes((u8 *)FIELD_STREAM_BANK_BUFFER, (u8 *)FIELD_STREAM_BANK_OVERFLOW,
                                        g_field_stream_bank_bytes);
                g_field_stream_state = FIELD_STREAM_WAIT_UPLOAD;
            }
            else
            {
                bcopy(g_field_stream_sector, (void *)(g_field_stream_bank_bytes + FIELD_STREAM_BANK_BUFFER),
                      FIELD_STREAM_SECTOR_SIZE);
                if (g_field_stream_bytes_left < (u32)FIELD_STREAM_SECTOR_SIZE)
                {
                    /* Last sector: upload what is left, then play once the transfer is done. */
                    g_field_stream_bank_bytes += g_field_stream_bytes_left;
                    akao_streaming_upload_tick((u8 *)FIELD_STREAM_BANK_BUFFER, g_field_stream_bank_bytes, 1);
                    g_field_stream_state = FIELD_STREAM_WAIT_LAST_UPLOAD;
                    g_field_stream_bank_bytes = 0;
                }
                else
                {
                    g_field_stream_bank_bytes += FIELD_STREAM_SECTOR_SIZE;
                    akao_streaming_upload_tick((u8 *)FIELD_STREAM_BANK_BUFFER, FIELD_STREAM_SECTOR_SIZE, 1);
                    g_field_stream_state = FIELD_STREAM_WAIT_UPLOAD;
                    g_field_stream_bank_bytes -= FIELD_STREAM_SECTOR_SIZE;
                    field_stream_copy_bytes((u8 *)FIELD_STREAM_BANK_BUFFER, (u8 *)FIELD_STREAM_BANK_OVERFLOW,
                                            g_field_stream_bank_bytes);
                }
            }
            g_field_stream_bank_pending = 0;
            return;
        case FIELD_STREAM_FIRST_SECTOR:
            g_field_stream_song_remaining = FIELD_MUSIC_STREAM_HEADER->bank_offset - FIELD_MUSIC_STREAM_HEADER->song_offset;
            bcopy(g_field_stream_sector + FIELD_MUSIC_STREAM_HEADER->song_offset, D_8003ECA0,
                  FIELD_STREAM_SECTOR_SIZE - FIELD_MUSIC_STREAM_HEADER->song_offset);
            g_field_stream_sector_ready = 0;
            g_field_stream_song_bytes = FIELD_STREAM_SECTOR_SIZE - FIELD_MUSIC_STREAM_HEADER->song_offset;
            g_field_stream_song_remaining -= FIELD_STREAM_SECTOR_SIZE - FIELD_MUSIC_STREAM_HEADER->song_offset;
            return;
        case FIELD_STREAM_WAIT_UPLOAD:
            if (akao_get_xfer_state() == 0)
            {
                g_field_stream_sector_ready = 0;
            }
            break;
        case FIELD_STREAM_WAIT_LAST_UPLOAD:
            if (akao_get_xfer_state() == 0)
            {
                g_field_stream_sector_ready = 0;
                field_play_song();
                g_field_stream_resource = 0;
            }
            break;
        case FIELD_STREAM_IDLE:
        default:
            break;
        }
    }
}

/**
 * @brief Copy bytes forward.
 * @param dst Destination.
 * @param src Source.
 * @param count Number of bytes; nothing is copied when it is not positive.
 */
static void field_stream_copy_bytes(u8 *dst, u8 *src, s32 count)
{
    if (count > 0)
    {
        do
        {
            *dst++ = *src++;
        } while (--count != 0);
    }
}

/**
 * @brief CD read callback of the music stream: hand out the next sector buffer.
 * @param bytes_transferred Bytes read so far; bit 11 selects the upper sector buffer.
 * @param bytes_remaining Bytes still to read.
 * @return The sector buffer to read into, or NULL while the last sector is unprocessed.
 */
static u8 *field_stream_sector_callback(s32 bytes_transferred, u32 bytes_remaining)
{
    u8 *buffer;

    if (g_field_stream_sector_ready == 0)
    {
        if (bytes_transferred & FIELD_STREAM_SECTOR_SIZE)
        {
            g_field_stream_sector = buffer = (u8 *)FIELD_STREAM_SECTOR_BUFFER_UPPER;
        }
        else
        {
            g_field_stream_sector = buffer = (u8 *)FIELD_STREAM_SECTOR_BUFFER;
        }
        if (bytes_transferred == 0)
        {
            g_field_stream_state = FIELD_STREAM_FIRST_SECTOR;
        }
        else
        {
            g_field_stream_state = FIELD_STREAM_SECTOR;
        }
        g_field_stream_bytes_done = bytes_transferred;
        g_field_stream_bytes_left = bytes_remaining;
        g_field_stream_sector_ready = 1;
        return buffer;
    }
    return NULL;
}

/**
 * @brief Clear the saved ring menu selections, last entry first.
 */
void field_reset_ring_selections(void)
{
    s32 i = FIELD_RING_SELECTION_COUNT - 1;
    u8 *p = &g_field_ring_saved_selections[i];

    for (; i >= 0; i--)
    {
        *p = 0;
        p--;
    }
}
