#ifndef _MIDI_H_
#define _MIDI_H_

#include <Arduino.h>
#include "USBComposite.h"
#include "usb_midi_device.h"
#include "SysexParser.h"

union EVENT_t
{
    uint32 i;
    uint8 b[4];
    MIDI_EVENT_PACKET_t p;
};

typedef void (*onSysexDataHandlerT)(unsigned char data);
typedef void (*onSysexEndHandlerT)(void);
typedef void (*onCcHandlerT)(unsigned int channel, unsigned int controller, unsigned int value);

class myMidi : public USBMIDI
{
    onSysexDataHandlerT onSysexDataHandler = 0;
    onSysexEndHandlerT onSysexEndHandler = 0;
    onCcHandlerT onCcHandler = 0;

    USBCompositeSerial CompositeSerial;

    virtual void handleNoteOff(unsigned int channel, unsigned int note, unsigned int velocity)
    {
        Serial1.println("noteOff");
    }

    virtual void handleNoteOn(unsigned int channel, unsigned int note, unsigned int velocity)
    {
        Serial1.println("NoteOn");
    }

    void handleControlChange(unsigned int channel, unsigned int controller, unsigned int value) {
        if (onCcHandler) {
            onCcHandler(channel, controller, value);
        }
    }

    virtual void handleSysExEnd(void)
    {
        if (onSysexEndHandler)
        {
            onSysexEndHandler();
        }
    }

    virtual void handleSysExData(unsigned char data)
    {
        if (onSysexDataHandler)
        {
            onSysexDataHandler(data);
        }
    }

public:
    void setOnSysexEndHandler(onSysexEndHandlerT handler)
    {
        onSysexEndHandler = handler;
    }

    void setOnSysexDataHandler(onSysexDataHandlerT handler)
    {
        onSysexDataHandler = handler;
    }
    
    void setOnCcHandler(onCcHandlerT handler)
    {
        onCcHandler = handler;
    }

    void setup()
    {
        registerComponent();
        CompositeSerial.registerComponent();
        USBComposite.begin();
    }

    void dispatchPacket(uint32 p)
    {
        union EVENT_t e;

        e.i = p;
        switch (e.p.cin)
        {
        case CIN_SYSEX:
            //Serial1.println("SysEx");
            handleSysExData(e.p.midi0);
            handleSysExData(e.p.midi1);
            handleSysExData(e.p.midi2);
            break;
        case CIN_SYSEX_ENDS_IN_1:
            //Serial1.println("SysEx1");
            handleSysExData(e.p.midi0);
            handleSysExEnd();
            break;
        case CIN_SYSEX_ENDS_IN_2:
            //Serial1.println("SysEx2");
            handleSysExData(e.p.midi0);
            handleSysExData(e.p.midi1);
            handleSysExEnd();
            break;
        case CIN_SYSEX_ENDS_IN_3:
            //Serial1.println("SysEx3");
            handleSysExData(e.p.midi0);
            handleSysExData(e.p.midi1);
            handleSysExData(e.p.midi2);
            handleSysExEnd();
            break;
        case CIN_3BYTE_SYS_COMMON:
            Serial1.println("CIN_3BYTE_SYS_COMMON");
            if (e.p.midi0 == MIDIv1_SONG_POSITION_PTR)
            {
                handleSongPosition(((uint16)e.p.midi2) << 7 | ((uint16)e.p.midi1));
            }
            break;

        case CIN_2BYTE_SYS_COMMON:
            Serial1.println("CIN_2BYTE_SYS_COMMON");
            switch (e.p.midi0)
            {
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
            handlePitchChange(((uint16)e.p.midi2) << 7 | ((uint16)e.p.midi1));
            break;
        case CIN_1BYTE:
            switch (e.p.midi0)
            {
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
        while (available())
        {
            dispatchPacket(readPacket());
        }
    }
};

#endif