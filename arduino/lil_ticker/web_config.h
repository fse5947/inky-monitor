#ifndef WEB_CONFIG_H
#define WEB_CONFIG_H

#include <Arduino.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "index_html.h"

struct Config {
  int ticker;
  int currency;
  int timeframe;
};

extern Config config;
extern WebServer server;
extern Preferences prefs;

void handleRoot();
void handleConfig();
void handleSet();

void saveConfigToNVS();
void loadConfigFromNVS();

#endif

