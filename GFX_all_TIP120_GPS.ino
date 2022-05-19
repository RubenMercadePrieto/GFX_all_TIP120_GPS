//-------------- Olimex ESP32-EVB ------------------------
//-------------- MOD-LCD2.8RTP Screen configuration ------------------------
#include "Wire.h"
#include "Adafruit_STMPE610.h"
#include <Arduino_GFX_Library.h>

#define TFT_DC 15
#define TFT_CS 17
#define TFT_MOSI 2
#define TFT_CLK 14
#define TFT_MISO 0
#define TFT_RST 0

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_CLK, TFT_MOSI, TFT_MISO);
Arduino_GFX *gfx = new Arduino_ILI9341(bus, TFT_RST, 0 /* rotation */);
/*******************************************************************************
   End of Arduino_GFX setting
 ******************************************************************************/


//-------------- Wifi configuration ------------------------
// place in a separate file if sensitive
#include <WiFi.h>
const char* ssid     = "ESP32_wifi";
const char* password = "HolaHolaHola";
//-------------- Time server ------------------------
// Must have wifi initially to access NTP server and read time from internet
#include "time.h"
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;
struct tm timeinfoStart;
struct tm timeinfoNow;



//-------------- Internal Time RTC ------------------------
// This ESP32Time library is used in order to keep time, after initial configuration with NTP,
// even if there is no wifi. Configuration within Setup, otherwise errors with PNG
#include <ESP32Time.h>
ESP32Time rtc;

//-------------- DHT11 ------------------------
// whatever sensor you can use
#include "DHTesp.h"
int dhtPin = 18;
DHTesp dht;
TempAndHumidity newDHTValues;

//-------------- Touch screen ------------------------
// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 290
#define TS_MINY 285
#define TS_MAXX 7520
#define TS_MAXY 7510
#define TS_I2C_ADDRESS 0x4d

Adafruit_STMPE610 ts = Adafruit_STMPE610();

//-------------- Relay pins ------------------------
#define RELAY1  32
#define RELAY2  33

//-------------- microSD Card - SDMMC & LittleFS ------------------------
// ESP32 memory must be formatted first with LittleFS
/*
   Connect the SD card to the following pins:

   SD Card | ESP32   4-bit SD bus
   https://www.instructables.com/Select-SD-Interface-for-ESP32/
   https://www.olimex.com/forum/index.php?topic=7086.0
      D2/RES?             12    Not in Olimex diagram
      D3                  13    Not in Olimex diagram
      CMD/DI              15     Ok
      VSS                 GND
      VDD                 3.3V
      CLK                 14     Ok
      VSS                 GND
      D0/DAT0/MISO?       2       Ok   (add 1K pull up after flashing)
      D1/RES?             4     Not in Olimex diagram
*/
#define FS_NO_GLOBALS //allow LittleFS to coexist with SD card, define BEFORE including FS.h
#include "FS.h"
#include "LITTLEFS.h"
#include "SD_MMC.h"
#define SPIFFS LITTLEFS
#include "SDMMC_func.h"   //auxiliary file with all necessary functions for files"

//-------------- PNG images ------------------------
#include <PNGdec.h>
PNG png;
fs::File pngFile;
int16_t w, h, xOffset, yOffset;
#include "PNG_func.h"   //auxiliary file with all necessary functions for PNG images"

//-------------- Graph from large file ------------------------
#include "LongFile_Graph.h"

//-------------- HTTPS Virtual Earth maps------------------------
#include <HTTPClient.h>

//dev.virtualearth.net
//expire 13/5/2025
const char* root_ca = \
                      "-----BEGIN CERTIFICATE-----\n" \
                      "MIIDdzCCAl+gAwIBAgIEAgAAuTANBgkqhkiG9w0BAQUFADBaMQswCQYDVQQGEwJJ\n" \
                      "RTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJlclRydXN0MSIwIAYD\n" \
                      "VQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTAwMDUxMjE4NDYwMFoX\n" \
                      "DTI1MDUxMjIzNTkwMFowWjELMAkGA1UEBhMCSUUxEjAQBgNVBAoTCUJhbHRpbW9y\n" \
                      "ZTETMBEGA1UECxMKQ3liZXJUcnVzdDEiMCAGA1UEAxMZQmFsdGltb3JlIEN5YmVy\n" \
                      "VHJ1c3QgUm9vdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAKMEuyKr\n" \
                      "mD1X6CZymrV51Cni4eiVgLGw41uOKymaZN+hXe2wCQVt2yguzmKiYv60iNoS6zjr\n" \
                      "IZ3AQSsBUnuId9Mcj8e6uYi1agnnc+gRQKfRzMpijS3ljwumUNKoUMMo6vWrJYeK\n" \
                      "mpYcqWe4PwzV9/lSEy/CG9VwcPCPwBLKBsua4dnKM3p31vjsufFoREJIE9LAwqSu\n" \
                      "XmD+tqYF/LTdB1kC1FkYmGP1pWPgkAx9XbIGevOF6uvUA65ehD5f/xXtabz5OTZy\n" \
                      "dc93Uk3zyZAsuT3lySNTPx8kmCFcB5kpvcY67Oduhjprl3RjM71oGDHweI12v/ye\n" \
                      "jl0qhqdNkNwnGjkCAwEAAaNFMEMwHQYDVR0OBBYEFOWdWTCCR1jMrPoIVDaGezq1\n" \
                      "BE3wMBIGA1UdEwEB/wQIMAYBAf8CAQMwDgYDVR0PAQH/BAQDAgEGMA0GCSqGSIb3\n" \
                      "DQEBBQUAA4IBAQCFDF2O5G9RaEIFoN27TyclhAO992T9Ldcw46QQF+vaKSm2eT92\n" \
                      "9hkTI7gQCvlYpNRhcL0EYWoSihfVCr3FvDB81ukMJY2GQE/szKN+OMY3EU/t3Wgx\n" \
                      "jkzSswF07r51XgdIGn9w/xZchMB5hbgF/X++ZRGjD8ACtPhSNzkE1akxehi/oCr0\n" \
                      "Epn3o0WC4zxe9Z2etciefC7IpJ5OCBRLbf1wbWsaY71k5h+3zvDyny67G7fyUIhz\n" \
                      "ksLi4xaNmjICq44Y3ekQEe5+NauQrz4wlHrQMz2nZQ/1/I6eYs9HRCwBXbsdtTLS\n" \
                      "R9I4LtD+gdwyah617jzV/OeBHRnDJELqYzmp\n" \
                      "-----END CERTIFICATE-----\n";

//-------------- TIP120 NPN power transistor ------------------------
int pinTIP120 = 12;

//-------------- GPS ------------------------
#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
static const int RXPin = 36; // GPS connection
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;  // The TinyGPSPlus object
SoftwareSerial ssGPS(RXPin);
String GPSdefault = "41.403833,2.118889";
String GPScoorStr;
int GPS_num_sat;
bool GPS_valid_coor = false;
float GPS_lat, GPS_long, GPS_alt_meter, GPS_hdop;
long GPS_age;
#include "GPS_func.h"

//-------------- Global variables ------------------------
unsigned long TimePressed, NTPLastUpdate, TimeNow, TimeDHT;
int ButtonPressed = 0;
bool Relay1ON = false;
bool Relay2ON = false;
bool ScreenOff = false;



void setup()
{
  Serial.begin(115200);
  // while (!Serial);

  //-------------- Power the screen ------------------------
  //  pinMode(pinTIP120, OUTPUT); // Set pin for output to control TIP120 Base pin
  //  digitalWrite(pinTIP120, HIGH);

  // ----------- Initialize the screen --------------------------
  // Screen can only start after closing SDMMC
  gfx->begin();
  gfx->setRotation(0);
  gfx->fillScreen(WHITE);


  //-------------- Wifi connection ------------------------
  Serial.print("Connecting to "); Serial.println(ssid);
  PrintCharTFT("Connecting to", 20, 20, BLACK, WHITE, 1);
  PrintCharTFT(ssid, 110, 20, RED, WHITE, 1);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() > 10000) break;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    PrintCharTFT("IP: " + String(WiFi.localIP().toString().c_str()), 20, 30, BLACK, WHITE, 1);
  }
  else {
    Serial.println("No WiFi access");
    PrintCharTFT("No WIFI!", 20, 30, BLACK, WHITE, 1);
  }

  //-------------- Get local time NTP ------------------------
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (WiFi.status() == WL_CONNECTED) {
    if (getLocalTime(&timeinfoStart)) {
      rtc.setTimeStruct(timeinfoStart);
    }
    else Serial.println("Failed to obtain time");
    Serial.println(&timeinfoStart, "%D, %H:%M:%S");
    timeinfoNow = timeinfoStart;
    PrintCharTFT("NTP server accessed", 20, 40, BLACK, WHITE, 1);
  }
  else {
    Serial.println("NTP server didnt check as there is no Wifi");
    Serial.println("Set some fake time");
    rtc.setTime(0, 0, 0, 1, 1, 2022);
    PrintCharTFT("No NTP server access", 20, 40, BLACK, WHITE, 1);
  }
  Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S"));
  PrintCharTFT(rtc.getTime("%A, %B %d %Y %H:%M:%S"), 20, 50, BLACK, WHITE, 1);

  //-------------- Initialize DHT sensor ------------------------
  dht.setup(dhtPin, DHTesp::DHT11);
  Serial.println("DHT initiated");
  if (dht.getStatus() != 0) {
    Serial.println("DHT11 error status: " + String(dht.getStatusString()));
  }
  PrintCharTFT("DHT sensor initiated", 20, 60, BLACK, WHITE, 1);

  //-------------- Initialize touch screen ------------------------
  Wire.begin();
  pinMode(TFT_DC, OUTPUT);
  // read diagnostics (optional but can help debug problems)
  //uint8_t x = gfx->readcommand8(ILI9341_RDMODE);
  delay(1000);
  // Touch screen start
  ts.begin(TS_I2C_ADDRESS);
  PrintCharTFT("Touch screen initiated", 20, 70, BLACK, WHITE, 1);

  //-------------- Define Relay pins as outputs ------------------------
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);


  TimePressed = millis();
  NTPLastUpdate = TimePressed;
  TimeDHT = TimePressed;

  // ----------- Initialize SD MMC --------------------------
  PrintCharTFT("Starting SDMMC... screen off", 20, 80, BLACK, WHITE, 1);
  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD_MMC.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD_MMC card attached");
    return;
  }

  Serial.print("SD_MMC Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }


  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("SD_MMC Card Size: %lluMB\n", cardSize);

  // ----------- Example Functions for SD MMC --------------------------
  listDir(SD_MMC, "/", 0);
  //  createDir(SD_MMC, "/mydir");
  //  listDir(SD_MMC, "/", 0);
  //  removeDir(SD_MMC, "/mydir");
  //  listDir(SD_MMC, "/", 2);
  //  writeFile(SD_MMC, "/hello.txt", "Hello ");
  //  appendFile(SD_MMC, "/hello.txt", "World!\n");
  //  readFile(SD_MMC, "/hello.txt");
  //  deleteFile(SD_MMC, "/foo.txt");
  //  renameFile(SD_MMC, "/hello.txt", "/foo.txt");
  //  readFile(SD_MMC, "/foo.txt");
  //  testFileIO(SD_MMC, "/test.txt");
  Serial.printf("Total space: %lluMB\n", SD_MMC.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));

  // ----------- Initialize LittleFS --------------------------
  Serial.println("Testing Little FS");
  if (!LITTLEFS.begin()) {
    Serial.println("LITTLEFS Mount Failed");
    return;
  }
  // ----------- Example Functions LittleFS --------------------------
  Serial.println("----list Dir LittleFS 1----");
  listDir(LITTLEFS, "/", 1);
  //  Serial.println("----remove old dir----");
  //  removeDir(LITTLEFS, "/mydir");
  //  Serial.println("----create a new dir----");
  //  createDir(LITTLEFS, "/mydir");
  //  Serial.println("----remove the new dir----");
  //  removeDir(LITTLEFS, "/mydir");
  //  Serial.println("----create the new again----");
  //  createDir(LITTLEFS, "/mydir");
  //  Serial.println("----create and work with file----");
  //  writeFile(LITTLEFS, "/mydir/hello.txt", "Hello ");
  //  appendFile(LITTLEFS, "/mydir/hello.txt", "World!\n");
  //  Serial.println("----list 2----");
  //  listDir(LITTLEFS, "/", 1);
  //  Serial.println("----attempt to remove dir w/ file----");
  //  removeDir(LITTLEFS, "/mydir");
  //  Serial.println("----remove dir after deleting file----");
  //  deleteFile(LITTLEFS, "/mydir/hello.txt");
  //  removeDir(LITTLEFS, "/mydir");
  //  Serial.println("----list 3----");
  //  listDir(LITTLEFS, "/", 1);

  // Test to copy from LittleFS a SDMMC
  //https://www.reddit.com/r/esp32/comments/b9l8kb/copying_file_sd_spiffs_arduino_ide/
  //  Serial.println("Copy from LittleFS to SDMMC");
  //  writeFile(LITTLEFS, "/ruben.txt", "Hello ");
  //  appendFile(LITTLEFS, "/ruben.txt", "World! Test!\n");
  //  fs::File sourceFile = LITTLEFS.open("/ruben.txt");
  //  fs::File destFile = SD_MMC.open("/rubenSD.txt", FILE_APPEND);
  //  static uint8_t buf[512];
  //  while ( sourceFile.read( buf, 512) ) {
  //    destFile.write( buf, 512 );
  //  }
  //  destFile.close();
  //  sourceFile.close();
  //  listDir(SD_MMC, "/", 2);
  //  readFile(SD_MMC, "/rubenSD.txt");

  //Get initial DHT data for fun
  getTemperature();

  // ----------- Delete old DHT data file --------------------------
  if (LITTLEFS.remove("/dataDHT.csv")) {
    Serial.println("Old dataDHT.csv file deleted");
  }
  // ----------- Create new DHT data file --------------------------
  fs::File datafile = LITTLEFS.open("/dataDHT.csv", FILE_APPEND);
  // save the date, time, temperature and humidity, comma separated
  datafile.println("AAPL_date,AAPL_time,AAPL_Temp,AAPL_Hum"); //column headers
  datafile.close();
  readFile(LITTLEFS, "/dataDHT.csv");   //check that file was created succesfully
  SD_MMC.end(); //close SD properly, in order that no crashes later on
  Serial.println("Stoping SD MMC");
  Serial.println( "LittleFS & SD MMC Test complete" );

  gfx->begin();
  PrintCharTFT("Done testing SD files", 20, 90, BLACK, WHITE, 1);
  PrintCharTFT("Testing PNG images", 20, 100, BLACK, WHITE, 1);

  PrintCharTFT("Prof. Ruben Mercade Prieto", 40, 140, BLACK, WHITE, 1);
  DrawPNG("/IQS60_40.png", 90, 160);


  //-------------- GPS ------------------------
  ssGPS.begin(GPSBaud);
  getnewGPScoor(4000);
  if (gps.charsProcessed() < 10) {
    Serial.println(F("No GPS data received: check wiring"));
    PrintCharTFT("GPS nonnection not working!", 20, 110, BLACK, WHITE, 1);
  }
  else {
    Serial.println(F("Connection to GPS sensor ok"));
    PrintCharTFT("Connection to GPS sensor ok", 20, 110, BLACK, WHITE, 1);
    printGPSserial();
  }
  GPS_valid_coor = gps.location.isValid();
  if (GPS_valid_coor == false) {    //if no valid GPS data, assume IQS
    GPScoorStr = GPSdefault;
  }


  // ----------- Test virtual earth png dowloading and display --------------------------
  // dowload gps image from internet
  //  getVirtualEarth(15, true);
  //  PrintCharTFT("VirtualEarth PNG downloaded", 20, 110, BLUE, WHITE, 1);
  //  delay(1000);
  //  gfx->fillScreen(BLACK);
  //  DrawPNG("/virtualearth.png", 0, 0);

  delay(1000);
  gfx->fillScreen(BLACK);
}

void loop()
{
  TimeNow = millis();
  // Clear Screen
  gfx->fillScreen(BLACK);
  //Print IQS School of Engineering Logo on top of the screen
  DrawPNG("/IQSSE240_82.png", 0, 0);

  //  gfx->draw16bitRGBBitmap(0, 0, (const uint16_t *)IQSSEBitmap, IQSSE_WIDTH, IQSSE_HEIGHT);
  // Write additional information over the logo, looks cool and dont waste space
  //Cannot use font size 1 with this library... need to use adhoc funciton with chars

  PrintCharTFT("Prof. Ruben Mercade Prieto", 80, 0, BLACK, WHITE, 1);
  if (WiFi.status() == WL_CONNECTED) {
    PrintCharTFT(ssid, 80, 10, BLACK, WHITE, 1);
    PrintCharTFT("IP: " + String(WiFi.localIP().toString().c_str()), 80, 20, BLACK, WHITE, 1);
  }
  else {
    PrintCharTFT("WIFI NOT AVAILABLE", 100, 15, BLACK, WHITE, 1);
  }
  if (GPS_valid_coor) {   //print GPS data in black if valid, in red if default
    PrintCharTFT("GPS: " + GPScoorStr, 80, 30, BLACK, WHITE, 1);
  }
  else {
    PrintCharTFT("GPS: " + GPScoorStr, 80, 30, RED, WHITE, 1);
  }
  PrintCharTFT(rtc.getTime("%D, %H:%M:%S"), 80, 40, BLACK, WHITE, 1);

  if (TimeNow - TimeDHT > 10000) {
    getTemperature();
    TimeDHT = TimeNow;
  }
  // Printing DHT data on the screen.
  //With Font size 2, we can use normal printing to screen
  gfx->setTextColor(WHITE);  gfx->setTextSize(2);
  gfx->setCursor(10, 90); gfx->print("Temp.: ");
  gfx->setTextColor(YELLOW); gfx->print(newDHTValues.temperature); gfx->print("C");
  gfx->setTextColor(WHITE); gfx->setCursor(10, 110); gfx->print("Hum. : ");
  gfx->setTextColor(YELLOW); gfx->print(newDHTValues.humidity); gfx->print("%");


  // Printing 6 buttons
  gfx->setTextColor(WHITE);  gfx->setTextSize(2);
  gfx->fillRoundRect(0, 170, 120, 50, 8, PINK);
  gfx->setCursor(10, 190);
  gfx->println("Graph");
  gfx->drawRoundRect(0, 170, 120, 50, 8, WHITE);

  if (WiFi.status() == WL_CONNECTED) {
    gfx->fillRoundRect(120, 170, 120, 50, 8, YELLOW);
    gfx->setCursor(130, 190);
    gfx->println("Wifi ON");
    gfx->drawRoundRect(120, 170, 120, 50, 8, WHITE);
  }
  else {
    //    gfx->fillRoundRect(120, 170, 120, 50, 8, YELLOW);
    gfx->setTextColor(YELLOW);  gfx->setTextSize(2);
    gfx->setCursor(130, 190);
    gfx->println("Wifi OFF");
    gfx->drawRoundRect(120, 170, 120, 50, 8, YELLOW);
  }

  gfx->setTextColor(WHITE);  gfx->setTextSize(2);
  gfx->fillRoundRect(0, 220, 120, 50, 8, BLUE);
  gfx->setCursor(10, 240);
  gfx->println("Read File");
  gfx->drawRoundRect(0, 220, 120, 50, 8, WHITE);

  gfx->fillRoundRect(120, 220, 120, 50, 8, CYAN);
  gfx->setCursor(130, 240);
  gfx->println("Save SD");
  gfx->drawRoundRect(120, 220, 120, 50, 8, WHITE);

  // The appearance for the Relay buttons depends if the relays are activated or not
  if (Relay1ON == false) {
    gfx->setTextColor(WHITE);  gfx->setTextSize(2);
    gfx->fillRoundRect(0, 270, 120, 50, 8, RED);
    gfx->setCursor(10, 290);
    gfx->println("Relay 1");
    gfx->drawRoundRect(0, 270, 120, 50, 8, WHITE);
  }
  else if (Relay1ON == true) {
    gfx->setTextColor(RED);  gfx->setTextSize(2);
    //  gfx->fillRoundRect(20, 250, 100, 50, 8, RED);
    gfx->setCursor(10, 290);
    gfx->println("Relay 1");
    gfx->drawRoundRect(0, 270, 120, 50, 8, RED);
  }

  //  if (Relay2ON == false) {
  gfx->setTextColor(WHITE);  gfx->setTextSize(2);
  gfx->fillRoundRect(120, 270, 120, 50, 8, GREEN);
  gfx->setCursor(130, 290);
  gfx->println("GPS Sat.");
  gfx->drawRoundRect(120, 270, 120, 50, 8, WHITE);
  //  }
  //  else if (Relay2ON == true) {
  //    gfx->setTextColor(GREEN);  gfx->setTextSize(2);
  //    //  gfx->fillRoundRect(120, 250, 100, 50, 8, GREEN);
  //    gfx->setCursor(130, 270);
  //    gfx->println("Relay 2");
  //    gfx->drawRoundRect(120, 250, 120, 50, 8, GREEN);
  //  }


  ButtonPressed = Get_Button();
  // Program will only continue to this point when the screen is touched. If not touched, it will stay within the function Get_button()
  // Define what to do if any of the 4 buttons is pressed

  if (ButtonPressed == 2)  {  //GPS Virtual Earth
    //    gfx->fillScreen(GREEN);
    //    gfx->setCursor(10, 100);
    //    gfx->setTextColor(WHITE);  gfx->setTextSize(3);
    //    if (Relay2ON == false) {
    //      gfx->println("Relay 2 ON!");
    //      Relay2ON = true;
    //      digitalWrite(RELAY2, HIGH);
    //      delay(500);
    //    }

    gfx->fillScreen(BLACK);
    PrintCharTFT("Getting new GPS data....", 20, 20, WHITE, BLACK, 1);
    getnewGPScoor(2000);
    if (gps.location.isValid() && (gps.satellites.value() >0)) {
      PrintCharTFT("GPS coordinates obtained!", 20, 30, WHITE, BLACK, 1);
      PrintCharTFT(GPScoorStr, 20, 40, WHITE, BLACK, 1);
      PrintCharTFT("Altitude (m):" + String(GPS_alt_meter, 1), 20, 50, WHITE, BLACK, 1);
      PrintCharTFT("Num Sat:" + String(GPS_num_sat), 20, 60, WHITE, BLACK, 1);
      PrintCharTFT("Resolution (HDOP):" + String(GPS_hdop), 20, 70, WHITE, BLACK, 1);
    }
    else {
      PrintCharTFT("No new GPS coordinates obtained... :(", 20, 30, WHITE, BLACK, 1);
      }


    if (WiFi.status() == WL_CONNECTED) {
      PrintCharTFT("Downloading VirtualEarth PNG....", 20, 80, WHITE, BLACK, 1);
      for (int i = 15; i <= 19; i = i + 2) {
        getVirtualEarth(i, true);    //true = satellite, false = road map
        gfx->fillScreen(BLACK);
        DrawPNG("/virtualearth.png", 0, 0);
        DrawPNG("/IQS60_40.png", 0, 0);
        gfx->fillRect(0, 290, 240, 30, BLACK);
        gfx->setTextColor(YELLOW); gfx->setTextSize(2);
        gfx->setCursor(0, 290);
        gfx->println(GPScoorStr);
        gfx->print("Zoom: "); gfx->print(i);
        gfx->fillCircle(120, 160, 3, YELLOW); //circle in the middle...
      }
    }
    else {
      PrintCharTFT("No Wifi access....", 20, 20, WHITE, BLACK, 1);
    }

    delay(2500);

    //    else {
    //      digitalWrite(RELAY2, LOW);
    //      Relay2ON = false;
    //      gfx->println("Relay 2 OFF!");
    //      delay(500);
    //    }
  }
  else if (ButtonPressed == 1) {    // RELAY 1
    gfx->fillScreen(RED);
    gfx->setCursor(10, 100);
    gfx->setTextColor(WHITE);  gfx->setTextSize(3);
    if (Relay1ON == false) {
      gfx->println("Relay 1 ON!");
      Relay1ON = true;
      digitalWrite(RELAY1, HIGH);
      delay(500);
    }
    else {
      digitalWrite(RELAY1, LOW);
      Relay1ON = false;
      gfx->println("Relay 1 OFF!");
      delay(500);
    }
  }
  else if (ButtonPressed == 4) {  // SHOW DHT DATA
    // reads the LittleFS file with the DHT data, displayed in serial and in the screen
    readFile(LITTLEFS, "/dataDHT.csv");
    readFileTFTScreen(LITTLEFS, "/dataDHT.csv");
    delay(5000);
  }
  else if (ButtonPressed == 3) {  //SAVE DATA TO SD MMC
    gfx->fillScreen(CYAN);
    gfx->setCursor(0, 100);
    gfx->setTextColor(WHITE);  gfx->setTextSize(2);
    gfx->println("Saving Data to SD!");
    SaveSDcard();
    delay(1000);
    // restart tft screen after saving data with SDMMC
    gfx->begin();
  }
  else if (ButtonPressed == 5) {  //Wifi button
    if (WiFi.status() == WL_CONNECTED) {
      WiFi.disconnect();
      gfx->fillScreen(YELLOW);
      gfx->setCursor(20, 100);
      gfx->setTextColor(BLACK);  gfx->setTextSize(2);
      gfx->println("Wifi disconnected!");
      delay(2000);
    }
    else {
      WiFi.disconnect();
      delay(50);
      gfx->fillScreen(YELLOW);
      gfx->setCursor(20, 100);
      gfx->setTextColor(BLACK);  gfx->setTextSize(2);
      gfx->println("Trying to \n  reconect Wifi...");
      WiFi.begin(ssid, password);
      delay(8000);
      gfx->setCursor(20, 150);
      if (WiFi.status() == WL_CONNECTED) {
        gfx->println("Conected again! :D");
        //obtain time from NTP server
        if (getLocalTime(&timeinfoNow)) {
          rtc.setTimeStruct(timeinfoNow);
        }
      }
      else {
        gfx->println("Could not connect! :(");
      }
      delay(2000);
    }
  }
  else if (ButtonPressed == 6) {  //Graph DHT data
    int NumDataPoints = LoadFileMatrixDHT();
    Serial.print("Number of lines loaded: NumDataPoints ="); Serial.println(NumDataPoints);
    if (NumDataPoints > 3) {
      gfx->setRotation(3);
      drawPlotFile(NumDataPoints);
      delay(9000);
      gfx->setRotation(0);
    }
    else {
      gfx->fillScreen(YELLOW);
      gfx->setCursor(20, 100);
      gfx->setTextColor(BLACK);  gfx->setTextSize(2);
      gfx->println("Not enough\n data points...");
      delay(2000);
    }

  }
  delay(100); // leave some time for other stuff?

}



// Function to control the touch screen and the buttons
int Get_Button(void) {
  int result = 0;   //value to be returned by the function

  TS_Point p;
  while (1) { //important while. meanwhile screen not touched, it will loop here indefinetelly
    delay(50);

    p = ts.getPoint();    //get data from the touch screen
    TimeNow = millis();

    // the sleep function of the ILI9341 doesnt work, screen becomes fully powered white when sent command 10 hex. White consumed less than black in LCD, apparently
    if ((TimeNow > (TimePressed + 60000)) && ScreenOff == false) {  //turn screen black if didnt touch for 1 min
      ScreenOff = true;
      gfx->fillScreen(BLACK);
      PrintCharTFT("Screen off....", 90, 150, WHITE, BLACK, 1);
      Serial.print(ScreenOff); Serial.println("Screen power off in 2 s");
      delay(2000);
      digitalWrite(pinTIP120, LOW);  //cuts power to screen
    }

    // Get, store in memory, and print in screen new DHT data every 10 s, even if no touching
    if (TimeNow - TimeDHT > 10000) {
      getTemperature();

      //getnewGPScoor(200);  //GPS new data

      TimeDHT = TimeNow;
      if (ScreenOff == false) {   //only update screen if not black
        gfx->fillRect(0, 90, 240, 40, BLACK); //deleate previous DHT data from screen
        gfx->setTextColor(WHITE);  gfx->setTextSize(2);
        gfx->setCursor(10, 90); gfx->print("Temp.: "); //Print new DHT data to screen
        gfx->setTextColor(YELLOW); gfx->print(newDHTValues.temperature); gfx->print("C");
        gfx->setTextColor(WHITE); gfx->setCursor(10, 110); gfx->print("Hum. : ");
        gfx->setTextColor(YELLOW); gfx->print(newDHTValues.humidity); gfx->print("%");
        //update time on screen
        gfx->fillRect(80, 40, 160, 10, WHITE); //deleate previous time on screen
        PrintCharTFT(rtc.getTime("%D, %H:%M:%S"), 80, 40, BLACK, WHITE, 1);

      }
    }

    if (TimeNow > (TimePressed + 3000)) { //provide a time delay to check next touch, otherwise fake double activated button
      //wake up screen if touched
      if ((p.z > 127) && ScreenOff == true) {
        ScreenOff = false;
        digitalWrite(pinTIP120, HIGH);  //power screen
        Serial.print(ScreenOff); Serial.println("Screen alive again");
        TimePressed = millis();
        return 0;
      }
      else {
        if (p.z < 10 || p.z > 140) {     // =0 should be sufficient for no touch. sometimes it give a signal p.z 255 when not touching
          //    Serial.println("No touch");
        }
        else if (p.z != 129) {    //for a first touch, p.z = 128. A value of 129 is for continuous touching
          // scale the touch screen coordinates for the TFT screen resolution
          p.x = map(p.x, TS_MINX, TS_MAXX, 0, gfx->width());
          p.y = map(p.y, TS_MINY, TS_MAXY, 0, gfx->height());
          p.y = 320 - p.y;

          gfx->fillCircle(p.x, p.y, 5, YELLOW);  //show in the screen where you touched

          // If list to identify the location of all the buttons
          if ((p.y > 270) && (p.y < 320) && (p.x > 0) && (p.x < 120)) {
            result = 1;
          }
          else if ((p.y > 270) && (p.y < 320) && (p.x > 120) && (p.x < 240)) {
            result = 2;
          }
          else if ((p.y > 220) && (p.y < 270) && (p.x > 120) && (p.x < 240)) {
            result = 3;
          }
          else if ((p.y > 220) && (p.y < 270) && (p.x > 0) && (p.x < 120)) {
            result = 4;
          }
          else if ((p.y > 170) && (p.y < 220) && (p.x > 120) && (p.x < 240)) {
            result = 5;
          }
          else if ((p.y > 170) && (p.y < 220) && (p.x > 0) && (p.x < 120)) {
            result = 6;
          }
          else {
            result = 0;
          }
          if (result != 0) {
            TimePressed = millis();
            //          Serial.println(TimePressed);
          }
          Serial.print("X = "); Serial.print(p.x);
          Serial.print("\tY = "); Serial.print(p.y);
          Serial.print("\tPressure = "); Serial.print(p.z);
          Serial.print("\tButton = "); Serial.println(result);
          return result;  //return which button has been pressed

        }
      }
    }
  }
}

// function to get data from the sensor, print it in the Serial console and save in LittleFS file
void getTemperature(void) {
  newDHTValues = dht.getTempAndHumidity();
  Serial.println(" T:" + String(newDHTValues.temperature) + " H:" + String(newDHTValues.humidity));
  fs::File datafile = LITTLEFS.open("/dataDHT.csv", FILE_APPEND);
  if (datafile == true) { // if the file is available, write to it
    TempAndHumidity newValues = dht.getTempAndHumidity();
    // save new DHT data in the local file
    datafile.print(rtc.getTime("%D")); datafile.print(',');
    datafile.print(rtc.getTime("%H:%M:%S")); datafile.print(',');
    datafile.print(String(newValues.temperature)); datafile.print(',');
    datafile.println(String(newValues.humidity)); datafile.close();
    datafile.close();

  }
  else {
    Serial.println("Cannot open LittleFS dataDHT.csv file");
  }
}


// Function to copy data from LittleFS file to microSD as backup
//String filenameSD;
char *pathCharSD;

void SaveSDcard(void) {
  //As SDMMC cannot work together with the TFT screen, it must be started and stopped each time called.
  Serial.println("Starting SD MMC");
  if (!SD_MMC.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  delay(1000);

  // create a filename using the RTC time - new file every minute

  String filenameSD = "/" + rtc.getTime("%Y%m%d_%H%M") + ".csv";
  // need to convert string to char
  if (filenameSD.length() != 0) {
    pathCharSD = const_cast<char*>(filenameSD.c_str());
  }
  //  Serial.println(pathCharSD);
  //  Serial.println(filenameSD);
  //  appendFile(SD_MMC, pathCharSD, "Testing...");
  //  readFile(SD_MMC, pathCharSD);


  // Copy from LittleFS to SDMMC - improve?
  //https://www.reddit.com/r/esp32/comments/b9l8kb/copying_file_sd_spiffs_arduino_ide/
  Serial.println("Copy DHT data from LittleFS to SDMMC");

  fs::File sourceFile = LITTLEFS.open("/dataDHT.csv");
  //rewrite (instead of append) to avoid problems?
  fs::File destFile = SD_MMC.open(pathCharSD, FILE_WRITE);
  if (!destFile) {
    Serial.println("Cannot open data SD file");
    return;
  }
  // is this the best way to copy between LittleFS and SD? often some errors, e.g. bad lines
  static uint8_t buf[512];
  while ( sourceFile.read( buf, 512) ) {
    destFile.write( buf, 512 );
  }
  delay(100);
  destFile.close();
  sourceFile.close();
  readFile(SD_MMC, pathCharSD); //display the copy file, can see any mistakes
  listDir(SD_MMC, "/", 2);
  Serial.printf("Used space: %lluMB\n", SD_MMC.usedBytes() / (1024 * 1024));
  //Must close SDMMC to use TFT screen later on
  SD_MMC.end();
  Serial.println("Stoping SD MMC");
}

String filenamepath(int ZoomInt, bool Aerial) {
  String host = "https://dev.virtualearth.net/REST/v1/Imagery/Map/";
  String mapType;
  if (Aerial == true) {
    mapType = "aerial/";
  } else {
    mapType = "road/";
  }
  String Key = "AqfSSky-hFybSc0iEj8cpd2GQEJt8R2XPk9gNOKz4UWBqhB3u1o5gXsl-0zW7Uke";
  String Middle = "?mapSize=240,320&fmt=png&key=";
  String resturnStr = "";
  resturnStr += host;
  resturnStr += mapType;
  resturnStr += GPScoorStr;
  resturnStr += "/" + String(ZoomInt);
  resturnStr += Middle;
  resturnStr += Key;
  return resturnStr;
}

void getVirtualEarth(int ZoomInt, bool Aerial) {
  String StrPath = filenamepath(ZoomInt, Aerial);
  Serial.println(StrPath);

  fs::File file = LITTLEFS.open("/virtualearth.png", "w+");

  if (!file) {
    Serial.println("Virualearth.png - failed to open file for writing");
    return;
  }
  if ((WiFi.status() == WL_CONNECTED)) { //Check the current connection status

    HTTPClient http;

    //    http.begin( "https://dev.virtualearth.net/REST/v1/Imagery/Map/aerial/41.4037,2.1190/17?mapSize=240,320&fmt=png&key=AqfSSky-hFybSc0iEj8cpd2GQEJt8R2XPk9gNOKz4UWBqhB3u1o5gXsl-0zW7Uke", root_ca); //Specify the URL and certificate
    http.begin( StrPath, root_ca); //Specify the URL and certificate
    int httpCode = http.GET();                                                  //Make the request

    if (httpCode == HTTP_CODE_OK) { //Check for the returning code

      http.writeToStream(&file);
    }
    else {
      Serial.println("Error on HTTP request");
      //      String payload = http.getString();
      //      Serial.println(httpCode);
      //      Serial.println(payload);
    }
    file.close();
    Serial.println("you have finished downloading");
    http.end(); //Free the resources
  }

  //  delay(1000);
}
