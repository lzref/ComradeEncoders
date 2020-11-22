#include <Arduino.h>
#include "display.h"
#include "midi.h"

Display display;

void onParamRefreshHandler(DisplayStringsT displayStrings)
{
  for (int i = 0; i < displayHeight; i++) {
    for (int j = 0; j < displayWidth; j++) {
      display.printmsg(j, i, displayStrings[i][j].c_str());
    }
  }
}

myMidi midi;

void setup() {
  Serial1.begin(115200);
  Serial1.println("Setup");

  midi.setup();

  display.setup();
  
  midi.setOnParamRefreshHandler(onParamRefreshHandler);

  Serial1.println("Setup done");
}

void loop() {
  midi.poll();
}