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

const char* currencyOptions[] = {
  "AUD", "CAD", "CHF", "EUR", "GBP", "JPY", "USD"
};

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