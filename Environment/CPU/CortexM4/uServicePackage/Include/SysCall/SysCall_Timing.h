/*******************************************************************************
 *
 * @file SysCall_Timing.h
 *
 * @brief Timing Interface
 *
 ******************************************************************************
 *
 * Copyright (c) 2016-2025 ZAYA Tech Ltd - All rights reserved.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/

#ifndef __SYSCALL_TIMING_H
#define __SYSCALL_TIMING_H

/********************************* INCLUDES ***********************************/
#include "SysCall_Common.h"

#include "ztypes.h"

/***************************** MACRO DEFINITIONS ******************************/

/*
 * User Timer Size.
 * Kernel needs 12 bytes for each timers.
 */
#define SYS_USER_TIMER_SIZE                             (16)

/* Macro to calculate required container size for user timers */
#define SYS_USER_TIMER_CONTAINER_SIZE(_timerCount)      ((_timerCount) * SYS_USER_TIMER_SIZE)

/**
 * Initialises User Timers
 * It can be called in a function (like Microcontainer init) which is
 * recommended to reduce the scope. In this way, you can avoid mistaken accesses.
 *
 * @param[out] retVal Return value after timer initialisation. Microcontainer
 *                    needs to check it before timer usage.
 *
 *                  - SysStatus_Success Success
 *                  - SysStatus_AlreadyInitialised Timer already initialised before.
 *                  - SysStatus_InvalidContainerSize Invalid container size.
 *                      Not aligned with SYS_USER_TIMER_SIZE.
 *
 * @param _maxTimerCount Required User Timer Count.
 */
#define SYS_INITIALISE_USER_TIMERS(_retVal, _maxTimerCount) \
{ \
    static volatile uint32_t userTimerContainer[SYS_USER_TIMER_CONTAINER_SIZE(_maxTimerCount)/sizeof(uint32_t)]; \
    _retVal = Sys_InitialiseTimer((volatile uint8_t*)userTimerContainer, SYS_USER_TIMER_CONTAINER_SIZE(_maxTimerCount)); \
}

/***************************** TYPE DEFINITIONS *******************************/

/* Event Flag For Timer */
typedef SyscallEventFlag TimerFlag;

/*************************** FUNCTION DEFINITIONS *****************************/

/**
 * Initialises Microcontainer Software Timers.
 *
 * If a Microcontainer needs to use timers, it needs to provide a memory space.
 *
 * IMP : Please use SYS_INITIALISE_USER_TIMERS() macro instead of calling this
 * function directly.
 *
 * Please see SYS_INITIALISE_USER_TIMERS() for detailed description.
 *
 * @param timerContainer Timer Container (Memory Area) for user timers.
 * @param containerSize Timer Container Size
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_AlreadyInitialised Timer already initialised before.
 * @retval SysStatus_InvalidContainerSize Invalid container size.
 *          - Not aligned with SYS_USER_TIMER_SIZE
 */
SysStatus Sys_InitialiseTimer(volatile uint8_t* timerContainer, uint32_t containerSize);

/**
 * Sets a timer.
 * This timer interface does not call any callback to inform the Microcontainer
 * when timeout occurred. Instead just sets a timeout flag.
 *
 * @param timeoutInMs Timer Timeout Value
 * @param timeoutFlag to be set flag in case of timer timeout
 *
 * @retval SysStatus_Success Success
 * @retval SysStatus_AddressAccessViolation Address violation.
 *          Pointer shows an invalid address.
 * @retval SysStatus_NoSlotAvailable All user timer slots are in use.
 * @retval SysStatus_InvalidParameter Invalid timeout value.
 */
SysStatus Sys_SetTimer(uint32_t timeoutInMs, TimerFlag* timeoutFlag);

#endif    /* __SYSCALL_TIMING_H */
