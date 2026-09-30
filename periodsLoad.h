byte PeriodsLoad() {

  byte number;
  byte periodsCount;
  const char* beCsengetes;
  const char* kiCsengetes;

  String baseUrl = "https://jedlikinfo.jedlik.eu/api/api/timetable/ringsystem/";
  http.begin(baseUrl + datumJo);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  httpCode = http.GET();
  // httpCode will be negative on error
  if (httpCode > 0) {
    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
    } else {
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  // https://arduinojson.org/v7/assistant/#/step1     - A JSON deserializálásához automatikus kódgeneráló és optimalizáló
  deserializeJson(doc, payload);
  JsonArray periods = doc.as<JsonArray>();
  for (int i = 0; i < periods.size(); i++) {
    number = periods[i]["óra"];
    beCsengetes = periods[i]["becsengetés"];
    kiCsengetes = periods[i]["kicsengetés"];
    periodusok[i] = new Period(number, beCsengetes, kiCsengetes);
  }

  periodsCount = 10;

  if (periods.size() < periodsCount) {
    for (int j = periods.size(); j <= periodsCount; j++) {
      periodusok[j] = new Period(normPeriodusok[j]->number, normPeriodusok[j]->beCseng, normPeriodusok[j]->kiCseng);
    }
  }
  return periodsCount;
}