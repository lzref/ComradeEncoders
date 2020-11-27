#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <Arduino.h>

//#define NO_DEBUG

#ifndef NO_DEBUG
    #define DBG_INIT() Serial1.begin(115200)
    #define DBG(message) Serial1.print(message)
    #define DBGF(message, format) Serial1.print(message, format)
    #define DBGL(message) Serial1.println(message)
    #define DBGFL(message, format) Serial1.println(message, format)
#else
    #define DBG_INIT() 1
    #define DBG(message) 1
    #define DBGF(message, format) 1
    #define DBGL(message) 1
    #define DBGFL(message, format) 1
#endif

#endif