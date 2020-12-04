#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <Arduino.h>
#include <USBComposite.h>

//#define NO_DEBUG

extern USBCompositeSerial CompositeSerial;

#ifndef NO_DEBUG
    #define DBG_INIT() CompositeSerial.begin()
    #define DBG(message) CompositeSerial.print(message)
    #define DBGF(message, format) CompositeSerial.print(message, format)
    #define DBGL(message) CompositeSerial.println(message)
    #define DBGFL(message, format) CompositeSerial.println(message, format)
#else
    #define DBG_INIT() 1
    #define DBG(message) 1
    #define DBGF(message, format) 1
    #define DBGL(message) 1
    #define DBGFL(message, format) 1
#endif

#endif