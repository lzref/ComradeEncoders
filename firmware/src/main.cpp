#include "USBComposite.h"
#include "usb_midi_device.h"
#include <Arduino.h>


//LCD pins  |D7 |D6 |D5 |D4 |D3 |D2 |D1 |D0 | |RD |WR |RS |CS |RST| |SD_SS|SD_DI|SD_DO|SD_SCK|
//STM32 pin |PA7|PA6|PA5|PA4|PA3|PA2|PA1|PA0| |PB0|PB6|PB7|PB8|PB9| |PA15 |PB5  |PB4  |PB3   | **ALT-SPI1**


#define LCD_CS PB8
#define LCD_CD PB7
#define LCD_WR PB6
#define LCD_RD PB0
#define LCD_RESET PB9

#include <SPI.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

int ledPin = PC13;

union EVENT_t {
    uint32 i;
    uint8 b[4];
    MIDI_EVENT_PACKET_t p;
};

bool SpecialMsgEncountered = false;
bool SpecialMsgHeaderSuccessful = false;
bool readingText = false;
int SpecialMsgHeaderPos = 0;

const int SpecialMsgHeaderSize = 8;
uint8 SpecialMsgHeader[SpecialMsgHeaderSize] = {240, 0, 32, 41, 2, 10, 1, 2};

int bytesSkipped = 0;
String text;

int hPosition = 0;
int vPosition = 0;

const int displayWidth = 9;
const int displayHeight = 2;
String displayStrings[displayHeight][displayWidth];

void printmsg(int x, int y, const char *msg)
{
  const int xDist = 120;
  const int yDist = 50;
  const int yValDist = 30;

  int xout = 2 + xDist * (x % 4);
  int yout = 2 + yDist * (2 * (x / 4)) + yValDist * y;

  tft.fillRect(xout, yout, 114, 25, BLACK);

  tft.setTextSize(2);
  tft.setTextColor(YELLOW, BLACK);
  tft.setCursor(xout + 3, yout + 3);
  tft.println(msg);
}

class myMidi : public USBMIDI {
  virtual void handleNoteOff(unsigned int channel, unsigned int note, unsigned int velocity) {
    Serial1.println("noteOff");
  }

  virtual void handleNoteOn(unsigned int channel, unsigned int note, unsigned int velocity) {
    Serial1.println("NoteOn");
  }

  
  void clearState() {
    SpecialMsgEncountered = false;
    SpecialMsgHeaderPos = 0;
    SpecialMsgHeaderSuccessful = false;
    bytesSkipped = 0;
    readingText = false;
    text = "";
    hPosition = 0;
    vPosition = 0;
  }

  virtual void handleSysExEnd(void) {
    Serial1.println("SysExEnd");

    Serial1.println("Display:");
    for (int i = 0; i < displayHeight; i++) {
      for (int j = 0; j < displayWidth; j++) {
        printmsg(j, i, displayStrings[i][j].c_str());
      }
    }

    clearState();
  }

  virtual void handleSysExData(unsigned char data) {
    Serial1.print("SysExData ");
    Serial1.println(data, HEX);
    
    if (readingText) {
      if (0 == data) {
        Serial1.print("Finished reading text: ");
        Serial1.println(text.c_str());

        displayStrings[vPosition][hPosition] = text;
        
        SpecialMsgHeaderSuccessful = true;
        bytesSkipped = 0;
        readingText = false;
        text = "";
      } else {
        char data2 = data;
        String tmpStr(data2);
        
        text = text + tmpStr;
      }
    } else if (SpecialMsgHeaderSuccessful) {
      switch (bytesSkipped) {
        case 0:
          if (data >= displayWidth) {
            Serial1.print("Invalid hPosition: ");
            Serial1.println(data);
            clearState();
          } else {
            hPosition = data;
          }
          break;
          
        case 1:
          if (1 != data) {
            Serial1.print("Expected text property byte (0x01). Got: ");
            Serial1.println(data);
            clearState();
          }
          break;
          
        default:
          if (data >= displayHeight) {
            Serial1.print("Invalid vPosition: ");
            Serial1.println(data);
            clearState();
          } else {
            vPosition = data;
          }

          Serial1.println("Skipping finished. Reading text...");
          readingText = true;
          text = "";
          break;
      }

      bytesSkipped++;
      
    } else if (!SpecialMsgEncountered && data == SpecialMsgHeader[0]) {
      Serial1.println("Detected first byte!");
      SpecialMsgEncountered = true;
      SpecialMsgHeaderPos++;
    } else if (SpecialMsgEncountered) {
      if (data == SpecialMsgHeader[SpecialMsgHeaderPos]) {
        SpecialMsgHeaderPos++;
  
        Serial1.println("Advancing header cursor!");
  
        if (SpecialMsgHeaderPos > SpecialMsgHeaderSize - 1) {
          SpecialMsgHeaderPos = 0;
          bytesSkipped = 0;
          SpecialMsgHeaderSuccessful = true;
          Serial1.println("Finished reading header!");
        }
      }  else {
        Serial1.print("Unknown message encountered. Expected: ");
        Serial1.println(SpecialMsgHeader[SpecialMsgHeaderPos]);
        clearState();
      }
    }
  }
  
public:

  void dispatchPacket2(uint32 p)
  {
    union EVENT_t e;

    e.i=p;
    switch (e.p.cin) {
        case CIN_SYSEX:
            Serial1.println("SysEx");
            handleSysExData(e.p.midi0);
            handleSysExData(e.p.midi1);
            handleSysExData(e.p.midi2);
            break;
        case CIN_SYSEX_ENDS_IN_1:
            Serial1.println("SysEx1");
            handleSysExData(e.p.midi0);
            handleSysExEnd();
            break;
        case CIN_SYSEX_ENDS_IN_2:
            Serial1.println("SysEx2");
            handleSysExData(e.p.midi0);
            handleSysExData(e.p.midi1);
            handleSysExEnd();
            break;
        case CIN_SYSEX_ENDS_IN_3:
            Serial1.println("SysEx3");
            handleSysExData(e.p.midi0);
            handleSysExData(e.p.midi1);
            handleSysExData(e.p.midi2);
            handleSysExEnd();
            break;
        case CIN_3BYTE_SYS_COMMON:
            Serial1.println("CIN_3BYTE_SYS_COMMON");
            if (e.p.midi0 == MIDIv1_SONG_POSITION_PTR) {
                handleSongPosition(((uint16)e.p.midi2)<<7|((uint16)e.p.midi1));
            }
            break;

        case CIN_2BYTE_SYS_COMMON:
            Serial1.println("CIN_2BYTE_SYS_COMMON");
             switch (e.p.midi0) {
                 case MIDIv1_SONG_SELECT:
                     handleSongSelect(e.p.midi1);
                     break;
                 case MIDIv1_MTC_QUARTER_FRAME:
                     // reference library doesnt handle quarter frame.
                     break;
             }
            break;
        case CIN_NOTE_OFF:
            handleNoteOff(MIDIv1_VOICE_CHANNEL(e.p.midi0), e.p.midi1, e.p.midi2);
            break;
        case CIN_NOTE_ON:
            handleNoteOn(MIDIv1_VOICE_CHANNEL(e.p.midi0), e.p.midi1, e.p.midi2);
            break;
        case CIN_AFTER_TOUCH:
            handleVelocityChange(MIDIv1_VOICE_CHANNEL(e.p.midi0), e.p.midi1, e.p.midi2);
            break;
        case CIN_CONTROL_CHANGE:
            handleControlChange(MIDIv1_VOICE_CHANNEL(e.p.midi0), e.p.midi1, e.p.midi2);
            break;
        case CIN_PROGRAM_CHANGE:
            handleProgramChange(MIDIv1_VOICE_CHANNEL(e.p.midi0), e.p.midi1);
            break;
        case CIN_CHANNEL_PRESSURE:
            handleAfterTouch(MIDIv1_VOICE_CHANNEL(e.p.midi0), e.p.midi1);
            break;

        case CIN_PITCH_WHEEL:
            handlePitchChange(((uint16)e.p.midi2)<<7|((uint16)e.p.midi1));
            break;
        case CIN_1BYTE:
            switch (e.p.midi0) {
                case MIDIv1_CLOCK:
                    handleSync();
                    break;
                case MIDIv1_TICK:
                    break;
                case MIDIv1_START:
                    handleStart();
                    break;
                case MIDIv1_CONTINUE:
                    handleContinue();
                    break;
                case MIDIv1_STOP:
                    handleStop();
                    break;
                case MIDIv1_ACTIVE_SENSE:
                    handleActiveSense();
                    break;
                case MIDIv1_RESET:
                    handleReset();
                    break;
                case MIDIv1_TUNE_REQUEST:
                    handleTuneRequest();
                    break;

                default:
                Serial1.print("Invalid CIN_1BYTE packet ");
                Serial1.println(e.p.midi0);
                    break;
            }
            break;

            default:
              Serial1.print("Invalid SysEx packet ");
              Serial1.println(e.p.cin);
            break;
      }
  }

  // Try to read data from USB port & pass anything read to processing function
  void poll(void)
  {
       while(available()) {
          dispatchPacket2(readPacket());
      }
  }
};

myMidi midi;
USBCompositeSerial CompositeSerial;

void setup() {
  Serial1.begin(115200);
  Serial1.println("Setup");

  midi.registerComponent();
  CompositeSerial.registerComponent();
  USBComposite.begin();

  uint16_t ID = tft.readID();
  Serial1.print("ID = 0x");
  Serial1.println(ID, HEX);
  if (ID == 0xD3D3) ID = 0x9481; // write-only shield
  tft.begin(ID);

  tft.setRotation(1);
  tft.invertDisplay(true);
  tft.fillScreen(WHITE);

  Serial1.println("setup done");
}

void loop() {
  //Serial1.println("loop");
  midi.poll();
}