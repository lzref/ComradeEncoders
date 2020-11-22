#include <Arduino.h>
#include "Display.h"
#include "Midi.h"
#include "SysexParser.h"

Display display;
myMidi midi;
SysexParser sysexParser;

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
  }
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

  Serial1.println("Setup done");
}

void loop()
{
  midi.poll();
}