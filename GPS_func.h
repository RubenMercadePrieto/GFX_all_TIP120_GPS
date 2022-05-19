//GPS functions

// This custom version of delay() ensures that the gps object
// is being "fed".

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ssGPS.available())
      gps.encode(ssGPS.read());
  } while (millis() - start < ms);
}


static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i = strlen(sz); i < len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len - 1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i = 0; i < len; ++i)
    Serial.print(i < slen ? str[i] : ' ');
  smartDelay(0);
}

void printGPSserial(void) {
	Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Chars Sentences Checksum"));
  Serial.println(F("           (deg)      (deg)       Age                      Age  (m)      RX    RX        Fail"));
  Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));
    printInt(gps.satellites.value(), true, 5);
  printFloat(gps.hdop.hdop(), true, 6, 1);
  printFloat(gps.location.lat(), true, 11, 6);
  printFloat(gps.location.lng(), true, 12, 6);
  printInt(gps.location.age(), true, 5);
  printDateTime(gps.date, gps.time);
  printFloat(gps.altitude.meters(), true, 7, 2);
  //printFloat(gps.course.deg(), true, 7, 2);
  //printFloat(gps.speed.kmph(), true, 6, 2);
  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
	Serial.println();
}

void getnewGPScoor(int time_ms){
	smartDelay(time_ms);
    printGPSserial();
	if (gps.location.isValid() && (gps.satellites.value() > 0)) {
	GPS_num_sat = gps.satellites.value();
	GPS_lat = gps.location.lat();
	GPS_long = gps.location.lng();
	GPS_alt_meter = gps.altitude.meters();
	GPS_age = gps.location.age();
	GPS_hdop = gps.hdop.hdop();
	GPS_valid_coor = true;
	GPScoorStr = String(GPS_lat,6)+",";
	GPScoorStr += String(GPS_long,6);
	}
	
}