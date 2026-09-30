#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESP32httpUpdate.h>

// WiFi hálózatok listája - próbálkozás sorrendjében
const char *ssid[] = {"IoT005705", "jcs"};
const char *password[] = {"WP1x5EUZgXWj2", "Jcsengo2023"};

const int wifiNetworkCount = 2; // Hálózatok száma
const int wifiTimeout = 20;     // Próbálkozási idő hálózatonként (másodperc)

const char *fwImageURL = "http://zeus.jedlik.eu:8000/timetable.ino.bin";
const char *fwVersionURL = "http://zeus.jedlik.eu:8000/version";
// const char* fwImageURL = "https://timetable-jedlik.koyeb.app/update.bin";
// const char* fwVersionURL = "https://timetable-jedlik.koyeb.app/version";
//const char *fwImageURL = "https://added-dell-jedlik-c6786fd9.koyeb.app/update.bin";Í
//const char *fwVersionURL = "https://added-dell-jedlik-c6786fd9.koyeb.app/version";

void check_OTA()
{
  HTTPClient http;
  if (http.begin(fwVersionURL))
  {
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
    {
      String newFWVersion = http.getString();
      float newVersion = newFWVersion.toFloat();
      DEBUG_PRINT("A régi verzió: ");
      DEBUG_PRINTLN(FW_VERSION);
      DEBUG_PRINT("Az új verzió: ");
      DEBUG_PRINTLN(newVersion);
      if (newVersion > FW_VERSION)
      {
        delay(100);
        DEBUG_PRINTLN(F("Frissítés folyamatban..."));
        t_httpUpdate_return ret = ESPhttpUpdate.update(fwImageURL); // Telepíti a letöltött timetable.ino.bin firmware-t, majd újraindítja a mikrokontrollert
        // Az újraindulást követően a soros monitoron megjelenik a "ets Jun 8 2016 00:22:57" üzenet. Ez a ROM bejelentkezési üzenete; ez az első dolog, ami kinyomtatódik, miután az ESP32 bekapcsol és a CPU elindul.
      }
    }
  } // end if http.begin
} // end check_OTA()

void wifiLoad()
{
  WiFi.setAutoReconnect(true);
  bool connected = false;

  // Próbálkozás az összes WiFi hálózattal sorban
  for (int i = 0; i < wifiNetworkCount && !connected; i++)
  {
    DEBUG_PRINT(F("Csatlakozás a(z) "));
    DEBUG_PRINT(ssid[i]);
    DEBUG_PRINTLN(F(" hálózathoz..."));

    WiFi.begin(ssid[i], password[i]);

    int szamlalo = 0;
    int maxProba = wifiTimeout * 2; // 500ms-onként próbál, tehát 2x annyi próba kell

    while (WiFi.status() != WL_CONNECTED && szamlalo < maxProba)
    {
      delay(500);
      Serial.print(F("."));
      szamlalo++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
      connected = true;
      DEBUG_PRINTLN();
      DEBUG_PRINT(F("Sikeres csatlakozás! IP cím: "));
      DEBUG_PRINTLN(WiFi.localIP());
    }
    else
    {
      DEBUG_PRINTLN();
      DEBUG_PRINT(F("Nem sikerült csatlakozni a(z) "));
      DEBUG_PRINT(ssid[i]);
      DEBUG_PRINTLN(F(" hálózathoz."));
      WiFi.disconnect();
      delay(1000); // Kis szünet a következő próbálkozás előtt
    }
  }

  // Ha egyik hálózathoz sem sikerült csatlakozni
  if (!connected)
  {
    DEBUG_PRINTLN(F("HIBA: Egyetlen WiFi hálózathoz sem sikerült csatlakozni!"));
    errorCode = 1;
    return;
  }

  DEBUG_PRINTLN();
  DEBUG_PRINT(F("MAC Address: "));
  String mac = WiFi.macAddress();
  DEBUG_PRINTLN(mac);
  if (mac == "24:DC:C3:81:FD:F4")
    Terem = 102;
  else if (mac == "24:DC:C3:8D:71:F8")
    Terem = 103;
  else if (mac == "38:18:2B:61:B7:70")
    Terem = 104;
  else if (mac == "38:18:2B:17:A3:A4")
    Terem = 106;
  else if (mac == "08:D1:F9:71:2E:A0")
    Terem = 108;
  else if (mac == "38:18:2B:69:F9:74")
    Terem = 110;
  else if (mac == "38:18:2B:18:62:18")
    Terem = 112;
  else if (mac == "3C:8A:1F:54:DF:10")
    Terem = 114;
  else if (mac == "94:54:C5:60:06:98")
    Terem = 115;
  else if (mac == "3C:8A:1F:54:1C:84")
    Terem = 116;
  else if (mac == "3C:8A:1F:54:03:28")
    Terem = 117;
  else if (mac == "3C:8A:1F:54:29:0C")
    Terem = 202;
  else if (mac == "3C:8A:1F:51:75:F8")
    Terem = 203;
  else if (mac == "38:18:2B:6A:F6:10")
    Terem = 204;
  else if (mac == "24:DC:C3:83:BE:B4")
    Terem = 205;
  else if (mac == "38:18:2B:69:E7:A0")
    Terem = 207;
  else if (mac == "38:18:2B:18:34:10")
    Terem = 208;
  else if (mac == "24:DC:C3:85:04:48")
    Terem = 209;
  else if (mac == "38:18:2B:14:58:5C")
    Terem = 210;
  else if (mac == "38:18:2B:19:59:F8")
    Terem = 212;
  else if (mac == "3C:8A:1F:54:96:20")
    Terem = 302;
  else if (mac == "3C:8A:1F:51:5F:0C")
    Terem = 303;
  // ... ide jön a többi terem majd
  else
    Terem = 41; // TESZTELÉSHEZ

  check_OTA();
}
