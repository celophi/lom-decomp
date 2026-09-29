#ifndef SDK_LIBGTE_H
#define SDK_LIBGTE_H

#include "ps1_types.h"

#define ONE 4096

typedef struct {
    short m[3][3];
    Ps1Long t[3];
} MATRIX;

typedef struct {
    Ps1Long vx;
    Ps1Long vy;
    Ps1Long vz;
    Ps1Long pad;
} VECTOR;

typedef struct {
    short vx;
    short vy;
    short vz;
    short pad;
} SVECTOR;

/** @brief Four-byte pointer to SVECTOR in PS1 storage. */
typedef SVECTOR* PS1_PTR32 SVECTORPtr;

typedef struct {
    short vx;
    short vy;
} DVECTOR;

/** @brief Four-byte pointer to DVECTOR in PS1 storage. */
typedef DVECTOR* PS1_PTR32 DVECTORPtr;

/** @brief Psy-Q color vector, including its primitive command byte. */
typedef struct {
    unsigned char r, g, b, cd;
} CVECTOR;

extern void InitGeom();
extern VECTOR *ApplyMatrixLV(MATRIX *m, VECTOR *v0, VECTOR *v1);
extern MATRIX *RotMatrix(SVECTOR *r, MATRIX *m);
extern MATRIX *RotMatrix_gte(SVECTOR *r, MATRIX *m);
extern MATRIX *RotMatrixX(long r, MATRIX *m);
extern MATRIX *RotMatrixY(long r, MATRIX *m);
extern MATRIX *RotMatrixZ(long r, MATRIX *m);
extern MATRIX *TransMatrix(MATRIX *m, VECTOR *v);
extern MATRIX *ScaleMatrix(MATRIX *m, VECTOR *v);
extern MATRIX *CompMatrix(MATRIX *m0, MATRIX *m1, MATRIX *m2);
extern void MatrixNormal(MATRIX *m, MATRIX *n);
extern void SetRotMatrix(MATRIX *m);
extern void SetTransMatrix(MATRIX *m);
extern void PushMatrix();
extern void PopMatrix();
extern void SetGeomOffset(long ofx, long ofy);
extern void SetGeomScreen(long h);
extern long NormalClip(long sxy0, long sxy1, long sxy2);
extern long VectorNormalS(VECTOR *v0, SVECTOR *v1);
extern long SquareRoot0(long value);
extern void InvSquareRoot(long value, long *mantissa, long *exponent);
extern int rcos(int angle);
extern int rsin(int angle);
extern int ccos(int angle);
extern int csin(int angle);
extern long ratan2(long y, long x);

#endif
