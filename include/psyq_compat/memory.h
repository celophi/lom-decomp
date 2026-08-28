#ifndef LOM_PSYQ_COMPAT_MEMORY_H
#define LOM_PSYQ_COMPAT_MEMORY_H

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#ifndef NULL
#define NULL 0
#endif

/* Keep the historically unprototyped forms where LoM compiled against them. */
extern void *memcpy();
extern void *memchr(const unsigned char *src, unsigned char value, int count);
extern void *memset();
extern void *bcopy(const unsigned char *src, unsigned char *dst, int count);
extern void *bzero(unsigned char *dst, int count);

#endif
