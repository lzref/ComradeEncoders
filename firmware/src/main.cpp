#include "USBComposite.h"
#include "usb_midi_device.h"
#include <Arduino.h>

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
        Serial1.print("\"");
        Serial1.print(displayStrings[i][j].c_str());
        Serial1.println("\"");
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
  //delay(300);

  Serial1.begin(115200);
  Serial1.println("Setup");

  midi.registerComponent();
  CompositeSerial.registerComponent();
  USBComposite.begin();

  Serial1.println("setup done");
}

void loop() {
  //Serial1.println("loop");
  midi.poll();
}