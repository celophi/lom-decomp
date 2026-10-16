#ifndef CHECKPS_CDROM_H
#define CHECKPS_CDROM_H

#include "overlays/checkps/checkps.h"

#define CHECKPS_READ_RESPONSE_BYTE(destination) ((destination) = *g_cd_response_register)

extern volatile u8* g_cd_response_register;

void start_cd_integrity_check(void) __attribute__((section(".text.cdrom")));
s32 run_cd_integrity_check(s32 single_step) __attribute__((section(".text.cdrom")));

#endif
