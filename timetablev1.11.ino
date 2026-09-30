#define ARDUINO_ARCH_ESP32
#define ESP32

#include "config.h"

void setup() {
  pinMode(5, OUTPUT);
  pinMode(ANALOG_PIN, INPUT);  // A ESP32-E lapkán az A0-re (IO36) pinre fogjuk forrasztani a 2:1 arányú fesz. osztót mivel a meglévő ESP32 v4.0 lapkákon is így van!!! Az új ESP32-C6 lapkákon a 4-es analógpin lesz az akkuhoz rendelve!
  Serial.begin(115200);        // A ESP32 v4.0 lapkákon nincs belsőleg kivezetve analóg pinre az akku feszültség, az ESP32-E lapkán az A2-re van kivezetve, de hogy egységesen tudjuk kezelni ide is fesz. osztót forrasztunk az A0-ra

  wifiLoad();   // A Wifi kapcsolat beállítása, majd a frissítések ellenőrzése
  ++bootCount;  // A bootolások száma, hogy tudjuk ha nem sikerül egyből frissíteni az aktuális nap órarendjét (frissítések után visszaáll 1-re), illetve hogy sikerült-e egyből frissíteni az órarendet

  if (errorCode != 1) {            // Ha az errorCode = 1, akkor nem jött létre WiFi kapcsolat
    setClock();                    // Az aktuális dátum és idő lekérése egy időszerverről
    periodusSzam = PeriodsLoad();  // Betölti az aktuális nap időrendjét (lehet, hogy rövidített órák vannak) a /ringsystem végpontról a periodusok listába
    kartyaSzam = CardsLoad();      // Betölti az aktuális nap óráit a /cards végpontról a cards listába
    DEBUG_PRINT("Peródusok száma: ");
    DEBUG_PRINTLN(periodusSzam);
    DEBUG_PRINT("Kártyák száma: ");
    DEBUG_PRINTLN(kartyaSzam);

    //timeToSleep = (360-15);         // TESZTELÉSHEZ: 6 percenként frissít, a frissítés kb. 15 mp-ig tart  //
    setTimeToSleep();  // Beállítja a szükséges alvási időtartamot

    if (timeToSleep > 600 && !visszaAlszik) {  // Ha valamilyen hiba miatt újra kell frissíteni, akkor nem küldünk adatot a merülési táblázatba ill. emailre
      int accuLevel = analogRead(ANALOG_PIN);  // 3,3V -> 4095 (12 bit)
      DEBUG_PRINT("Akku szint: ");
      DEBUG_PRINTLN(accuLevel);
      //sendBatteryLevel(accuLevel);             // Elküldi az aktuális elemfeszültség értékét a Google táblázatba, hogy lássuk a merülés tendenciáját
      if(accuLevel < warningAccuLevel) {
        emailSend(accuLevel);   // Elem merülés esetén figyelmeztető emailt küld
        errorCode = 4;
      }
    }
  }

  drawing();  // Kirajzolja az E-ink kijelzőre az aktuális nap órarendjét, illetve ha hiba volt, akkor a hibát

  esp_sleep_enable_timer_wakeup(timeToSleep * uS_TO_S_FACTOR);  // Beállítja a következő ébredésig visszalévő időt
  char msg[80];
  sprintf(msg, "Az ébredésig hátralévő idő: %lu másodperc", timeToSleep);
  DEBUG_PRINTLN(msg);
  DEBUG_PRINTLN("Most elalszom...");
  Serial.flush();
  esp_deep_sleep_start();  // Mélyalvásba küldi a mikrokontrollert
}

void loop(void) {}
