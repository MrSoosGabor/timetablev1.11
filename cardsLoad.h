byte CardsLoad() {

  const char* nev;    // "Hétfő"
  const char* datum;  // "2024.03.11"
  const char* het;    // "A"
  byte hetNapja;      // 1  (hétfő)

  int oratol;  // 6
  int oraDb;   // 1
  byte startOra;
  byte startPerc;
  byte vegOra;
  byte vegPerc;
  const char* oraNevRovid;
  const char* oraNev;  // "Angol nyelv"
  const char* osztalyNevRovid;
  const char* osztalyNev;
  const char* tanarRovid;
  const char* tanar;  // "Furján Gergely"
  const char* terem;
  const char* osztaly;  // "09A"
  int startPercEjfeltol;
  int vegPercEjfeltol;
  byte fajta;
  byte dual;

  http.begin("https://jedlikinfo.jedlik.eu/api/api/timetable/cards");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String eleje = "{\"class\": \"\", \"classroom\": \"";
  String kozepe = "\", \"teacher\": \"\", \"full\": false, \"fromDate\": \"";
  String vege = "\"}";
  requestData = eleje + Terem.c_str() + kozepe + datumAkt + vege;  /////////////////// AKTUÁLIS DÁTUM (aktDatum) kell ide (kötőjeles formátum) ///////////////////
  // Serial.println("A POST kérés törzse: "+requestData);
  httpCode = http.POST(requestData);

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
      //Serial.print("payload: ");
      //Serial.println(payload);
    } else {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      errorCode = 2;
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    errorCode = 2;
  }
  http.end();

  // https://arduinojson.org/v7/assistant/#/step1     - A JSON deserializálásához automatikus kódgeneráló és optimalizáló
  filter["days"] = true;
  deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  JsonArray days = doc["days"];
  for (int i = 0; i < 5; i++) {
    nev = days[i]["name"];
    datum = days[i]["date"];
    het = days[i]["week"];
    hetNapja = days[i]["dayOfWeek"];

    napok[i] = new Day(nev, datum, het, hetNapja);
  }

  filter["cards"] = true;
  DeserializationError error = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    errorCode = 2;
    return 0;
  }
  JsonArray cards = doc["cards"];
  // Ebben a listában annyi elem van, ahány kártya van az aktuális héten, viszont a dupla és tripla órák egynek számítanak

  int i = 0;

  do {
    hetNapja = cards[i]["dayOfWeek"];
    datum = cards[i]["date"];
    oratol = cards[i]["fromPeriod"];
    oraDb = cards[i]["periodsCount"];  // Ha egynél több óra van összevonva, akkor mindig 2 az értéke
    startOra = cards[i]["startHour"];
    startPerc = cards[i]["startMinute"];
    vegOra = cards[i]["endHour"];
    vegPerc = cards[i]["endMinute"];
    oraNevRovid = cards[i]["text"];
    oraNev = cards[i]["textTitle"];
    osztalyNevRovid = cards[i]["rightBottom"];
    osztalyNev = cards[i]["rightBottomTitle"];
    tanarRovid = cards[i]["leftBottom"];
    tanar = cards[i]["leftBottomTitle"];
    terem = cards[i]["filter"];
    startPercEjfeltol = cards[i]["startMinuteFromMidnight"];
    vegPercEjfeltol = cards[i]["endMinuteFromMidnight"];
    dual = 0;
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // fajta = 1 -> A délelőtti oktatásban a legkésőbbi órák a 10. órában vannak (felvételi előkészítők), vagyis az endHour kisebb kell, hogy legyen 17-nél
    // fajta = 2 -> Az emelt szintű felkészítők felirata töre
    // fajta = 3 -> A Felvételi előkészítők felirata Fe
    // fajta = 4 -> A NagyiNet-et felirata 
    // fajta = 8 -> Az estis órák mindig 15 óra valahány perckor kezdődik és 19 óra valahány perckor van végük???
    // fajta = 9 -> A többi óra egyéb kategóriába kerül (Szülői értekezlet, Nyitott kapuk napja, OSZTV Szoftverfejlesztő és -tesztelő, Matematika I. OKTV, Arany Dániel verseny stb.)
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (startOra == 15 && vegOra == 19) fajta = 8;  // Estis
    // else if (String(oraNevRovid) == "töre") fajta = 2;
    // else if (String(oraNevRovid) == "Fe") fajta = 3;
    // else if (String(oraNevRovid) == "SZG") fajta = 4;  //
    // else if (String(oraNevRovid) == "Szoftverfejlesztő szakmai vizsga") fajta = 5;
    else if(startOra >= 16) fajta = 9;  // 
    else fajta = 1;
    kartyak[i] = new Card(hetNapja, datum, oratol, oraDb, startOra, startPerc, vegOra, vegPerc, oraNevRovid, oraNev, osztalyNevRovid, osztalyNev, tanarRovid, tanar, terem, startPercEjfeltol, vegPercEjfeltol, fajta, dual);
    i++;
  } while (oraDb != 0);

  byte aktKartyakSzama = 0;

  // Kiválogatjuk az aktuális naphoz tartozó órákat a heti órákat tartalmazó kartyak tömbből az aktkartyak nevű tömbbe
  // Ha az aktuális napon van két egyidőben kezdődő óra, akkor a dual értékét beállítjuk 1-re ill. 2-re 
  for (int j = 0; j < i; j++) {
    if (kartyak[j]->date == datumJo)  /////////////////// AKTUÁLIS JÓ DÁTUM (joDatum) kell ide (pontozott formátum) ///////////////////
    {
      for (int k = 0; k < aktKartyakSzama; k++) {
        if (kartyak[j]->startHour == aktKartyak[k]->startHour && kartyak[j]->startMinute == aktKartyak[k]->startMinute) {  // Van-e már az eddig kiválogatott kártyák között olyan, ami ugyanakkor kezdődik
          aktKartyak[k]->dual = 1;
          kartyak[j]->dual = 2;
          break;
        }
      }
      aktKartyak[aktKartyakSzama] = kartyak[j];
      aktKartyakSzama++;
    }
  }

  // TESZTELÉSHEZ
  // for (int k = 0; k < aktKartyakSzama; k++) {
  //   Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  //   Serial.print("startOra");
  //   Serial.println(aktKartyak[k]->startHour);
  //   Serial.print("startPerc");
  //   Serial.println(aktKartyak[k]->startMinute);
  //   Serial.print("vegOra");
  //   Serial.println(aktKartyak[k]->endHour);
  //   Serial.print("vegPerc");
  //   Serial.println(aktKartyak[k]->endMinute);
  //   Serial.print("oraNevRovid");
  //   Serial.println(aktKartyak[k]->text);
  //   Serial.print("fajta");
  //   Serial.println(aktKartyak[k]->fajta);
  //   Serial.print("dual");
  //   Serial.println(aktKartyak[k]->dual);
  //   Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  // }

  if (aktKartyakSzama == 0) errorCode = 3;

  return aktKartyakSzama;
}