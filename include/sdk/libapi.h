#ifndef SDK_LIBAPI_H
#define SDK_LIBAPI_H

#include "kernel.h"

extern long SetRCnt(unsigned long counter, unsigned short target, long mode);
extern long GetRCnt(unsigned long counter);
extern long ResetRCnt(unsigned long counter);
extern long StartRCnt(unsigned long counter);
extern long StopRCnt(unsigned long counter);
extern long OpenEvent(unsigned long desc, long spec, long mode, long (*func)());
extern long CloseEvent(long event);
extern long WaitEvent(long event);
extern long TestEvent(long event);
extern long EnableEvent(long event);
extern long DisableEvent(long event);
extern void DeliverEvent(unsigned long desc, unsigned long spec);
extern void UnDeliverEvent(unsigned long desc, unsigned long spec);
extern long open(char *path, unsigned long mode);
extern long close(long fd);
extern long read(long fd, void *buf, long size);
extern long write(long fd, void *buf, long size);
extern struct DIRENTRY *firstfile(char *path, struct DIRENTRY *entry);
extern struct DIRENTRY *nextfile(struct DIRENTRY *entry);
extern long erase(char *path);
extern long format(char *path);
extern long rename(char *old_path, char *new_path);
extern long Load(char *path, struct EXEC *exec);
extern long InitPAD(char *buf0, long len0, char *buf1, long len1);
extern long StartPAD(void);
extern void StopPAD(void);
extern void EnablePAD(void);
extern void DisablePAD(void);
extern void FlushCache(void);
extern void ReturnFromException(void);
extern int EnterCriticalSection(void);
extern void ExitCriticalSection(void);
extern long SetConf(unsigned long event, unsigned long stack, unsigned long mode);
extern void SetMem(long size);
extern long Krom2RawAdd(unsigned long sjis);
extern void _96_remove(void);
extern void ChangeClearPAD(long mode);
extern void InitCARD(long mode);
extern long StartCARD(void);
extern long StopCARD(void);
extern void _bu_init(void);
extern long _card_info(long chan);
extern long _card_clear(long chan);
extern long _card_load(long chan);
extern void _new_card(void);
extern long _card_status(long drive);
extern long _card_wait(long drive);
extern long _card_write(long chan, long block, unsigned char *buf);
extern long _card_read(long chan, long block, unsigned char *buf);
extern long _card_format(long chan);

#endif
