void tokenStatusCallback(TokenInfo info);

void tokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
  } else {
    GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
  }
}

void sendBatteryLevel(int accuLevel) {

  GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);
  // Set the callback for Google API access token generation status (for debug only)
  GSheet.setTokenCallback(tokenStatusCallback);

  // Set the seconds to refresh the auth token before expire (60 to 3540, default is 300 seconds)
  GSheet.setPrerefreshSeconds(10 * 60);

  // Begin the access token generation for Google API authentication
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);


  bool ready = GSheet.ready();
  if (ready) {
    FirebaseJson response;

    //Serial.println("\nAppend spreadsheet values...");
    //Serial.println("----------------------------");

    FirebaseJson valueRange;

    valueRange.add("majorDimension", "ROWS");

    String frissitesiIdo;
    frissitesiIdo.reserve(7);
    frissitesiIdo += aktOraSzam;
    frissitesiIdo += ":";
    frissitesiIdo += aktPercSzam;
    frissitesiIdo += ":";
    frissitesiIdo += aktMpSzam;

    valueRange.set("values/[0]/[0]", aktDatum);  
    valueRange.set("values/[0]/[1]", (accuLevel/2048.0)*3.3);  // Akkufeszültség voltban
    valueRange.set("values/[0]/[2]", frissitesiIdo);  

    // For Google Sheet API ref doc, go to https://developers.google.com/sheets/api/reference/rest/v4/spreadsheets.values/append
    // Append values to the spreadsheet
    String setTab;
    setTab.reserve(7);
    setTab += Terem;
    setTab += "!A2";
    bool success = GSheet.values.append(&response /* returned response */, spreadsheetId /* spreadsheet Id to append */, setTab /* range to append */, &valueRange /* data range to append */);
    if (success) {
      response.toString(Serial, true);
      valueRange.clear();
    } else {
      DEBUG_PRINTLN(GSheet.errorReason());
    }
    DEBUG_PRINTLN();
    DEBUG_PRINTLN(ESP.getFreeHeap());
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////