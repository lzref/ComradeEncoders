#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Debug.h"

#include <Adafruit_ILI9341.h>

#define TFT_DC PA3

#define TFT_CS 0

#define TFT_MOSI PA7
#define TFT_CLK PA5

#define TFT_RST 0
#define TFT_MISO PA6

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

const int xDist = 80;
const int yDist = 40;
const int yValDist = 20;
const int textSize = 1;
const int outerBoxSizeX = 76;
const int outerBoxSizeY = 19;
const int innerBoxSizeX = outerBoxSizeX - 4;
const int innerBoxSizeY = outerBoxSizeY - 4;


class Display: Adafruit_ILI9341 {
public:
    Display(): Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST) {
    }

    void printmsg(int x, int y, const char *msg)
    {
        int xout = 2 + xDist * (x % 4);
        int yout = 2 + yDist * (2 * (x / 4)) + yValDist * y;

        fillRect(xout, yout, outerBoxSizeX, outerBoxSizeY, BLACK);

        setTextSize(textSize);
        setTextColor(YELLOW, BLACK);
        setCursor(xout + 3, yout + 3);
        println(msg);
    }

    void showValue(int x, int value)
    {
        int xout = 2 + xDist * (x % 4);
        int yout = 2 + yDist * (2 * (x / 4)) + 2 * yValDist;

        fillRect(xout, yout, outerBoxSizeX, outerBoxSizeY, BLACK);
        fillRect(xout + 2, yout + 2, innerBoxSizeX * value / 127, innerBoxSizeY, YELLOW);
    }

    void setup() {
        begin();

        fillScreen(GREEN);
        setRotation(3);
        //invertDisplay(true);
    }
};

#endif