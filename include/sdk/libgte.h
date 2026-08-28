#ifndef SDK_LIBGTE_H
#define SDK_LIBGTE_H

#define ONE 4096

typedef struct {
    short m[3][3];
    long t[3];
} MATRIX;

typedef struct {
    long vx;
    long vy;
    long vz;
    long pad;
} VECTOR;

typedef struct {
    short vx;
    short vy;
    short vz;
    short pad;
} SVECTOR;

typedef struct {
    short vx;
    short vy;
} DVECTOR;

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
