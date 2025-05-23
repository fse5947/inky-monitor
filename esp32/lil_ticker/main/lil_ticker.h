/* Refresh Rate in seconds */
#define REFRESH_RATE_S (300)

/*Max History Length*/
#define MAX_HISTORY (30)

/* Inverted colors */
#define INVERTED 0

/* SLEEP MODE */
#define SLEEP_MODE 0

/* Time Zone Offset */
#define TIME_ZONE_OFFSET (+0)
const char* timezone = "UTC";

/* GRAPH MODE 0 - Line, 1 - Candle*/
#define GRAPH_MODE 1

#if (INVERTED)
  #define TEXT_COLOR GxEPD_WHITE
  #define BACKGROUND_COLOR GxEPD_BLACK
#else
  #define TEXT_COLOR GxEPD_BLACK
  #define BACKGROUND_COLOR GxEPD_WHITE
#endif

/* 2.9'' EPD Module */
GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT> display(GxEPD2_290_BS(/*CS=D1*/ 3, /*DC=D3*/ 5, /*RES=D0*/ 2, /*BUSY=D5*/ 7));  // DEPG0290BS 128x296, SSD1680

/* WiFi Credentials */
const char* ssid = "VIRGIN502";
const char* password = "417D36937F9C";

/*Time Constants*/
const char* monthNames[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

/*Chart Mode*/
uint8_t chartMode = 0; // 0 = 1D, 1 = 1W, 2 = 1M
const char* modeLabel[] = {"1D", "1W", "1M"};

const int httpsPort = 443;
const String cryptoCode = "BTC";
const String fiatCode = "USD";
const String url_usd = "https://api.coinbase.com/v2/prices/" + cryptoCode + "-" + fiatCode + "/spot";
const String timeURL = "https://api.coinbase.com/v2/time";
const String basehistoryURL = "https://api.exchange.coinbase.com/products/" + cryptoCode + "-" + fiatCode + "/candles";
const String exchangeURL = "https://api.coinbase.com/v2/exchange-rates?currency=USD";  //TODO: Add support for other currencies

double btcusd;
double priceChange;
double percentChange;
double zeroLinePrice;

bool chartAvailable = true;

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

void getTime();
void getCurrentBitcoinPrice();
void getPercentChange();
void getBitcoinHistory();
void calculateMinMax();
void updateDisplay();
void parseISO8601(const String &isoTime, tmElements_t &tm);
time_t applyTimezoneOffset(const time_t utcTime, const int timezoneOffset);
void createTimeString(const time_t time, String &timeString);
void createDateString(const time_t time, String &dateString);
String createPriceString(double number, bool showSign, int decimalPlaces);
String formatPercentChange(double percent);
void displayError();
