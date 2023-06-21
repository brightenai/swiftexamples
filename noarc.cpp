//
//  noarc.cpp
//

#define NOARC 1

#include <stdlib.h>
#include <stdio.h>
#include <list>
#include <thread>
#include <unordered_set>
#include <unordered_map>
#include <atomic>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#define _Bool bool

extern "C"
{
extern int markedNoArc(void *object);

void brighten_StartNoArc();
extern int activeNoArc(const char* classname, size_t len);
void brighten_MarkNoArc(void *object);

int nextWorkID();
void begin_DebuggingARC();
void end_DebuggingARC();

typedef void HeapObject;
typedef void HeapMetadata;

extern void *(*_swift_retain)(void *);
extern void *(*_swift_release)(void *);
extern void *(*_swift_retain_n)(void *, uint32_t);

extern HeapObject *(*_swift_allocObject)(HeapMetadata const *metadata,
                                             size_t requiredSize,
                                             size_t requiredAlignmentMask);

static bool sNoArcActive = false;
static void *(*_old_swift_retain)(void*);
static void *(*_old_swift_release)(void*);
static void *(*_old_swift_allocObject)(void const*,size_t,size_t);

size_t swift_retainCount(void *);
const char *swift_getTypeName(void *classObject, _Bool qualified);

void addClassNameToNoArc( const char* name);

}

typedef uintptr_t __swift_uintptr_t;
    
typedef struct {
  __swift_uintptr_t refCounts;
} InlineRefCountsPlaceholder;

typedef InlineRefCountsPlaceholder InlineRefCounts;

#define SWIFT_HEAPOBJECT_NON_OBJC_MEMBERS       \
  InlineRefCounts refCounts

#ifndef __ptrauth_objc_isa_pointer
   #define __ptrauth_objc_isa_pointer
    #endif

static std::unordered_map<std::string,std::string>* valuesDict = new std::unordered_map<std::string,std::string>();

typedef struct HeapClass HeapClass;

struct objc_protocol_list
{
    void **List;
    unsigned NumElts;
};


struct objc_ivar_list
{
    void **List;
    unsigned NumElts;
};


struct objc_methodlist
{
    void **List;
    unsigned NumElts;
};


struct objc_cache
{
    void **List;
    unsigned NumElts;
};

struct HeapClass
{
    HeapClass* isa;
    HeapClass* super;
    const char* name;
    long version;
    long info;
    long instancesize;
    objc_ivar_list* ivars;
    objc_methodlist* methods;
    objc_cache* cache;
    objc_cache* protoColL;
    uint32_t flags;
    uint32_t instanceAddressPoint;
    uint32_t instanceSize;
    uint16_t alignMaskAndBits;
    uint16_t reserved;
    uint32_t classSize;
    uint32_t classAddressPoint;
    void* description;
};

typedef struct
{
    HeapClass* isa;
    SWIFT_HEAPOBJECT_NON_OBJC_MEMBERS;
    uint32_t mightBeBeef;
    uint32_t storedPropsB;
    uint32_t storedPropsC;
    uint32_t e;
//    uint32_t f;

} HeapObjectX;


typedef struct
{
    uint32_t isa;
//    uint32_t a;
//    uint32_t b;
//    uint32_t c;
//    uint32_t d;
//    uint32_t e;
//    uint32_t f;

} HeapObjectXSimple;


static std::string* noArc = new std::string("NoArc");

static bool debugging = false;

static std::thread::id* debuggingThread = NULL;
static std::thread::id debuggingThreadX = std::this_thread::get_id();
static int releasesForDebugging = 0;

static std::atomic<int> id;

int nextWorkID()
{
    return id++;
}

void begin_DebuggingARC()
{
#if DEBUGTHREADS
    debuggingThreadX = std::this_thread::get_id();
    debuggingThread = &debuggingThreadX;
    debugging = true;
    releasesForDebugging = 0;
#endif
}

void end_DebuggingARC()
{
#if DEBUGTHREADS
    debuggingThread = NULL;

    debugging = false;

    if (releasesForDebugging > 0)
    {
        fprintf(stderr, "end_DebuggingARC, cheap releases %d\n", (int) releasesForDebugging);
    }
#endif
}

static void report()
{
    fprintf(stderr, "");
}

static bool isNoArcObject(void *object)
{
#ifdef NOARCIMP
#else
#warning("no arc not working on ubuntu")
    return false;
#endif
    
    if (object)
    {
        HeapObjectX* objectX = (HeapObjectX*) object;
        if (objectX->mightBeBeef == 0xDEADBEEF)
        {
            return true;
        }
        
            return false;
    }
    else
    {
        return true;    // return true for NULL so we dont call built in
    }
    
    return false;
}

static void *swift_retain_hook(void *object) {
    
        if (isNoArcObject(object))
        {
            return object;
        }

#if DEBUGTHREADS
    if (object)
    {
        std::thread::id* dt = debuggingThread;
        if (debugging) //(dt != NULL && *dt == std::this_thread::get_id())
        {
            void *isa = *(void**)object;
            HeapObjectX* guessing = (HeapObjectX*) isa;
            
            const char *className = swift_getTypeName(isa, 0);
            fprintf(stderr, "full retain for %s %x \n", className,guessing->isa);
            report();
            
        }
    }
#endif
    return _old_swift_retain(object);
}

static void *swift_release_hook(void *object) {
        if (isNoArcObject(object))
        {
            return object;
        }

#if DEBUGTHREADS
    if (object)
    {
          std::thread::id* dt = debuggingThread;
        if (debugging) //(dt != NULL && *dt == std::this_thread::get_id())
        {
        void *isa = *(void**)object;
            HeapObjectX* guessing = (HeapObjectX*) isa;
            
        const char *className = swift_getTypeName(isa, 0);
        fprintf(stderr, "full release for %s %x \n", className,guessing->isa);
        report();
            
        }
#endif
    return _old_swift_release(object);
}

// this is like static init for dylibs..
//__attribute__((constructor))
void brighten_StartNoArc() {
#if NOARC
    if (!sNoArcActive)
    {
        sNoArcActive = true;
        _old_swift_retain = _swift_retain;
        _swift_retain = swift_retain_hook;
        
        _old_swift_release = _swift_release;
        _swift_release = swift_release_hook;
    }
#endif
}

