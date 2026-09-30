// Uncomment for development
#define DEBUG

#ifdef DEBUG
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

const float FW_VERSION = 1.11;   // Ezt mindig át kell írni, ha egy újabb verziót akarunk publikálni! 
                                // Ezt követően lefordítani timetable.ino.bin állományra, majd átnevezni update.bin-re, majd feltölteni a frissítési webhelyre és a version fájlt is ugyanezzel a verzió számmal!

String Terem = "";              // A teremszámok az órarendek MAC címéhez lesznek társítva, tehát minden órarend egy előre meghatározott teremhez lesz rendelve
int errorCode = 0;              // 0 - nincs hiba, 1 - A WiFi hálózat nem elérhető, 2 - Az órarend API nem elérhető, 3 - Nincs órarend adat (pl.: hétvége, ünnep, szünet stb.), 4 - Merülőben az akku
String errorMessages[10] = { "", "A WiFi nem elérhető", "Az API nem elérhető", "Nincs órarend", "Akku töltés kell!" };

// 470k/470k feszültségosztót használunk mindhárom lapkatípushoz. (47nF kondenzátor a lefelezett akkufeszültség simításához) Az alábbi értékek pontosításához kalirálni kell!
int warningAccuLevel = 2000;   // Ha az órarend elemfeszültsége 3,45 Volt alá esik, akkor figyelmeztető emailt küldünk. Ilyenkor töltés szükséges!!!
int criticalAccuLevel = 1900;  // Ha az órarend elemfeszültsége 3,35 Volt alá esik, akkor figyelmeztető emailt küldünk. Ilyenkor kritikus figyelmeztetést küldünk!!!
int urgentAccuLevel = 1800;    // Ha az órarend elemfeszültsége 3,25 Volt alá esik, akkor figyelmeztető emailt küldünk. Ilyenkor sürgősen töltés szükséges!!! 3,2V-nál már nem tud frissülni az órarend!!!
// FireBeetle 2 – univerzális ADC pin kiválasztás
#if defined(CONFIG_IDF_TARGET_ESP32)
  // FireBeetle 2 ESP32 v4.0  és ESP32-E (DFR0654 / DFR0478)
  #define ANALOG_PIN 36
  //#pragma message("Target: ESP32 detected → Using ANALOG_PIN = GPIO36")
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
  // FireBeetle 2 ESP32-C6 (DFR1095)
  // ESP32-C6-on NINCS IO36, ezért más lábat kell használni
  #define ANALOG_PIN 4  // van beépített akkuellenőrző pin ADC bemenet ESP32-C6-on (nincs kivezetve!), ez lineárisan! teszteli az akku állapotát, de az egységes kezelés miatt ezen a lapkán is saját fesz. osztót forrasztunk be a 4-es pinre
  //#pragma message("Target: ESP32-C6 detected → Using ANALOG_PIN = GPIO4")
#else
  #error "Ismeretlen ESP32 verzió – kérlek add meg pontosan a board típust!"
#endif


#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <HTTPClient.h>
#include "wifiLoad.h"
#include <ArduinoJson.h>
#include "classes.h"

#define MAX_ORA_SZAM 10                  // Az utolsó óra amit megjelenítünk a 10. óra. Ennél kettővel több sor kell a fejléc és a 0. óra miatt.

#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_3C.h>
#define GxEPD2_DISPLAY_CLASS GxEPD2_3C
#include <U8g2_for_Adafruit_GFX.h>
#define GxEPD2_DRIVER_CLASS GxEPD2_750c  // GDEW075T8   640x384, UC8159c (IL0371), (WF0583CZ09)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul  // e.g.
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=5*/ 15, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));
#undef MAX_DISPLAY_BUFFER_SIZE
#undef MAX_HEIGHT
#if defined(ESP32) && defined(USE_HSPI_FOR_EPD)
SPIClass hspi(HSPI);
#endif

#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */

char aktDatum[11];  // Pl.: "2024-04-12"  --- A kártyák lekéréséhez ilyen formátumú dátum kell a POST kérésben      --- A setClock fgv. állítja be az értékét
char joDatum[11];   // Pl.: "2024.04.12"  --- A periódusok lekéréséhez (jedlikcsengő) ilyen formátum kell a GET-ben --- A setClock fgv. állítja be az értékét
char tesztAktDatum[11] = "2026-01-26";
char tesztJoDatum[11] = "2026.01.26";
//const char* datumJo = tesztJoDatum;    //         --- Hogy a tesztelésnél csak itt kelljen megadni a tesztelendő dátumot
//const char* datumAkt = tesztAktDatum;  //         --- Hogy a tesztelésnél csak itt kelljen megadni a tesztelendő dátumot
const char *datumJo = joDatum;           //         --- Hogy az élesítésnél csak itt kelljen megadni az aktuális dátumot
const char *datumAkt = aktDatum;         //         --- Hogy az élesítésnél csak itt kelljen megadni az aktuális dátumot

RTC_DATA_ATTR int bootCount = 0;

uint8_t racs_magassag;
int time_racs_szel;
U8G2_FOR_ADAFRUIT_GFX u8g2Fonts;

int timeToSleep = 86400;  // Külön fgv. állítja be az aktuális értékét a hét napja illetve a szüneteket is figyelembe véve, alapértéke 1 nap = 86400 mp
bool visszaAlszik = false;

int aktHo;                // Ebben tároljuk, hogy az aktuális frissítés az év hányadik hónapjában történik, (A szünetek meghatározásához kell)
int aktNap;               // Ebben tároljuk, hogy a frissítés az aktuális hónap hányadik napjában történik, (A szünetek meghatározásához kell)
int hetAktNapSzama;       // Ebben tároljuk, hogy az aktuális frissítés a hét hányadik napján következik be, ha ez 5 (péntek), akkor csak 2 nap múlva kell újra frissíteni!
int aktOraSzam;           // Ebben tároljuk, hogy az aktuális frissítés a nap hányadik órájában történik, ha ez nagyobb, mint 10 akkor már nem próbálkozunk többet az aznapi frissítéssel
int aktPercSzam;          // Ebben tároljuk, hogy a frissítés az aktuális frissítés órájának hányadik percében történik
int aktMpSzam;            // Ebben tároljuk, hogy a frissítés az aktuális frissítés percének hányadik másodpercében történik
byte minStartDiff = 100;  // Ebben tároljuk, hogy ha az adott rendezvény nem az órahatáron kezdődik, akkor mennyivel később
byte minEndDiff = 100;    // Ebben tároljuk, hogy ha az adott rendezvény nem az órahatáron végződik, akkor mennyivel előbb

HTTPClient http;
int httpCode;
String requestData;
String payload;
JsonDocument doc;
JsonDocument filter;
const char *jsonBuffer;

int kartyaSzam = 0;
int periodusSzam = 0;

Day *napok[6];
Card *kartyak[32];
Card *aktKartyak[8];
Period *periodusok[16];  // Mindig 1-gyel nagyobbat kell foglalni, mint a max elemszám!!!
Period *normPeriodusok[16] ={
  new Period(0, "7:10", "7:55"),
  new Period(1, "8:00", "8:45"),
  new Period(2, "8:55", "9:40"),
  new Period(3, "9:50", "10:35"),
  new Period(4, "10:50", "11:35"),
  new Period(5, "11:45", "12:30"),
  new Period(6, "12:40", "13:25"),
  new Period(7, "13:35", "14:20"),
  new Period(8, "14:30", "15:15"),
  new Period(9, "15:15", "15:55"),
  new Period(10, "16:00", "16:40"),
  new Period(11, "16:45", "17:25"),
  new Period(12, "17:30", "18:10"),
  new Period(13, "18:15", "18:55"),
  new Period(14, "19:00", "19:40")
};

/////////////BEGIN BatteryLevel//////////////////////
#include <ESP_Google_Sheet_Client.h>
#include <GS_SDHelper.h>

// Google Project ID
#define PROJECT_ID "timetable-battery-levels"

// Service Account's client email
#define CLIENT_EMAIL "timetable-battery-levels@timetable-battery-levels.iam.gserviceaccount.com"

// Service Account's private key
const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCYdjho3y765EBW\n8ri7pDidljvS6GJrNIIA7RWUr2sdy4sgjiC74JbZN6eT4zpXFWGgeL7Jg3yLKRDs\n/5AtE5X+GXJJC5RQSwmvYoe6EP/Fmg2RAnCReZsVVml5AnXhYnAMY/lVDh4I69tC\nj0C8q9QlXENhNkOnffdp8RBWEUtqLgXfZdJwQZBeBVssso1Vl5mx6lLOUovXFaXI\n8/8DODMHu7EIe2RSFyYB9XU7jIjUDaVJ5Bxxn4TAO8grEBFJPgiyxr4gn2g8T/FM\n2pI0PhA/gaw7fZePVBxZIwM0xZQ9IAH5ZPLmft0KytZxYbnH0qj6uroJ1Yy4DlYN\nL3gL59XJAgMBAAECggEAGNeGeHwJJSNE4oX5GMX43aCJjeOEidBL8Iew8QGmM/wi\n5sKRzdsZ6TnaycuICYF331RdTetpQymFUwCPRvD+iyEtUH6dA3o+kT++OtCjRFjG\nJh8eJW4JmW/zjp6ag4dtF2/AxB2hQjXgC5fY5gUfc5j1d6UGYAez5SS80zFNNvcZ\nVKF+6fFwO4U2t/LJMczELUV+MpEtUvJRTNAVs9GS18MrxqQS0QLYMPNqUhD6MrWx\n3M+JQhbJJe8qY/xr9r1mY+ttkcfYjhl/REpH9EjWj26NpDzLjrcWTeawNoX3hDeW\ng7ZPEVIdjpPy0QCwWizuetLHPwQXp0F+RSZ1vR1/AwKBgQDPKs9E+DBEBYt8wvoc\nYE93sJUEiv7a/weVzl1J63k7o/ZqyDYmbiHITSrfZct/bunqZCEI2OM6iZlf4OPO\nb5SYS71HPvo2DZRr44o84uAMniwKsRo5lOE3uRLZ+Kkyj8VtKygmtw/zy2pA8o/v\nctFe0cHNwlTLoux2lSW/0JzkfwKBgQC8ZkuM/bLpJQfJHuY9L315Zjuz7smnkDQH\nxt5saF+3VjOKB0Psm0qM+YVCetoL7oih5KNADWOYN7SnxIaR9WcBcicq7b3ttC3g\noqIQkpFhg/0C9iZufukOuQso7hjbRA9lJrpBdcdgPCQHmRUNHPv/9MzPa5Y35fwv\npP6cUyQBtwKBgCbuiJ0RdyCRWjFMAuyqPMTlblu4M0S8gJRqf6WQluvaWEig72wx\nbi/SMKhi3OdJ1LOztXGQMMOjcCtQe/8jgO1N7nghIaxeyPGthSEEEc7NOsGXnrIL\nhHeQVDaaQFG8KwHlBlqgNGNAauthqiL5lNcY3C7qwjyylXWDeeCeN53JAoGAP6v+\n2p6fSaZHNFSqEYGsUUwY313TbzsQKpSudUrtFmQetQZxkVSGZJCr1eT5LlEmgjho\nZ+xefYJOHukPZeLfNiDjVeHVb7GWU7YDOcCfO6rE5qxUHsHreBtXxzR42Bnvbgsh\npZ/tivuoXZ0B/VWPYVjVo0BEG44xpdSIbEK5B48CgYBkbQjvP243it89CxFW7Fho\nE2ojQUPIoNVqb3FkW8QxvnEF9q1Vw5T47otsSQ4mY9JllnWpvNcqVj7n1n7LLg2D\naglMSdKT8Xi1lZZFDAKVFrsYug23TYlOJ/W7iJw8TpUbT7xy/P2nJpmNhqtwRhhe\nhxz0Nz3YxaPTBYCf/heQbw==\n-----END PRIVATE KEY-----\n";

// The ID of the spreadsheet where you'll publish the data
const char spreadsheetId[] = "12lrVW-G6uZ1-vRiJYu9R-yj87K6MjOh1QlgkI_X6-rA";
////////////END BatteryLevel///////////////////////////

#include "utilities.h"
#include "periodsLoad.h"
#include "cardsLoad.h"
#include "drawing.h"
#include "sendBatteryData.h"
#include "sendEmail.h"