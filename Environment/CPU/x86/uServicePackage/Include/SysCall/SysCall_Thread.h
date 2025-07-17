/*******************************************************************************
 *
 * @file SysCall_Thread.h
 *
 * @brief Microcontainer Thread Interface
 *
 * ZAYA Kernel provides thread pools for Microcontainers and Microcontainers
 * allocates a thread from the thread pool.
 *
 * ZAYA Kernel requests a memory area from the Microcontainer and the
 * Microcontainer needs to initialise a thread pool before allocating a thread.
 *
 * A Microcontainer calls the SYS_INITIALISE_THREAD_POOL() macro to
 * allocate a container and initialise the thread pool.
 *
 *
 ******************************************************************************
 *
 * Copyright (c) 2016-2025 ZAYA Tech Ltd - All rights reserved.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/

#ifndef __SYSCALL_THREAD_H
#define __SYSCALL_THREAD_H

/********************************* INCLUDES ***********************************/
#include "SysCall_Common.h"

#include "ztypes.h"

/***************************** MACRO DEFINITIONS ******************************/

/*
 * User Thread Size.
 *  - Core Functionality: [104 Bytes] : Core Thread Fields
 *  - Custom Fields     : [24  Bytes] : Optional Fields for Custom Schedulers
 *
 * IMPORTANT : NEVER CHANGE THIS VALUE
 */
#define SYS_USER_THREAD_CORE_SIZE                               (120)
#define SYS_USER_THREAD_CUSTOM_SIZE                             (8)
#define SYS_USER_THREAD_SIZE                                    (SYS_USER_THREAD_CORE_SIZE + SYS_USER_THREAD_CUSTOM_SIZE)

/* Minimum User Thread Priority */
#define SYS_USER_THREAD_MIN_PRIORITY                            (0)
/* Maximum priority for general User Threads/Containers */
#define SYS_USER_THREAD_MAX_PRIORITY                            (15)
/* Maximum priority for sensitive User Threads/Containers */
#define SYS_USER_THREAD_SENSITIVE_MAX_PRIORITY                  (31)

/* Macro to calculate required container size for thread pool */
#define SYS_USER_THREADPOOL_CONTAINER_SIZE(_maxThreadCount)     ((_maxThreadCount) * SYS_USER_THREAD_SIZE)

/**
 * Initialise Thread Pool
 * It can be called in a function (like Microcontainer init) which is
 * recommended to reduce the scope. In this way, you can avoid mistaken accesses.
 *
 * @param[out] retVal Return value after thread pool initialisation.
 *                    Microcontainer needs to check it before thread usage.
 *
 *                  - SysStatus_Success Success Thread Pool Initialised
 *                  - SysStatus_AlreadyInitialised ThreadPool already initialised before.
 *                  - SysStatus_InvalidContainerSize Invalid container size.
 *                  - SysStatus_InvalidOffsetAlignment Container offset is not word aligned.
 *
 * @param _maxThreadCount Thread Pool Capacity.
 */
#define SYS_INITIALISE_THREAD_POOL(_retVal, _maxThreadCount) \
{ \
    static volatile uint32_t threadPoolContainer[SYS_USER_THREADPOOL_CONTAINER_SIZE(_maxThreadCount)/sizeof(uint32_t)]; \
    _retVal = Sys_InitialiseThreadPool((volatile uint8_t*)threadPoolContainer, SYS_USER_THREADPOOL_CONTAINER_SIZE(_maxThreadCount)); \
}

/*
 * Size of a Thread Synchronization Primitive Size.
 * Kernel needs 4 bytes per sync primitive.
 *
 * IMPORTANT : NEVER CHANGE THIS VALUE
 */
#define SYS_THREAD_SYNC_PRIMITIVE_SIZE                          (4)

/* Macro to calculate required container size for thread sync pool */
#define SYS_USER_THREADSYNCPOOL_CONTAINER_SIZE(_maxThreadSyncCount) ((_maxThreadSyncCount) * SYS_THREAD_SYNC_PRIMITIVE_SIZE)

/**
 * Initialises Thread Sync Pool
 * It can be called in a function (like Microcontainer init) which is
 * recommended to reduce the scope. In this way, you can avoid mistaken accesses.
 *
 * @param[out] retVal Return value after thread sync pool initialisation.
 *                    Microcontainer needs to check it before thread usage.
 *
 *                  - SysStatus_Success Success Thread Pool Initialised
 *                  - SysStatus_AlreadyInitialised ThreadPool already initialised before.
 *                  - SysStatus_InvalidContainerSize Invalid container size.
 *                  - SysStatus_InvalidOffsetAlignment Container offset is not word aligned.
 *
 * @param _maxThreadSyncCount Thread Sync Pool Capacity.
 */
#define SYS_INITIALISE_THREADSYNC_POOL(_retVal, _maxThreadSyncCount) \
{ \
    static volatile uint32_t threadSyncPoolContainer[SYS_USER_THREADSYNCPOOL_CONTAINER_SIZE(_maxThreadSyncCount)/sizeof(uint32_t)]; \
    _retVal = Sys_InitialiseThreadSyncPool((volatile uint8_t*)threadSyncPoolContainer, SYS_USER_THREADSYNCPOOL_CONTAINER_SIZE(_maxThreadSyncCount)); \
}

/***************************** TYPE DEFINITIONS *******************************/

/*
 * Thread Start Point Type
 */
typedef void (*Sys_ThreadStartPoint)(void* args);

typedef enum
{
    /* Not a valid thread; Not allocated in Thread Pool */
    SysThreadState_None,
    /* Thread is waiting for an event */
    SysThreadState_Waiting,
    /* Thread is ready to run */
    SysThreadState_Ready,
    /* Thread is suspended */
    SysThreadState_Suspended,
    /* Thread is already running */
    SysThreadState_Running
} Sys_ThreadState;

/*************************** FUNCTION DEFINITIONS *****************************/

/*
 * Set the priority for the main thread
 *
 * @param priority Main Thread Priority
 *
 * @return SysStatus_Success Main Thread Priority Set Successfully
 * @return SysStatus_Thread_InvalidPriority Higher priority than allowed.
 *         By default, priority can be up to SYS_USER_THREAD_MAX_PRIORITY
 *         However, time or security sensitive containers can have higher
 *         priority up to SYS_USER_THREAD_SENSITIVE_MAX_PRIORITY
 */
SysStatus Sys_SetMainThreadPriority(uint32_t priority);

/**
 * Initialises Thread Pool.
 *
 * If a Microcontainer needs threads, it needs to provide a memory space.
 *
 * IMP : Please use SYS_INITIALISE_THREAD_POOL() macro instead of calling this
 * function directly.
 *
 * @param threadPoolContainer Thread Pool Container (Memory Area) for user threads.
 * @param containerSize Thread Pool Container Size
 *
 * @retVal SysStatus_Success Success Thread Pool Initialised
 * @retVal SysStatus_AlreadyInitialised ThreadPool already initialised before.
 * @retVal SysStatus_InvalidContainerSize Invalid container size.
 *         Zero Size or Not aligned with SYS_USER_THREAD_SIZE or higher than allowed maximum thread number.
 * @retVal SysStatus_InvalidOffsetAlignment Container offset is not word aligned.
 */
SysStatus Sys_InitialiseThreadPool(volatile uint8_t* threadPoolContainer, uint32_t containerSize);

/**
 * Get a thread from the thread pool.
 * The default thread state is suspend when a thread obtained from the pool; Use
 * Sys_ResumeThread() API to start a thread.
 *
 * @param startPoint Thread Start Point (Code offset)
 * @param stack Thread Stack Address Offset (Not top of stack)
 * @param stackSize Thread Stack Size
 * @param priority Thread Priority. Higher values represents higher priority.
 *                 Up to SYS_USER_THREAD_MAX_PRIORITY, by default.
 *                 Up to SYS_USER_THREAD_SENSITIVE_MAX_PRIORITY for
 *                 the Containers marked as "Time Sensitive Threads".
 *                 See the "Time-Sensitive Container" flag in Container Metadata
 * @param args Thread Arguments
 * @param[out] threadID The Thread ID
 *
 * @retVal SysStatus_Success Success Success
 * @retVal SysStatus_NotInitialised ThreadPool not initialised yet.
 * @retVal SysStatus_NoSlotAvailable No available thread in the thread pool.
 * @retVal SysStatus_Thread_InvalidStackSize Stack Size is less then minimum size allowed.
 * @retVal SysStatus_Thread_InvalidPriority Higher priority than allowed.
 *         By default, priority can be up to SYS_USER_THREAD_MAX_PRIORITY
 *         However, time or security sensitive containers can have higher
 *         priority up to SYS_USER_THREAD_SENSITIVE_MAX_PRIORITY
 */
SysStatus Sys_GetThread(Sys_ThreadStartPoint startPoint, uint8_t* stack, uint32_t stackSize, uint32_t priority, void* args, uint32_t* threadID);

/**
 * NonSecure version of Sys_GetThread() function.
 *
 * It is not recommended to use this function unless porting some guest RTOSes which does not
 * pass stack size.
 *
 * @param startPoint Thread Start Point (Code offset)
 * @param topOfStack Thread Stack Address Offset (Not top of stack)
 * @param priority Thread Priority
 * @param args Thread Arguments
 * @param[out] threadID The Thread ID
 *
 * @retVal SysStatus_Success Success Success
 * @retVal SysStatus_NotInitialised ThreadPool not initialised yet.
 * @retVal SysStatus_NoSlotAvailable No available thread in the thread pool.
 */
SysStatus Sys_GetThreadNonSecure(Sys_ThreadStartPoint startPoint, uint8_t* topOfStack, uint32_t priority, void* args, uint32_t* threadID);

/**
 * Release a thread in the thread pool.
 * After the release, the thread wll be available for the next time
 * (Sys_GetThread)
 *
 * @param threadID to be released Thread
 *
 * @retval SysStatus_Success Success Success
 * @retval SysStatus_Thread_Invalid
 *         - A Microcontainer cannot release other Microcontainer's thread.
 *         - Main thread cannot be released
 *         - Invalid Thread ID
 * @retval SysStatus_NotInitialised Thread pool not initialised yet.
 * @retval SysStatus_Thread_InvalidPriority A thread cannot release higher priority thread.
 *
 */
SysStatus Sys_FreeThread(uint32_t threadID);

/**
 * Aborts the current thread itself.
 * After the abort, the thread will be available for the next time
 * (Sys_GetThread)
 *
 * @param threadID to be released Thread
 *
 * @retval SysStatus_Success Success Success
 * @retval SysStatus_NotInitialised Thread pool not initialised yet.
 *
 */
SysStatus Sys_AbortThread(void);

/**
 * Suspends a thread.
 *
 * @param threadID to be suspended Thread
 *
 * @retval SysStatus_Success Success Success
 * @retval SysStatus_Thread_Invalid
 *         - A Microcontainer cannot suspend other Microcontainer's thread.
 *         - Main thread cannot be suspended
 *         - Invalid Thread ID
 * @retval SysStatus_NotInitialised Thread pool not initialised yet.
 * @retval SysStatus_Thread_InvalidPriority A thread cannot suspend higher priority thread.
 * @retval SysStatus_Thread_AlreadySuspended Thread is already suspended before.
 *
 */
SysStatus Sys_SuspendThread(uint32_t threadID);

/**
 * Resume a thread.
 *
 * @param threadID to be resume Thread
 *
 * @retval SysStatus_Success Success Success
 * @retval SysStatus_Thread_Invalid
 *         - A Microcontainer cannot resume other Microcontainer's thread.
 *         - Main thread cannot be resumed
 *         - Invalid Thread ID
 * @retval SysStatus_NotInitialised Thread pool not initialised yet.
 * @retval SysStatus_Thread_InvalidPriority A thread cannot resume higher priority thread.
 * @retval SysStatus_Thread_AlreadySuspended Thread is already running.
 *
 */
SysStatus Sys_ResumeThread(uint32_t threadID);

/**
 * Gets the running thread id
 *
 * @param[out] threadID current thread id
 *
 * @retval SysStatus_Success Success Success
 *
 */
SysStatus Sys_GetThreadID(uint32_t* threadID);

/**
 * Gets The Thread State
 *
 * @param threadID Thread ID to get the Thread State
 * @param[out] threadState Thread State
 *
 * @retval SysStatus_Success Success Success
 * @retval SysStatus_Thread_Invalid
 *         - A Microcontainer cannot ask for thread state of any other Microcontainers.
 *         - Invalid Thread ID
 * @retval SysStatus_NotInitialised Thread pool not initialised yet.
 *
 */
SysStatus Sys_GetThreadState(uint32_t threadID, Sys_ThreadState* threadState);

/**
 * Initialises Thread Synchronisation (Mutex/Semaphore) Pool.
 *
 * If a Microcontainer needs to use thread sync primitives, it needs to provide
 * a memory space.
 *
 * IMP : Please use SYS_INITIALISE_THREADSYNC_POOL() macro instead of calling this
 * function directly.
 *
 * @param threadSyncPoolContainer Thread Sync Pool Container.
 * @param containerSize Thread Sync Pool Container Size
 *
 * @retVal SysStatus_Success Success Thread Sync Pool Initialised
 * @retVal SysStatus_AlreadyInitialised Thread Sync Pool already initialised before.
 * @retVal SysStatus_InvalidContainerSize Invalid container size.
 * @retVal SysStatus_InvalidOffsetAlignment Container offset is not word aligned.
 */
SysStatus Sys_InitialiseThreadSyncPool(volatile uint8_t* threadSyncPoolContainer, uint32_t containerSize);

/**
 * Get a Semaphore from the thread sync pool.
 *
 * @param maximumCount The maximum number of requests for the semaphore.
 * @param[out] semaphoreID The Semaphore ID
 *
 * @retVal SysStatus_Success Success Success
 * @retVal SysStatus_NotInitialised ThreadPool not initialised yet.
 * @retVal SysStatus_NoSlotAvailable No available semaphore.
 */
SysStatus Sys_GetSemaphore(uint32_t maximumCount, uint32_t* semaphoreID);

/**
 * Wait a semaphore.
 *
 * @param semaphoreID Waiting Semaphore
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_ThreadSync_Invalid Invalid Semaphore ID
 * @retval SysStatus_NotInitialised Thread Sync pool not initialised yet.
 *
 */
SysStatus Sys_WaitSemaphore(uint32_t semaphoreID);

/**
 * Try to wait for a semaphore; if the semaphore is not available,
 * this function returns immediately.
 *
 * @param semaphoreID Waiting Semaphore
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_WaitNotPerformed Semaphore is not available
 * @retval SysStatus_ThreadSync_Invalid Invalid Semaphore ID
 * @retval SysStatus_NotInitialised Thread Sync pool not initialised yet.
 *
 */
SysStatus Sys_TryWaitSemaphore(uint32_t semaphoreID);

/**
 * Release a semaphore.
 *
 * @param semaphoreID to be released Semaphore
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_ThreadSync_Invalid Invalid Semaphore ID
 * @retval SysStatus_NotInitialised Thread Sync pool not initialised yet.
 *
 */
SysStatus Sys_ReleaseSemaphore(uint32_t semaphoreID);

/**
 * Get a Mutex from the thread sync pool.
 *
 * @param[out] mutexID The Mutex ID
 *
 * @retVal SysStatus_Success Success Success
 * @retVal SysStatus_NotInitialised ExecutionThreadSyncPool not initialised yet.
 * @retVal SysStatus_NoSlotAvailable No available mutex.
 */
SysStatus Sys_GetMutex(uint32_t* mutexID);

/**
 * Wait a mutex.
 *
 * @param mutexID Waiting Mutex
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_ThreadSync_Invalid Invalid Mutex ID
 * @retval SysStatus_NotInitialised Thread Sync pool not initialised yet.
 *
 */
SysStatus Sys_WaitMutex(uint32_t mutexID);

/**
 * Try to wait for a mutex; if the mutex is not available,
 * this function returns immediately.
 *
 * @param mutexID Waiting Mutex
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_WaitNotPerformed Mutex is not available
 * @retval SysStatus_ThreadSync_Invalid Invalid Mutex ID
 * @retval SysStatus_NotInitialised Thread Sync pool not initialised yet.
 *
 */
SysStatus Sys_TryWaitMutex(uint32_t mutexID);

/**
 * Release a Mutex.
 *
 * @param mutexID to be released Mutex
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_ThreadSync_Invalid Invalid Mutex ID
 * @retval SysStatus_NotInitialised Thread Sync pool not initialised yet.
 *
 */
SysStatus Sys_ReleaseMutex(uint32_t mutexID);

#endif    /* __SYSCALL_THREAD_H */
