void timeWrite(byte ora, String tol, String ig) {
  int alap = 25;
  String oraSzam;
  String ido;

  oraSzam = String(ora) + ". óra";
  u8g2Fonts.setFont(u8g2_font_helvB18_tf); // u8g2_font_lubB18_tf
  u8g2Fonts.setCursor(kozepKezdes(0, time_racs_szel, u8g2Fonts.getUTF8Width(oraSzam.c_str())), alap + (ora + 1) * racs_magassag);
  u8g2Fonts.print(oraSzam);

  ido = tol + " - " + ig;
  u8g2Fonts.setFont(u8g2_font_helvB14_tf); // u8g2_font_lubR14_tf
  u8g2Fonts.setCursor(kozepKezdes(0, time_racs_szel, u8g2Fonts.getUTF8Width(ido.c_str())), alap + (ora + 1) * racs_magassag + 25);
  u8g2Fonts.print(ido);
}

void timeWindow() {
  uint16_t y;
  y = 0;
  do {
    display.drawLine(0, y, time_racs_szel, y, GxEPD_BLACK);
    y += racs_magassag;
  } while (y < display.height());

  String aktHet = napok[0]->week + " hét";
  u8g2Fonts.setFont(u8g2_font_lubB18_tf);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2Fonts.setCursor(kozepKezdes(0, time_racs_szel, u8g2Fonts.getUTF8Width(aktHet.c_str())), 25);
  u8g2Fonts.print(aktHet);
  u8g2Fonts.setFont(u8g2_font_courB18_tf);
  u8g2Fonts.setCursor(kozepKezdes(0, time_racs_szel, u8g2Fonts.getUTF8Width(Terem.c_str())), 50);
  u8g2Fonts.print(Terem);
  for (uint8_t i = 0; i < MAX_ORA_SZAM; i++) {
    timeWrite(i, periodusok[i]->beCseng, periodusok[i]->kiCseng);
  }
}

void lessonWrite(Card* kartya, int kozepe, byte kSSz, byte vSSz, byte fajta, byte diffStart, byte diffEnd) {
  int alap;
  alap = racs_magassag + 32;

  u8g2Fonts.setFont(u8g2_font_ncenB18_te);
  int lesson_racs_szel_fele = (time_racs_szel + display.width()) / 2;

  // Az adott óra feletti és alatti vizszintes vonal kirajzolása, ez független attól, hogy két különböző óra van egy teremben azonos időben
  // Serial.println("(kezdoOraSzam): " + String((kezdoOraSzam)));
  // Serial.println("(vegOraSzam+ 1): " + String((vegOraSzam+1)));
  display.drawLine(time_racs_szel, kSSz * racs_magassag + diffStart, display.width(), kSSz * racs_magassag + diffStart, GxEPD_BLACK);
  display.drawLine(time_racs_szel, vSSz * racs_magassag - diffEnd, display.width(), vSSz * racs_magassag - diffEnd, GxEPD_BLACK);
  if (kartya->dual == 1) {
    u8g2Fonts.setCursor(kozepKezdes(time_racs_szel, lesson_racs_szel_fele, u8g2Fonts.getUTF8Width(kartya->text)), kozepe + 10);
    u8g2Fonts.print(kartya->text);
    u8g2Fonts.setFont(u8g2_font_helvR18_tf);
    u8g2Fonts.setCursor(time_racs_szel + 5, vSSz * racs_magassag - 5);
    u8g2Fonts.print(kartya->leftBottom);
    u8g2Fonts.setCursor(lesson_racs_szel_fele - u8g2Fonts.getUTF8Width(kartya->rightBottom.c_str()) - 5, vSSz * racs_magassag - 5);
    u8g2Fonts.print(kartya->rightBottom);
    // Függőleges felező vonal meghúzása
    display.drawLine(lesson_racs_szel_fele, kSSz * racs_magassag, lesson_racs_szel_fele, vSSz * racs_magassag, GxEPD_BLACK);
  } else if (kartya->dual == 2) {
    u8g2Fonts.setCursor(kozepKezdes(lesson_racs_szel_fele, display.width(), u8g2Fonts.getUTF8Width(kartya->text)), kozepe + 10);
    u8g2Fonts.print(kartya->text);
    u8g2Fonts.setFont(u8g2_font_helvR18_tf);
    u8g2Fonts.setCursor(lesson_racs_szel_fele + 5, vSSz * racs_magassag - 5);
    u8g2Fonts.print(kartya->leftBottom);
    u8g2Fonts.setCursor(display.width() - u8g2Fonts.getUTF8Width(kartya->rightBottom.c_str()) - 5, vSSz * racs_magassag - 5);
    u8g2Fonts.print(kartya->rightBottom);
  } else {
    // Ha a tárgy szöveg szélesebb a lesson_racs_szel-nél, akkor kisebb betű, normál és csak a tárgy első 24 karakterét írjuk ki.
    // Esetleg lehetne soremeléssel két sorban kiírni.
    if (u8g2Fonts.getUTF8Width(kartya->text) > (display.width() - time_racs_szel)) {  // Ha nem fér ki akkor trükközünk
      u8g2Fonts.setFont(u8g2_font_ncenB14_te);
      if (strlen(kartya->text) > 24) {  // Ha még így sem fér ki
        String szoveg = kartya->text;
        const char* sub = szoveg.substring(0, 24).c_str();
        u8g2Fonts.setCursor(kozepKezdes(time_racs_szel, display.width(), u8g2Fonts.getUTF8Width(sub)), kozepe + 10);
        u8g2Fonts.print(sub);
      } else {
        u8g2Fonts.setCursor(kozepKezdes(time_racs_szel, display.width(), u8g2Fonts.getUTF8Width(kartya->text)), kozepe + 10);
        u8g2Fonts.print(kartya->text);
      }
    } else {
      u8g2Fonts.setCursor(kozepKezdes(time_racs_szel, display.width(), u8g2Fonts.getUTF8Width(kartya->text)), kozepe + 10);
      u8g2Fonts.print(kartya->text);
    }

    u8g2Fonts.setFont(u8g2_font_helvR18_tf);

    u8g2Fonts.setCursor(time_racs_szel + 15, vSSz * racs_magassag - 5);
    u8g2Fonts.print(kartya->leftBottom);
    u8g2Fonts.setCursor(display.width() - u8g2Fonts.getUTF8Width(kartya->rightBottom.c_str()) - 15, vSSz * racs_magassag - 5);
    u8g2Fonts.print(kartya->rightBottom);
  }
}

void lessonWindow(byte targyDb) {
  uint16_t y;
  uint16_t kezdoOraSzam;  // A tényleges kezdő óraszámot tartalmazza (pl. ha a 2. és 3. órában van egy dupla óra, akkor ez 2)                       (pl.: ha a 6. és 7. órában van egy dupla óra, akkor ez 6)
  uint16_t vegOraSzam;    // A tényleges vég óraszámot tartalmazza (pl. ha a 2. és 3. órában van egy dupla óra, akkor ez 3)                         (pl.: ha a 6. és 7. órában van egy dupla óra, akkor ez 7)
  byte kezdoSorszam;      // A megjelenítési rácsban a kezdősor sorszáma az aktuális kártyának (pl.: duplakijelzős délelőtti órarend esetén ez 3)   (pl.: ha a 6. és 7. órában van egy dupla óra, akkor duplakijelzős délutáni órarend esetén ez 0)
  byte vegSorszam;        // A megjelenítési rácsban a végsor sorszáma az aktuális kártyának (pl.: duplakijelzős délelőtti órarend esetén ez 5)     (pl.: ha a 6. és 7. órában van egy dupla óra, akkor duplakijelzős délutáni órarend esetén ez 2)
  byte fajta;
  uint16_t kozep;
  byte diffStart;
  byte diffEnd;

  // Fejléc vízszintes szegélyei
  display.drawLine(0, 0, display.width(), 0, GxEPD_BLACK);
  display.drawLine(0, racs_magassag, display.width(), racs_magassag, GxEPD_BLACK);

  // Napnév és dátum kiírása (A napnév helyére kerül a hibaüzenet kiírása)
  u8g2Fonts.setFont(u8g2_font_ncenB18_te);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  if (errorCode == 0) {
    u8g2Fonts.setCursor(kozepKezdes(time_racs_szel, display.width(), u8g2Fonts.getUTF8Width(napNev(datumJo).c_str())), 25);
    u8g2Fonts.print(napNev(datumJo));
  }
  else {
    u8g2Fonts.setCursor(kozepKezdes(time_racs_szel, display.width(), u8g2Fonts.getUTF8Width(errorMessages[errorCode].c_str())), 25);
    u8g2Fonts.print(errorMessages[errorCode]);
  }
  
  // Dátum kiírása minden esetben, kivéve WiFi hiba esetén (errorCode == 1)
  if (errorCode != 1) {
    u8g2Fonts.setFont(u8g2_font_courB18_tf);
    u8g2Fonts.setCursor(kozepKezdes(time_racs_szel, display.width(), u8g2Fonts.getUTF8Width(datumJo)), 50);
    u8g2Fonts.print(datumJo);
  }

  if (targyDb >= 1) {

    for (uint8_t i = 0; i < targyDb; i++) {
      fajta = aktKartyak[i]->fajta;

      kezdoOraSzam = kOSz(periodusok, periodusSzam, aktKartyak[i]->startHour, aktKartyak[i]->startMinute);
      vegOraSzam = vOSz(periodusok, periodusSzam, aktKartyak[i]->endHour, aktKartyak[i]->endMinute);

      kezdoSorszam = kezdoOraSzam + 1;
      vegSorszam = vegOraSzam + 2;

      if (fajta == 8) {  //Estis képzés - EZ AZÉRT KELL, MERT AZ UTOLSÓ ÓRÁKAT MÁR NEM ÍRJUK KI AZ ESTIS ÓRÁKNÁL!
        vegSorszam = 12;
      } else if (fajta == 9) {
        //Egyéb rendezvények
        // Ebben az esetben, mivel nem a rendes órakezdéskor kezdődik és végződik, ezért megkeressük a megfelelő kezdési és végzési koordinátákat a startMinuteFromMidnight alapján
        // A diff azt a rácsmagasságban mért távolságot jelenti az y tengelyen, amennyivel lejjebb vagy feljebb kell meghúzni a kezdési és vég időpontokhoz tartozó vizszintes vonalakat.
        // diffStart = minStartDiff / 45.0 * racs_magassag;
        // diffEnd = minEndDiff / 45.0 * racs_magassag;
        vegSorszam = 12;
      } else {
        // További esetek is lesznek...
      }

      kozep = (kezdoSorszam * racs_magassag + vegSorszam * racs_magassag) / 2;
      if (errorCode == 0 || errorCode == 4) lessonWrite(aktKartyak[i], kozep, kezdoSorszam, vegSorszam, fajta, diffStart, diffEnd);

    }
  }

  // Verziószám kiírása 
  char ver[6];  // Buffer big enough for 5-character float
  dtostrf(FW_VERSION, 4, 2, ver);
  DEBUG_PRINTLN("Verzió szám: " + String(FW_VERSION));
  u8g2Fonts.setFont(u8g2_font_5x7_tf);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2Fonts.setCursor(kozepKezdes(time_racs_szel, display.width(), u8g2Fonts.getUTF8Width(ver)), 12*racs_magassag + 2);
  u8g2Fonts.print(ver);
}


void drawing() {

  digitalWrite(D8, HIGH);  // Ezzel feszültség alá helyezzük az E-ink vezérlőt. Az órarend kirajzolása után ki fogjuk kapcsolni. (Hogy alvás közben ne fogyasszon áramot)
  drawInit();              // Az e-ink kijelzőre való rajzolás kezdeti beállításai

  do {
    // Függőleges rácsvonalak kirajzolása
    display.drawLine(0, 0, 0, display.height(), GxEPD_BLACK);
    display.drawLine(time_racs_szel, 0, time_racs_szel, display.height(), GxEPD_BLACK);
    if (bootCount == 1) {  // Ha volt frissítés akkor a bootcount változó újra 1-re lesz állítva és ilyenkor a középső függőleges vonalat duplán rajzoljuk ki.
      display.drawLine(time_racs_szel + 2, 0, time_racs_szel + 2, display.height(), GxEPD_BLACK);
    }
    display.drawLine(display.width() - 1, 0, display.width() - 1, display.height(), GxEPD_BLACK);

    if (errorCode != 1) timeWindow();
    lessonWindow(kartyaSzam);

  } while (display.nextPage());
  display.hibernate();
  display.end();
  digitalWrite(D8, LOW);  // Lekapcsoljuk a feszültséget az E-ink kijelzőről
}