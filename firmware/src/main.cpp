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

void setup()
{
  Serial1.begin(115200);
  Serial1.println("Setup");

  midi.setup();
  display.setup();

  midi.setOnSysexDataHandler(onSysexData);
  midi.setOnSysexEndHandler(onSysexEnd);
  sysexParser.setOnParamRefreshHandler(onParamRefreshHandler);

  Serial1.println("Setup done");
}

void loop()
{
  midi.poll();
}