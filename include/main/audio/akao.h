#ifndef _AKAO_H
#define _AKAO_H

#include "common.h"

/** @brief One 32-bit command slot, interpreted according to its opcode. */
typedef union
{
    s32 value;
    void* buffer;
} AkaoCommandParam;

extern AkaoCommandParam g_akao_cmd_params[];

/*
 * Halfword views of the driver's Q16 fixed-point words (integer part in the
 * high halfword).
 */
#define HALF_LOW_U16(word) (((u16*)&(word))[0])
#define HALF_HIGH_U16(word) (((u16*)&(word))[1])
#define HALF_HIGH_S16(word) (((s16*)&(word))[1])

/** @brief Number of SPU hardware voices; also the "no voice assigned" marker. */
#define AKAO_VOICE_COUNT 24

/** @brief Mask of the 24 SPU voice bits; also bounds the SFX channel-bit scan. */
#define AKAO_VOICE_MASK ((1 << AKAO_VOICE_COUNT) - 1)

/** @brief Number of channels in one song's channel table. */
#define AKAO_CHANNEL_COUNT 32

/** @brief Number of SFX channels (g_sfx_channels). */
#define AKAO_SFX_CHANNEL_COUNT 12

/** @brief First SPU voice assigned to SFX channels; SFX use voices 12-23. */
#define AKAO_SFX_FIRST_VOICE 12

/** @brief Channel-mask bit of the first SFX channel; SFX channels use bits 12-23. */
#define AKAO_SFX_FIRST_CHANNEL_BIT 0x1000

/** @brief Mask retaining the low 10 bits of a sound id. */
#define AKAO_SFX_ID_MASK 0x3FF

/** @brief Mask retaining the low 24 caller-defined bits of an SFX tag. */
#define AKAO_SFX_TAG_MASK 0xFFFFFF

/** @brief AkaoChannelState::sfx_tag bit that exempts an SFX from global volume/pan/pause control. */
#define AKAO_SFX_FLAG_SUPPRESS 0x02000000

/** @brief Stop mode selecting the oldest untagged SFX channels. */
#define AKAO_SFX_STOP_OLDEST 0x40000000

/** Maximum representable value for AKAO's 7-bit volume controls. */
#define AKAO_VOLUME_MAX 0x7F

/** @brief Mask retaining the low 8 bits of a pan command parameter. */
#define AKAO_PAN_MASK 0xFF

/** @brief Mask retaining the low 8 bits of a pitch-bend command parameter. */
#define AKAO_PITCH_BEND_MASK 0xFF

/** @brief Mask retaining the low 8 bits of a master-volume command parameter. */
#define AKAO_MASTER_VOLUME_MASK 0xFF

/** @brief Mask retaining the low 8 bits of a combined master pan/volume parameter. */
#define AKAO_MASTER_PAN_VOLUME_MASK 0xFF

/**
 * @brief Commands queued by akao_send_command.
 *
 * The game stores up to six 32-bit parameters in g_akao_cmd_params and queues
 * one of these opcodes. akao_send_command dispatches it through
 * g_akao_command_handlers; 0x98, 0x99 and 0xD8-0xDA fan out to several
 * handlers. The akao_cmd.c wrappers document each command's parameters.
 */
typedef enum AkaoCmd
{
    AKAO_CMD_IGNORE                          = 0x00, /**< Ignore the command. */
    AKAO_CMD_PLAY_SONG                       = 0x10, /**< Play a song (slot 0: AKAO sequence). */
    AKAO_CMD_STOP_SONG                       = 0x11, /**< Stop the song whose key matches slot 0. */
    AKAO_CMD_PLAY_SONG_WITH_TICKS            = 0x12, /**< Play a song and seed its pending tick count. */
    AKAO_CMD_START_SONG_CHANNELS             = 0x14, /**< Start a song on the channels in slot 1. */
    AKAO_CMD_SWITCH_SONG                     = 0x19, /**< Start a song, keeping the current one playing as the secondary song. */
    AKAO_CMD_PLAY_SFX                        = 0x20, /**< Play an SFX from the loaded banks (id, tag, pan, volume). */
    AKAO_CMD_STOP_SFX                        = 0x21, /**< Stop SFX channels by id or tag mask. */
    AKAO_CMD_PLAY_SFX_LIST                   = 0x24, /**< Play the SFX list in an AKAO buffer (buffer, tag, pan, volume). */
    AKAO_CMD_PLAY_SOUND                      = 0x30, /**< Play an SFX with the default tag, pan and volume. */
    AKAO_CMD_SUSPEND_SONG                    = 0x40, /**< Save the playing song so it can be resumed. */
    AKAO_CMD_SELECT_STEREO                   = 0x80, /**< Select stereo output. */
    AKAO_CMD_SELECT_MONO                     = 0x81, /**< Select mono output. */
    AKAO_CMD_MUTE_SONG_CHANNELS              = 0x90, /**< Silence the primary song channels in slot 0. */
    AKAO_CMD_SET_SONG_CONDITION              = 0x92, /**< Set the value tested by sequence op FE 07. */
    AKAO_CMD_RESUME_ALL                      = 0x98, /**< Resume songs, SFX and the streamed voices (0x9A, 0x9C, 0x9E). */
    AKAO_CMD_PAUSE_ALL                       = 0x99, /**< Pause songs, SFX and the streamed voices (0x9B, 0x9D, 0x9F). */
    AKAO_CMD_RESUME_SONG                     = 0x9A, /**< Resume the primary song. */
    AKAO_CMD_PAUSE_SONG                      = 0x9B, /**< Pause the primary song. */
    AKAO_CMD_RESUME_SFX                      = 0x9C, /**< Resume the SFX channels. */
    AKAO_CMD_PAUSE_SFX                       = 0x9D, /**< Pause the SFX channels. */
    AKAO_CMD_RESUME_XA                       = 0x9E, /**< Resume the streamed voice pair. */
    AKAO_CMD_PAUSE_XA                        = 0x9F, /**< Pause the streamed voice pair. */
    AKAO_CMD_SET_SFX_VOLUME                  = 0xA0, /**< Set the volume of SFX selected by id or tag mask. */
    AKAO_CMD_FADE_SFX_VOLUME                 = 0xA1, /**< Fade the volume of SFX selected by id or tag mask. */
    AKAO_CMD_SET_SFX_PAN                     = 0xA2, /**< Set the pan of SFX selected by id or tag mask. */
    AKAO_CMD_FADE_SFX_PAN                    = 0xA3, /**< Fade the pan of SFX selected by id or tag mask. */
    AKAO_CMD_SET_SFX_PITCH_BEND              = 0xA4, /**< Set the pitch bend of SFX selected by id or tag mask. */
    AKAO_CMD_FADE_SFX_PITCH_BEND             = 0xA5, /**< Fade the pitch bend of SFX selected by id or tag mask. */
    AKAO_CMD_SET_ALL_SFX_VOLUME              = 0xA8, /**< Set the volume of every SFX without AKAO_SFX_FLAG_SUPPRESS. */
    AKAO_CMD_FADE_ALL_SFX_VOLUME             = 0xA9, /**< Fade the volume of every SFX without AKAO_SFX_FLAG_SUPPRESS. */
    AKAO_CMD_SET_ALL_SFX_PAN                 = 0xAA, /**< Set the pan of every SFX without AKAO_SFX_FLAG_SUPPRESS. */
    AKAO_CMD_FADE_ALL_SFX_PAN                = 0xAB, /**< Fade the pan of every SFX without AKAO_SFX_FLAG_SUPPRESS. */
    AKAO_CMD_SET_ALL_SFX_PITCH_BEND          = 0xAC, /**< Set the pitch bend of every SFX without AKAO_SFX_FLAG_SUPPRESS. */
    AKAO_CMD_FADE_ALL_SFX_PITCH_BEND         = 0xAD, /**< Fade the pitch bend of every SFX without AKAO_SFX_FLAG_SUPPRESS. */
    AKAO_CMD_SET_SONG_VOLUME                 = 0xC0, /**< Set a song volume. */
    AKAO_CMD_FADE_SONG_VOLUME                = 0xC1, /**< Fade a song volume. */
    AKAO_CMD_FADE_SONG_VOLUME_FROM           = 0xC2, /**< Fade a song volume from a start level. */
    AKAO_CMD_SET_CD_VOLUME                   = 0xC8, /**< Set the CD audio volume. */
    AKAO_CMD_FADE_CD_VOLUME                  = 0xC9, /**< Fade the CD audio volume. */
    AKAO_CMD_FADE_CD_VOLUME_FROM             = 0xCA, /**< Fade the CD audio volume from a start level. */
    AKAO_CMD_SET_MASTER_PAN                  = 0xD0, /**< Set the master pan. */
    AKAO_CMD_FADE_MASTER_PAN                 = 0xD1, /**< Fade the master pan. */
    AKAO_CMD_FADE_MASTER_PAN_FROM            = 0xD2, /**< Fade the master pan from a start level. */
    AKAO_CMD_SET_MASTER_VOLUME               = 0xD4, /**< Set the master volume. */
    AKAO_CMD_FADE_MASTER_VOLUME              = 0xD5, /**< Fade the master volume. */
    AKAO_CMD_FADE_MASTER_VOLUME_FROM         = 0xD6, /**< Fade the master volume from a start level. */
    AKAO_CMD_SET_MASTER_PAN_AND_VOLUME       = 0xD8, /**< 0xD0 and 0xD4 with one value. */
    AKAO_CMD_FADE_MASTER_PAN_AND_VOLUME      = 0xD9, /**< 0xD1 and 0xD5 with one value. */
    AKAO_CMD_FADE_MASTER_PAN_AND_VOLUME_FROM = 0xDA, /**< 0xD2 and 0xD6 with one value. */
    AKAO_CMD_PLAY_XA_BUFFER                  = 0xE0, /**< Stream an XA program from a RAM buffer. */
    AKAO_CMD_STOP_XA                         = 0xE2, /**< Stop the streamed voice pair. */
    AKAO_CMD_SET_XA_VOLUME                   = 0xE4, /**< Set the streamed voice volume. */
    AKAO_CMD_FADE_XA_VOLUME                  = 0xE5, /**< Fade the streamed voice volume. */
    AKAO_CMD_SET_XA_PAN                      = 0xE6, /**< Set the streamed voice pan. */
    AKAO_CMD_PREPARE_XA_RING                 = 0xE8, /**< Prepare a CD-fed XA ring stream. */
    AKAO_CMD_PLAY_XA_ONE_SHOT                = 0xEC, /**< Upload an XA program to SPU RAM and play it once. */
    AKAO_CMD_PLAY_STAGED_XA                  = 0xED, /**< Play the XA program staged by akao_upload_xa_program. */
    AKAO_CMD_STOP_ALL_SONGS                  = 0xF0, /**< Stop the primary and secondary songs. */
    AKAO_CMD_RELEASE_ALL_SFX                 = 0xF1  /**< Release every SFX channel. */
} AkaoCmd;

/** @brief Creation time embedded in every AKAO file header; all fields are BCD. */
typedef struct AkaoTimeStamp
{
    u8 year_bcd;
    u8 month_bcd;
    u8 day_bcd;
    u8 hours_bcd;
    u8 minutes_bcd;
    u8 seconds_bcd;
} AkaoTimeStamp;

/**
 * @brief Common 16-byte header of every AKAO audio resource (songs, banks, XA programs).
 *
 * akao_check_magic validates it before the payload is handed to the driver.
 */
typedef struct AkaoHeader
{
    u32 magic; /**< "AKAO" (0x4F414B41). */
    u16 id;
    u16 length;
    u16 reverb_type;
    AkaoTimeStamp timestamp;
} AkaoHeader;

/**
 * @brief Header of an AKAO instrument bank (e.g. the EFFECT.SET fragments).
 *
 * articulation_count AkaoArticulation entries follow the header, then the
 * sample data. akao_upload_bank uploads the samples to the SPU, and
 * akao_relocate_articulations rebases the entries onto the SPU address before
 * installing them in the driver's articulation table.
 */
typedef struct AkaoBankHeader
{
    AkaoHeader header;
    u32 spu_dest_addr;      /**< SPU address the samples are uploaded to. */
    u32 sample_size;        /**< Bytes of sample data. */
    u32 bank_id;            /**< First entry of the driver articulation table to fill. */
    u32 articulation_count; /**< Number of articulation entries. */
    u32 unknown_0x20;
    u8 reserved[0x1C];
} AkaoBankHeader;

/**
 * @brief Fixed prefix of an offset-based AKAO resource container.
 *
 * Container offsets are byte offsets from the start of this header. Known
 * two-section resources store resident data first and an uploadable instrument
 * bank second.
 */
typedef struct
{
    u32 section_count;
    u32 section_offsets[2];
} AkaoContainerHeader;

/** Locate a byte offset within an AKAO container. */
#define AKAO_CONTAINER_DATA_AT(container, offset) \
    ((u8*)(container) + (offset))

s32 akao_register_bank(AkaoHeader* bank);
s32 akao_play_song(AkaoHeader* sequence);
s32 akao_set_song_volume(s32 song_handle, s32 volume);
void akao_upload_bank_blocking(AkaoBankHeader* bank, s32 wait_for_completion);

/**
 * @brief One entry of an AKAO instrument-bank articulation table.
 *
 * akao_relocate_articulations copies the entries into g_akao_articulation_slots
 * and adds the SPU upload base to sample_addr and loop_addr. The last two
 * words are SPU voice settings copied verbatim.
 */
typedef struct AkaoArticulation
{
    u32 sample_addr; /**< SPU sample start, relative to the bank until relocated. */
    u32 loop_addr;   /**< SPU loop point, relative to the bank until relocated. */
    union
    {
        u32 word;
        struct
        {
            s16 lo; /**< Fine tune added in akao_compute_pitch. */
            s16 hi; /**< Transpose in semitones added in akao_compute_pitch. */
        } half;
    } adsr;
    union
    {
        u32 word;
        struct
        {
            u16 lo; /**< Masked with 0x80FF when a note binds the articulation. */
            u16 hi; /**< Masked with 0x0020 when a note binds the articulation. */
        } half;
    } pitch_misc;
} AkaoArticulation;

/** @brief AkaoSongState.flags: a note found no voice, even by stealing one. */
#define AKAO_SONG_VOICE_DROPPED 0x1
/** @brief AkaoSongState.flags: a note had to steal a voice. */
#define AKAO_SONG_VOICE_STOLEN 0x2
/** @brief Song flag selecting the lower XA SPU area while channels are active or parked. */
#define AKAO_SONG_LOWER_XA_AREA_FLAG 0x40
/** @brief Song flag set when descriptor unk14 differs from g_akao_song_descriptor_match_value[0]. */
#define AKAO_SONG_DESCRIPTOR_MISMATCH 0x20
/** @brief Song flags selected by comparing the descriptor with g_akao_song_descriptor_match_value[0]. */
#define AKAO_SONG_DESCRIPTOR_FLAGS 0x60

/**
 * @brief Fixed header of an AKAO song (sequence) file.
 *
 * channel_offsets holds one entry for each channel in channel_mask, in bit
 * order; each is the offset of that channel's bytecode from the entry itself.
 * key_map_offset and note_table_offset are relative to their own fields.
 */
typedef struct AkaoSongDescriptor
{
    AkaoHeader header;
    u32 unk10;
    s32 unk14;                /**< Compared with g_akao_song_descriptor_match_value to choose song flag 0x40 or 0x20. */
    u8 _pad18[8];
    s32 channel_mask;         /**< Channels the song uses. */
    s32 voice_alloc_low_mask; /**< Initial AkaoSongMasks::voice_alloc_low_mask. */
    s32 static_voice_mask;    /**< Initial AkaoSongMasks::static_voice_mask. */
    u32 unk2C;
    s32 key_map_offset;       /**< Key map table offset, or 0 for none. */
    s32 note_table_offset;    /**< Note table offset, or 0 for none. */
    u8 _pad38[8];
    u16 channel_offsets[1];
} AkaoSongDescriptor;

/** @brief Channel masks of a song that drive voice allocation. */
typedef struct AkaoSongMasks
{
    u32 active_mask;          /**< Channels being ticked; the song ends when it reaches 0. */
    u32 voice_alloc_low_mask; /**< Channels that may take voices below voice_alloc_base. */
    u32 static_voice_mask;    /**< Channels that always play on the voice matching their index. */
    u32 key_on_mask;          /**< Channels whose note still needs a voice keyed on. */
} AkaoSongMasks;

/** @brief Channel flag retaining the full note duration before key-off. */
#define AKAO_CH_FULL_GATE 0x40

/** @brief Channel flag permitting SFX pitch modulation on the second channel of a pair. */
#define AKAO_CH_SFX_PITCH_MOD 0x10000

/** @brief Channel flag deferring stop requests until a sequence loop ends. */
#define AKAO_CH_DEFERRED_STOP 0x100000
/** @brief Channel flag recording a stop request for the sequence loop opcodes. */
#define AKAO_CH_STOP_PENDING 0x200000

/**
 * @brief Per-song sequencer state.
 *
 * The primary song lives in g_akao_seq_master_state and is reached through
 * g_akao_seq_channel0; a demoted song keeps playing from g_akao_suspended_song
 * through g_akao_seq_channel1. The channel masks use one bit per channel of
 * the song's AkaoChannelState table.
 */
typedef struct AkaoSongState
{
    u32 flags;                         /**< AKAO_SONG_* bits. */
    AkaoSongMasks masks;               /**< Channel masks used by the voice allocator. */
    u32 note_on_mask;                  /**< Channels with a note sounding. */
    u32 key_off_mask;                  /**< Channels whose voice still needs a key-off. */
    s32 parked_mask;                   /**< active_mask while the song is paused. */
    u32 tempo;                         /**< Q16 tick rate; the high half is added to tempo_acc per driver tick. */
    s32 tempo_step;                    /**< Tempo slide step per tick. */
    u32 tempo_acc;                     /**< Tick accumulator; a carry out of the low half advances one tick. */
    u8* song_data;                     /**< Address of the song descriptor in RAM. */
    u8* key_map_base;                  /**< Key-to-articulation map table of the song (ext op FE 14). */
    u8* note_table;                    /**< Note/articulation table used by drum-mode channels. */
    s32 voice_alloc_base;              /**< First voice the sequencer may allocate. */
    u32 noise_mask;                    /**< Channels enabled in the SPU noise bitmap. */
    u32 reverb_mask;                   /**< Channels enabled in the SPU reverb bitmap. */
    u32 pitch_mod_mask;                /**< Channels enabled in the SPU pitch-modulation bitmap. */
    s32 reverb_depth;                  /**< Reverb depth set by the song (Q16; SpuSetReverbModeDepth takes bits 12-27). */
    s32 reverb_depth_step;             /**< Per-tick step of the reverb depth slide. */
    s32 volume;                        /**< Volume set by the game (command 0xC0), Q16. */
    s32 volume_step;                   /**< Per-tick step of the volume fade. */
    u16 volume_fade_ticks;             /**< Ticks left in the volume fade. */
    s16 reverb_depth_fade_ticks;       /**< Ticks left in the reverb depth slide. */
    u16 tempo_fade_ticks;              /**< Ticks left in the tempo slide. */
    u16 song_id;                       /**< Id from the song's AKAO header; 0 when no song is loaded. */
    u16 condition;                     /**< Value tested by the conditional-jump opcode FE 07. */
    u16 noise_freq;                    /**< SPU noise clock (6 bits). */
    u16 beats_per_measure;             /**< Set by ext op FE 15. */
    u16 beat;                          /**< Beat within the measure. */
    u16 ticks_per_beat;                /**< Set by ext op FE 15. */
    u16 tick;                          /**< Tick within the beat. */
    u16 measure;                       /**< Measure counter (ext op FE 16). */
    u8 _pad6E[2];
} AkaoSongState;

/** @brief Packed pitch-LFO depth and its coefficient scaled by the current pitch. */
typedef struct
{
    u16 scaled; /**< Coefficient used to scale pitch-LFO waveform samples. */
    u16 raw;    /**< Q8 depth; bit 15 selects absolute scaling. */
} AkaoPitchLfoDepth;

/**
 * @brief State of one sequencer channel (a song track or an SFX channel).
 *
 * Each song owns a table of AKAO_CHANNEL_COUNT of these (g_akao_seq_channels
 * for the primary song); the SFX channels live in g_sfx_channels. A channel
 * runs its own bytecode stream with a loop stack, volume/pan/expression
 * envelopes and three LFOs, and keeps the register image of its SPU voice.
 */
typedef struct AkaoChannelState
{
    u8* seq_cursor;             /**< Next bytecode byte. */
    u8* loop_cursor[4];         /**< Loop start of each loop-stack level. */
    u8* return_cursor;          /**< Return address of a subroutine call (ext ops FE 0E/0F). */
    u8* key_map;                /**< Selected key-to-articulation map (ext op FE 14). */
    s16* pitch_lfo_cursor;      /**< Pitch LFO waveform position. */
    s16* volume_lfo_cursor;     /**< Volume LFO waveform position. */
    s16* pan_lfo_cursor;        /**< Pan LFO waveform position. */
    u32 sfx_tag;                /**< SFX: caller tag given to akao_play_sfx; also holds AKAO_SFX_FLAG_SUPPRESS. */
    s32 pitch;                  /**< Current SPU pitch (akao_compute_pitch result). */
    s32 pitch_slide_acc;        /**< Fractional part of the pitch slide. */
    s32 flags;                  /**< AKAO_CH_* bits. */
    s32 sfx_bank;               /**< SFX: bank slot the program was found in. */
    u32 sfx_id;                 /**< SFX: sound id. */
    u32 sfx_pitch_bend;         /**< SFX: pitch bend (Q8 in the low half). */
    u32 sfx_pitch_bend_step;    /**< SFX: per-tick step of the pitch bend fade. */
    s32 expression;             /**< Expression accumulator (value << 23). */
    s32 expression_step;        /**< Per-tick expression slide step. */
    s32 pitch_slide_step;       /**< Per-tick pitch slide (portamento) step. */
    s32 detune_pitch_delta;     /**< Detune contribution to the SPU pitch. */
    s32 sfx_age;                /**< SFX: ticks since the effect started; the oldest is stopped first. */
    s32 expression_preset;      /**< Expression loaded when a note starts. */
    s32 expression_preset_step; /**< Expression step loaded when a note starts. */
    u16 is_sfx_channel;         /**< Non-zero for an SFX channel. */
    u16 note_ticks;             /**< Ticks left in the current note. */
    u16 gate_ticks;             /**< Ticks left before the early key-off. */
    u16 articulation;           /**< Current articulation index. */
    u16 _pad6C;

    u16 pan_bias; /**< Pan bias added to the pan accumulator (ext FE 17/18). */
    u16 pan_bias_fade_ticks; /**< Pan-bias fade-tick countdown. */
    u16 opcode_count; /**< Opcodes executed; saved/restored by the loop ops. */
    u16 loop_count[4]; /**< Iteration counter per loop-stack level. */
    u16 loop_opcode_count[4]; /**< Saved opcode_count per loop-stack level. */
    u16 volume; /**< Channel volume accumulator (high byte is the level). */
    u16 volume_fade_ticks; /**< Volume fade-tick countdown. */
    u16 sfx_pitch_bend_fade_ticks; /**< SFX: ticks left in the pitch-bend fade. */
    u16 expression_fade_ticks; /**< Expression fade-tick countdown. */
    u16 note_expression_ticks; /**< Note-start expression fade length (ext FE 19). */
    u16 volume_scale_fade_ticks; /**< Ticks left in the volume_scale fade. */
    u16 pan; /**< Pan accumulator (high byte is the pan). */
    u16 pan_fade_ticks; /**< Pan fade-tick countdown. */
    u16 pitch_slide_ticks; /**< Pitch-slide ticks remaining. */
    u16 octave; /**< Current octave. */
    u16 pitch_slide_duration; /**< Pitch-slide duration in ticks. */
    u16 prev_key; /**< Previous note key (portamento source). */
    u16 portamento_speed; /**< Portamento speed; 0 disables portamento. */
    u16 note_flags; /**< Note flags: 0x1 tie armed, 0x2 note is tied, 0x4 SFX full gate time. */
    u16 _padA0;
    u16 pitch_lfo_delay; /**< Pitch-LFO delay. */
    u16 pitch_lfo_delay_ticks; /**< Pitch-LFO delay countdown. */
    s16 pitch_lfo_period; /**< Pitch-LFO period. */
    u16 pitch_lfo_restart; /**< Pitch-LFO restart flag (1 = waveform not started). */
    u16 pitch_lfo_waveform; /**< Pitch-LFO waveform index into g_akao_lfo_waveforms. */
    AkaoPitchLfoDepth pitch_lfo_depth; /**< Raw and pitch-scaled LFO depth. */
    u16 pitch_lfo_depth_fade_ticks; /**< Pitch-LFO depth-slide tick countdown. */
    u16 pitch_lfo_depth_step; /**< Pitch-LFO depth-slide step. */
    u16 _padB4;
    u16 volume_lfo_delay; /**< Volume-LFO delay. */
    u16 volume_lfo_delay_ticks; /**< Volume-LFO delay countdown. */
    s16 volume_lfo_period; /**< Volume-LFO period. */
    u16 volume_lfo_restart; /**< Volume-LFO restart flag. */
    u16 volume_lfo_waveform; /**< Volume-LFO waveform index. */
    u16 volume_lfo_depth; /**< Volume-LFO depth. */
    u16 volume_lfo_depth_fade_ticks; /**< Volume-LFO depth-slide tick countdown. */
    u16 volume_lfo_depth_step; /**< Volume-LFO depth-slide step. */
    u16 _padC6;
    u16 pan_lfo_period; /**< Pan-LFO period. */
    u16 pan_lfo_restart; /**< Pan-LFO restart flag. */
    u16 pan_lfo_waveform; /**< Pan-LFO waveform index. */
    u16 pan_lfo_depth; /**< Pan-LFO depth. */
    u16 pan_lfo_depth_fade_ticks; /**< Pan-LFO depth-slide tick countdown. */
    u16 pan_lfo_depth_step; /**< Pan-LFO depth-slide step. */
    u16 noise_toggle_ticks; /**< Ticks until the noise enable bit is toggled back. */
    u16 pitch_mod_toggle_ticks; /**< Ticks until the pitch-mod enable bit is toggled back. */
    u16 loop_depth; /**< Loop-stack index (0..3, wraps). */
    u16 pitch_scale; /**< Pitch scalar applied to the computed pitch (Q8). */
    s16 note_duration; /**< Last note duration set by opcode 0xA2. */
    u16 note_duration_adjust; /**< Note-duration adjustment (opcode 0xDC). */
    s16 volume_step; /**< Volume slide step. */
    s16 pan_bias_step; /**< Pan-bias slide step. */
    u16 volume_scale; /**< Extra volume scale; high byte is a signed Q7 factor. */
    u16 volume_scale_step; /**< Per-tick step of the volume_scale fade (signed). */
    s16 pan_step; /**< Pan slide step. */
    u16 transpose; /**< Transpose in semitones (opcodes 0xC0/0xC1). */
    s16 detune; /**< Fine detune (opcodes 0xD8/0xD9). */
    u16 note_key; /**< Key of the note currently sounding. */
    u16 pitch_slide_delta; /**< Pending pitch-slide delta in semitones. */
    s16 prev_transpose; /**< Transpose in effect for the previous note. */
    s16 pitch_lfo_value; /**< Current pitch-LFO output. */
    s16 volume_lfo_value; /**< Current volume-LFO output. */
    s16 pan_lfo_value; /**< Current pan-LFO output. */
    u16 _padFA;
    u32 voice; /**< Assigned SPU voice index (0x18 = none). */
    s32 update_flags; /**< Pending SPU register update flags. */
    s32 spu_sample_addr; /**< SPU sample start. */
    s32 spu_loop_addr; /**< SPU loop point. */
    u16 spu_pitch; /**< SPU pitch/sample-rate register image. */
    u16 spu_adsr_low; /**< SPU ADSR low halfword. */
    u16 spu_adsr_high; /**< SPU ADSR high halfword. */
    u16 spu_volume_scale; /**< Optional Q7 scale. */
    s16 spu_volume_left;
    s16 spu_volume_right;
} AkaoChannelState;

#endif
