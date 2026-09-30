void drawInit() {
  //display.init(115200);  // DESPI-C02
  display.init(115200, true, 2, false);  // WaveShare Hat
  display.setRotation(1);

  //Serial.print("MAGASSÁG: "); Serial.println(display.height()); Serial.print("SZÉLESSÉG: "); Serial.println(display.width());
  racs_magassag = (display.height()) / (MAX_ORA_SZAM + 2);
  time_racs_szel = 3 * display.width() / 10;
  //display.setPartialWindow(0, 0, display.width(), display.height());
  display.fillScreen(GxEPD_WHITE);
  u8g2Fonts.begin(display);                   // connect u8g2 procedures to Adafruit GFX
  u8g2Fonts.setForegroundColor(GxEPD_BLACK);  // vagy GxEPD_RED, ha rövidített órák lesznek
  u8g2Fonts.setBackgroundColor(GxEPD_WHITE);
}

void setClock() {
  // CET-1CEST,M3.5.0,M10.5.0/3 = Közép-európai időzóna, nyári időszámítással
  // Az ESP32-specifikus configTzTime() használata az időzóna stringgel
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "193.225.219.43", "time.nist.gov");
  
  DEBUG_PRINT("Waiting for NTP time sync: ");
  struct tm timeinfo;
  int retryCount = 0;
  while (!getLocalTime(&timeinfo) && retryCount < 40) {
    delay(500);
    DEBUG_PRINT(".");
    retryCount++;
  }
  DEBUG_PRINTLN();

  DEBUG_PRINT("Current time: ");
  DEBUG_PRINTLN(asctime(&timeinfo));
  DEBUG_PRINT("Nyári időszámítás?: ");
  DEBUG_PRINTLN(timeinfo.tm_isdst);

  aktHo = timeinfo.tm_mon + 1;  // 0-11   0-január, 11-december
  aktNap = timeinfo.tm_mday;    // 1-31
  hetAktNapSzama = timeinfo.tm_wday;
  aktOraSzam = timeinfo.tm_hour;  // Ez azért kell, hogy délelőtt 10 óra után már ne frissítsünk
  aktPercSzam = timeinfo.tm_min;
  aktMpSzam = timeinfo.tm_sec;

  strftime(aktDatum, 11, "%Y-%m-%d", &timeinfo);
  strftime(joDatum, 11, "%Y.%m.%d", &timeinfo);
}

void setTimeToSleep() {
  // Csak a téli és nyári szüneteket vesszük figyelembe, ekkor nem fog frissülni az órarend
  // A téli szünet december 23 - január 2-ig tart az órarend számára minden évben
  // A nyári szünet az órarend számára mindig június 22-én kezdődik és szeptember 1-ig tart.
  if (aktHo == 12 && aktNap > 22) {
    timeToSleep = ((31 - aktNap) * 86400) + ((2 * 86400) - 15);  //
    return;
  } else if (aktHo == 6 && aktNap > 21) {
    timeToSleep = ((30 - aktNap) * 86400) + ((62 * 86400) - 15);
    return;
  } else if (aktHo == 7) {
    timeToSleep = ((31 - aktNap) * 86400) + ((31 * 86400) - 15);
    return;
  } else if (aktHo == 8) {
    timeToSleep = (31 - aktNap) * 86400 - 15;
    return;
  }

  if (errorCode == 2) {
    timeToSleep = 600;  // Ha az API nem elérhető, akkor 10 percenként újra próbálkozunk
    return;
  }

  // Ha korábban ébredne a kontroller, mint fél 8, akkor fél 8-ig még vissza altatjuk, mert előfordul, hogy csak fél 8-kor frissítik az órarendet
  int napbolElteltMp = aktOraSzam * 3600 + aktPercSzam * 60 + aktMpSzam;  // Fél 8-ig a napból 7*3600+1800 mp = 27000 mp
  if (napbolElteltMp < 27000) {
    timeToSleep = 27000 - napbolElteltMp;
    visszaAlszik = true;
    return;
  }

  if (kartyaSzam != 0) {
    // ha hétfő és csütörtök közötti nap van és sikeresen betöltődött az órarend, akkor már csak a következő nap fél 8-kor próbálkozunk
    // a péntek reggeli sikeres frissítés után plusz 3 napot altatjuk, vagyis hétfő fél 8-ig aludjon! (3 nap = 3*24*3600 mp = 3*86400 mp = 259200 mp)
    // ha véletlenül szombaton, vagy vasárnap indulna újra a kontroller, akkor hétfő fél 8-ig altatjuk
    if (hetAktNapSzama == 5) {  //Péntek
      // A következő 3 napból éjfélig visszalévő mp-ek plusz a reggel fél 8-ig visszalévő mp-ek (27000-15 a kijelző frissülése)
      // Serial.print(F("Ma éjfélig hátralévő másodpercek száma:")); Serial.println(aktOraSzam*3600+aktPercSzam*60+aktMpSzam);
      timeToSleep = 259200 - (aktOraSzam * 3600 + aktPercSzam * 60 + aktMpSzam) + 26985;
    } else if (hetAktNapSzama == 6) {  //Szombat
      // A következő 2 napból éjfélig visszalévő mp-ek plusz a reggel fél 6-ig visszalévő mp-ek (27000-15 a kijelző frissülése)
      timeToSleep = 172800 - (aktOraSzam * 3600 + aktPercSzam * 60 + aktMpSzam) + 26985;
    } else {
      // Az adott napból éjfélig visszalévő mp-ek plusz az éjféltől a következő reggel fél 6-ig visszalévő mp-ek (27000-15 a kijelző frissülése)
      timeToSleep = 86400 - (aktOraSzam * 3600 + aktPercSzam * 60 + aktMpSzam) + 26985;  //Vasárnap=0, Hétfő=1, ... Csütörtök=4
    }
  } else {
    // Ha hétvége, ünnepnap, szünet stb. van, akkor csak másnap fél 8-kor próbálkozunk megint adatot letölteni az API-ról
    timeToSleep = 86400 - (aktOraSzam * 3600 + aktPercSzam * 60 + aktMpSzam) + 26985;
  }
}


byte kOSz(Period* periodusok[], byte pSz, byte sH, byte sM) {
  String start;
  byte startH;                  // Csengetési rend szerinti tanóra kezdési időpontjának óra része
  byte startM;                  // Csengetési rend szerinti tanóra kezdési időpontjának perc része
  int startPercben;             // Az csengetési rend szerinti óra kezdési ideje percben
  int sPercben = sH * 60 + sM;  // A nem órarend szerinti rendezvény kezdési ideje percben
  byte kezdoOra = 0;

  minStartDiff = 100;
  for (int p = 0; p <= pSz; p++) {
    start = periodusok[p]->beCseng;
    // Serial.print("Becseng: ");Serial.println(start);
    startH = (start.substring(0, start.length() - 3)).toInt();
    startM = (start.substring(start.length() - 2, start.length())).toInt();
    startPercben = startH * 60 + startM;

    if (startH == sH && startM == sM) {
      kezdoOra = periodusok[p]->number;
      minStartDiff = 0;
      break;
    } else {
      // Az egyéb rendezvények nem kezdődnek a szabályos órakezdéskor
      // megkeressük azt az órát, amelyik a legkésőbbi olyan, ami előbb kezdődik, mint a rendezvény kezdése
      //
      if (startPercben < sPercben && sPercben - startPercben < minStartDiff) {
        minStartDiff = sPercben - startPercben;
        kezdoOra = periodusok[p]->number;
      }
    }
  }

  return kezdoOra;
}

byte vOSz(Period* periodusok[], byte pSz, byte eH, byte eM) {
  String end;
  byte endM;
  byte endH;
  int endPercben;               // Az órarend szerinti óra befejezési ideje percben
  int ePercben = eH * 60 + eM;  // A rendezvény vége percben
  byte vegOra = 0;

  minEndDiff = 100;
  for (int p = 0; p <= pSz; p++) {
    end = periodusok[p]->kiCseng;
    endH = (end.substring(0, end.length() - 3)).toInt();
    endM = (end.substring(end.length() - 2, end.length())).toInt();
    endPercben = endH * 60 + endM;

    if (endH == eH && endM == eM) {
      vegOra = periodusok[p]->number;
      minEndDiff = 0;
      break;
    } else {
      // Az egyéb rendezvények nem végződnek a szabályos óravégzéskor
      // megkeressük azt az órát, amelyik a legkorábbi olyan, ami később végződik, mint a rendezvény vége
      if (endPercben > ePercben && endPercben - ePercben < minEndDiff) {
        minEndDiff = endPercben - ePercben;
        vegOra = periodusok[p]->number;
      }
    }
  }

  return vegOra;
}

// String napNev(String datum) {
//   for (int i = 0; i < 5; i++) {
//     if (napok[i]->date == datum) {
//       return napok[i]->name;
//     }
//   }
//   return "Nincs adat...";
// }

// Ha String helyett const char* lenne a típus akkor ezt a formát használhatnád: if (strcmp(napok[i]->date.c_str(), datum.c_str()) == 0)
// String napNev(const String& datum) {
//   String normalizedDatum = datum;
//   normalizedDatum.replace(".", "-");

//   for (int i = 0; i < 5; i++) {
//     if (napok[i] == nullptr) continue;

//     String d = napok[i]->date;
//     d.replace(".", "-");

//     if (normalizedDatum.equals(d)) {
//       return napok[i]->name;
//     }
//   }
//   return "Nincs adat...";
// }

int kozepKezdes(int racskezd, int racsveg, int szovegHossz) {
  return (((racsveg + racskezd) / 2) - (szovegHossz / 2));
}
