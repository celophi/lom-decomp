
typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned char undefined;
typedef unsigned char undefined1;
typedef unsigned short undefined2;
typedef unsigned int undefined4;
typedef int s32;
typedef unsigned int u32;
typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef signed short s16;
typedef enum CdResourceId
{
  CD_RES_SYSTEM_CNF = 0,
  CD_RES_SLUS_010_13 = 1,
  CD_RES_FIELD_BIN = 2,
  CD_RES_WMAP_BIN = 3,
  CD_RES_TITLE_BIN = 4,
  CD_RES_GNAME_BIN = 5,
  CD_RES_MENU_BIN = 6,
  CD_RES_SHOP_BIN = 7,
  CD_RES_ZUKAN_BIN = 8,
  CD_RES_GOLEM_BIN = 9,
  CD_RES_GOVER_BIN = 10,
  CD_RES_MOVIE_BIN = 11,
  CD_RES_CARDA_BIN = 12,
  CD_RES_GOSUB_BIN = 13,
  CD_RES_WSEL_BIN = 14,
  CD_RES_CHECKPS_BIN = 15,
  CD_RES_CLOAD_BIN = 16,
  CD_RES_NIKI_BIN = 17,
  CD_RES_ADDHERO_BIN = 18,
  CD_RES_SOUND_EFFECT_SET = 21
} CdResourceId;
typedef struct
{
    u8  _unk000[0x18];          /**< 0x000: not yet mapped. */
    s32 unk018;                 /**< 0x018: main.c masks 0xFE000000 and ORs in 6. */
    s16 unk01C;                 /**< 0x01C: copied to companion global D_8003EC94. */
    s8  unk01E;                 /**< 0x01E: copied to companion global D_80046FD8. */
    u8  unk01F;                 /**< 0x01F: not yet mapped. */
    u32 unk020;                 /**< 0x020: -> D_80046FDE; index into D_800351A0[]. */
    u16 unk024;                 /**< 0x024: -> D_8003EC90; mode/scene id. */
    u8  unk026;                 /**< 0x026: copied to companion global D_80042FCC. */
    u8  unk027;                 /**< 0x027: copied to companion global D_80042FC4. */
    u32 unk028;                 /**< 0x028: flag word; bits 0xC tested together. */
    u8  _unk02C[0x34 - 0x2C];   /**< 0x02C: not yet mapped. */
    s32 unk034[0xB];            /**< 0x034: 11 s32 slots; cleared per non-selected save slot. */
    u8  _unk060[0xD4 - 0x60];   /**< 0x060: not yet mapped. */
    s16 rng_seed;               /**< 0x0D4: composed random value (rand() based). */
    u8  _unk0D6[0x2E0 - 0xD6];  /**< 0x0D6: not yet mapped. */
    s32 mode_flags;             /**< 0x2E0: state bitfield; bit 0 = "continue mode". */
    u8  _unk2E4[0x608 - 0x2E4]; /**< 0x2E4: not yet mapped. */
    s32 slot_flags;             /**< 0x608: low 7 bits + bit 0 (continue). */
} MenuLayout;   
typedef struct
{
  short x;
  short y;
  short w;
  short h;
} RECT;
extern u32 g_overlayLoadAddress;
extern u8 D_800351A0[];
extern u32 D_8003EC88;
extern s32 D_8003EC8C;
extern u16 D_8003EC90;
extern s32 D_8003EC94;
extern s32 D_8003EC98;
extern s32 D_8003EC9C;
extern s32 D_80042FC4;
extern s32 D_80042FCC;
extern s32 D_80042FD0;
extern u32 g_gameDataBasePtr;
extern s32 D_80046FD8;
extern u16 D_80046FDE;
extern s32 D_800473E0;
extern u32 g_gameState;
extern u32 g_previousGameState;
void __main(void);
extern void SetMem(long);
extern long SetConf(unsigned long, unsigned long, unsigned long);
extern int ResetGraph(int mode);
extern void SetDispMask(int mask);
extern int SetGraphDebug(int level);
extern void SpuInit(void);
void InitializeControllers(undefined1 controllerMode);
int ResetCallback(void);
extern void _96_remove(void);


extern void InitCARD(long val);
void cdrom_init(void);
u32 *func_80015C48(void);
void cdrom_load_resource_table(int lba, int dataSizeBytes);
long SetVideoMode(long mode);
undefined4 FUN_80021fbc(void);
s32 cdrom_stream(s32 resourceIndex, u32 destination);
void InitVSyncController(void);
extern long StartCARD(void);
void McxStartCom();
u32 func_8004FC8C(u32);
void srand(u_int param_1);
extern void _bu_init(void);
extern void InitGeom();
void func_8004FD14(s32);
s32 func_801400C4(void);
undefined *FUN_80015c18(void);
s32 akao_cmd_f0(void);
int VSync(int mode);
u32 *FUN_80015c28(void);
void cdrom_wait_queue_empty(void);
void func_80051FBC(u32);
void FUN_80011638(int param_1);
void movie_play(s32 movie_index);
extern int DrawSync(int mode);
extern void ChangeClearPAD(long);
s32 func_8004FC74(s32);
u32 run_overlay(u32, u32, u32, s32, s32, u32, s32);

/**
 * decomp.me (100%) https://decomp.me/scratch/Tc7j3
 */
 void Main(void)
{
  RECT rect;
  MenuLayout *temp_s2;
  u8 *ptrB;
  u32 *ptrA;
  int new_var;
  s32 new_var3;
  long cd_stop_ret;
  u32 state;
  int new_var2;
  __main();
  SetMem(2);
  SetConf(0x10, 4, 0x801FFF00);
  ResetGraph(0);
  SetGraphDebug(0);
  SetDispMask(0);
  _96_remove();
  ResetCallback();
  SetVideoMode(0);
  SpuInit();
  FUN_80015c18();
  cdrom_init();
  InitGeom();
  InitCARD(0);
  StartCARD();
  _bu_init();
  McxStartCom();
  ChangeClearPAD(0);
  InitializeControllers(0);
  cdrom_load_resource_table(0x18, 0xB598);
  FUN_80021fbc();
  cdrom_stream(0xB2, 0x801E1200);
  InitVSyncController();
  srand(1);
  D_80042FC4 = 0;
  D_80046FD8 = -1;
  D_8003EC94 = -1;
  D_8003EC90 = 0;
  D_8003EC9C = 7;
  D_800473E0 = 0;
  D_8003EC8C = 0xB;
  D_80042FD0 = 0x13;
  g_gameState = 8U;
  FUN_80015c28();
  cdrom_stream(15, g_overlayLoadAddress);
  cdrom_wait_queue_empty();
  RunCheckPS(0x80100000);
  DrawSync(0);
  VSync(0);
  g_previousGameState = 0xFF;
  while (1)
  {
    {
      state = g_gameState;
      ptrB = (u8 *) 0x80040000;
      switch (state)
      {
        case 0:

        case 9:

        case 10:
          SetDispMask(0);
          VSync(0);
          DrawSync(0);
          FUN_80015c28();
          cdrom_stream(CD_RES_FIELD_BIN, g_overlayLoadAddress);
          if (g_gameState != 0)
        {
          cdrom_stream(11, 0x80140000);
          cdrom_wait_queue_empty();
          if ((unsigned long) (g_gameState == 9))
          {
            movie_play(1);
          }
          else
          {
            movie_play(2);
            movie_play(3);
            movie_play(4);
          }
        }
        else
        {
          cdrom_wait_queue_empty();
        }
          D_80042FCC = 0;
          *((u32 *) (ptrB - 0x1378)) = 0;
          g_gameState = FUN_80015c58();
          akao_cmd_f0();
          akao_cmd_f1();
          akao_cmd_c0(0, 0x7F);
          g_previousGameState = 0;
          break;

        case 1:
          FUN_80015c38();
          cdrom_stream(3, g_overlayLoadAddress);
          GFX_Transition(0);
          rect.x = 0;
          rect.y = 0;
          rect.w = 320;
          rect.h = 464;
          ClearImage(&rect, 0, 0, 0);
          DrawSync(0);
          VSync(0);
          akao_cmd_f0();
          akao_cmd_f1();
          cdrom_wait_queue_empty();
          g_gameState = FUN_80060814();
          akao_cmd_f0();
          akao_cmd_f1();
          if (((g_gameState != 2) && (g_gameState != 9)) && (g_gameState != 10))
        {
          FUN_80011638(D_800351A0[D_80046FDE]);
        }
          D_80046FD8 = -1;
          g_previousGameState = 1;
          break;

        case 2:
        {
          cd_stop_ret = (long) func_80015C48();
          cdrom_stop();
          cdrom_stream(4, g_overlayLoadAddress);
          GFX_Transition(0);
          cdrom_wait_queue_empty();
          new_var2 = 2;
          g_gameState = func_8004FC74(cd_stop_ret);
          DrawSync(0);
          VSync(0);
          g_previousGameState = new_var2;
          break;
        }

        case 3:
          FUN_80015c28();
          cdrom_stream(CD_RES_FIELD_BIN, g_overlayLoadAddress);
          cdrom_stream(CD_RES_GNAME_BIN, 0x80140000);
          GFX_Transition(0);
          cdrom_wait_queue_empty();
          func_800A3534();
          func_80051FBC(0);
          D_8003EC98 = 0;
          ptrA = &g_gameDataBasePtr;
          temp_s2 = (MenuLayout *) (((u8 *) ptrA) - 0x5F0);
          g_gameState = run_overlay(0x80160000, (u32) ptrA, (u32) ptrA, ((*((u8 *) (&temp_s2->slot_flags))) & 0x7F) + 4, 0, (u32) ptrA, 1);
          DrawSync(0);
          VSync(0);
          g_previousGameState = 3;
          break;

        case 5:
          FUN_80015c28();
          cdrom_stream(14, g_overlayLoadAddress);
          GFX_Transition(0);
          cdrom_wait_queue_empty();
          g_gameState = func_8004FC8C(0x80170000);
          DrawSync(0);
          VSync(0);
          g_previousGameState = 5;
          break;

        case 7:
          FUN_80015c28();
          cdrom_stream((float) 2, g_overlayLoadAddress);
          cdrom_stream(16, 0x80140000);
          GFX_Transition(0);
          cdrom_wait_queue_empty();
          func_80051FBC(0);
          D_8003EC9C = 7;
          if (func_801400C4() != 0)
        {
          g_gameState = 2;
        }
        else
        {
          new_var = (u32) (new_var3 = ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk018);
          new_var = new_var & 0xFE000000U;
          new_var = new_var | 6;
          *((u32 *) (ptrB - 0x1378)) = 6;
          ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk018 = new_var;
          D_8003EC90 = ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk024;
          D_80042FCC = ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk026;
          D_80042FC4 = ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk027;
          D_8003EC94 = ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk01C;
          D_80046FD8 = ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk01E;
          ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk018 = new_var;
          {
            u32 tmp_u20 = ((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk020;
            D_80046FDE = (u16) tmp_u20;
          }
          if ((((MenuLayout *) (((u8 *) (&g_gameDataBasePtr)) - 0x5F0))->unk028 & 0xC) == 0xC)
          {
            g_gameState = 5;
          }
          else
          {
            GFX_Transition(0);
            g_gameState = FUN_80015c58();
          }
        }
          DrawSync(0);
          VSync(0);
          g_previousGameState = 0;
          break;

        case 8:
          func_80015C48();
          cdrom_stream(11, 0x80140000);
          GFX_Transition(0);
          cdrom_wait_queue_empty();
          movie_play(0);
          g_gameState = 2;
          DrawSync(0);
          VSync(0);
          g_previousGameState = 8;
          break;

      }

    }
    if (g_gameState == 4)
    {
      g_gameState = 2;
    }
  }

}