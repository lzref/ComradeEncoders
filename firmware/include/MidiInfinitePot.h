#ifndef _MIDI_INFINITE_POT_H_
#define _MIDI_INFINITE_POT_H_

#include <Arduino.h>
#include "Debug.h"
#include "InfinitePot.h"

typedef void (*onEncoderTurnedHandlerT)(unsigned int midiCc, int delta);
typedef void (*onEncoderPressedHandlerT)(unsigned int midiCc);
typedef void (*onEncoderReleasedHandlerT)(unsigned int midiCc);

class MidiInfinitePot
{
    onEncoderTurnedHandlerT onEncoderTurnedHandler = 0;
    onEncoderPressedHandlerT onEncoderPressedHandler = 0;
    onEncoderReleasedHandlerT onEncoderReleasedHandler = 0;

public:
    MidiInfinitePot(int cc, int pinButton):
        midiCc(cc), buttonPin(pinButton)
    {
        pinMode(pinButton, INPUT_PULLUP);
    }

    void setOnEncoderTurnedHandler(onEncoderTurnedHandlerT handler)
    {
        onEncoderTurnedHandler = handler;
    }

    void setOnEncoderPressedHandler(onEncoderPressedHandlerT handler)
    {
        onEncoderPressedHandler = handler;
    }

    void setOnEncoderReleasedHandler(onEncoderReleasedHandlerT handler)
    {
        onEncoderReleasedHandler = handler;
    }

    void setup(int pinAValue, int pinBValue)
    {
        infinitePot.setup(pinAValue, pinBValue);
    }

    void checkForTurns(int pinAValue, int pinBValue)
    {
        int encoderDelta = infinitePot.update(pinAValue, pinBValue);
        if (encoderDelta != 0 && onEncoderTurnedHandler) {
            onEncoderTurnedHandler(midiCc, encoderDelta);
        }
    }

    int getValue()
    {
        return infinitePot.getValue();
    }

    void checkButtonState()
    {
        int buttonState = digitalRead(buttonPin);
        int curTime = millis();
        const int debouncePeriod = 20;

        if (buttonState != lastButtonState && curTime - lastButtonEventTime > debouncePeriod)
        {
            lastButtonState = buttonState;
            lastButtonEventTime = curTime;

            if (LOW == buttonState)
            {
                if (onEncoderPressedHandler) {
                    onEncoderPressedHandler(midiCc);
                }
            }
            else if (HIGH == buttonState)
            {
                if (onEncoderReleasedHandler) {
                    onEncoderReleasedHandler(midiCc);
                }
            }
        }
    }

protected:
    InfinitePot infinitePot;
    int midiCc;
    int buttonPin;
    int lastButtonState = HIGH;
    int lastButtonEventTime = 0;
};

#endif // _MIDI_INFINITE_POT_H_