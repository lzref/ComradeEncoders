#ifndef _SYSEX_PARSER_H_
#define _SYSEX_PARSER_H_

#include <Arduino.h>
#include "Debug.h"

const int SpecialMsgHeaderSize = 8;
const int displayWidth = 9;
const int displayHeight = 2;

typedef String DisplayStringsT[displayHeight][displayWidth];
typedef void (*onTextRefreshHandlerT)(int hPosition, int vPosition, String text);
typedef uint8_t ColorIndexT;

class SysexParser
{
    onTextRefreshHandlerT onTextRefreshHandler = 0;

    bool SpecialMsgEncountered = false;
    bool SpecialMsgHeaderSuccessful = false;
    bool readingText = false;
    bool readingColor = false;
    bool readingValue = false;
    int SpecialMsgHeaderPos = 0;
    int propertyType = 0;

    uint8 SpecialMsgHeader[SpecialMsgHeaderSize] = {240, 0, 32, 41, 2, 10, 1, 2};

    int bytesSkipped = 0;
    String text;

    int hPosition = 0;
    int vPosition = 0;

    DisplayStringsT displayStrings;

    ColorIndexT displayColors[displayHeight][displayWidth];

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
        readingColor = false;
        readingValue = false;
        propertyType = 0;
    }

public:

    void setOnTextRefreshHandler(onTextRefreshHandlerT handler)
    {
        onTextRefreshHandler = handler;
    }

    virtual void handleSysExEnd(void)
    {
        DBGL("SysExEnd");

        DBGL("Colors:");
        for (int i = 0; i < displayHeight; i++) {
            for (int j = 0; j < displayWidth; j++) {
                DBG(displayColors[i][j]);
                DBG(" ");
            }
            DBGL(" ");
        }

        clearState();
    }

    virtual void handleSysExData(unsigned char data)
    {
        DBG("SysExData ");
        DBGFL(data, HEX);

        if (readingText)
        {
            if (0 == data)
            {
                if (hPosition >= displayWidth || vPosition >= displayHeight) {
                    DBG("Ignoring text property with invalid position (");
                    DBG(hPosition);
                    DBG(", ");
                    DBG(vPosition);
                    DBG("): ");
                    DBGL(text.c_str());
                } else {
                    DBG("Finished reading text property (");
                    DBG(hPosition);
                    DBG(", ");
                    DBG(vPosition);
                    DBG("): ");
                    DBGL(text.c_str());

                    displayStrings[vPosition][hPosition] = text;

                    if (onTextRefreshHandler) {
                        onTextRefreshHandler(hPosition, vPosition, text);
                    }
                }
                
                SpecialMsgHeaderSuccessful = true;
                bytesSkipped = 0;
                readingText = false;
                readingColor = false;
                readingValue = false;
                text = "";
            }
            else
            {
                char data2 = data;
                String tmpStr(data2);

                text = text + tmpStr;

                //DBG("Found character ");
                //DBG(tmpStr.c_str());
                //DBG("; text so far: ");
                //DBGL(text.c_str());
            }
        }
        else if (readingColor) {
            if (hPosition >= displayWidth || vPosition >= displayHeight) {
                DBG("Ignoring color property with invalid position (");
                DBG(hPosition);
                DBG(", ");
                DBG(vPosition);
                DBG("): ");
                DBGL(data);
            } else {
                DBG("Finished reading color property (");
                DBG(hPosition);
                DBG(", ");
                DBG(vPosition);
                DBG("): ");
                DBGL(data);

                displayColors[vPosition][hPosition] = data;
            }
            
            SpecialMsgHeaderSuccessful = true;
            bytesSkipped = 0;
            readingText = false;
            readingColor = false;
            readingValue = false;
            text = "";
        }
        else if (readingValue) {
            if (hPosition >= displayWidth || vPosition >= displayHeight) {
                DBG("Ignoring value property with invalid position (");
                DBG(hPosition);
                DBG(", ");
                DBG(vPosition);
                DBG("): ");
                DBGL(data);
            } else {
                DBG("Finished reading value property (");
                DBG(hPosition);
                DBG(", ");
                DBG(vPosition);
                DBG("): ");
                DBGL(data);
            }
            
            SpecialMsgHeaderSuccessful = true;
            bytesSkipped = 0;
            readingText = false;
            readingColor = false;
            readingValue = false;
            text = "";
        }
        else if (SpecialMsgHeaderSuccessful)
        {
            switch (bytesSkipped)
            {
            case 0:
                hPosition = data;
                break;

            case 1:
                switch (data) {
                    case 1: case 2: case 3:
                        propertyType = data;
                        break;

                    default:
                        DBG("Unexpected property type: ");
                        DBGL(data);
                        clearState();
                        break;

                }
                break;

            default:
                vPosition = data;

                switch (propertyType) {
                    case 1:
                        DBG("Skipping finished. Reading text (");
                        DBG(hPosition);
                        DBG(", ");
                        DBG(vPosition);
                        DBGL(")...");
                        readingText = true;
                        text = "";
                        break;

                    case 2:
                        DBG("Skipping finished. Reading color (");
                        DBG(hPosition);
                        DBG(", ");
                        DBG(vPosition);
                        DBGL(")...");
                        readingColor = true;
                        break;

                    case 3:
                        DBG("Skipping finished. Reading value (");
                        DBG(hPosition);
                        DBG(", ");
                        DBG(vPosition);
                        DBGL(")...");
                        readingValue = true;
                        break;

                    default:
                        DBG("Unexpected property type: ");
                        DBGL(data);
                        clearState();
                        break;
                }
                break;
            }

            bytesSkipped++;
        }
        else if (!SpecialMsgEncountered && data == SpecialMsgHeader[0])
        {
            DBGL("Detected first byte!");
            SpecialMsgEncountered = true;
            SpecialMsgHeaderPos++;
        }
        else if (SpecialMsgEncountered)
        {
            if (data == SpecialMsgHeader[SpecialMsgHeaderPos])
            {
                SpecialMsgHeaderPos++;

                //DBGL("Advancing header cursor!");

                if (SpecialMsgHeaderPos > SpecialMsgHeaderSize - 1)
                {
                    SpecialMsgHeaderPos = 0;
                    bytesSkipped = 0;
                    SpecialMsgHeaderSuccessful = true;
                    DBGL("Finished reading header!");
                }
            }
            else
            {
                DBG("Unknown message encountered. Expected: ");
                DBGL(SpecialMsgHeader[SpecialMsgHeaderPos]);
                clearState();
            }
        }
    }
};

#endif