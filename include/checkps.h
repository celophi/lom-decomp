#ifndef _CHECKPS_H
#define _CHECKPS_H

#include "common.h"
#include "akao.h"
#include "display.h"
#include "pad.h"
#include "psyq/libgte.h"
#include "psyq/libgpu.h"
#include "psyq/libapi.h"
#include "psyq/memory.h"
#include "psyq/strings.h"

#define MAX_GLYPH_ENTRIES 256
#define CHECKPS_ORDERING_TABLE_LENGTH 0x1000
#define CHECKPS_PRIMITIVE_BUFFER_SIZE 0x4000

/* Bit 16 is cleared at the start of each frame and set when a cache slot is emitted. */
#define GLYPH_USED_FLAG 0x10000

typedef struct
{
    s32 red;
    s32 green;
    s32 blue;
    s32 steps;
} FadeColor;

/* The first two bytes of the CD response area have named uses in the state machine. */
typedef struct
{
    u8 status;
    u8 detail;
} CdResponsePrefix;

typedef union
{
    u32 raw;
    struct
    {
        u16 character_code;
        struct
        {
            u16 used_this_frame : 1;
            u16 reserved : 15;
        } flags;
    } data;
} GlyphCacheEntry;

/** One entry in the CHECKPS CD command protocol table. */
typedef struct
{
    u8 opcode;
    u8 parameter_count;
    u8 response_count;
    u8 irq_code_sum_target; /* sum of CD IRQ codes required before consuming the response */
} CdCommandDescriptor;

typedef enum
{
    CHECKPS_CD_CMD_NOP = 0,
    CHECKPS_CD_CMD_GET_TN,
    CHECKPS_CD_CMD_GET_TD,
    CHECKPS_CD_CMD_SETLOC,
    CHECKPS_CD_CMD_SEEK_P,
    CHECKPS_CD_CMD_SETMODE,
    CHECKPS_CD_CMD_INIT,
    CHECKPS_CD_CMD_MUTE,
    CHECKPS_CD_CMD_PLAY,
    CHECKPS_CD_CMD_TEST_04,
    CHECKPS_CD_CMD_TEST_05,
    CHECKPS_CD_CMD_PAUSE,
    CHECKPS_CD_CMD_READ_TOC,
    CHECKPS_CD_CMD_GET_ID,
} CheckPSCdCommandIndex;

/* Internal states of the CD-ROM integrity check sequence. */
typedef enum
{
    CHECKPS_STATE_IDLE = 0,
    CHECKPS_STATE_START_GET_TN,
    CHECKPS_STATE_WAIT_GET_TN,
    CHECKPS_STATE_WAIT_INIT,
    CHECKPS_STATE_WAIT_GET_TD,
    CHECKPS_STATE_WAIT_READ_TOC,
    CHECKPS_STATE_WAIT_GET_ID,
    CHECKPS_STATE_WAIT_SETLOC,
    CHECKPS_STATE_WAIT_SETMODE,
    CHECKPS_STATE_SEEK_DELAY,
    CHECKPS_STATE_WAIT_SEEK_P,
    CHECKPS_STATE_WAIT_MUTE,
    CHECKPS_STATE_WAIT_PLAY,
    CHECKPS_STATE_WAIT_TEST_04,
    CHECKPS_STATE_TEST_05_DELAY,
    CHECKPS_STATE_WAIT_TEST_05,
    CHECKPS_STATE_WAIT_FAILURE_NOP,
    CHECKPS_STATE_WAIT_RECOVERY_NOP,
    CHECKPS_STATE_WAIT_PAUSE,
    CHECKPS_STATE_PAUSE_DELAY,
} CheckPSState;

/* poll_cd_response return values. */
typedef enum
{
    CHECKPS_CD_POLL_SHELL_OPEN = -2,
    CHECKPS_CD_POLL_DISK_ERROR = -1,
    CHECKPS_CD_POLL_PENDING = 0,
    CHECKPS_CD_POLL_COMPLETE = 1,
} CheckPSCdPollResult;

/** Two Shift-JIS glyph codes followed by the text renderer's terminator. */
typedef struct
{
    u16 first_glyph;
    u16 second_glyph;
    s16 terminator;
} EncodedGlyphPair;

/* Same eight-byte layout as KanjiDrawState, expressed as four halfword stores. */
typedef struct
{
    s16 x;
    s16 y;
    s16 width;
    s16 height;
} KanjiDrawStateWords;

typedef struct
{
    union
    {
        struct
        {
            s16 x;
            s16 y;
        } coord;
        s32 packed;
    } position;
    u32 packed_size;
} KanjiDrawState;

/** GPU sprite packet used by the cached 16x16 text renderer. */
typedef struct
{
    union
    {
        u32 raw;
        struct
        {
            u8 _pad0[3];
            u8 word_count;
        } byte;
    } tag;
    u8 r;
    u8 g;
    u8 b;
    u8 code;
    s16 x;
    s16 y;
    u8 u;
    u8 v;
    u16 clut;
} GlyphSpritePacket;

/** One of the two display/draw environment pairs used by CHECKPS. */
typedef struct
{
    DISPENV disp;      // +0x00
    DRAWENV draw;      // +0x14
    RECT clear_rect;    // +0x70
} CheckPSDisplayBuffer;

/** One 0xBCCC-byte CHECKPS render frame. */
typedef struct
{
    u8 reserved_header[0x40];
    u_long ordering_table[CHECKPS_ORDERING_TABLE_LENGTH];
    CheckPSDisplayBuffer display;
    u8 primitive_buffer[CHECKPS_PRIMITIVE_BUFFER_SIZE];
    u8* primitive_cursor;
    u8 reserved_tail[0x3C10];
} CheckPSFrame;

/** Double-buffered CHECKPS renderer state. */
typedef struct
{
    CheckPSFrame frames[2];
} CheckPSRenderState;

extern s32 g_previousGameState;

/* Hardware-failure screen data. */
extern u8 g_hardware_pattern_size_table[][2];
extern const u32 g_hardware_modification_warning[15];

/* Embedded/generated CHECKPS assets whose binary symbols are not yet split further. */
extern u32 g_embedded_checkps_akao;
extern u8 g_checkps_image_asset[];
extern u16 g_decimal_glyph_table[];
extern u16 g_hex_glyph_table[];

/* CD-ROM integrity-check state machine globals. */
extern s32 g_checkps_state;
extern s32 g_checkps_vsync_timestamp;
extern s32 g_cd_last_track_bcd;
extern u8 g_cd_seek_position_bcd[8];
extern const s32 g_checkps_unused_constant17;
extern CdCommandDescriptor g_cd_command_table[];
extern u8 g_cd_command_parameters[3];
extern CdResponsePrefix g_cd_response;
extern s32 g_cd_irq_code_sum;
extern u8 g_cd_response_byte2;
extern u8 g_cd_response_payload[2];
extern volatile u8* g_cd_status_register;
extern volatile u8* g_cd_response_register;
extern volatile u8* g_cd_data_register;
extern volatile u8* g_cd_irq_register;
extern u8 g_controller_device_type;

s32 run_checkps(s32 render_state_address);
void run_checkps_display_loop(CheckPSRenderState* render_state);
void init_checkps_display(CheckPSRenderState* render_state);
void load_embedded_checkps_audio(void);
void load_checkps_song_from_disc(s32 song_index);
void stop_checkps_song(void);
void play_loaded_checkps_song(void);
void play_checkps_sfx(u32 sound_id, u32 volume, u32 pan);
void reset_fade_state(void);
void update_and_draw_fade(CheckPSFrame* frame);
void set_fade_target(s32 red, s32 green, s32 blue, s32 steps);
void update_checkps_input_and_timeout(void);
void draw_checkps_image(CheckPSFrame* frame);
void load_checkps_image(void);
s32 poll_input_device(void);
void process_controller_input(void);
void update_controller_input(void);

void* draw_signed_decimal(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void draw_hex_byte(void* primitive, u_long* ot_tag, s32 value, s32 x, s32 y, s32 alignment);
void* draw_cached_text(void* primitive, u_long* ot_tag, const u8* text, s32 x, s32 y, s32 palette, s32 alignment);
void* render_cached_glyph(void* primitive, u_long* ot_tag, s32 character_code, s32 palette);
GlyphSpritePacket* emit_glyph_sprite(GlyphSpritePacket* packet, u_long* ot_tag, s32 cache_slot);
void begin_glyph_cache_frame(void);
void evict_unused_glyphs(void);
void reset_glyph_renderer(void);

void draw_kanji_string(const char* text, KanjiDrawState* draw_state, s32 color);
void draw_kanji_glyph(KanjiDrawState* draw_state, u8* bitmap, s32 color);
void draw_hardware_check_pattern(void);

void start_cd_integrity_check(void);
s32 run_cd_integrity_check(s32 single_step);
CheckPSCdPollResult poll_cd_response(CheckPSCdCommandIndex command);
void send_cd_command(CheckPSCdCommandIndex command);
void show_hardware_modification_warning_and_exit(void);

#endif
