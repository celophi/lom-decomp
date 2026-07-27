#ifndef _LIBPAD_H_
#define _LIBPAD_H_

/*
 * Controller communication states returned by PadGetState().
 */
#define PadStateDiscon  0
#define PadStateFindPad 1
#define PadStateFindCTP1 2
#define PadStateFindCTP2 3
#define PadStateReqInfo 4
#define PadStateExecCmd 5
#define PadStateStable  6
#define PadStateError   7

/*
 * PadInfoMode() query selectors.
 */
#define InfoModeCurID     1
#define InfoModeCurExID   2
#define InfoModeCurExOffs 3
#define InfoModeIdTable   4

/*
 * PadInfoAct() property selectors.
 */
#define InfoActFunc 1
#define InfoActSub  2
#define InfoActSize 3
#define InfoActCurr 4
#define InfoActSign 5

/*
 * PadSetMainMode() lock values.
 */
#define PadModeUnlock 0
#define PadModeLock   3

#endif /* _LIBPAD_H_ */
