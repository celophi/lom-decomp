#ifndef _COMMON_H
#define _COMMON_H

#include "include_asm.h"

typedef unsigned char	u_char;
typedef struct {
	u_char	val0;		/* volume for CD(L) -> SPU (L) */
	u_char	val1;		/* volume for CD(L) -> SPU (R) */
	u_char	val2;		/* volume for CD(R) -> SPU (L) */
	u_char	val3;		/* volume for CD(R) -> SPU (R) */
} CdlATV;	

void CD_SetAudioVolume(u_char volume, int stereoChannel);
int CdMix(CdlATV *vol);

#endif