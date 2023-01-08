// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */

#ifdef OLED_DISPLAY

#include "OledDisplay.h"
#include "Configuration.h"
#include "MqttSettings.h"
#include "NetworkSettings.h"
#include <Hoymiles.h>

/* esp32: SCL = 22, SDA = 21 */
#include <Wire.h>

#ifdef OLED_DISPLAY_SH1106
#include <SH1106Wire.h>
SH1106Wire _display(0x3c, SDA, SCL); // ADDRESS, SDA, SCL
// By default SH1106Wire set I2C frequency to 700000, you can use set either another frequency or skip setting the frequency by providing -1 value
// SH1106Wire(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, 400000); //set I2C frequency to 400kHz
// SH1106Wire(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, -1); //skip setting the I2C bus frequency
#elif OLED_DISPLAY_SSD1306
#include <SSD1306Wire.h>
// for 128x64 displays:
SSD1306Wire _display(0x3c, SDA, SCL); // ADDRESS, SDA, SCL
// for 128x32 displays:
// SSD1306Wire display(0x3c, SDA, SCL, GEOMETRY_128_32);  // ADDRESS, SDA, SCL, GEOMETRY_128_32 (or 128_64)
// for using 2nd Hardware I2C (if available)
// SSD1306Wire(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_TWO); //default value is I2C_ONE if not mentioned
// By default SD1306Wire set I2C frequency to 700000, you can use set either another frequency or skip setting the frequency by providing -1 value
// SSD1306Wire(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, 400000); //set I2C frequency to 400kHz
// SSD1306Wire(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, -1); //skip setting the I2C bus frequency
#endif

#define DISP_PROGMEM PROGMEM

static uint8_t bmp_flash[] DISP_PROGMEM = {
    B00000000, B00011100, B00011100, B00001110, B00001110, B11111110, B01111111,
    B01110000, B01110000, B00110000, B00111000, B00011000, B01111111, B00111111,
    B00011110, B00001110, B00000110, B00000000, B00000000, B00000000, B00000000};

static uint8_t activeSymbol[] PROGMEM = {
    B00000000,
    B00000000,
    B00011000,
    B00100100,
    B01000010,
    B01000010,
    B00100100,
    B00011000
};

static uint8_t inactiveSymbol[] PROGMEM = {
    B00000000,
    B00000000,
    B00000000,
    B00000000,
    B00011000,
    B00011000,
    B00000000,
    B00000000
};

OledDisplayClass OledDisplay;

// Include the UI lib
#include "OLEDDisplayUi.h"
OLEDDisplayUi _ui(&_display);

void drawTotalPowerOverlay(OLEDDisplay* display, OLEDDisplayUiState* state)
{
    char fmtText[10];
    display->drawXbm(10, 5, 8, 17, bmp_flash);
    display->setFont(ArialMT_Plain_24);
    sprintf(fmtText, "%3.0f W", OledDisplay.totalPower);
    display->drawString(37, 0, fmtText);
}

void drawTotalYieldDayFrame(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    char fmtText[32];
    _display.setFont(ArialMT_Plain_16);
    sprintf(fmtText, "Today %4.0f Wh", OledDisplay.totalYieldDay);
    _display.drawString(5 + x, 32 + y, fmtText);
}

void drawTotalYieldTotalFrame(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    char fmtText[32];
    _display.setFont(ArialMT_Plain_16);
    sprintf(fmtText, "Total %.1f kWh", OledDisplay.totalYieldTotal);
    _display.drawString(5 + x, 32 + y, fmtText);
}

void drawDateFrame(OLEDDisplay* display, OLEDDisplayUiState* state, int16_t x, int16_t y)
{
    struct tm timeinfo;
    getLocalTime(&timeinfo, 5);
    char fmtText[24];
    _display.setFont(ArialMT_Plain_16);
    strftime(fmtText, sizeof(fmtText), "%d.%m.%y %H:%M:%S", &timeinfo);
    _display.drawString(0 + x , 32 + y, fmtText);
}

// This array keeps function pointers to all frames
// frames are the single views that slide in
FrameCallback frames[] = { drawTotalYieldDayFrame, drawTotalYieldTotalFrame, drawDateFrame };
// how many frames are there?
int frameCount = 3;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { drawTotalPowerOverlay };
int overlaysCount = 1;

void OledDisplayClass::init()
{
    // The ESP is capable of rendering 60fps in 80Mhz mode
    // but that won't give you much time for anything else
    // run it in 160Mhz mode or just set it to 30 fps
    _ui.setTargetFPS(30);

    _ui.setTimePerFrame(8000);

    // Customize the active and inactive symbol
    _ui.setActiveSymbol(activeSymbol);
    _ui.setInactiveSymbol(inactiveSymbol);

    // You can change this to
    // TOP, LEFT, BOTTOM, RIGHT
    _ui.setIndicatorPosition(BOTTOM);

    // Defines where the first frame is located in the bar.
    _ui.setIndicatorDirection(LEFT_RIGHT);

    // You can change the transition that is used
    // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
    _ui.setFrameAnimation(SLIDE_LEFT);

    // Add frames
    _ui.setFrames(frames, frameCount);

    // Add overlays
    _ui.setOverlays(overlays, overlaysCount);

    // Initialising the UI will init the display too.
    _ui.init();

    //_display.flipScreenVertically();
}

void OledDisplayClass::loop()
{
    _ui.update();

    if (millis() - _lastInvUpdateCheck < 1000) {
        return;
    }
    _lastInvUpdateCheck = millis();

    uint32_t maxTimeStamp = 0;
    for (uint8_t i = 0; i < Hoymiles.getNumInverters(); i++) {
        auto inv = Hoymiles.getInverterByPos(i);

        if (inv == nullptr) {
            continue;
        }

        if (inv->Statistics()->getLastUpdate() > maxTimeStamp) {
            maxTimeStamp = inv->Statistics()->getLastUpdate();
        }
    }

    // Update on every inverter change or at least after Dtu_PollInterval seconds
    const CONFIG_T& config = Configuration.get();

    if (millis() - _lastPublish > (config.Dtu_PollInterval * 1000) || (maxTimeStamp != _newestInverterTimestamp))
    {
        float tmpTotalPower = 0.0;
        float tmpTotalYieldDay = 0.0;
        float tmpTotalYieldTotal = 0.0;
        
        int numInv = Hoymiles.getNumInverters();
        // Loop all inverters
        for (uint8_t i = 0; i < numInv; i++) {
            auto inv = Hoymiles.getInverterByPos(i);
            if (inv == nullptr) {
                continue;
            }

            if (inv->Statistics()->getLastUpdate() > _newestInverterTimestamp) {
                _newestInverterTimestamp = inv->Statistics()->getLastUpdate();
            }

            tmpTotalPower += inv->Statistics()->getChannelFieldValue(CH0, FLD_PAC);
            tmpTotalYieldDay += inv->Statistics()->getChannelFieldValue(CH0, FLD_YD);
            tmpTotalYieldTotal += inv->Statistics()->getChannelFieldValue(CH0, FLD_YT);
        }

        this->totalPower = tmpTotalPower;
        this->totalYieldDay = tmpTotalYieldDay;
        this->totalYieldTotal = tmpTotalYieldTotal;

        _lastPublish = millis();
    }
}
#endif