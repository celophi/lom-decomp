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
        u16 characterCode;
        struct
        {
            u16 usedThisFrame : 1;
            u16 reserved : 15;
        } flags;
    } data;
} GlyphCacheEntry;

/** One entry in the CHECKPS CD command protocol table. */
typedef struct
{
    u8 opcode;
    u8 parameterCount;
    u8 responseCount;
    u8 irqCodeSumTarget; /* sum of CD IRQ codes required before consuming the response */
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

/* PollCdResponse return values. */
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
    u16 firstGlyph;
    u16 secondGlyph;
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
    u32 packedSize;
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
            u8 wordCount;
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
    RECT clearRect;    // +0x70
} CheckPSDisplayBuffer;

/** One 0xBCCC-byte CHECKPS render frame. */
typedef struct
{
    u8 reservedHeader[0x40];
    u_long orderingTable[CHECKPS_ORDERING_TABLE_LENGTH];
    CheckPSDisplayBuffer display;
    u8 primitiveBuffer[CHECKPS_PRIMITIVE_BUFFER_SIZE];
    u8* primitiveCursor;
    u8 reservedTail[0x3C10];
} CheckPSFrame;

/** Double-buffered CHECKPS renderer state. */
typedef struct
{
    CheckPSFrame frames[2];
} CheckPSRenderState;

extern s32 g_previousGameState;

/* Hardware-failure screen data. */
extern u8 g_hardwarePatternSizeTable[][2];
extern const u32 g_hardwareModificationWarning[15];

/* Embedded/generated CHECKPS assets whose binary symbols are not yet split further. */
extern u32 g_embeddedCheckpsAkao;
extern u8 g_checkpsImageAsset[];
extern u16 g_decimalGlyphTable[];
extern u16 g_hexGlyphTable[];

/* CD-ROM integrity-check state machine globals. */
extern s32 g_checkpsState;
extern s32 g_checkpsVsyncTimestamp;
extern s32 g_cdLastTrackBcd;
extern u8 g_cdSeekPositionBcd[8];
extern const s32 g_checkpsUnusedConstant17;
extern CdCommandDescriptor g_cdCommandTable[];
extern u8 g_cdCommandParameters[3];
extern CdResponsePrefix g_cdResponse;
extern s32 g_cdIrqCodeSum;
extern u8 g_cdResponseByte2;
extern u8 g_cdResponsePayload[2];
extern volatile u8* g_cdStatusRegister;
extern volatile u8* g_cdResponseRegister;
extern volatile u8* g_cdDataRegister;
extern volatile u8* g_cdIrqRegister;
extern u8 g_controllerDeviceType;

s32 RunCheckPS(s32 renderStateAddress);
void RunCheckPSDisplayLoop(CheckPSRenderState* renderState);
void InitCheckPSDisplay(CheckPSRenderState* renderState);
void LoadEmbeddedCheckPSAudio(void);
void LoadCheckPSSongFromDisc(s32 songIndex);
void StopCheckPSSong(void);
void PlayLoadedCheckPSSong(void);
void PlayCheckPSSfx(u32 soundId, u32 volume, u32 pan);
void ResetFadeState(void);
void UpdateAndDrawFade(CheckPSFrame* frame);
void SetFadeTarget(s32 red, s32 green, s32 blue, s32 steps);
void UpdateCheckPSInputAndTimeout(void);
void DrawCheckPSImage(CheckPSFrame* frame);
void LoadCheckPSImage(void);
s32 PollInputDevice(void);
void ProcessControllerInput(void);
void UpdateControllerInput(void);

void* DrawSignedDecimal(void* primitive, u_long* otTag, s32 value, s32 x, s32 y, s32 palette, s32 alignment);
void DrawHexByte(void* primitive, u_long* otTag, s32 value, s32 x, s32 y, s32 alignment);
void* DrawCachedText(void* primitive, u_long* otTag, const u8* text, s32 x, s32 y, s32 palette, s32 alignment);
void* RenderCachedGlyph(void* primitive, u_long* otTag, s32 characterCode, s32 palette);
GlyphSpritePacket* EmitGlyphSprite(GlyphSpritePacket* packet, u_long* otTag, s32 cacheSlot);
void BeginGlyphCacheFrame(void);
void EvictUnusedGlyphs(void);
void ResetGlyphRenderer(void);

void DrawKanjiString(const char* text, KanjiDrawState* drawState, s32 color);
void DrawKanjiGlyph(KanjiDrawState* drawState, u8* bitmap, s32 color);
void DrawHardwareCheckPattern(void);

void StartCdIntegrityCheck(void);
s32 RunCdIntegrityCheck(s32 singleStep);
CheckPSCdPollResult PollCdResponse(CheckPSCdCommandIndex command);
void SendCdCommand(CheckPSCdCommandIndex command);
void ShowHardwareModificationWarningAndExit(void);

#endif
