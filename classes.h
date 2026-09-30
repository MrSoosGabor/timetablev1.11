#include "Arduino.h" 

#define classes_h

class Card{
  public:
    byte dayOfWeek;    // 1 - Hétfő 
    String date;       // "2024.03.11"
    int fromPeriod;   // Pl.: 6. órától, de -99 is lehet
    int periodsCount;        // Pl.: 2 - ha dupla, vagy tripla óra, de -99 is lehet
    byte startHour;
    byte startMinute;
    byte endHour;
    byte endMinute;
    const char* text;              // Pl.: "prog"
    const char* textTitle;         // Pl.: "Programozási alapok"
    String rightBottom;       // Pl.: "09A"
    String rightBottomTitle;  // Pl.: "09A"
    String leftBottom;        // Pl.: "NL"
    String leftBottomTitle;   // Pl.: "Nits László"
    String filter;            // Pl.: "102"
    int startMinuteFromMidnight;
    int endMinuteFromMidnight;
    byte fajta;               // 1 - Nappali, 2 - Esti, 3 - Nyitott kapuk napja
    byte dual;                // 0 - Csak ez a kártya van az adott időpontban, 1 - Az egyik kártya, 2 - A másik kártya az adott időpontban
  
  public:
    Card(byte dOW, String d, int fP, int pC, byte sH, byte sM, byte eH, byte eM, const char* t, const char* tT, String rB, String rBT, String lB, String lBT, String f, int sMFM, int eMFM, byte faj, byte dual);
    
};

class Day{
  public:
    String name;       // "Hétfő" 
    String date;       // "2024.03.11"
    String week;       // "A"
    byte dayOfWeek;    // 1
  
  public:
    Day(String n, String d, String w, byte dOW);

};

class Period{
  public:
    byte number;
    String beCseng;
    String kiCseng;
  
  public:
    Period(byte o, String beCs, String kiCs);

};
