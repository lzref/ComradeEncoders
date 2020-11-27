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

void onParamRefreshHandler(DisplayStringsT displayStrings)
{
  for (int i = 0; i < displayHeight; i++)
  {
    for (int j = 0; j < displayWidth; j++)
    {
      display.printmsg(j, i, displayStrings[i][j].c_str());
    }
  }
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
    Serial1.print("Encoder value: ");
    Serial1.print(controller - encoder0Cc);
    Serial1.print(" = ");
    Serial1.println(value);

    display.showValue(controller - encoder0Cc, value);
    encoder0Value = value;
  }
}


void timerIsr() {
  encoder.service();
}

void setup()
{
  Serial1.begin(115200);
  Serial1.println("Setup");

  midi.setup();
  display.setup();

  midi.setOnSysexDataHandler(onSysexData);
  midi.setOnSysexEndHandler(onSysexEnd);
  midi.setOnCcHandler(onCc);
  sysexParser.setOnParamRefreshHandler(onParamRefreshHandler);

  //attachInterrupt(digitalPinToInterrupt(PB13), timerIsr, CHANGE);
  //attachInterrupt(digitalPinToInterrupt(PB14), timerIsr, CHANGE);

  Serial1.println("Setup done");
}

void loop()
{
  midi.poll();

  encoder.service();

  int encoder0Delta = encoder.getValue();


  int encoder0Relative = encoder0Delta >= 0 ? encoder0Delta : 128 + encoder0Delta;

  if (encoder0Delta != 0) {
    midi.sendControlChange(15, encoder0Cc, encoder0Relative);
    Serial1.print("Sending encoder val: ");
    Serial1.println(encoder0Relative);
  }
}