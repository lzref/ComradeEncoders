#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include <MCUFRIEND_kbv.h>

//LCD pins  |D7 |D6 |D5 |D4 |D3 |D2 |D1 |D0 | |RD |WR |RS |CS |RST| |SD_SS|SD_DI|SD_DO|SD_SCK|
//STM32 pin |PA7|PA6|PA5|PA4|PA3|PA2|PA1|PA0| |PB0|PB6|PB7|PB8|PB9| |PA15 |PB5  |PB4  |PB3   | **ALT-SPI1**

#define LCD_CS PB8
#define LCD_CD PB7
#define LCD_WR PB6
#define LCD_RD PB0
#define LCD_RESET PB9

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

class Display: MCUFRIEND_kbv {
public:
    Display(): MCUFRIEND_kbv(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET) {
    }

    void printmsg(int x, int y, const char *msg)
    {
        const int xDist = 120;
        const int yDist = 50;
        const int yValDist = 30;

        int xout = 2 + xDist * (x % 4);
        int yout = 2 + yDist * (2 * (x / 4)) + yValDist * y;

        fillRect(xout, yout, 114, 25, BLACK);

        setTextSize(2);
        setTextColor(YELLOW, BLACK);
        setCursor(xout + 3, yout + 3);
        println(msg);
    }

    void setup() {
        uint16_t ID = readID();
        Serial1.print("ID = 0x");
        Serial1.println(ID, HEX);
        if (ID == 0xD3D3) ID = 0x9481; // write-only shield
        begin(ID);

        setRotation(1);
        invertDisplay(true);
        fillScreen(WHITE);
    }
};

#endif