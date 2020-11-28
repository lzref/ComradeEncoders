#include <Arduino.h>
#include "Display.h"
#include "Midi.h"
#include "SysexParser.h"

//#define ENC_DECODER ENC_FLAKY
//#define ENC_HALFSTEP
#include <ClickEncoder.h>

Display display;
myMidi midi;
SysexParser sysexParser;

class MidiEncoder
{
public:
  MidiEncoder(int cc, int pinA, int pinB, int pinButton):
    physicalEncoder(pinA, pinB, -1), midiCc(cc), buttonPin(pinButton)
  {
    pinMode(pinButton, INPUT_PULLUP);
  }

  void service()
  {
    physicalEncoder.service();

    int encoderDelta = physicalEncoder.getValue() * 5;
    int encoderRelative = encoderDelta >= 0 ? encoderDelta : 128 + encoderDelta;

    if (encoderDelta != 0) {
      midi.sendControlChange(15, midiCc, encoderRelative);
      DBG("Sending encoder ");
      DBG(midiCc);
      DBG(" val: ");
      DBGL(encoderRelative);
    }

    int buttonState = digitalRead(buttonPin);
    int curTime = millis();
    const int debouncePeriod = 20;

    if (buttonState != lastButtonState && curTime - lastButtonEventTime > debouncePeriod) {
      lastButtonState = buttonState;
      lastButtonEventTime = curTime;

      if (LOW == buttonState) {
        DBG("Button pressed: ");
        DBGL(midiCc);
        midi.sendNoteOn(15, midiCc, 127);
      } else if (HIGH == buttonState) {
        DBG("Button released: ");
        DBGL(midiCc);
        midi.sendNoteOff(15, midiCc, 0);
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

const int numEncoders = 8;

MidiEncoder encoders[numEncoders] = {
  MidiEncoder(21, PC13, PC14, PC15),
  MidiEncoder(22, PA0, PA1, PA2),
  MidiEncoder(23, PB0, PB1, PB10),
  MidiEncoder(24, PB12, PB13, PB11),
  MidiEncoder(25, PA8, PB15, PB14),
  MidiEncoder(26, PB3, PA15, PB2),
  MidiEncoder(27, PB6, PB5, PB4),
  MidiEncoder(28, PB9, PB8, PB7),
};

void onTextRefreshHandler(int hPosition, int vPosition, String text)
{
  display.printmsg(hPosition, vPosition, text.c_str());
}

void onSysexData(unsigned char data)
{
  sysexParser.handleSysExData(data);
}

void onSysexEnd()
{
  sysexParser.handleSysExEnd();
}

const int encoder0Cc = 21;

void onCc(unsigned int channel, unsigned int controller, unsigned int value)
{
  if (controller >= encoder0Cc && controller < encoder0Cc + 8) {
    int encoderIndex = controller - encoder0Cc;

    DBG("Received encoder value: ");
    DBG(encoderIndex);
    DBG(" = ");
    DBGL(value);

    display.showValue(encoderIndex, value);
  }
}

void setup()
{
  DBG_INIT();

  DBGL("Doing MIDI setup...");

  midi.setup();

  DBGL("...MIDI setup done. Doing display setup...");

  display.setup();

  DBGL("...display setup done. Setting up hooks...");

  midi.setOnSysexDataHandler(onSysexData);
  midi.setOnSysexEndHandler(onSysexEnd);
  midi.setOnCcHandler(onCc);
  sysexParser.setOnTextRefreshHandler(onTextRefreshHandler);

  DBGL("...hooks are set up. Setup is finished");

  pinMode(PB3, INPUT_PULLUP);
  pinMode(PA15, INPUT_PULLUP);
}

void serviceEncoders()
{
  for (int i = 0; i < numEncoders; i++) {
    encoders[i].service();
  }
}

int lastPin1 = HIGH;
int lastPin2 = HIGH;

void loop()
{
  midi.poll();

  serviceEncoders();
  //DBGL("loop");

  int pin1 = digitalRead(PB3);
  int pin2 = digitalRead(PA15);

  /*if (pin1 != lastPin1 || pin2 != lastPin2) {
    lastPin1 = pin1;
    lastPin2 = pin2;

    DBG("Encoder pins: ");
    DBG(pin1);
    DBGL(pin2);
  }*/
}