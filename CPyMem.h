#include <Python.h>
#include <sys/mman.h>
#include "FishType.h"

typedef struct{
    Ptr point;
    Size size;
} CMemory;

static inline u0 FishMemCopy(Ptr D,const Ptr S,Size CpSize);
static inline u0 FishMemTrun(Ptr* From,Ptr* To);
static inline Ptr FishMemAlloc(Size AllocSize);
static inline u0 FishMemFree(CMemory* ptr);
