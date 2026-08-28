#ifndef LOM_PSYQ_COMPAT_STRINGS_H
#define LOM_PSYQ_COMPAT_STRINGS_H

#ifndef NULL
#define NULL 0
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#include "memory.h"

extern char *strcat(char *dst, char *src);
extern int strcmp();
extern int strncmp(char *lhs, char *rhs, int count);
extern char *strcpy();
extern char *strncpy(char *dst, char *src, int count);
extern int strlen();
extern char *index(char *s, char value);

#endif
