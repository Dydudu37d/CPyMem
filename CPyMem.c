#include "CPyMem.h"
#include "FishType.h"
#include "methodobject.h"
#include "modsupport.h"
#include "object.h"
#include "pytypedefs.h"
#include <string.h>
#include <stdlib.h> 

#if defined(_WIN32) || defined(_WIN64)
    #define IS_WINDOWS
#elif defined(__linux__)
    #define IS_LINUX
#endif

#if defined(__x86_64__) || defined(_M_X64)
    #define ARCH_X86_64
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_ARM64
#elif defined(__arm__) || defined(_M_ARM)
    #define ARCH_ARM32
#endif

#if defined(IS_WINDOWS)
#include <windows.h>
#else
#include <sys/mman.h>
#endif

static inline u0 FishMemCopy(Ptr D, const Ptr S, Size CpSize) {
    #ifdef ARCH_X86_64
    __asm__ __volatile__ (
        ".intel_syntax noprefix\n\t"
        "rep movsb\n\t"
        ".att_syntax\n\t"
        :
        : "D"(D), "S"(S), "c"(CpSize)
        : "memory"
    );
    #else
    memcpy(D, S, CpSize);
    #endif
}

static inline u0 FishMemTrun(Ptr* From, Ptr* To) {
    Ptr TempFrom = *From;
    *From = *To;
    *To = TempFrom;
}

static inline Ptr FishMemAlloc(Size AllocSize){
    #if defined(IS_WINDOWS)
    return (Ptr)VirtualAlloc(NULL, AllocSize, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    #else
    return (Ptr)mmap(NULL, AllocSize, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    #endif
}

static inline u0 FishMemFree(CMemory* ptr){
    if (!ptr) return;
    #if defined(IS_WINDOWS)
    if (ptr->point) VirtualFree(ptr->point, 0, MEM_RELEASE);
    VirtualFree(ptr, 0, MEM_RELEASE);
    #elif defined(IS_LINUX)
    if (ptr->point) munmap(ptr->point, ptr->size);
    free(ptr);
    #endif
}

static PyObject* FishFree(PyObject* self, PyObject* args){
    PyObject* capsule;
    if (!PyArg_ParseTuple(args, "O", &capsule)) return NULL;

    if (!PyCapsule_IsValid(capsule, "CMemory")) {
        PyErr_SetString(PyExc_TypeError, "Is Not CMemory!Fuck You!");
        return NULL;
    }

    CMemory* Mem = (CMemory*)PyCapsule_GetPointer(capsule, "CMemory");
    if (Mem) {
        FishMemFree(Mem);
        PyCapsule_SetPointer(capsule, NULL);
        PyCapsule_SetDestructor(capsule, NULL);
    }
    Py_RETURN_NONE;
}

static u0 FishDestructor(PyObject* capsule) {
    CMemory* Mem = (CMemory*)PyCapsule_GetPointer(capsule, "CMemory");
    if (Mem) {
        FishMemFree(Mem);
    }
}

static PyObject* FishAlloc(PyObject* self, PyObject* args){
    unsigned long long input_size;
    if (!PyArg_ParseTuple(args, "K", &input_size)) return NULL;

    Ptr ptr = FishMemAlloc((Size)input_size);
    if (!ptr) return PyErr_NoMemory();

    CMemory* Mem = (CMemory*)malloc(sizeof(CMemory));
    if (!Mem) {
        #if defined(IS_WINDOWS)
        VirtualFree(ptr, 0, MEM_RELEASE);
        #else
        munmap(ptr, (Size)input_size);
        #endif
        return PyErr_NoMemory();
    }

    Mem->point = ptr;
    Mem->size = (Size)input_size;
    return PyCapsule_New(Mem, "CMemory", FishDestructor);
}

#include <frameobject.h> 

static PyObject* FishDictPingPong(PyObject* self, PyObject* args) {
    PyObject *obj1, *obj2;
    if (!PyArg_ParseTuple(args, "OO", &obj1, &obj2)) return NULL;

    #if PY_VERSION_HEX >= 0x030b0000
    PyFrameObject* frame = PyEval_GetFrame();
    if (!frame) {
        PyErr_SetString(PyExc_RuntimeError, "Can't get current Frame!");
        return NULL;
    }
    PyObject* locals = PyFrame_GetLocals(frame);
    #else
    PyFrameObject* frame = PyThreadState_Get()->frame;
    if (!frame) return NULL;
    PyObject* locals = frame->f_locals;
    Py_XINCRE افزایش(locals);
    #endif

    if (!locals || !PyDict_Check(locals)) {
        PyErr_SetString(PyExc_RuntimeError, "Locals is not a Dict!");
        return NULL;
    }

    PyObject *key, *value;
    Py_ssize_t pos = 0;
    PyObject *key1 = NULL, *key2 = NULL;

    while (PyDict_Next(locals, &pos, &key, &value)) {
        if (value == obj1 && !key1) {
            key1 = key;
            Py_INCREF(key1);
        } else if (value == obj2 && !key2) {
            key2 = key;
            Py_INCREF(key2);
        }
        if (key1 && key2) break;
    }

    if (key1 && key2) {
        PyDict_SetItem(locals, key1, obj2);
        PyDict_SetItem(locals, key2, obj1);
        
        #if PY_VERSION_HEX >= 0x030b0000
        #endif
    } else {
        PyErr_SetString(PyExc_ValueError, "Could not find variable names in locals! Are they constants?");
        Py_XDECREF(key1);
        Py_XDECREF(key2);
        return NULL;
    }

    Py_DECREF(key1);
    Py_DECREF(key2);
    
    #if PY_VERSION_HEX < 0x030b0000
    Py_DECREF(locals);
    #endif

    Py_RETURN_NONE;
}

static PyObject* FishCopy(PyObject* self, PyObject* args){
    PyObject* dest_capsule;
    PyObject* src_capsule;
    unsigned long long size;
    
    if (!PyArg_ParseTuple(args, "OOK", &dest_capsule, &src_capsule, &size)) return NULL;

    if (!PyCapsule_IsValid(dest_capsule, "CMemory") || !PyCapsule_IsValid(src_capsule, "CMemory")) {
        PyErr_SetString(PyExc_TypeError, "Arguments must be CMemory capsules!");
        return NULL;
    }

    CMemory* Dest = (CMemory*)PyCapsule_GetPointer(dest_capsule, "CMemory");
    CMemory* Src = (CMemory*)PyCapsule_GetPointer(src_capsule, "CMemory");

    if (Dest && Src) {
        FishMemCopy(Dest->point, Src->point, (Size)size);
    }
    Py_RETURN_NONE;
}

static inline u0 FishMemWrite(CMemory* Mem, Size offset, u8 Byte){
    ((u8*)Mem->point)[offset] = Byte;
}

static PyObject* FishWrite(PyObject* self, PyObject* args){
    PyObject* capsule;
    unsigned long long offset;
    unsigned char Byte;
    
    if (!PyArg_ParseTuple(args, "OKB", &capsule, &offset, &Byte)){
        return NULL;
    }

    if (!PyCapsule_IsValid(capsule, "CMemory")) {
        PyErr_SetString(PyExc_TypeError, "Is Not CMemory!");
        return NULL;
    }

    CMemory* Mem = (CMemory*)PyCapsule_GetPointer(capsule, "CMemory");
    if (Mem) {
        FishMemWrite(Mem, (Size)offset, (u8)Byte);
    }
    Py_RETURN_NONE;
}

static inline u8 FishMemRead(CMemory* Mem, Size offset){
    return ((u8*)Mem->point)[offset];
}

static PyObject* FishRead(PyObject* self, PyObject* args){
    PyObject* capsule;
    unsigned long long offset;
    
    if (!PyArg_ParseTuple(args, "OK", &capsule, &offset)){
        return NULL;
    }

    if (!PyCapsule_IsValid(capsule, "CMemory")) {
        PyErr_SetString(PyExc_TypeError, "Is Not CMemory!");
        return NULL;
    }

    CMemory* Mem = (CMemory*)PyCapsule_GetPointer(capsule, "CMemory");
    if (Mem) {
        return Py_BuildValue("B", FishMemRead(Mem, (Size)offset));
    }
    return NULL;
}

static PyMethodDef MyMethods[] = {
    {"PingPong", FishPingPong, METH_VARARGS, "A,B=B,A"         },
    {"Copy"    , FishCopy    , METH_VARARGS, "Copy Src to Dest"},
    {"Alloc"   , FishAlloc   , METH_VARARGS, "Alloc Memory"    },
    {"Free"    , FishFree    , METH_VARARGS, "Free Memory"     },
    {"Write"   , FishWrite   , METH_VARARGS, "Write Memory"    },
    {"Read"    , FishRead    , METH_VARARGS, "Read Memory"     },
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef cpymem = {
    PyModuleDef_HEAD_INIT,
    "cpymem",
    "A C Pointer & Memory Module",
    -1,
    MyMethods
};

PyMODINIT_FUNC PyInit_cpymem(void) {
    return PyModule_Create(&cpymem);
}

