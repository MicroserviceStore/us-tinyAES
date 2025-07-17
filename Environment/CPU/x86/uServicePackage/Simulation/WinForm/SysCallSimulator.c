/*
 * @file uS_tinyAES.h
 *
 * @brief Microservice/Container Syscall Simulator for Windows
 *
 ******************************************************************************/

 /********************************* INCLUDES ***********************************/

#include "SysCall.h"
#include "uService.h"

#include <Windows.h>
#include <time.h>

#include "Simulation.h"

/***************************** MACRO DEFINITIONS ******************************/

/* In Windows Simulation, it may use more stack than an embedded, so just in case, multiply the stack */
#define STACK_SIM_FACTOR                            16
#define KERNEL_MINIMUM_STACK_SIZE                   256

#define KERNEL_SYSEVENT_MAXEVENT                    8

#define KERNEL_MAX_THREAD_COUNT_PER_EXECUTION       32
#define KERNEL_MAX_MESSAGE_LENGTH                   256

#define KERNEL_IPC_MESSAGEBOX_MAX_CAPACITY          256

#define SIM_MAX_CONTAINER_COUNT                     16

#define SYS_MICROSEVICE_MAX_HANDLE                  64

#define GET_THREAD_INDEX(__threadID)                (((uint32_t)(__threadID))& 0xFF)

#define KERNEL_EVENT_MASK(_eventNo)         (((uint64_t)1) << (_eventNo))
#define KERNEL_IS_WAITED_EVENT_SET(_thread, _eventNo) \
            (((_thread)->waitedEvents & KERNEL_EVENT_MASK(_eventNo)) != 0)

#define GET_THREAD_ID(__execIndex, __threadIndex)   ((((uint32_t)(__execIndex))<<8) | (__threadIndex))

#define KERNEL_SET_WAITED_EVENT_FLAG(_thread, _eventNo) \
            ((_thread)->waitedEvents |= KERNEL_EVENT_MASK(_eventNo))

#define PIPE_NAME_FORMAT                            "\\\\.\\pipe\\IPC%d"

/***************************** TYPE DEFINITIONS *******************************/

typedef enum
{
    ThreadState_None,
    ThreadState_Ready,
    ThreadState_Waiting,
    ThreadState_Suspended
} ThreadState;

typedef struct
{
    struct
    {
        uint32_t inUse;
        uint32_t isMainThread;
    } flags;

    uint32_t id;
    ThreadState state;
    uint8_t* userStack;
    uint32_t initialPriority;

    HANDLE winHandle;
    DWORD winThreadId;
    HANDLE ipcEvent;
    HANDLE timerEvent;

    uint64_t waitedEvents;
} SysThreadCore;

typedef struct
{
    uint32_t inUse;
} SysThreadSyncCore;

typedef struct
{
    uint32_t dummy;
} SysIPCMBCore;

typedef struct
{
    SysThreadCore core;
    //uint8_t __reserved[SYS_USER_THREAD_SIZE - sizeof(SysThreadCore)];
} SysThread;

typedef struct
{
    SysThreadSyncCore core;
    //uint8_t __reserved[SYS_THREAD_SYNC_PRIMITIVE_SIZE - sizeof(SysThreadSyncCore)];
} SysThreadSync;

typedef struct
{
    SysIPCMBCore core;

    //uint8_t __reserved[SYS_IPC_MESSAGE_SIZE - sizeof(SysIPCMBCore)];
} SysIPCMB;

typedef struct
{
    uint32_t length;
    uint32_t senderID;
} IPCPackageHeader;

typedef struct
{
    IPCPackageHeader header;
    uint8_t payload[1];
} IPCPackage;

typedef struct
{
    #define PIPE_BUFFER_SIZE            KERNEL_MAX_MESSAGE_LENGTH
    char readBuffer[PIPE_BUFFER_SIZE];
    uint32_t receivedLen;
    uint32_t readOffset;
} IPCBuffer;

typedef struct
{
    struct
    {
        uint32_t inCS : 1;
        uint32_t threadPoolInitialised : 1;
        uint32_t threadSyncPoolInitialised : 1;
        uint32_t messageBoxInitialised : 1;
        uint32_t ucInitialised : 1;
    } flags;

    CRITICAL_SECTION cs;
    HANDLE ipcListenerPipe;
    HANDLE ipcClientPipe[SIM_MAX_CONTAINER_COUNT];
    uint32_t ipcSequenceNo;

    uint64_t refTime;

    SysThread mainThread;

    struct
    {
        SysThread* pool;
        uint32_t capacity;
        uint32_t count;
    } threadPool;

    struct
    {
        SysThreadSync* pool;
        uint32_t capacity;
        uint32_t count;

        HANDLE syncHandles[KERNEL_MAX_THREAD_COUNT_PER_EXECUTION];
    } threadSyncPool;

    struct
    {
        SysIPCMB* pool;
        uint32_t capacity;
        uint32_t count;
    } messageBox;

    Sys_SimulationDetails simDetails;
} SysCallSettings;

/**************************** FUNCTION PROTOTYPES *****************************/

/******************************** VARIABLES ***********************************/

PRIVATE SysCallSettings sysSettings;

PRIVATE LOGCB SIMLogPrint;

PRIVATE IPCBuffer ipcBuffer;

/***************************** PRIVATE FUNCTIONS *******************************/
PRIVATE bool isSizeExponential(uint32_t value)
{
    return !(value & (value - 1));
}

PRIVATE bool CPUCore_IsValidStackRegion(uint32_t stackOffset, uint32_t stackSize)
{
    bool isExponential = true;
    bool validStackSize = false;
    char* cpuCoreName = sysSettings.simDetails.simCPUCoreName;

    if (strncmp(cpuCoreName, "CortexM23", SIM_CPUCORE_NAME_LENGTH) == 0 ||
        strncmp(cpuCoreName, "CortexM33", SIM_CPUCORE_NAME_LENGTH) == 0)
    {
        isExponential = false;
    }

    if (isExponential)
    {
        if (!isSizeExponential(stackSize))
        {
            return false;
        }

        if (stackSize % stackSize != 0)
        {
            return false;
        }
    }

    return true;
}

PRIVATE SysThread* GetThreadInstanceByThreadID(uint32_t threadID)
{
    if (threadID == 0)
    {
        return &sysSettings.mainThread;
    }

    if (sysSettings.flags.threadPoolInitialised)
    {
        SysThread* thread = (SysThread*)sysSettings.threadPool.pool;

        for (uint32_t i = 0; i < sysSettings.threadPool.capacity; i++)
        {
            /* We found an available thread in the thread pool */
            if (thread->core.flags.inUse && thread->core.id == threadID)
            {
                return thread;
            }

            thread = (SysThread*)(((uint32_t)thread) + SYS_USER_THREAD_SIZE);
        }
    }

    return NULL;
}

PRIVATE SysThread* GetActiveThread()
{
    HANDLE currentThreadHandle;

    DuplicateHandle(
        GetCurrentProcess(),
        GetCurrentThread(),    // This is 0xFFFFFFFE (pseudo)
        GetCurrentProcess(),
        &currentThreadHandle,     // This will be a real HANDLE
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS
    );
    
    if (GetThreadId(currentThreadHandle) == GetThreadId(sysSettings.mainThread.core.winHandle))
    {
        return &sysSettings.mainThread;
    }

    if (sysSettings.flags.threadPoolInitialised)
    {
        SysThread* thread = (SysThread*)sysSettings.threadPool.pool;

        for (uint32_t i = 0; i < sysSettings.threadPool.capacity; i++)
        {
            /* We found an available thread in the thread pool */
            if (thread->core.flags.inUse && GetThreadId(thread->core.winHandle) == GetThreadId(currentThreadHandle))
            {
                return thread;
            }

            thread = (SysThread*)(((uint32_t)thread) + SYS_USER_THREAD_SIZE);
        }
    }

    return NULL;
}

PRIVATE SysStatus ThreadCheckForSuspendAndResume(uint32_t threadID, SysThread** threadObj)
{
    SysThread* thread;
    uint32_t threadIndex = GET_THREAD_INDEX(threadID);
    SysThread* activeThread = GetActiveThread();

    *threadObj = NULL;

    /* Main thread can not be suspended/resumed */
    if (threadIndex == 0)
    {
        return SysStatus_Thread_Invalid;
    }

    /* Thread pool is not initialised yet */
    if (!sysSettings.flags.threadPoolInitialised)
    {
        return SysStatus_NotInitialised;
    }

    thread = GetThreadInstanceByThreadID(threadID);
    if (thread == NULL)
    {
        /* Invalid thread (maybe released before) */
        return SysStatus_Thread_Invalid;
    }

     /*
      * If the caller thread is not main thread and a lower priority thread can
      * not suspend/resume any other thread
      */
    if (GET_THREAD_INDEX(activeThread->core.id) != 0 && thread->core.initialPriority > activeThread->core.initialPriority)
    {
        return SysStatus_Thread_InvalidPriority;
    }

    /* Return the thread object */
    *threadObj = thread;

    return SysStatus_Success;
}

PRIVATE void NotifyWaitingThread(SysEvent event)
{
    if (sysSettings.mainThread.core.ipcEvent != 0)
    {
        if (KERNEL_IS_WAITED_EVENT_SET(&sysSettings.mainThread.core, event))
        {
            sysSettings.mainThread.core.waitedEvents = 0;

            SetEvent(sysSettings.mainThread.core.ipcEvent);
        }
    }

    if (sysSettings.flags.threadPoolInitialised)
    {
        SysThread* thread = (SysThread*)sysSettings.threadPool.pool;

        for (uint32_t tIndex = 0; tIndex < sysSettings.threadPool.capacity; tIndex++)
        {
            if (thread->core.flags.inUse)
            {
                HANDLE eventHandler = NULL;

                switch (event)
                {
                    case SysEvent_IPCMessage: eventHandler = thread->core.ipcEvent; break;
                    case SysEvent_Timer: eventHandler = thread->core.timerEvent; break;
                    default:
                        break;
                }

                if (KERNEL_IS_WAITED_EVENT_SET(&thread->core, event))
                {
                    thread->core.waitedEvents = 0;

                    if (eventHandler != NULL)
                    {
                        SetEvent(eventHandler);
                        CloseHandle(eventHandler);
                    }
                }
            }
        }
    }
}

PRIVATE DWORD WINAPI PipeListenerThread(LPVOID lpParam)
{
    char pipeName[32];

    sprintf(pipeName, PIPE_NAME_FORMAT, sysSettings.simDetails.simExecIndex);

    sysSettings.ipcListenerPipe = CreateNamedPipeA(
        pipeName,
        PIPE_ACCESS_DUPLEX,  // Allow both reading and writing
        PIPE_TYPE_BYTE | PIPE_WAIT,
        1,               // Max instances
        PIPE_BUFFER_SIZE, PIPE_BUFFER_SIZE,      // Output/Input buffer size
        0,               // Default timeout
        NULL             // Security attributes
    );

    if (sysSettings.ipcListenerPipe == INVALID_HANDLE_VALUE ||
        sysSettings.ipcListenerPipe == (HANDLE)0)
    {
        LOG_PRINTF("[IPC] Failed to create pipe. Error: %lu\n", GetLastError());
        Sys_Exit();

        return -1;
    }

    ConnectNamedPipe(sysSettings.ipcListenerPipe, NULL);

    while (1)
    {
        uint32_t receivedLen = 0;
        BOOL result = ReadFile(sysSettings.ipcListenerPipe, 
                               &ipcBuffer.readBuffer[ipcBuffer.readOffset],
                               sizeof(ipcBuffer.readBuffer) - ipcBuffer.readOffset,
                               &receivedLen, NULL);
        
        if (result)
        {
            ipcBuffer.receivedLen += receivedLen;

            if (receivedLen > 0)
            {
                NotifyWaitingThread(SysEvent_IPCMessage);
            }
        }
    }

    return 0;
}

PRIVATE SysStatus GetSemaphoreHandleByID(uint32_t semaphoreID, HANDLE* semaphoreHandle)
{
    /* Do not allow to get a new semaphore if the pool is not initialised yet */
    if (!sysSettings.flags.threadSyncPoolInitialised)
    {
        return SysStatus_NotInitialised;
    }

    /* Invalid Semaphore ID */
    if (semaphoreID >= sysSettings.threadSyncPool.capacity)
    {
        return SysStatus_InvalidParameter;
    }

    *semaphoreHandle = sysSettings.threadSyncPool.syncHandles[semaphoreID];

    return SysStatus_Success;
}

INTERNAL void SIM_RegisterLOGCallback(LOGCB logCB)
{
    SIMLogPrint = logCB;
}

/***************************** PUBLIC FUNCTIONS *******************************/

SysStatus Sys_Log(const char* format, ...)
{
    static char logBuffer[512];
    va_list args;

    va_start(args, format);
    vsprintf(logBuffer, format, args);
    va_end(args);

    SIMLogPrint((uint8_t*)logBuffer);

    return 0;
}

void Sys_SimulationInit(Sys_SimulationDetails* simDetails)
{
    if (simDetails->simExecIndex >= SIM_MAX_CONTAINER_COUNT)
    {
        LOG_PRINTF("Invalid Execution Index %d. Max Allowed %d", simDetails->simExecIndex, SIM_MAX_CONTAINER_COUNT);
        Sys_Exit();
    }

    memcpy(sysSettings.simDetails.simCPUCoreName, simDetails->simCPUCoreName, 16);
    sysSettings.simDetails.simCPUCoreName[15] = 0;
    sysSettings.simDetails.simExecIndex = simDetails->simExecIndex;

    InitializeCriticalSection(&sysSettings.cs);

    sysSettings.refTime = (uint64_t)GetTickCount64();

    {
        sysSettings.mainThread.core.id = 0;
        sysSettings.mainThread.core.flags.inUse = true;
        sysSettings.mainThread.core.flags.isMainThread = true;
        sysSettings.mainThread.core.state = ThreadState_Ready;

        DuplicateHandle(
            GetCurrentProcess(),
            GetCurrentThread(),      // Pseudo-handle (only valid in this thread)
            GetCurrentProcess(),
            &sysSettings.mainThread.core.winHandle,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
        );

        sysSettings.mainThread.core.winThreadId = GetThreadId(sysSettings.mainThread.core.winHandle);
    }
}

void Sys_Exit(void)
{
    LOG_PRINTF("Exit...");

    // TODO Find and suspend all threads

    while (1);
}

void Sys_Yield(void)
{
    SwitchToThread();
}

void Sys_EnterCriticalSection(void)
{
    sysSettings.flags.inCS = true;
    EnterCriticalSection(&sysSettings.cs);
}

void Sys_ExitCriticalSection(void)
{
    sysSettings.flags.inCS = false;
    LeaveCriticalSection(&sysSettings.cs);
}

SysStatus Sys_Sleep(timeoutInMs)
{
    if (sysSettings.flags.inCS)
    {
        return SysStatus_CannotSleepInCS;
    }

    // Max 1 day in ms
    if (timeoutInMs > (24 * 3600 * 1000))
    {
        return SysStatus_InvalidParameter;
    }

    Sleep(timeoutInMs);

    return SysStatus_Success;
}

SysStatus Sys_WaitForEvent(SysEvent event)
{
    SysThread* activeThread = GetActiveThread();
    if (activeThread == NULL)
    {
        return SysStatus_Fail;
    }

    switch (event)
    {
        case SysEvent_IPCMessage:
            break;
        case SysEvent_ThreadSync:
            return SysStatus_InvalidOperation;
        default:
            return SysStatus_UnsupportedEvent;
    }

    if (sysSettings.flags.inCS)
    {
        return SysStatus_CannotSleepInCS;
    }

    /* Event must be positive*/
    if (event < ((SysEvent)1) ||
        event >= (SysEvent)KERNEL_SYSEVENT_MAXEVENT)
    {
        return SysStatus_UnsupportedEvent;
    }

    //if (event >= (SysEvent)KERNEL_SYSEVENT_MAXEVENT)
    //{
    //    if (!KERNEL_IS_CUSTOM_EVENT_ENABLED(event))
    //    {
    //        /* If custom event is not enabled, discard message!  */
    //        return SysStatus_UnsupportedEvent;
    //    }
    //    else
    //    {
    //        /* Do not sleep the thread if the event is already pending */
    //        if (kernelSettings.pendingEvents & KERNEL_EVENT_MASK(event))
    //        {
    //            kernelSettings.pendingEvents &= (~KERNEL_EVENT_MASK(event));
    //            return SysStatus_Success;
    //        }
    //    }
    //}
    //else

    HANDLE eventHandle;
    {
        /* Supported OS Events */
        switch (event)
        {
        case SysEvent_IPCMessage:
            if (activeThread->core.ipcEvent == (HANDLE)0)
            {
                activeThread->core.ipcEvent = CreateEvent(NULL, FALSE, FALSE, NULL); // auto-reset
            }
            eventHandle = activeThread->core.ipcEvent;
            break;
        case SysEvent_ThreadSync:
            return SysStatus_InvalidOperation;
        case SysEvent_Timer:
        default: return SysStatus_UnsupportedEvent;
        }

        if (eventHandle == 0)
        {
            return SysStatus_Fail;
        }
    }

    /* Set the flag */
    KERNEL_SET_WAITED_EVENT_FLAG(&activeThread->core, event);
 
    /* Change state to waiting to remove from ready list */
    activeThread->core.state = ThreadState_Waiting;

    WaitForSingleObject(eventHandle, INFINITE);

    return SysStatus_Success;
}

SysStatus Sys_IsPendingEvent(SysEvent event, bool clearAfterRead, bool* pending)
{
    // We check for Custom Event which we dont support
    return SysStatus_UnsupportedEvent;
}

SysStatus Sys_ClearPendingEvent(SysEvent event)
{
    return SysStatus_NotSupported;
}

uint64_t Sys_GetTimeInMs(void)
{
    return (uint64_t)GetTickCount64() - sysSettings.refTime;
}

uint32_t Sys_GetEPOCTime(void)
{
    return (uint32_t)time(NULL);
}

SysStatus Sys_WDTInit(uint32_t timeoutInMs)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_WDTEnable(void)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_WDTDisable(void)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_WDTFeed(void)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_GetDeviceUID(uint32_t uID[], uint32_t* uIDLength)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_StorageGetSize(uint32_t* storageSize)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_StorageRead(uint32_t offset, uint32_t length, uint8_t* buffer)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_StorageWrite(uint32_t offset, uint32_t length, uint8_t* buffer)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_StorageClear(void)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_GetRootParamItem(uint32_t tag, uint8_t* buffer, uint32_t bufferSize, uint32_t* itemLength)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_SetISRStack(uint8_t* stackBase, uint32_t stackSize)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_SetHWISR(int32_t irqNo, Sys_UserISR userISR)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_WaitResource(uint32_t resourceID)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_TryWaitResource(uint32_t resourceID)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_ReleaseResource(uint32_t resourceID)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_GetExecutionIndexByName(const char name[SYS_EXEC_NAME_MAX_LENGTH], uint32_t* execIndex)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_GetExecutionIndexByUID(const uint8_t uid[SYS_EXEC_UID_LENGTH], uint32_t* execIndex)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_GetExecutionIndexByValue(uint32_t value, uint32_t* execIndex)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_InitialiseThreadPool(volatile uint8_t* threadPoolContainer, uint32_t containerSize)
{
    if (sysSettings.flags.threadPoolInitialised)
    {
        return SysStatus_AlreadyInitialised;
    }

    if ((containerSize < SYS_USER_THREAD_SIZE) ||
        ((containerSize % SYS_USER_THREAD_SIZE) != 0) ||
        ((containerSize / SYS_USER_THREAD_SIZE) > KERNEL_MAX_THREAD_COUNT_PER_EXECUTION))
    {
        return SysStatus_InvalidContainerSize;
    }

    if (((uint32_t)threadPoolContainer) % sizeof(int) != 0)
    {
        return SysStatus_InvalidOffsetAlignment;
    }

    sysSettings.threadPool.pool = (SysThread*)threadPoolContainer;
    sysSettings.threadPool.capacity = containerSize / SYS_USER_THREAD_SIZE;
    sysSettings.threadPool.count = 0;

    memset((uint8_t*)threadPoolContainer, 0, containerSize);

    sysSettings.flags.threadPoolInitialised = true;

    return SysStatus_NotSupported;
}

SysStatus Sys_GetThread(Sys_ThreadStartPoint startPoint, uint8_t* stack, uint32_t stackSize, uint32_t priority, void* args, uint32_t* threadID)
{
    uint32_t allowedMaxPriority = 16;

    if (!sysSettings.flags.threadPoolInitialised)
    {
        return SysStatus_NotInitialised;
    }

    if (stackSize < KERNEL_MINIMUM_STACK_SIZE)
    {
        return SysStatus_Thread_InvalidStackSize;
    }

    if (priority > allowedMaxPriority)
    {
        return SysStatus_Thread_InvalidPriority;
    }

    if (!CPUCore_IsValidStackRegion((uint32_t)stack, stackSize))
    {
        return SysStatus_Thread_InvalidStackAlignment;
    }

    if (sysSettings.threadPool.count >= sysSettings.threadPool.capacity)
    {
        return SysStatus_NoSlotAvailable;
    }

    SysThread* thread = (SysThread*)sysSettings.threadPool.pool;

    for (uint32_t i = 0; i < sysSettings.threadPool.capacity; i++)
    {
        /* We found an available thread in the thread pool */
        if (!thread->core.flags.inUse)
        {
            DWORD threadId;
            HANDLE threadHandle;

            threadHandle = CreateThread(
                NULL,                               // Default security attributes
                stackSize * STACK_SIM_FACTOR,       // Stack Size with a Simulator Factor
                (LPTHREAD_START_ROUTINE)startPoint,                         // Thread function
                (LPVOID)args,                       // Argument to thread
                CREATE_SUSPENDED,                   // Creation flags (0 = start immediately)
                &threadId                           // Receives thread ID
            );

            if (!threadHandle)
            {
                return SysStatus_Fail;
            }

            /* Clean the thread */
            memset(thread, 0, SYS_USER_THREAD_SIZE);

            /* Calculate the thread ID */
            thread->core.id = (uint16_t)GET_THREAD_ID(0, i + 1);

            thread->core.winHandle = threadHandle;
            thread->core.winThreadId = threadId;

            break;
        }

        thread = (SysThread*)(((uint32_t)thread) + SYS_USER_THREAD_SIZE);
    }

    /* Suspend the thread by default obtained from thread pool */
    thread->core.state = ThreadState_Suspended;
    thread->core.initialPriority = priority;

    sysSettings.threadPool.count++;

    *threadID = thread->core.id;

    return SysStatus_Success;
}

SysStatus Sys_SetMainThreadPriority(uint32_t priority)
{
    return SysStatus_NotSupported;
}

SysStatus Sys_FreeThread(uint32_t threadID)
{
    SysThread* thread;
    uint32_t threadIndex = GET_THREAD_INDEX(threadID);
    SysThread* activeThread = GetActiveThread();

    if (activeThread == NULL)
    {
        /* It shall never happen, but just in case, like in case TerminateThread does not work etc */
        return SysStatus_Fail;
    }

    /* Main thread can not be released */
    if (threadIndex == 0)
    {
        return SysStatus_Thread_Invalid;
    }

    /* Do not allow to release a new thread if thread pool is not initialised */
    if (!sysSettings.flags.threadPoolInitialised)
    {
        return SysStatus_NotInitialised;
    }

    if (threadIndex > sysSettings.threadPool.capacity)
    {
        return SysStatus_Thread_Invalid;
    }

    thread = GetThreadInstanceByThreadID(threadID);
    if (thread == NULL)
    {
        /* The thread may already released before */
        return SysStatus_Thread_Invalid;
    }

    if (GET_THREAD_INDEX(activeThread->core.id) != 0 && thread->core.initialPriority > activeThread->core.initialPriority)
    {
        return SysStatus_Thread_InvalidPriority;
    }

    (void)TerminateThread(thread->core.winHandle, 0);

    thread->core.flags.inUse = false;

    sysSettings.threadPool.count--;

    return SysStatus_Success;
}

SysStatus Sys_AbortThread(void)
{
    SysThread* activeThread = GetActiveThread();

    if (activeThread == NULL)
    {
        /* It shall never happen, but just in case, like in case TerminateThread does not work etc */
        return SysStatus_Fail;
    }

    return Sys_FreeThread(activeThread->core.id);
}

SysStatus Sys_SuspendThread(uint32_t threadID)
{
    SysThread* thread;
    SysThread* activeThread = GetActiveThread();

    if (activeThread == NULL)
    {
        return SysStatus_Fail;
    }

    SysStatus status = ThreadCheckForSuspendAndResume(threadID, &thread);
    if (status != SysStatus_Success)
    {
        return status;
    }

    /* Does not suspend, it was already */
    if (thread->core.state == ThreadState_Suspended)
    {
        return SysStatus_Thread_AlreadySuspended;
    }

    /* Change the thread state */
    thread->core.state = ThreadState_Suspended;


    /* If the thread suspends itself,  switch to next thread */
    if (thread->core.id == activeThread->core.id)
    {
        SwitchToThread();
    }
    else
    {
        SuspendThread(thread->core.winHandle);
    }

    return SysStatus_Success;
}

SysStatus Sys_ResumeThread(uint32_t threadID)
{
    SysThread* thread;
    SysThread* activeThread = GetActiveThread();

    if (activeThread == NULL)
    {
        return SysStatus_Fail;
    }

    SysStatus status = ThreadCheckForSuspendAndResume(threadID, &thread);
    if (status != SysStatus_Success)
    {
        return status;
    }

    /* Do not resume, it was already running */
    if (thread->core.state != ThreadState_Suspended)
    {
        return SysStatus_Thread_AlreadyRunning;
    }

    /* Change the thread state */
    thread->core.state = ThreadState_Ready;

    ResumeThread(thread);

    return SysStatus_Success;
}

SysStatus Sys_GetThreadID(uint32_t* threadID)
{
    SysThread* activeThread = GetActiveThread();
    if (activeThread == NULL)
    {
        return SysStatus_Fail;
    }

    *threadID = activeThread->core.id;

    return SysStatus_Success;
}

SysStatus Sys_GetThreadState(uint32_t threadID, Sys_ThreadState* threadState)
{
    uint32_t threadIndex = GET_THREAD_INDEX(threadID);
    Sys_ThreadState state = SysThreadState_None;
    SysStatus retVal = SysStatus_Success;
    SysThread* activeThread = GetActiveThread();

    if (activeThread == NULL)
    {
        return SysStatus_Fail;
    }

    if (threadIndex == 0)
    {
        /* Main Thread States (Main Thread state be only running or ready) */
        if (activeThread->core.id == threadID)
        {
            state = SysThreadState_Running;
        }
        else
        {
            state = SysThreadState_Ready;
        }
    }
    else
    {
        if (sysSettings.flags.threadPoolInitialised)
        {
            SysThread* thread = GetThreadInstanceByThreadID(threadID);

            if (thread != NULL && thread->core.flags.inUse)
            {
                switch (thread->core.state)
                {
                case ThreadState_Ready:
                {
                    /* If a thread is asking for itself, state is running otherwise ready */
                    if (thread->core.id == activeThread->core.id) { state = SysThreadState_Running; }
                    else { state = SysThreadState_Ready; }
                }
                break;
                case ThreadState_Waiting:   state = SysThreadState_Waiting;     break;
                case ThreadState_Suspended: state = SysThreadState_Suspended;   break;
                default: break;
                }
            }
            else
            {
                if (thread == NULL)
                {
                    /* Thread may released before */
                    retVal = SysStatus_Thread_Invalid;
                }
            }
        }
        else
        {
            retVal = SysStatus_NotInitialised;
        }
    }

    *threadState = state;

    return retVal;
}

SysStatus Sys_InitialiseThreadSyncPool(volatile uint8_t* threadSyncPoolContainer, uint32_t containerSize)
{
    if (sysSettings.flags.threadSyncPoolInitialised)
    {
        return SysStatus_AlreadyInitialised;
    }

    if ((containerSize == 0) || /* Container size cannot be zero */
        (containerSize % SYS_THREAD_SYNC_PRIMITIVE_SIZE != 0) ||  /* Container size must be aligned with Thread Size */
        ((containerSize / SYS_THREAD_SYNC_PRIMITIVE_SIZE) > KERNEL_MAX_THREAD_COUNT_PER_EXECUTION))   /* Allowed maximum primitive count */
    {
        return SysStatus_InvalidContainerSize;
    }

    /* Container offset must be word aligned */
    if (((uint32_t)threadSyncPoolContainer) % sizeof(int) != 0)
    {
        return SysStatus_InvalidOffsetAlignment;
    }

    sysSettings.threadSyncPool.pool = (SysThreadSync*)threadSyncPoolContainer;
    sysSettings.threadSyncPool.capacity = containerSize;
    sysSettings.threadSyncPool.count = 0;

    memset((void*)threadSyncPoolContainer, 0, containerSize);

    sysSettings.flags.threadSyncPoolInitialised = true;

    return SysStatus_Success;
}

SysStatus Sys_GetSemaphore(uint32_t maximumCount, uint32_t* semaphoreID)
{
    if (sysSettings.flags.threadSyncPoolInitialised)
    {
        return SysStatus_NotInitialised;
    }

    if (maximumCount == 0 || maximumCount > KERNEL_MAX_THREAD_COUNT_PER_EXECUTION)
    {
        return SysStatus_InvalidParameter;
    }

    if (sysSettings.threadSyncPool.count >= sysSettings.threadSyncPool.capacity)
    {
        return SysStatus_NoSlotAvailable;
    }

    sysSettings.threadSyncPool.count++;

    return SysStatus_Success;
}

SysStatus Sys_WaitSemaphore(uint32_t semaphoreID)
{
    HANDLE semaphoreHandle;
    SysStatus retVal = GetSemaphoreHandleByID(semaphoreID, &semaphoreHandle);

    if (retVal != SysStatus_Success)
    {
        return retVal;
    }

    WaitForSingleObject(semaphoreHandle, INFINITE);  // 0 ms timeout = non-blocking

    return SysStatus_Success;
}

SysStatus Sys_TryWaitSemaphore(uint32_t semaphoreID)
{
    HANDLE semaphoreHandle;
    SysStatus retVal = GetSemaphoreHandleByID(semaphoreID, &semaphoreHandle);

    if (retVal != SysStatus_Success)
    {
        return retVal;
    }

    DWORD result = WaitForSingleObject(semaphoreHandle, 0);  // 0 ms timeout = non-blocking
    if (result == WAIT_OBJECT_0)
    {
        return SysStatus_Success;
    }
    else if (result == WAIT_TIMEOUT)
    {
        return SysStatus_WaitNotPerformed;
    }

    return SysStatus_Fail;
}

SysStatus Sys_ReleaseSemaphore(uint32_t semaphoreID)
{
    HANDLE semaphoreHandle;
    SysStatus retVal = GetSemaphoreHandleByID(semaphoreID, &semaphoreHandle);

    if (retVal != SysStatus_Success)
    {
        return retVal;
    }

    ReleaseSemaphore(semaphoreHandle, 1, NULL);

    return SysStatus_Success;
}

SysStatus Sys_GetMutex(uint32_t* mutexID)
{
    return Sys_GetSemaphore(1, mutexID);
}

SysStatus Sys_WaitMutex(uint32_t mutexID)
{
    return Sys_WaitSemaphore(mutexID);
}

SysStatus Sys_TryWaitMutex(uint32_t mutexID)
{
    return Sys_TryWaitSemaphore(mutexID);
}

SysStatus Sys_ReleaseMutex(uint32_t mutexID)
{
    return Sys_ReleaseSemaphore(mutexID);
}

SysStatus Sys_InitialiseMessageBox(volatile uint8_t* messageBoxContainer, uint32_t containerSize)
{
    uint32_t capacity = containerSize / SYS_IPC_MESSAGE_SIZE;

    if (sysSettings.flags.messageBoxInitialised)
    {
        return SysStatus_AlreadyInitialised;
    }

    if (containerSize == 0 || 
        ((containerSize % SYS_IPC_MESSAGE_SIZE) != 0) ||
        capacity > KERNEL_IPC_MESSAGEBOX_MAX_CAPACITY)
    {
        return SysStatus_InvalidContainerSize;
    }

    if (((uint32_t)messageBoxContainer) % sizeof(int) != 0)
    {
        return SysStatus_InvalidOffsetAlignment;
    }

    sysSettings.messageBox.pool = (SysIPCMB*)messageBoxContainer;
    sysSettings.messageBox.capacity = capacity;
    sysSettings.messageBox.count = 0;

    memset((uint8_t*)messageBoxContainer, 0, containerSize);

    sysSettings.flags.messageBoxInitialised = true;

    (void)CreateThread(NULL, 0, PipeListenerThread, NULL, 0, NULL);

    return SysStatus_Success;
}

SysStatus Sys_SendMessage(uint8_t destinationID, uint8_t* message, uint32_t messageLength, uint32_t* sequenceNo)
{
    SysThread* activeThread = GetActiveThread(); if (activeThread == NULL) { SysStatus_Fail; }
    DWORD writtenLen;

    if (!sysSettings.flags.messageBoxInitialised)
    {
        return SysStatus_NotInitialised;
    }

    /* Let us allow another */
    if (destinationID == sysSettings.simDetails.simExecIndex ||
        destinationID >= SIM_MAX_CONTAINER_COUNT)
    {
        return SysStatus_IPC_InvalidDestination;
    }

    if (messageLength == 0 || messageLength > KERNEL_MAX_MESSAGE_LENGTH)
    {
        return SysStatus_IPC_InvalidMessage;
    }

    IPCPackageHeader header;
    header.length = messageLength;
    header.senderID = sysSettings.simDetails.simExecIndex;

    if (sysSettings.ipcClientPipe[destinationID] == (HANDLE)0)
    {
        char pipeName[32];

        sprintf(pipeName, PIPE_NAME_FORMAT, destinationID);

        uint64_t timeout = Sys_GetTimeInMs() + 1000;
        HANDLE hPipe;

        do
        {
            hPipe = CreateFileA(
                pipeName,
                GENERIC_READ | GENERIC_WRITE,
                0, NULL, OPEN_EXISTING, 0, NULL
            );

        } while (hPipe == INVALID_HANDLE_VALUE && timeout > Sys_GetTimeInMs());

        if (hPipe == INVALID_HANDLE_VALUE || hPipe == (HANDLE)0)
        {
            return SysStatus_IPC_DestinationNotHaveMessageBox;
        }

        sysSettings.ipcClientPipe[destinationID] = hPipe;
    }


    {
        uint8_t* allInOneMem;
        uint32_t totalLen = sizeof(header) + messageLength;
        allInOneMem = (uint8_t*)malloc(totalLen);
        if (allInOneMem == NULL)
        {
            return SysStatus_Fail;
        }

        memcpy(allInOneMem, &header, sizeof(header));
        memcpy(&allInOneMem[sizeof(header)], message, messageLength);


        WriteFile(sysSettings.ipcClientPipe[destinationID], allInOneMem, totalLen, &writtenLen, NULL);

        if (writtenLen != totalLen)
        {
            return SysStatus_Fail;
        }
    }

    *sequenceNo = ++sysSettings.ipcSequenceNo;

    return SysStatus_Success;
}

SysStatus Sys_SendMessageByName(char destinationName[SYS_IPC_MAX_NAME_LENGTH], uint8_t* message, uint32_t len, uint32_t* sequenceNo)
{
    return Sys_SendMessage(0, message, len, sequenceNo);
}

SysStatus Sys_ReceiveMessage(uint8_t* senderID, uint8_t* message, uint32_t len, uint32_t* sequenceNo)
{
    IPCPackage* recvPackage;
    uint32_t recvPayLoadLen;

    if (!sysSettings.flags.messageBoxInitialised)
    {
        return SysStatus_NotInitialised;
    }

    // We havent read
    if (ipcBuffer.receivedLen < sizeof(IPCPackageHeader))
    {
        return SysStatus_IPC_MessageBoxEmpty;
    }

    recvPayLoadLen = ipcBuffer.receivedLen - sizeof(IPCPackageHeader);

    if (ipcBuffer.readOffset + len > recvPayLoadLen)
    {
        len = recvPayLoadLen - ipcBuffer.readOffset;
    }

    recvPackage = (IPCPackage*)ipcBuffer.readBuffer;

    memcpy(message, &recvPackage->payload[ipcBuffer.readOffset], len);
    ipcBuffer.readOffset += len;

    *sequenceNo = sysSettings.ipcSequenceNo;

    if (ipcBuffer.readOffset == recvPackage->header.length)
    {
        ipcBuffer.readOffset = 0;
        ipcBuffer.receivedLen = 0;

        sysSettings.ipcSequenceNo++;
    }

    *senderID = recvPackage->header.senderID;

    return SysStatus_Success;

}

SysStatus Sys_ReceiveMessageByName(char senderIDName[SYS_IPC_MAX_NAME_LENGTH], uint8_t* message, uint32_t len, uint32_t* sequenceNo)
{
    uint8_t senderID;
    SysStatus retVal;

    if (!sysSettings.flags.messageBoxInitialised)
    {
        return SysStatus_NotInitialised;
    }
    
    retVal = Sys_ReceiveMessage(&senderID, message, len, sequenceNo);

    if (retVal == SysStatus_Success)
    {
        snprintf(senderIDName, SYS_IPC_MAX_NAME_LENGTH, "Exec%d", (int)senderID);
    }

    return retVal;
}

SysStatus Sys_IsMessageReceived(bool* isReceived, uint32_t* messageLen, uint32_t* sequenceNo)
{
    if (!sysSettings.flags.messageBoxInitialised)
    {
        return SysStatus_NotInitialised;
    }

    *isReceived = ipcBuffer.receivedLen > sizeof(IPCPackageHeader);
    if (*isReceived)
    {
        *messageLen = (ipcBuffer.receivedLen - sizeof(IPCPackageHeader)) - ipcBuffer.readOffset;
        *sequenceNo = sysSettings.ipcSequenceNo;
    }

    return SysStatus_Success;
}

SysStatus uService_Initialise(const char* name, uint32_t* uServiceHandle)
{
    (void)name;

    /* For now, the microservice is 0 by default */
    *uServiceHandle = 0;

    sysSettings.flags.ucInitialised = true;

    return SysStatus_Success;
}

SysStatus uService_RequestBlocker(uint32_t uServiceHandle, uServicePackage* request, uServicePackage* response, uint32_t timeoutInMs)
{
    SysStatus retVal;
    bool messageReceived;
    uint32_t messageLen;
    uint32_t timeout;
    uint32_t sequenceNo; (void)sequenceNo;

    if (uServiceHandle >= SYS_MICROSEVICE_MAX_HANDLE ||
        !sysSettings.flags.ucInitialised)
    {
        return SysStatus_NotFound;
    }

    timeout = (uint32_t)Sys_GetTimeInMs() + timeoutInMs;

    retVal = Sys_SendMessage(uServiceHandle, (uint8_t*)request, request->header.length, &sequenceNo);
    if (retVal != SysStatus_Success)
    {
        return retVal;
    }

    do
    {
        (void)Sys_IsMessageReceived(&messageReceived, &messageLen, &sequenceNo);

        if (messageReceived && messageLen > 0)
        {
            uint8_t senderID; (void)senderID;

            (void)Sys_ReceiveMessage(&senderID, (uint8_t*)response, messageLen, &sequenceNo);

            if (response->header.operation != request->header.operation)
            {
                continue;
            }

            break;
        }

        if (timeout < (uint32_t)Sys_GetTimeInMs())
        {
            retVal = SysStatus_Timeout;
            break;
        }
    } while (true);

    return retVal;
}
