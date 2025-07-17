/*******************************************************************************
 *
 * @file SysCall_Log.h
 *
 * @brief Log Interface for Executions
 *
 ******************************************************************************
 *
 * Copyright (c) 2016-2025 ZAYA Tech Ltd - All rights reserved.
 *
 * Unauthorised copying of this file, via any medium is strictly prohibited.
 *
 ******************************************************************************/

#ifndef __SYS_LOG_H
#define __SYS_LOG_H

#include "SysCall_Common.h"

#include "ztypes.h"

/***************************** MACRO DEFINITIONS ******************************/

/* Default value for Info level logs */
#ifndef LOG_INFO_ENABLED
#define LOG_INFO_ENABLED            0
#endif /* LOG_INFO_ENABLED */

/* Default value for Warning level logs */
#ifndef LOG_WARNING_ENABLED
#define LOG_WARNING_ENABLED         0
#endif /* LOG_WARNING_ENABLED */

/* Default value for Error level logs */
#ifndef LOG_ERROR_ENABLED
#define LOG_ERROR_ENABLED           0
#endif /* LOG_ERROR_ENABLED */

/*
 * LOG MACROS
 */

/* LOG Output */
#define LOG_PRINTF(format, ...) \
                 Sys_Log(format, ## __VA_ARGS__)


/* Info Level LOG Output */
#if LOG_INFO_ENABLED

    #define LOG_INFO(format, ...) \
                 LOG_PRINTF(" > INF %s:%d " format "", __func__ , __LINE__, ## __VA_ARGS__)

#else

    /* Do not print info level logs */
    #define LOG_INFO(...)

#endif /* LOG_INFO_ENABLED */

/* Warning Level LOG Output */
#if LOG_WARNING_ENABLED

    #define LOG_WARNING(format, ...) \
                 LOG_PRINTF(" > WAR %s:%d " format "", __func__ , __LINE__, ## __VA_ARGS__)

#else

    /* Do not print warning level logs */
    #define LOG_WARNING(...)

#endif /* LOG_WARNING_ENABLED */

/* Error Level LOG Output */
#if LOG_ERROR_ENABLED

    #define LOG_ERROR(format, ...) \
                 LOG_PRINTF(" > ERR %s:%d " format "", __func__ , __LINE__, ## __VA_ARGS__)

#else

    /* Do not print error level logs */
    #define LOG_ERROR(...)

#endif /* LOG_ERROR_ENABLED */

/***************************** TYPE DEFINITIONS *******************************/

/*************************** FUNCTION DEFINITIONS *****************************/

/* System Call for Logging */
SysStatus Sys_Log(const char* format, ...);

/********************************* INCLUDES ***********************************/

#endif    /* __SYS_LOG_H */
