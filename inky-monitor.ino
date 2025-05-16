#include "inky-monitor-config.h"
#include <WiFi.h>
#include <Wire.h>
#include <HTTPClient.h>
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <TimeLib.h>
#include "jura_regular8pt7b.h"
#include "jura_regular7pt7b.h"
#include "jura_bold30pt7b.h"
#include "jura_bold12pt7b.h"
#include "wifi_off.h"

/* 2.9'' EPD Module */
GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(/*CS=D1*/ 3, /*DC=D3*/ 5, /*RES=D0*/ 2, /*BUSY=D5*/ 7)); // DEPG0290BS 128x296, SSD1680

const int httpsPort = 443;
const String url_usd = "https://api.coinbase.com/v2/prices/BTC-USD/spot";
const String timeURL = "https://api.coinbase.com/v2/time";
const String basehistoryURL = "https://api.exchange.coinbase.com/products/BTC-USD/candles";
const String cryptoCode = "BTC";

double btcusd;
double priceChange;
double percentChange;
double zeroLinePrice;

double pricehistory[MAX_HISTORY][4];
double history_min, history_max;

int historylength;
int granularity;

WiFiClient client;
HTTPClient http;

String formattedDate;
String dayStamp;
String timeStamp;
String lastUpdatedTimeStr;
String lastUpdatedDateStr;
int64_t lastUpdatedTime;

void setup()
{
  int i = 0;
  Serial.begin(115200);

  display.init(115200, true, 50, false);

  WiFi.begin(ssid, password);

  Serial.println();
  Serial.print("Connecting to WiFi ");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    
    Serial.print(".");
    
    i = i + 1;
    
    if (i == 20) 
    {
      Serial.println();
      displayError();
    }

    if (i == 40)
    {
      Serial.println();
      i = 21;
    }
  }
  Serial.println();

  Serial.print("Connected to: ");
  Serial.print(ssid);
  Serial.println();

  if (chartMode == 0) {          // 1-day chart
    granularity = 3600;          // 1 hour candles
    historylength = 24;
  } else if (chartMode == 1) {   // 1-week chart
    granularity = 21600;         // 6 candles per day (4hr)
    historylength = 28;
  } else if (chartMode == 2) {   // 30-day chart
    granularity = 86400;         // 1 day candles
    historylength = 30;
  }

}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    getTime();
    getCurrentBitcoinPrice();
    getPercentChange();
    getCurrentBitcoinCandle();
    getBitcoinHistory();
    updateDisplay();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }

  Serial.println();

#if (SLEEP_MODE)
  esp_sleep_enable_timer_wakeup(REFRESH_RATE_S * 1000000);
  esp_deep_sleep_start();
#else
  delay(REFRESH_RATE_S * 1000);
#endif
}

const char* getDaySuffix(int day) {
  if (day >= 11 && day <= 13) return "th";
  switch (day % 10) {
    case 1: return "st";
    case 2: return "nd";
    case 3: return "rd";
    default: return "th";
  }
}

void getTime()
{
  Serial.print("Connecting to ");
  Serial.println(timeURL);

  http.begin(timeURL);
  int httpCode = http.GET();
  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, http.getString());

  if (error)
  {
    Serial.print(F("deserializeJson Failed "));
    Serial.println(error.f_str());
    delay(2500);
    return;
  }

  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  lastUpdatedTimeStr = doc["data"]["iso"].as<String>();
  lastUpdatedTime = doc["data"]["epoch"].as<int64_t>();
  http.end();

  tmElements_t tm;
  parseISO8601(lastUpdatedTimeStr, tm);
  time_t utcTime = makeTime(tm);
  time_t localTime = applyTimezoneOffset(utcTime, TIME_ZONE_OFFSET);
  createTimeString(localTime, lastUpdatedTimeStr);
  createDateString(localTime, lastUpdatedDateStr);
}

void getCurrentBitcoinPrice()
{
  Serial.print("Connecting to ");
  Serial.println(url_usd);

  http.begin(url_usd);
  
  int httpCode = http.GET();
  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, http.getString());

  if (error)
  {
    Serial.print(F("deserializeJson Failed "));
    Serial.println(error.f_str());
    delay(2500);
    return;
  }

  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  String BTCUSDPrice = doc["data"]["amount"].as<String>();
  http.end();

  btcusd = BTCUSDPrice.toDouble();

  Serial.print("BTCUSD Price: ");
  Serial.println(BTCUSDPrice.toDouble());
}

void getPercentChange() {
  int64_t now = (lastUpdatedTime / 60) * 60;
  int64_t start = 0;
  if (chartMode == 0) {         // 1D
    start = now - 86400;
  } else if (chartMode == 1) {  // 1W
    start = now - (7 * 86400);
  } else if (chartMode == 2) {  // 30D
    start = now - (30 * 86400);
  }
  int64_t end = start + 60;

  // --- Fetch old candle ---
  String oldCandleURL = basehistoryURL + "?start=" + String(start) + "&end=" + String(end) + "&granularity=60";
  Serial.println("Getting old candle: " + oldCandleURL);

  http.begin(oldCandleURL);
  int httpCode = http.GET();
  StaticJsonDocument<1000> oldDoc;
  DeserializationError error = deserializeJson(oldDoc, http.getString());
  http.end();

  JsonArray oldArray = oldDoc.as<JsonArray>();
  if (error || oldArray.size() == 0) {
    Serial.println("Old candle fetch failed.");
    return;
  }

  zeroLinePrice = oldArray[0][3].as<double>();
  Serial.print("Zero Line Price: ");
  Serial.println(zeroLinePrice);

  // Compute price/percent change
  priceChange = (btcusd - zeroLinePrice);
  percentChange = ((btcusd - zeroLinePrice) / zeroLinePrice) * 100.0;
  Serial.printf("Percent change: %.2f%%\n", percentChange);
}

void getCurrentBitcoinCandle() {
  int64_t now = (lastUpdatedTime / 60) * 60;
  int64_t start = now - 60;
  int64_t end = now;

  // --- Fetch current candle ---
  String currentCandleURL = basehistoryURL + "?start=" + String(start) + "&end=" + String(end) + "&granularity=60";
  Serial.println("Getting current candle: " + currentCandleURL);

  http.begin(currentCandleURL);
  int httpCode = http.GET();
  StaticJsonDocument<1000> currentDoc;
  DeserializationError error = deserializeJson(currentDoc, http.getString());
  http.end();

  JsonArray currentArray = currentDoc.as<JsonArray>();
  if (error || currentArray.size() == 0) {
    Serial.println("Current candle fetch failed.");
    return;
  }

  // Update last entry in chart
  pricehistory[historylength - 1][0] = currentArray[0][1].as<double>(); // Low
  pricehistory[historylength - 1][1] = currentArray[0][2].as<double>(); // High
  pricehistory[historylength - 1][2] = currentArray[0][3].as<double>(); // Open
  pricehistory[historylength - 1][3] = currentArray[0][4].as<double>(); // Close

  Serial.print("BTCUSD Current Close: ");
  Serial.println(pricehistory[historylength - 1][3]);
}

void getBitcoinHistory()
{
  String historyURL;
  
  int64_t end = (lastUpdatedTime / granularity) * granularity;
  int64_t start = end - (historylength * granularity);
  
  /* Construct the Coinbase API URL */
  historyURL = basehistoryURL + "?start=" + String(start) + "&end=" + String(end) + "&granularity=" + String(granularity);
  
  Serial.print("Getting history... ");
  Serial.println(historyURL);

  http.begin(historyURL);
  int httpCode = http.GET();
  StaticJsonDocument<5000> doc;
  DeserializationError error = deserializeJson(doc, http.getString());
  http.end();

  if (error)
  {
    Serial.print(F("deserializeJson(History) Failed "));
    Serial.println(error.f_str());
    delay(2500);
    return;
  }

  Serial.print("HTTP Status Code: ");
  Serial.println(httpCode);

  /* Coinbase returns an array of arrays [timestamp, low, high, open, close, volume] */
  JsonArray coinbase_array = doc.as<JsonArray>();
  uint8_t array_count = coinbase_array.size();

  for (int i = 0; i < array_count && i < historylength; i++) 
  {
    JsonArray candle = coinbase_array[array_count - 1 - i];

    pricehistory[i][0] = candle[1].as<double>(); // Low
    pricehistory[i][1] = candle[2].as<double>(); // High
    pricehistory[i][2] = candle[3].as<double>(); // Open
    pricehistory[i][3] = candle[4].as<double>(); // Close
  }

  history_min = 999999;
  history_max = 0;

  for (int i = 0; i < array_count && i < historylength; i++) {
    double low = (GRAPH_MODE == 1) ? pricehistory[i][0] : pricehistory[i][3];
    double high = (GRAPH_MODE == 1) ? pricehistory[i][1] : pricehistory[i][3];
    if (low < history_min) history_min = low;
    if (high > history_max) history_max = high;
  }

  Serial.print("BTCUSD Price History: ");
  for (int i = 0; i < array_count && i < historylength; i++) {
    Serial.print(pricehistory[i][3]);
    Serial.print(", ");
  }
  Serial.println();
}

void updateDisplay()
{
  int16_t tbx, tby; 
  uint16_t tbw, tbh;
  uint16_t x, y, h;
  String str;

  uint16_t y_offset = 40;

  display.setRotation(1);
  display.setFullWindow();
  display.firstPage();
  
  do
  {
    display.fillScreen(BACKGROUND_COLOR);
    display.setTextColor(TEXT_COLOR);

    /* Draw current Bitcoin value */
    display.setFont(&Jura_Bold30pt7b);
    str = createPriceString(btcusd, false);
    //str = 999999;
    display.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx + 28;
    y = y_offset + 23;
    display.setCursor(x, y);
    display.print(str);

    /* Draw BTC/USD symbols */
    display.setFont(&Jura_Bold12pt7b);
    display.setCursor(10, y_offset + 0);
    display.print("BTC");
    display.setCursor(8, y_offset + 26);
    display.print("USD");
    display.fillRoundRect(7, y_offset + 5, 52, 3, 1, TEXT_COLOR);

    /* Draw last update date and time */
    display.setFont(&Jura_Regular7pt7b);
    display.getTextBounds(lastUpdatedDateStr, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = 5;
    y = 15;
    display.setCursor(x, y);
    display.print(lastUpdatedDateStr);

    display.setFont(&Jura_Regular7pt7b);
    display.getTextBounds(lastUpdatedTimeStr, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (display.width() - tbw) - 8;
    y = 15;
    display.setCursor(x, y);
    display.print(lastUpdatedTimeStr);

    /* Draw chart */
    uint16_t graph_x = 72;
    uint16_t graph_y = 82;
    uint16_t graph_w = 210;
    uint16_t graph_h = 38;
    uint16_t x0, x1, y0, y1;
    double graph_step, graph_delta;

    graph_step = ((double)graph_w - 2) / (double)(historylength - 1);
    graph_delta = ((double)graph_h - 2) / (history_max - history_min);

    /* History maximum */
    str = createPriceString(history_max, false);
    display.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor(graph_x - tbw - 6, graph_y + 4);
    display.print(str);

    /* History minimum */
    str = createPriceString(history_min, false);
    display.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
    display.setCursor(graph_x - tbw - 6, graph_y + graph_h);
    display.print(str);

    /* Doted border */
    for (int i = 0; i <= ((graph_w) / 2); i++)
    {
      display.drawPixel(graph_x + 2 * i, graph_y, TEXT_COLOR);
      display.drawPixel(graph_x + 2 * i, graph_y + graph_h, TEXT_COLOR);
    }

    for (int i = 0; i <= (graph_h / 2); i++)
    {
      display.drawPixel(graph_x, graph_y + 2 * i, TEXT_COLOR);
      display.drawPixel(graph_x + graph_w, graph_y + 2 * i, TEXT_COLOR);
    }

    if (GRAPH_MODE == 1)
    {
      /* Candle graph */
      graph_step = ((double)graph_w - 2) / (double)(historylength);
      graph_delta = ((double)graph_h - 2) / (history_max - history_min);

      for (int i = 0; i < (historylength); i++)
      {
        // Centered x-coordinate for the candle
        x0 = (uint16_t)(graph_x + graph_step - 2 + i * graph_step);
        x1 = x0;

        // Wick: from low to high
        y0 = (uint16_t)(graph_y + graph_h - 1 - ((pricehistory[i][0] - history_min) * graph_delta)); // low
        y1 = (uint16_t)(graph_y + graph_h - 1 - ((pricehistory[i][1] - history_min) * graph_delta)); // high
        display.drawLine(x0, y0, x1, y1, TEXT_COLOR);

        // Body
        double open = pricehistory[i][2];
        double close = pricehistory[i][3];
        double top = max(open, close);
        double bottom = min(open, close);

        y0 = (uint16_t)(graph_y + graph_h - 1 - ((top - history_min) * graph_delta));
        h = (uint16_t)((top - bottom) * graph_delta);

        // Handle zero-height candles
        if (h == 0) h = 1;

        if (close > open) {
          // Green candle (up) — use outline with background fill
          display.drawRect(x0 - 2, y0, 5, h, TEXT_COLOR);
          if (h > 2) {
            display.fillRect(x0 - 1, y0 + 1 , 3, h - 2, BACKGROUND_COLOR);
          }
        } else {
          // Red candle (down) — solid fill
          display.fillRect(x0 - 2, y0, 5, h, TEXT_COLOR);
        }
      }
    }
    else
    {
      /* Graph line */
      graph_step = ((double)graph_w - 2) / (double)(historylength - 1);
      graph_delta = ((double)graph_h - 2) / (history_max - history_min);

      for (int i = 0; i < (historylength - 1); i++)
      {
        x0 = (uint16_t)(graph_x + 1 + i * graph_step);
        x1 = (uint16_t)(graph_x + 1 + (i + 1) * graph_step);
        y0 = (uint16_t)(graph_y + graph_h - 1 - ((pricehistory[i][3] - history_min) * graph_delta));
        y1 = (uint16_t)(graph_y + graph_h - 1 - ((pricehistory[i + 1][3] - history_min) * graph_delta));

        display.drawLine(x0, y0, x1, y1, TEXT_COLOR);
        display.drawLine(x0, y0 - 1, x1, y1 - 1, TEXT_COLOR);
        display.drawLine(x0, y0 + 1, x1, y1 + 1, TEXT_COLOR);
      }
    }

    /* Draw Zero Price Line */
    int y_zero = graph_y + graph_h - 1 - ((zeroLinePrice - history_min) * graph_delta);
    // for (int x = graph_x; x < graph_x + graph_w; x += 2) {
    //   display.drawPixel(x, y_zero, TEXT_COLOR);
    // }
    display.drawLine(graph_x, y_zero, graph_x + graph_w - 1, y_zero, TEXT_COLOR);  // Price line
    display.drawLine(graph_x - 3, y_zero, graph_x - 1, y_zero, TEXT_COLOR); // Price Dash

    /* Chart Style */
    str = modeLabel[chartMode];
    display.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = graph_x;
    y = graph_y - 2;
    display.setCursor(x, y);
    display.setFont(&Jura_Regular7pt7b);
    display.print(str);

    /* Percent Change */
    str = formatPercentChange(percentChange);
    display.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = (graph_x + graph_w) - tbw;
    y = graph_y - 4;
    display.setCursor(x, y);
    display.print(str);

    /* Price Change */
    str = createPriceString(priceChange, true);
    display.getTextBounds(str, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = x - tbw - 6;
    y = graph_y - 4;
    display.setCursor(x, y);
    display.print(str);

  }
  while (display.nextPage());
}

/* Function to parse ISO 8601 date-time string (e.g., "2025-01-17T12:30:45Z") into a tmElements_t structure. */
void parseISO8601(const String &isoTime, tmElements_t &tm) 
{
  if (isoTime.length() < 19) return;

  tm.Year = CalendarYrToTm(isoTime.substring(0, 4).toInt());
  tm.Month = isoTime.substring(5, 7).toInt();
  tm.Day = isoTime.substring(8, 10).toInt();
  tm.Hour = isoTime.substring(11, 13).toInt();
  tm.Minute = isoTime.substring(14, 16).toInt();
  tm.Second = isoTime.substring(17, 19).toInt();
}

/* Function to apply a timezone offset (in hours) to the given time. */
time_t applyTimezoneOffset(const time_t utcTime, const int timezoneOffset) 
{
  return utcTime + timezoneOffset * SECS_PER_HOUR;
}

void createTimeString(const time_t time, String &timeString)
{

  char buffer[32];
  sprintf(buffer, "%02d:%02d ", hour(time), minute(time));
  strcat(buffer, timezone);
  timeString = String(buffer);
}

void createDateString(const time_t time, String &dateString)
{
  int d = day(time);
  const char* suffix = getDaySuffix(d);
  const char* m = monthNames[month(time) - 1];

  char buffer[32];
  sprintf(buffer, "%s %d%s", m, d, suffix);
  dateString = String(buffer);
}

String createPriceString(double number, bool showSign) {
  String str = String(number, 0);  // no decimals
  String result = "";

  bool isNegative = str[0] == '-';
  int startIdx = isNegative ? 1 : 0;
  int len = str.length();

  for (int i = startIdx; i < len; i++) {
    if (i > startIdx && ((len - i) % 3 == 0)) {
      result += ',';
    }
    result += str[i];
  }

  if (isNegative) {
    result = "-" + result;
  } else if (showSign){
    result = "+" + result;
  }

  return result;
}

String formatPercentChange(double percent) {
  char buffer[16];

  if (percent >= 0) {
    sprintf(buffer, "[+%.2f%%]", percent);
  } else {
    sprintf(buffer, "[%.2f%%]", percent);
  }

  return String(buffer);
}

void displayError()
{
  int16_t tbx, tby; 
  uint16_t tbw, tbh;
  uint16_t x, y;

  String error1 = "WiFi not available";
  String error2 = "or wrong credentials.";
  display.setRotation(1);
  display.setFullWindow();
  display.firstPage();
  
  do
  {
    display.fillScreen(BACKGROUND_COLOR);
    display.setTextColor(TEXT_COLOR);
    display.drawInvertedBitmap((296 - 64) / 2, (128 - 64) / 2 - 20, epd_bitmap_allArray[0], 64, 64, TEXT_COLOR);

    display.setFont(&Jura_Regular8pt7b);
    display.getTextBounds(error1, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    y = 90;
    display.setCursor(x, y);
    display.print(error1);

    display.getTextBounds(error2, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    y = 110;
    display.setCursor(x, y);
    display.print(error2);
  }
  while (display.nextPage());
}
