// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#ifdef OLED_DISPLAY

    #include <cstdint>
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
    #endif

    #define DISP_PROGMEM PROGMEM

    static uint8_t bmp_flash[] DISP_PROGMEM = {
    B00000000, B00011100, B00011100, B00001110, B00001110, B11111110, B01111111,
    B01110000, B01110000, B00110000, B00111000, B00011000, B01111111, B00111111,
    B00011110, B00001110, B00000110, B00000000, B00000000, B00000000, B00000000};

    class OledDisplayClass {

    public:
        void init();
        void loop();

    private:
        uint32_t _lastPublish;
    };

    extern OledDisplayClass OledDisplay;

#endif