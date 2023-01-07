// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#ifdef OLED_DISPLAY

    #include <cstdint>

    class OledDisplayClass {

    public:
        void init();
        void loop();

    private:
        uint32_t _lastInvUpdateCheck = 0;
        uint32_t _lastPublish = 0;
        uint32_t _newestInverterTimestamp = 0;

        void DrawStartingOpenDTU();
        void DrawWaitingForSun();
        void DrawDataScreen();
    };

    extern OledDisplayClass OledDisplay;

#endif