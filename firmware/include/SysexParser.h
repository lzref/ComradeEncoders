#ifndef _SYSEX_PARSER_H_
#define _SYSEX_PARSER_H_

#include <Arduino.h>

const int SpecialMsgHeaderSize = 8;
const int displayWidth = 9;
const int displayHeight = 2;

typedef String DisplayStringsT[displayHeight][displayWidth];
typedef void (*onParamRefreshHandlerT)(DisplayStringsT displayStrings);

class SysexParser
{
    onParamRefreshHandlerT onParamRefreshHandler = 0;

    bool SpecialMsgEncountered = false;
    bool SpecialMsgHeaderSuccessful = false;
    bool readingText = false;
    int SpecialMsgHeaderPos = 0;

    uint8 SpecialMsgHeader[SpecialMsgHeaderSize] = {240, 0, 32, 41, 2, 10, 1, 2};

    int bytesSkipped = 0;
    String text;

    int hPosition = 0;
    int vPosition = 0;

    DisplayStringsT displayStrings;

    void clearState()
    {
        SpecialMsgEncountered = false;
        SpecialMsgHeaderPos = 0;
        SpecialMsgHeaderSuccessful = false;
        bytesSkipped = 0;
        readingText = false;
        text = "";
        hPosition = 0;
        vPosition = 0;
    }

public:
    void setOnParamRefreshHandler(onParamRefreshHandlerT handler)
    {
        onParamRefreshHandler = handler;
    }

    virtual void handleSysExEnd(void)
    {
        Serial1.println("SysExEnd");

        if (onParamRefreshHandler)
        {
            onParamRefreshHandler(displayStrings);
        }

        clearState();
    }

    virtual void handleSysExData(unsigned char data)
    {
        Serial1.print("SysExData ");
        Serial1.println(data, HEX);

        if (readingText)
        {
            if (0 == data)
            {
                Serial1.print("Finished reading text: ");
                Serial1.println(text.c_str());

                displayStrings[vPosition][hPosition] = text;

                SpecialMsgHeaderSuccessful = true;
                bytesSkipped = 0;
                readingText = false;
                text = "";
            }
            else
            {
                char data2 = data;
                String tmpStr(data2);

                text = text + tmpStr;
            }
        }
        else if (SpecialMsgHeaderSuccessful)
        {
            switch (bytesSkipped)
            {
            case 0:
                if (data >= displayWidth)
                {
                    Serial1.print("Invalid hPosition: ");
                    Serial1.println(data);
                    clearState();
                }
                else
                {
                    hPosition = data;
                }
                break;

            case 1:
                if (1 != data)
                {
                    Serial1.print("Expected text property byte (0x01). Got: ");
                    Serial1.println(data);
                    clearState();
                }
                break;

            default:
                if (data >= displayHeight)
                {
                    Serial1.print("Invalid vPosition: ");
                    Serial1.println(data);
                    clearState();
                }
                else
                {
                    vPosition = data;
                }

                Serial1.println("Skipping finished. Reading text...");
                readingText = true;
                text = "";
                break;
            }

            bytesSkipped++;
        }
        else if (!SpecialMsgEncountered && data == SpecialMsgHeader[0])
        {
            Serial1.println("Detected first byte!");
            SpecialMsgEncountered = true;
            SpecialMsgHeaderPos++;
        }
        else if (SpecialMsgEncountered)
        {
            if (data == SpecialMsgHeader[SpecialMsgHeaderPos])
            {
                SpecialMsgHeaderPos++;

                Serial1.println("Advancing header cursor!");

                if (SpecialMsgHeaderPos > SpecialMsgHeaderSize - 1)
                {
                    SpecialMsgHeaderPos = 0;
                    bytesSkipped = 0;
                    SpecialMsgHeaderSuccessful = true;
                    Serial1.println("Finished reading header!");
                }
            }
            else
            {
                Serial1.print("Unknown message encountered. Expected: ");
                Serial1.println(SpecialMsgHeader[SpecialMsgHeaderPos]);
                clearState();
            }
        }
    }
};

#endif