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
        SH1106Wire _display(0x3c, SDA, SCL);  // ADDRESS, SDA, SCL
        // By default SH1106Wire set I2C frequency to 700000, you can use set either another frequency or skip setting the frequency by providing -1 value
        // SH1106Wire(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, 400000); //set I2C frequency to 400kHz
        // SH1106Wire(0x3c, SDA, SCL, GEOMETRY_128_64, I2C_ONE, -1); //skip setting the I2C bus frequency
    #elif OLED_DISPLAY_SSD1306
        #include <SSD1306Wire.h>
        // for 128x64 displays:
        SSD1306Wire _display(0x3c, SDA, SCL);  // ADDRESS, SDA, SCL
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

OledDisplayClass OledDisplay;

void OledDisplayClass::init()
{
    _display.init();
    _display.flipScreenVertically();
    _display.setContrast(63);
    _display.setBrightness(63);

    DrawStartingOpenDTU();
    _lastPublish = 0;
}

void OledDisplayClass::loop()
{
    /*
    if (Hoymiles.getRadio()->isIdle()) {
        DrawWaitingForSun();
        return;
    }
    */

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

    if (millis() - _lastPublish > (config.Dtu_PollInterval * 1000) || (maxTimeStamp != _newestInverterTimestamp)) {

        DrawDataScreen();

        _lastPublish = millis();
    }
}

void OledDisplayClass::DrawStartingOpenDTU()
{
    _display.clear();
    _display.setFont(ArialMT_Plain_24);
    _display.setTextAlignment(TEXT_ALIGN_CENTER);

    _display.drawString(64,4,"Starting");
    _display.drawString(64,32,"OpenDTU");
    _display.display();
}

void OledDisplayClass::DrawWaitingForSun()
{
    _display.clear();
    _display.setBrightness(1);
    _display.setFont(ArialMT_Plain_24);
    _display.setTextAlignment(TEXT_ALIGN_CENTER);

    _display.drawString(64,4,"Inverter");
    _display.drawString(64,32,"Offline");
    _display.display();
}

void OledDisplayClass::DrawDataScreen()
{
    float totalPower = 587.0;
    float totalYieldDay = 291.0;
    float totalYieldTotal = 2.843999863;

    char fmtText[64];

    int  numInv = Hoymiles.getNumInverters();
    // Loop all inverters
    for (uint8_t i = 0; i < numInv; i++) {
        auto inv = Hoymiles.getInverterByPos(i);
        if (inv == nullptr) {
            continue;
        }

        if (inv->Statistics()->getLastUpdate() > _newestInverterTimestamp) {
            _newestInverterTimestamp = inv->Statistics()->getLastUpdate();
        }

        totalPower += inv->Statistics()->getChannelFieldValue(CH0, FLD_PAC);
        totalYieldDay += inv->Statistics()->getChannelFieldValue(CH0, FLD_YD);
        totalYieldTotal += inv->Statistics()->getChannelFieldValue(CH0, FLD_YT);
    }

    _display.clear();
    _display.setTextAlignment(TEXT_ALIGN_LEFT);

    //Draw acutal power
    _display.drawXbm(10,5,8,17,bmp_flash);
    _display.setFont(ArialMT_Plain_24);
    sprintf(fmtText,"%3.0f W",totalPower);
    _display.drawString(37,0, fmtText);

    //draw today & total
    _display.setFont(ArialMT_Plain_16);
    sprintf(fmtText,"today %4.0f Wh",totalYieldDay);
    _display.drawString(5,26, fmtText);

    sprintf(fmtText,"total %.1f kWh",totalYieldTotal);
    _display.drawString(5,39,fmtText);

    _display.display();
}


#endif