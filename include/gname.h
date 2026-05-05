#ifndef _GNAME_H
#define _GNAME_H

#include "common.h"

// Structure for the global data blocks D_8014F818 and D_8014F828
typedef struct {
    s32 unk0;
    s32 unk4;
    s32 unk8;
    s32 unkC;
} DataStruct;

// Structure for the argument object
typedef struct {
    s32 unk0;               // offset 0x00
    char pad[0x4040 - 4];   // padding up to offset 0x4040
    void* unk4040;          // offset 0x4040
} ArgStruct;

extern DataStruct D_8014F818;
extern DataStruct D_8014F828;

#endif