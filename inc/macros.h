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

/*--------------------------------
 * Macro definitions
 *--------------------------------*/
// Just a tweak to only print the relative path to the filename in D_TRACE
#undef __FILENAME__
#define __FILENAME__ (strrchr(__FILE__, '\\') ? \
  strrchr(__FILE__, '\\') + 1 : __FILE__)

#ifdef DEBUG_MODE
#define D_TRACE(s) do {         \
  Serial.print("[DEBUG] ");  \
  Serial.print(__FILENAME__);   \
  Serial.print("@");         \
  Serial.print(__LINE__);       \
  Serial.print(": {");       \
  Serial.print(__func__);       \
  Serial.print("} ");        \
  Serial.println(s);            \
} while(0)
#else
#define D_TRACE(s)
#endif // DEBUG_MODE

#endif // MACROS_H
