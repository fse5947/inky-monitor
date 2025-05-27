#include "web_config.h"

void handleRoot() {
  Serial.println("[HTTP] GET /");
  server.send(200, "text/html; charset=utf-8", index_html);
}

void handleConfig() {
  Serial.println("[HTTP] GET /config");

  StaticJsonDocument<256> doc;
  doc["ticker"] = config.ticker;
  doc["currency"] = config.currency;
  doc["timeframe"] = config.timeframe;

  String response;
  serializeJson(doc, response);

  Serial.print("[HTTP] Sending config: ");
  Serial.println(response);

  server.send(200, "application/json", response);
}

void handleSet() {
  Serial.println("[HTTP] POST /set");

  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing JSON");
    return;
  }

  StaticJsonDocument<256> doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  if (doc.containsKey("ticker")) {
    config.ticker = doc["ticker"];
  }

  if (doc.containsKey("currency")) {
    config.currency = doc["currency"];
  }

  if (doc.containsKey("timeframe")) {
    config.timeframe = doc["timeframe"];
  }

  saveConfigToNVS();

  server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void saveConfigToNVS() {
  Serial.println("[NVS] Saving config...");
  prefs.begin("config", false); // false = read/write
  prefs.putBytes("settings", &config, sizeof(config));
  prefs.end();
  Serial.println("Config saved.");
}

void loadConfigFromNVS() {
  prefs.begin("config", true); // true = read-only            
  // size_t len = prefs.getBytesLength("settings");
  // if (len == sizeof(Config)) {
  //   prefs.getBytes("settings", &config, sizeof(config));
  //   Serial.printf("Loaded config: led=%d, threshold=%d, mode=%s\n",
  //                 config.ticker, config.currency, config.timeframe);
  // } else {
  //   Serial.println("No valid config found, using defaults.");
  // }
  prefs.clear();
  prefs.end();
}