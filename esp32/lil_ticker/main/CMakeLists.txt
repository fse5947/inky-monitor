idf_component_register(SRCS "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES arduino-esp32 ArduinoJson GxEPD2 Adafruit_BusIO Adafruit-GFX-Library arduino_time
                )