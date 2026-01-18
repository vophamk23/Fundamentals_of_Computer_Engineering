/*
 *    LedControl.h - A library for controling Leds with a MAX7219/MAX7221
 *    Copyright (c) 2007 Eberhard Fahle
 *
 *    Permission is hereby granted, free of charge, to any person
 *    obtaining a copy of this software and associated documentation
 *    files (the "Software"), to deal in the Software without
 *    restriction, including without limitation the rights to use,
 *    copy, modify, merge, publish, distribute, sublicense, and/or sell
 *    copies of the Software, and to permit persons to whom the
 *    Software is furnished to do so, subject to the following
 *    conditions:
 *
 *    This permission notice shall be included in all copies or
 *    substantial portions of the Software.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *    OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef LedControl_h
#define LedControl_h

#include <Arduino.h>

#if defined(__AVR__)
#include <avr/pgmspace.h>
#elif defined(ESP8266) || defined(ESP32)
#include <pgmspace.h>
#else
#include <pgmspace.h>
#endif

class LedControl
{
private:
    byte spidata[16];
    void spiTransfer(int addr, byte opcode, byte data);
    byte status[64];
    int SPI_MOSI;
    int SPI_CLK;
    int SPI_CS;
    int maxDevices;

public:
    LedControl(int dataPin, int clkPin, int csPin, int numDevices = 1);
    int getDeviceCount();
    void shutdown(int addr, bool status);
    void setScanLimit(int addr, int limit);
    void setIntensity(int addr, int intensity);
    void clearDisplay(int addr);
    void setLed(int addr, int row, int col, boolean state);
    void setRow(int addr, int row, byte value);
    void setColumn(int addr, int col, byte value);
    void setDigit(int addr, int digit, byte value, boolean dp);
    void setChar(int addr, int digit, char value, boolean dp);
};

#endif