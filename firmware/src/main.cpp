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
    MidiEncoder(21, PB1, PB10, PB11),
    MidiEncoder(22, PA15, PB4, PB12),
    MidiEncoder(23, PA10, PB13, PA9),
    MidiEncoder(24, PC14, PC15, PC13),
    MidiEncoder(25, PB6, PB8, PB0),
    MidiEncoder(26, PB9, PB7, PB5),
    MidiEncoder(27, PA8, PB15, PB14),
    MidiEncoder(28, PA1, PA2, PA0),
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

void setup()
{
  USBComposite.clear();
  midi.setup();
  CompositeSerial.registerComponent();
  USBComposite.begin();

  //delay(5000);

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

  DBGL("...hooks are set up. Setup is finished");
}

void serviceEncoders()
{
  for (int i = 0; i < numEncoders; i++)
  {
    encoders[i].service();
  }
}

int lastPin1 = HIGH;
int lastPin2 = HIGH;

void loop()
{
  midi.poll();

  serviceEncoders();
}