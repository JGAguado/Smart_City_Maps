/*****************************************************************************
* | File      	:   epd7in3f.cpp
* | Author      :   Waveshare team (adapted for Smart Dashboard)
* | Function    :   7.3inch e-paper F display driver implementation
* | Info        :   Adapted for ESP32-S2 with custom GPIO configuration
******************************************************************************/

#include "epd7in3f.h"

EPD7in3f::EPD7in3f() {
    reset_pin = EPD_RST_PIN;
    dc_pin = EPD_DC_PIN;
    cs_pin = EPD_CS_PIN;
    busy_pin = EPD_BUSY_PIN;
    din_pin = EPD_DIN_PIN;
    sck_pin = EPD_SCK_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
}

EPD7in3f::~EPD7in3f() {
}

void EPD7in3f::digitalWrite(int pin, int value) {
    ::digitalWrite(pin, value);
}

int EPD7in3f::digitalRead(int pin) {
    return ::digitalRead(pin);
}

void EPD7in3f::delayMs(unsigned int delaytime) {
    delay(delaytime);
}

void EPD7in3f::spiTransfer(unsigned char data) {
    digitalWrite(cs_pin, LOW);
    SPI.transfer(data);
    digitalWrite(cs_pin, HIGH);
}

int EPD7in3f::ifInit(void) {
    pinMode(cs_pin, OUTPUT);
    pinMode(reset_pin, OUTPUT);
    pinMode(dc_pin, OUTPUT);
    pinMode(busy_pin, INPUT); 
    
    // Initialize SPI with custom pins for ESP32-S2
    SPI.begin(sck_pin, -1, din_pin, cs_pin);  // SCK, MISO, MOSI, CS
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    
    return 0;
}

int EPD7in3f::init(void) {
    if (ifInit() != 0) {
        return -1;
    }
    
    reset();
    delayMs(20);
    busyHigh();
    
    // Initialize display with Waveshare commands
    sendCommand(0xAA);    // CMDH
    sendData(0x49);
    sendData(0x55);
    sendData(0x20);
    sendData(0x08);
    sendData(0x09);
    sendData(0x18);

    sendCommand(0x01);
    sendData(0x3F);
    sendData(0x00);
    sendData(0x32);
    sendData(0x2A);
    sendData(0x0E);
    sendData(0x2A);

    sendCommand(0x00);
    sendData(0x5F);
    sendData(0x69);

    sendCommand(0x03);
    sendData(0x00);
    sendData(0x54);
    sendData(0x00);
    sendData(0x44); 

    sendCommand(0x05);
    sendData(0x40);
    sendData(0x1F);
    sendData(0x1F);
    sendData(0x2C);

    sendCommand(0x06);
    sendData(0x6F);
    sendData(0x1F);
    sendData(0x1F);
    sendData(0x22);

    sendCommand(0x08);
    sendData(0x6F);
    sendData(0x1F);
    sendData(0x1F);
    sendData(0x22);

    sendCommand(0x13);    // IPC
    sendData(0x00);
    sendData(0x04);

    sendCommand(0x30);
    sendData(0x3C);

    sendCommand(0x41);     // TSE
    sendData(0x00);

    sendCommand(0x50);
    sendData(0x3F);

    sendCommand(0x60);
    sendData(0x02);
    sendData(0x00);

    sendCommand(0x61);     // TRES
    sendData(0x03);
    sendData(0x20);
    sendData(0x01); 
    sendData(0xE0);

    sendCommand(0x82);     // VDCS
    sendData(0x1E);

    sendCommand(0x84);     // T_VDCS
    sendData(0x00);

    sendCommand(0x86);     // AGID
    sendData(0x00);

    sendCommand(0xE3);
    sendData(0x2F);

    sendCommand(0xE0);   // CCSET
    sendData(0x00); 

    sendCommand(0xE6);   // TSSET
    sendData(0x00);

    return 0;
}

void EPD7in3f::reset(void) {
    digitalWrite(reset_pin, HIGH);
    delayMs(20);
    digitalWrite(reset_pin, LOW);
    delayMs(2);
    digitalWrite(reset_pin, HIGH);
    delayMs(20);
}

void EPD7in3f::sendCommand(unsigned char command) {
    digitalWrite(dc_pin, LOW);
    spiTransfer(command);
}

void EPD7in3f::sendData(unsigned char data) {
    digitalWrite(dc_pin, HIGH);
    spiTransfer(data);
}

void EPD7in3f::busyHigh(void) {
    while(digitalRead(busy_pin) == 0) {      // LOW: idle, HIGH: busy
        delayMs(5);
    }
    delayMs(200);
}

void EPD7in3f::turnOnDisplay(void) {
    sendCommand(0x04); // POWER_ON
    busyHigh();

    sendCommand(0x12); // DISPLAY_REFRESH
    sendData(0x00);
    busyHigh();

    sendCommand(0x02); // POWER_OFF
    sendData(0x00);
    busyHigh();
}

void EPD7in3f::clear(UBYTE color) {
    UWORD Width, Height;
    Width = (EPD_WIDTH % 2 == 0)? (EPD_WIDTH / 2 ): (EPD_WIDTH / 2 + 1);
    Height = EPD_HEIGHT;

    sendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            sendData((color << 4) | color);
        }
    }
    turnOnDisplay();
}

void EPD7in3f::display(const UBYTE *image) {
    UWORD Width, Height;
    Width = (EPD_WIDTH % 2 == 0)? (EPD_WIDTH / 2 ): (EPD_WIDTH / 2 + 1);
    Height = EPD_HEIGHT;

    sendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            sendData(image[i + j * Width]);
        }
    }
    turnOnDisplay();
}

void EPD7in3f::displayPart(const UBYTE *image, UWORD xstart, UWORD ystart, 
                           UWORD image_width, UWORD image_height) {
    UWORD Width, Height;
    Width = (EPD_WIDTH % 2 == 0)? (EPD_WIDTH / 2 ): (EPD_WIDTH / 2 + 1);
    Height = EPD_HEIGHT;
    
    UWORD image_width_8 = (image_width % 2 == 0)? (image_width / 2 ): (image_width / 2 + 1);
    UWORD xstart_8 = (xstart % 2 == 0)? (xstart / 2 ): (xstart / 2 + 1);

    sendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            if(i >= xstart_8 && i < xstart_8 + image_width_8 && j >= ystart && j < ystart + image_height) {
                sendData(image[(i - xstart_8) + (j - ystart) * image_width_8]);
            } else {
                sendData(0x11);
            }
        }
    }
    turnOnDisplay();
}

void EPD7in3f::showColorBlocks(void) {
    UWORD Width, Height;
    Width = (EPD_WIDTH % 2 == 0)? (EPD_WIDTH / 2 ): (EPD_WIDTH / 2 + 1);
    Height = EPD_HEIGHT;

    sendCommand(0x10);
    for (UWORD j = 0; j < Height; j++) {
        for (UWORD i = 0; i < Width; i++) {
            if(j < Height/8) {
                sendData((EPD_7IN3F_BLACK << 4) | EPD_7IN3F_BLACK);
            } else if(j < Height/8*2) {
                sendData((EPD_7IN3F_WHITE << 4) | EPD_7IN3F_WHITE);
            } else if(j < Height/8*3) {
                sendData((EPD_7IN3F_GREEN << 4) | EPD_7IN3F_GREEN);
            } else if(j < Height/8*4) {
                sendData((EPD_7IN3F_BLUE << 4) | EPD_7IN3F_BLUE);
            } else if(j < Height/8*5) {
                sendData((EPD_7IN3F_RED << 4) | EPD_7IN3F_RED);
            } else if(j < Height/8*6) {
                sendData((EPD_7IN3F_YELLOW << 4) | EPD_7IN3F_YELLOW);
            } else if(j < Height/8*7) {
                sendData((EPD_7IN3F_ORANGE << 4) | EPD_7IN3F_ORANGE);
            } else {
                sendData((EPD_7IN3F_CLEAN << 4) | EPD_7IN3F_CLEAN);
            }
        }
    }
    turnOnDisplay();
}

void EPD7in3f::sleep(void) {
    sendCommand(0x07); // DEEP_SLEEP
    sendData(0xA5);
}
