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
const int yDist = 30;
const int yValDist = 14;
const int textSize = 1;
const int outerBoxSizeX = 76;
const int outerBoxSizeY = 14;
const int innerBoxSizeX = outerBoxSizeX - 4;
const int innerBoxSizeY = outerBoxSizeY - 4;


class Display: Adafruit_ILI9341 {
public:
    Display(): Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST) {
    }

    int boxCoordX(int x, int y)
    {
        return 2 + xDist * (x % 4);
    }

    int boxCoordY(int x, int y)
    {
        return 2 + yDist * (2 * (x / 4)) + yValDist * y;
    }

    String repeatString(String source, unsigned int count)
    {
        String result;
        for (unsigned int i = 0; i <= count; i++) {
            result += source;
        }

        return result;
    }

    void printMessage(int x, int y, String msg)
    {
        int xout = boxCoordX(x, y);
        int yout = boxCoordY(x, y);

        if (msg.length() < 9) {
            msg += repeatString(String(" "), 9 - msg.length());
        }

        setTextSize(textSize);
        setTextColor(YELLOW, BLACK);
        setCursor(xout + 3, yout + 3);
        println(msg.c_str());
    }

    void drawBoundingBox(int x, int y)
    {
        int xout = boxCoordX(x, y);
        int yout = boxCoordY(x, y);

        fillRect(xout, yout, outerBoxSizeX, outerBoxSizeY, BLACK);
    }

    void showValue(int x, int oldValue, int newValue)
    {
        int xout = boxCoordX(x, 2);
        int yout = boxCoordY(x, 2);

        int oldValueX = xout + 2 + (innerBoxSizeX * oldValue / 127);
        int newValueX = xout + 2 + (innerBoxSizeX * newValue / 127);

        if (newValue > oldValue) {
            fillRect(oldValueX, yout + 2, newValueX - oldValueX, innerBoxSizeY, YELLOW);
        } else {
            fillRect(newValueX, yout + 2, oldValueX - newValueX, innerBoxSizeY, BLACK);
        }
    }

    void setup() {
        begin();

        fillScreen(GREEN);
        setRotation(3);
        //invertDisplay(true);

        for (int i = 0; i < 16; i++) {
            drawBoundingBox(i, 0);
            drawBoundingBox(i, 1);
            drawBoundingBox(i, 2);
        }
    }
};

#endif