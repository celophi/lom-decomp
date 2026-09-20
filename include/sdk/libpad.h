#ifndef SDK_LIBPAD_H
#define SDK_LIBPAD_H

#define PadStateDiscon 0
#define PadStateFindPad 1
#define PadStateFindCTP1 2
#define PadStateReqInfo 4
#define PadStateExecCmd 5
#define PadStateStable 6

#define InfoModeCurExOffs 3
#define InfoModeIdTable 4

#define InfoActFunc 1
#define InfoActSize 3
#define InfoActCurr 4

#define PadModeUnlock 0

void PadInitDirect(unsigned char* port1_buffer, unsigned char* port2_buffer);
int PadGetState(int port);
int PadInfoMode(int port, int info_mode, int index);
int PadInfoAct(int port, int actuator, int property);
int PadSetActAlign(int port, unsigned char* alignment);
int PadSetMainMode(int port, int mode, int lock);
void PadSetAct(int port, unsigned char* actuator_data, int length);
int PadChkVsync(void);
void PadStopCom(void);

#endif
