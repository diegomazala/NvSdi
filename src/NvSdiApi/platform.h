// platform (primarily: OS) abstraction

#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#define USE_WIN32

#ifdef USE_WIN32

#define NOMINMAX // don't define min and max macros

#include <windows.h>

// just in case...:
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#endif // USE_WIN32

#ifdef USE_WIN32

// atomic operations
inline int interlocked_increment(int volatile *val) { return (int)InterlockedIncrement((LONG volatile *)val); }
inline int interlocked_decrement(int volatile *val) { return (int)InterlockedDecrement((LONG volatile *)val); }

// memory allocation
inline void *malloc_aligned(int size, int alignment) { return _aligned_malloc(size, alignment); }
inline void free_aligned(void *ptr) { _aligned_free(ptr); }

// synchronization
typedef CRITICAL_SECTION Lock;

inline bool lock_create( Lock *lock) { InitializeCriticalSection(lock); return true; }
inline void lock_destroy(Lock *lock) { DeleteCriticalSection(lock); }
inline void lock_acquire(Lock *lock) { EnterCriticalSection(lock); }
inline void lock_release(Lock *lock) { LeaveCriticalSection(lock); }

typedef HANDLE Semaphore;

inline bool semaphore_create(Semaphore *sem, int init_count, int max_count) { *sem = CreateSemaphore(NULL, init_count, max_count, NULL); return *sem != NULL; }
inline void semaphore_destroy(Semaphore *sem) { CloseHandle(*sem); *sem = NULL; }
inline void semaphore_signal(Semaphore *sem) { ReleaseSemaphore(*sem, 1, NULL); }
inline DWORD semaphore_wait(Semaphore *sem) { DWORD ret = WaitForSingleObject(*sem, INFINITE); return ret;}
inline DWORD semaphore_check(Semaphore *sem) { DWORD ret = WaitForSingleObject(*sem, 0); return ret;}

typedef HANDLE Event;

inline bool event_create(Event *evt) { *evt = CreateEvent(NULL, FALSE, FALSE, NULL); return *evt != NULL; }
inline void event_destroy(Event *evt) { CloseHandle(*evt); *evt = NULL; }
inline void event_signal(Event *evt) { SetEvent(*evt); }
inline DWORD event_wait(Event *evt) { DWORD ret = WaitForSingleObject(*evt, INFINITE); return ret;}

// thread support
typedef HANDLE Thread;

DWORD WINAPI ThreadStartProc(LPVOID lpParameter);

struct thread_create_s
{
    void (*func)(void *);
    void *par;
};

inline bool thread_create(Thread *thread, void (*func)(void *), void *par)
{
    thread_create_s *p = new thread_create_s;
    if (p == NULL)
        return false;
    p->func = func;
    p->par = par;
    *thread = CreateThread(NULL, 0, ThreadStartProc, p, 0, NULL);
    return *thread != NULL;
}

inline void thread_destroy(Thread *thread) { CloseHandle(*thread); *thread = NULL; }
inline void thread_wait(Thread *thread) { WaitForSingleObject(*thread, INFINITE); }
inline void thread_priority_set(Thread *thread, int priority) { SetThreadPriority(*thread, priority); }
inline int thread_priority_get(Thread *thread) { return GetThreadPriority(*thread); }
#define H264_THREAD_PRIORITY_TIME_CRITICAL THREAD_PRIORITY_TIME_CRITICAL//THREAD_PRIORITY_NORMAL//THREAD_PRIORITY_TIME_CRITICAL

// from xcodeutil.h
#define H264_CPU_FEATURE_MMX     0x01    // MMX
#define H264_CPU_FEATURE_ISSE    0x02    // Integer SSE support (AMD Athlon)
#define H264_CPU_FEATURE_SSE     0x04    // Full SSE support (P3, AthlonXP)
#define H264_CPU_FEATURE_SSE2    0x08    // SSE2 (P4, Athlon64)
#define H264_CPU_FEATURE_SSE3    0x10    // SSE3
#define H264_CPU_FEATURE_SSSE3   0x20    // Supplemental SSE3
#define H264_CPU_FEATURE_SSE41   0x40    // SSE4.1
#define H264_CPU_FEATURE_SSE42   0x80    // SSE4.2

int cpu_features();

#pragma intrinsic(_BitScanForward)
#pragma intrinsic(_BitScanReverse)
inline unsigned int bitscan_forward_32(unsigned int val)
{
    DWORD ndx;
    _BitScanForward(&ndx, val);
    return ndx;
}
inline unsigned int bitscan_reverse_32(unsigned int val)
{
    DWORD ndx;
    _BitScanReverse(&ndx, val);
    return ndx;
}

#endif // USE_WIN32

#endif // __PLATFORM_H__
