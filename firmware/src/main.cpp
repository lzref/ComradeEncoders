#include <Arduino.h>
#include "Display.h"
#include "Midi.h"
#include "SysexParser.h"
#include "MidiEncoder.h"
#include "MidiInfinitePot.h"
#include "AnalogMux.h"


Display display;
myMidi midi;
SysexParser sysexParser;
USBCompositeSerial CompositeSerial;
AnalogMux muxA(PB0, PA0, PC15, PC13, PC14);
AnalogMux muxB(PB1, PA0, PC15, PC13, PC14);

const int numEncoders = 2;
const int encoder0Cc = 21;

MidiInfinitePot encoders[numEncoders] = {
    MidiInfinitePot(21, PB11),
    MidiInfinitePot(22, PB12),
    /*MidiInfinitePot(23, PA9),
    MidiInfinitePot(24, PC13),
    MidiInfinitePot(25, PB0),
    MidiInfinitePot(26, PA1),
    MidiInfinitePot(27, PB14),
    MidiInfinitePot(28, PA0),*/
};

int encoderValues[numEncoders] = {0, 0/*, 0, 0, 0, 0, 0, 0*/};

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
  if (controller >= encoder0Cc && controller < encoder0Cc + numEncoders)
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
  DBG(" delta: ");
  DBGL(delta);
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

  DBGL("MIDI setup done. Doing display setup...");

  display.setup();

  DBGL("...display setup done. Setting up hooks...");

  midi.setOnSysexDataHandler(onSysexData);
  midi.setOnSysexEndHandler(onSysexEnd);
  midi.setOnCcHandler(onCc);
  sysexParser.setOnTextRefreshHandler(onTextRefreshHandler);

  muxA.setup();
  muxB.setup();

  for (int i = 0; i < numEncoders; i++) {
    encoders[i].setOnEncoderTurnedHandler(onEncoderTurnedHandler);
    encoders[i].setOnEncoderPressedHandler(onEncoderPressedHandler);
    encoders[i].setOnEncoderReleasedHandler(onEncoderReleasedHandler);

    muxA.selectChannel(i);
    int pinAValue = muxA.getSignal();
    int pinBValue = muxB.getSignal();

    DBG("Initial values for encoder ");
    DBG(i);
    DBG(":\t");
    DBG(pinAValue);
    DBG("\t");
    DBGL(pinBValue);

    encoders[i].setup(pinAValue, pinBValue);
  }

  DBGL("...hooks are set up. Setup is finished");
}

int lastPin1 = HIGH;
int lastPin2 = HIGH;

void loop()
{
  midi.poll();

  int encoderValues[numEncoders];
  int pinAValues[numEncoders];
  int pinBValues[numEncoders];
  
  for (int i = 0; i < numEncoders; i++) {
    muxA.selectChannel(i);
    delayMicroseconds(3000);

    int pinAValue = muxA.getSignal();
    int pinBValue = muxB.getSignal();

    pinAValues[i] = pinAValue;
    pinBValues[i] = pinBValue;

    encoders[i].checkForTurns(pinAValue, pinBValue);
    encoderValues[i] = encoders[i].getValue();
    //encoders[i].checkButtonState();
  }

  /*DBG("Encoder values: ");
  for (int i = 0; i < numEncoders; i++) {
    DBG("\t");
    DBG(pinAValues[i]);
    DBG("\t");
    DBG(pinBValues[i]);
    DBG("\t");
    DBG(encoderValues[i]);
  } 
  DBGL("");*/
}