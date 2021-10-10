#include "AnalogMux.h"

#define BIT0_MASK 1
#define BIT1_MASK 2
#define BIT2_MASK 4
#define BIT3_MASK 8

#define EXTRACT_BIT(number, bitPosition) (number >> bitPosition)

void AnalogMux::setup()
{
    pinMode(_signalPin, INPUT_ANALOG);
    pinMode(_pinA0, OUTPUT);
    pinMode(_pinA1, OUTPUT);
    pinMode(_pinA2, OUTPUT);
    pinMode(_pinA3, OUTPUT);
}

void AnalogMux::selectChannel(uint8_t channel)
{
    digitalWrite(_pinA0, (channel & BIT0_MASK) > 0 ? HIGH : LOW);
    digitalWrite(_pinA1, (channel & BIT1_MASK) > 0 ? HIGH : LOW);
    digitalWrite(_pinA2, (channel & BIT2_MASK) > 0 ? HIGH : LOW);
    digitalWrite(_pinA3, (channel & BIT3_MASK) > 0 ? HIGH : LOW);
}

int AnalogMux::read(uint8_t channel)
{
    digitalWrite(_pinA0, (channel & BIT0_MASK) > 0 ? HIGH : LOW);
    digitalWrite(_pinA1, (channel & BIT1_MASK) > 0 ? HIGH : LOW);
    digitalWrite(_pinA2, (channel & BIT2_MASK) > 0 ? HIGH : LOW);
    digitalWrite(_pinA3, (channel & BIT3_MASK) > 0 ? HIGH : LOW);

    return analogRead(_signalPin);
}

int AnalogMux::getSignal()
{
    return analogRead(_signalPin);
}