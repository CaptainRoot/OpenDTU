// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "OledDisplay.h"
#include "Configuration.h"
#include "MqttSettings.h"
#include "NetworkSettings.h"
#include <Hoymiles.h>

OledDisplayClass OledDisplay;

void OledDisplayClass::init()
{
    _display.init();
    _display.flipScreenVertically();
    _display.setContrast(63);
    _display.setBrightness(63);

    _display.clear();
    _display.setFont(ArialMT_Plain_24);
    _display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);

    _display.drawString(64,22,"Starting...");
    _display.display();
    _display.setTextAlignment(TEXT_ALIGN_LEFT);
}

void OledDisplayClass::loop()
{
    if (!MqttSettings.getConnected() || !Hoymiles.getRadio()->isIdle()) {
        return;
    }
    

    const CONFIG_T& config = Configuration.get();

    if (millis() - _lastPublish > (config.Dtu_PollInterval * 1000)) {
        /*
        MqttSettings.publish("dtu/uptime", String(millis() / 1000));
        MqttSettings.publish("dtu/ip", NetworkSettings.localIP().toString());
        MqttSettings.publish("dtu/hostname", NetworkSettings.getHostname());
        if (NetworkSettings.NetworkMode() == network_mode::WiFi) {
            MqttSettings.publish("dtu/rssi", String(WiFi.RSSI()));
        }
        */




        _lastPublish = millis();
    }
}