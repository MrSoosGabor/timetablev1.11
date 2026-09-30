#include "classes.h"

Card::Card(byte dOW, String d, int fP, int pC, byte sH, byte sM, byte eH, byte eM, const char* t, const char* tT, String rB, String rBT, String lB, String lBT, String f, int sMFM, int eMFM, byte faj, byte dua) {
  dayOfWeek = dOW;
  date = d;
  fromPeriod = fP;
  periodsCount = pC;
  startHour = sH;
  startMinute = sM;
  endHour = eH;
  endMinute = eM;
  text = t;
  textTitle = tT;
  rightBottom = rB;
  rightBottomTitle = rBT;
  leftBottom = lB;
  leftBottomTitle = lBT;
  filter = f;
  startMinuteFromMidnight = sMFM;
  endMinuteFromMidnight = eMFM;
  fajta = faj;
  dual = dua;
}


Day::Day(String n, String d, String w, byte dOW) {
  name = n;
  date = d;
  week = w;
  dayOfWeek = dOW;
}


Period::Period(byte o, String beCs, String kiCs) {
  number = o;
  beCseng = beCs;
  kiCseng = kiCs;
}
