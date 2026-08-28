#ifndef SDK_RAND_H
#define SDK_RAND_H

/* Legend of Mana's compiler-visible libc random-number interface. */
#define RAND_MAX 32767

extern int rand(void);
extern void srand(unsigned int seed);

#endif
