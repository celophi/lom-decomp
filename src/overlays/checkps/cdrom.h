#ifndef CHECKPS_CDROM_H
#define CHECKPS_CDROM_H

#include "checkps.h"

/** @brief Stored pointer to one of the CD controller's byte registers. */
typedef PS1_PTR(volatile u8) CheckPSCdRegisterPtr;

void start_cd_integrity_check(void) __attribute__((section(".text.cdrom")));
s32 run_cd_integrity_check(s32 single_step) __attribute__((section(".text.cdrom")));

#endif
