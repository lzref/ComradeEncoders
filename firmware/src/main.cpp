#include <Arduino.h>
#include "Display.h"
#include "Midi.h"
#include "SysexParser.h"
#include "MidiEncoder.h"


Display display;
myMidi midi;
SysexParser sysexParser;
USBCompositeSerial CompositeSerial;

const int numEncoders = 8;
const int encoder0Cc = 21;

MidiEncoder encoders[numEncoders] = {
    MidiEncoder(21, PB5, PB7, PB6),
    MidiEncoder(22, PB13, PB14, PB12),
    MidiEncoder(23, PB10, PB11, PB1),
    MidiEncoder(24, PB9, PC13, PB8),
    MidiEncoder(25, PA10, PA15, PB15),
    MidiEncoder(26, PA8, PA9, PB15),
    MidiEncoder(27, PA2, PA1, PB0),
    MidiEncoder(28, PC15, PA0, PC14),
};

int encoderValues[numEncoders] = {0, 0, 0, 0, 0, 0, 0, 0};

void onTextRefreshHandler(int hPosition, int vPosition, String text)
{
  display.printMessage(hPosition, vPosition, text);
}

void onSysexData(unsigned char data)
{
  sysexParser.handleSysExData(data);
}

void onSysexEnd()
{
  sysexParser.handleSysExEnd();
}

void onCc(unsigned int channel, unsigned int controller, unsigned int value)
{
  if (controller >= encoder0Cc && controller < encoder0Cc + 8)
  {
    int encoderIndex = controller - encoder0Cc;

    DBG("Received encoder value: ");
    DBG(encoderIndex);
    DBG(" = ");
    DBG(value);
    DBG(". Previous value: ");
    DBGL(encoderValues[encoderIndex]);

    display.showValue(encoderIndex, encoderValues[encoderIndex], value);

    encoderValues[encoderIndex] = value;
  }
}

void onEncoderTurnedHandler(unsigned int midiCc, int delta)
{
  int relativeValue = delta >= 0 ? delta : 128 + delta;

  midi.sendControlChange(15, midiCc, relativeValue);

  DBG("Sending encoder ");
  DBG(midiCc);
  DBG(" val: ");
  DBGL(relativeValue);
}

void onEncoderPressedHandler(unsigned int midiCc)
{
  DBG("Button pressed: ");
  DBGL(midiCc);
  midi.sendNoteOn(15, midiCc, 127);
}

void onEncoderReleasedHandler(unsigned int midiCc)
{
  DBG("Button released: ");
  DBGL(midiCc);
  midi.sendNoteOff(15, midiCc, 0);
}

void serviceEncoders()
{
  for (int i = 0; i < numEncoders; i++)
  {
    encoders[i].service();
  }
}

void setup()
{
  USBComposite.clear();
  midi.setup();
  CompositeSerial.registerComponent();
  USBComposite.begin();

  DBGL("MIDI setup done. Doing display setup...");

  display.setup();

  DBGL("...display setup done. Setting up hooks...");

  midi.setOnSysexDataHandler(onSysexData);
  midi.setOnSysexEndHandler(onSysexEnd);
  midi.setOnCcHandler(onCc);
  sysexParser.setOnTextRefreshHandler(onTextRefreshHandler);

  for (int i = 0; i < numEncoders; i++)
  {
    encoders[i].setOnEncoderTurnedHandler(onEncoderTurnedHandler);
    encoders[i].setOnEncoderPressedHandler(onEncoderPressedHandler);
    encoders[i].setOnEncoderReleasedHandler(onEncoderReleasedHandler);
  }

  Timer1.setPeriod(1000);
  Timer1.attachInterrupt(0, serviceEncoders); 

  DBGL("...hooks are set up. Setup is finished");
}

int lastPin1 = HIGH;
int lastPin2 = HIGH;

void loop()
{
  midi.poll();

  for (int i = 0; i < numEncoders; i++)
  {
    encoders[i].checkForTurns();
    encoders[i].checkButtonState();
  }
}