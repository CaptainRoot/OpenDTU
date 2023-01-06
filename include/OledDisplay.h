// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#ifdef OLED_DISPLAY

    #include <cstdint>

    class OledDisplayClass {

    public:
        void init();
        void loop();

    private:
        uint32_t _lastPublish;

        void DrawStartingOpenDTU();
        void DrawWaitingForSun();
    };

    extern OledDisplayClass OledDisplay;

#endif