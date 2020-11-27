#include <Arduino.h>
#include "Display.h"
#include "Midi.h"
#include "SysexParser.h"

#define WITHOUT_BUTTON
#define ENC_DECODER ENC_FLAKY
//#define ENC_HALFSTEP
#include <ClickEncoder.h>

Display display;
myMidi midi;
SysexParser sysexParser;

ClickEncoder encoder(PB13, PB14, PB15);

int encoder0Value = 0;

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
    DBG("Encoder value: ");
    DBG(controller - encoder0Cc);
    DBG(" = ");
    DBGL(value);

    display.showValue(controller - encoder0Cc, value);
    encoder0Value = value;
  }
}


void timerIsr() {
  encoder.service();
}

void setup()
{
  DBG_INIT();
  DBGL("Setup");

  midi.setup();
  display.setup();

  midi.setOnSysexDataHandler(onSysexData);
  midi.setOnSysexEndHandler(onSysexEnd);
  midi.setOnCcHandler(onCc);
  sysexParser.setOnTextRefreshHandler(onTextRefreshHandler);

  //attachInterrupt(digitalPinToInterrupt(PB13), timerIsr, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(PB14), timerIsr, CHANGE);

  DBGL("Setup done");
}

void loop()
{
  midi.poll();

  encoder.service();

  int encoder0Delta = encoder.getValue();


  int encoder0Relative = encoder0Delta >= 0 ? encoder0Delta : 128 + encoder0Delta;

  if (encoder0Delta != 0) {
    midi.sendControlChange(15, encoder0Cc, encoder0Relative);
    DBG("Sending encoder val: ");
    DBGL(encoder0Relative);
  }
}