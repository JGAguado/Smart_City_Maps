/*****************************************************************************
* | File      	:   epd7in3f.h
* | Author      :   Waveshare team (adapted for Smart Dashboard)
* | Function    :   7.3inch e-paper F display driver
* | Info        :   Adapted for ESP32-S2 with custom GPIO configuration
*----------------
* |	This version:   V1.1
* | Date        :   2025-08-03
* | Info        :   Modified for Smart Dashboard project
******************************************************************************/

#ifndef __EPD_7IN3F_H__
#define __EPD_7IN3F_H__

#include <Arduino.h>
#include <SPI.h>
#include "config.h"

// Display resolution
#define EPD_WIDTH       800
#define EPD_HEIGHT      480

#define UWORD   unsigned int
#define UBYTE   unsigned char
#define UDOUBLE  unsigned long

/**********************************
Color Index
**********************************/
#define EPD_7IN3F_BLACK   0x0	/// 000
#define EPD_7IN3F_WHITE   0x1	///	001
#define EPD_7IN3F_GREEN   0x2	///	010
#define EPD_7IN3F_BLUE    0x3	///	011
#define EPD_7IN3F_RED     0x4	///	100
#define EPD_7IN3F_YELLOW  0x5	///	101
#define EPD_7IN3F_ORANGE  0x6	///	110
#define EPD_7IN3F_CLEAN   0x7	///	111   unavailable  Afterimage

class EPD7in3f {
public:
    EPD7in3f();
    ~EPD7in3f();
    
    int init(void);
    void reset(void);
    void sleep(void);
    void clear(UBYTE color);
    void display(const UBYTE *image);
    void displayPart(const UBYTE *image, UWORD xstart, UWORD ystart, 
                     UWORD image_width, UWORD image_height);
    void showColorBlocks(void);
    void turnOnDisplay(void);
    void busyHigh(void);
    
    // Low level functions
    void sendCommand(unsigned char command);
    void sendData(unsigned char data);
    void digitalWrite(int pin, int value);
    int digitalRead(int pin);
    void delayMs(unsigned int delaytime);
    void spiTransfer(unsigned char data);

private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;
    unsigned int din_pin;
    unsigned int sck_pin;
    unsigned long width;
    unsigned long height;
    
    int ifInit(void);
};

#endif /* __EPD_7IN3F_H__ */
