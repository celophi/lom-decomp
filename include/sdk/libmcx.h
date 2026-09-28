#ifndef SDK_LIBMCX_H
#define SDK_LIBMCX_H

/* McxSync() return values. */
#define McxSyncRun 0
#define McxSyncNone (-1)
#define McxSyncFin 1

/* McxSync() command results. */
#define McxErrSuccess 0
#define McxErrNoCard 1
#define McxErrInvalid 2
#define McxErrNewCard 3

/** @brief McxSync() mode that blocks until the pending command finishes (project-defined). */
#define MCX_SYNC_WAIT 0

/** @brief Return value of an Mcx command function that accepted the command (project-defined). */
#define MCX_COMMAND_ISSUED 1

void McxStartCom(void);
int McxSync(int mode, long* cmd, long* result);
int McxGetTime(int port, unsigned char* time);
int McxCardType(int port);

#endif
