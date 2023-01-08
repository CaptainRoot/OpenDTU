// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#ifdef OLED_DISPLAY

    #include <cstdint>

    class OledDisplayClass {

    public:
        void init();
        void loop();

        float totalPower = 0.0;
        float totalYieldDay = 0.0;
        float totalYieldTotal = 0.0;

    private:
        uint32_t _lastInvUpdateCheck = 0;
        uint32_t _lastPublish = 0;
        uint32_t _newestInverterTimestamp = 0;
    };

    extern OledDisplayClass OledDisplay;

#endif