/* 
  macros.h
  =====================================
  Header file containing the custom macros used by the project.
 */

#ifndef MACROS_H
#define MACROS_H

/*--------------------------------
 * Dependencies
 *--------------------------------*/
#include <cstring>
#include <Arduino.h>
#include "utils/logger.h"

/*--------------------------------
 * Macro definitions
 *--------------------------------*/
// Just a tweak to only print the relative path to the filename in D_TRACE
#undef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '\\') ? \
  strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifdef DEBUG_MODE
#if ENV_FREERTOS == 1
#define D_TRACE(s) printLogSafe(s, __FILENAME__, __LINE__, __func__)
#else // ENV_FREERTOS == 0 (i.e. ENV_BARE_METAL == 1)
#define D_TRACE(s) printLog(s, __FILENAME__, __LINE__, __func__)
#endif // ENV_FREERTOS == 1
#else // DEBUG_MODE
#define D_TRACE(s)
#endif // DEBUG_MODE

#endif // MACROS_H
