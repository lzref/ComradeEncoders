#ifndef _MIDI_ENCODER_H_
#define _MIDI_ENCODER_H_

#include <Arduino.h>
#include "Debug.h"

//#define ENC_DECODER ENC_FLAKY
//#define ENC_HALFSTEP
#include <ClickEncoder.h>

typedef void (*onEncoderTurnedHandlerT)(unsigned int midiCc, int delta);
typedef void (*onEncoderPressedHandlerT)(unsigned int midiCc);
typedef void (*onEncoderReleasedHandlerT)(unsigned int midiCc);

class MidiEncoder
{
    onEncoderTurnedHandlerT onEncoderTurnedHandler = 0;
    onEncoderPressedHandlerT onEncoderPressedHandler = 0;
    onEncoderReleasedHandlerT onEncoderReleasedHandler = 0;

public:
    MidiEncoder(int cc, int pinA, int pinB, int pinButton) : physicalEncoder(pinA, pinB, -1), midiCc(cc), buttonPin(pinButton)
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

    void service()
    {
        physicalEncoder.service();
    }

    void checkForTurns()
    {
        int encoderDelta = physicalEncoder.getValue();
        if (encoderDelta != 0)
        {
            if (onEncoderTurnedHandler) {
                onEncoderTurnedHandler(midiCc, encoderDelta);
            }
        }
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
    ClickEncoder physicalEncoder;
    int midiCc;
    int buttonPin;
    int lastButtonState = HIGH;
    int lastButtonEventTime = 0;
};

#endif