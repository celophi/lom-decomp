#ifndef SDK_LIBCD_H
#define SDK_LIBCD_H

#define CdlModeSpeed      0x80
#define CdlModeRT         0x40
#define CdlModeSize1      0x20

#define CdlStatShellOpen  0x10
#define CdlStatStandby    0x02
#define CdlStatError      0x01
#define CdlStatNoDisk     0
#define CdlOtherFormat    1
#define CdlCdromFormat    2

#define CdlNop            0x01
#define CdlSetloc         0x02
#define CdlPlay           0x03
#define CdlForward        0x04
#define CdlBackward       0x05
#define CdlReadN          0x06
#define CdlStandby        0x07
#define CdlStop           0x08
#define CdlPause          0x09
#define CdlMute           0x0b
#define CdlDemute         0x0c
#define CdlSetfilter      0x0d
#define CdlSetmode        0x0e
#define CdlGetparam       0x0f
#define CdlGetlocL        0x10
#define CdlGetlocP        0x11
#define CdlGetTN          0x13
#define CdlGetTD          0x14
#define CdlSeekL          0x15
#define CdlSeekP          0x16
#define CdlReadS          0x1b

#define CdlDataReady      0x01
#define CdlComplete       0x02
#define CdlDiskError      0x05

typedef void (*CdlCB)(u_char status, u_char *result);

typedef struct {
    u_char minute;
    u_char second;
    u_char sector;
    u_char track;
} CdlLOC;

typedef struct {
    u_char val0;
    u_char val1;
    u_char val2;
    u_char val3;
} CdlATV;

void CdFlush(void);
CdlLOC *CdIntToPos(int sector, CdlLOC *pos);
char *CdComstr(u_char command);
char *CdIntstr(u_char interrupt);
int CdControl(u_char command, u_char *param, u_char *result);
int CdControlB(u_char command, u_char *param, u_char *result);
int CdControlF(u_char command, u_char *param);
int CdGetSector(void *dst, int words);
int CdGetSector2(void *dst, int words);
int CdDataSync(int mode);
int CdMix(CdlATV *volume);
int CdPosToInt(CdlLOC *pos);
int CdReady(int mode, u_char *result);
int CdSetDebug(int level);
int CdSync(int mode, u_char *result);
CdlCB CdReadyCallback(CdlCB func);
CdlCB CdSyncCallback(CdlCB func);
int CdInit(void);
int CdReset(int mode);
int CdStatus(void);
int CdLastCom(void);
CdlLOC *CdLastPos(void);
int CdMode(void);
int CdDiskReady(int mode);
int CdGetDiskType(void);

#endif
