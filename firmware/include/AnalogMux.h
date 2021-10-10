#ifndef ANALOG_MUX_H
#define ANALOG_MUX_H

#include "Arduino.h"

class AnalogMux
{
    uint32_t _signalPin;
    uint32_t _pinA0;
    uint32_t _pinA1;
    uint32_t _pinA2;
    uint32_t _pinA3;
public:
    AnalogMux(uint32_t signalPin, uint32_t pinA0, uint32_t pinA1, uint32_t pinA2, uint32_t pinA3):
        _signalPin(signalPin), _pinA0(pinA0), _pinA1(pinA1), _pinA2(pinA2), _pinA3(pinA3) {};
    
    void setup();
    int read(uint8_t channel);
    void selectChannel(uint8_t channel);
    int getSignal();
};

#endif // ANALOG_MUX_H