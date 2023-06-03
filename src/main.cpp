#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <RTClib.h>
#include <Fonts/FreeMono12pt7b.h>
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128

#define OLED_MOSI   13
#define OLED_CLK   14
#define OLED_DC    5
#define OLED_CS    15
#define OLED_RESET 4

#define SDA_RTC D4
#define SCL_RTC D3

#define	BLACK           0x0000
#define WHITE           0xFFFF


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
DS1307 rtc;

Adafruit_SSD1351 display = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_CS, OLED_DC, OLED_MOSI, OLED_CLK, OLED_RESET);  

//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};



void lcdTestPattern(void);

void setup() {
  display.begin();
  display.fillRect(0, 0, 128, 128, BLACK);

  lcdTestPattern();
  delay(500);
  
  display.fillRect(0, 0, 128, 128, BLACK);
  display.println("oled ok");
  display.setTextColor(0x07E0, BLACK);
  display.println("waiting for network");

  WiFi.begin("user iPhone", "010101010");
  int atemps = 0;
  while (WiFi.status() != WL_CONNECTED && atemps < 18) {
    delay(500);
    display.print(".");
    atemps++;
  }
  display.println();

  display.println("Connected");
  display.print("IP: ");
  display.println(WiFi.localIP());

  display.println("init ntp");

  timeClient.begin();
  
  if (timeClient.isTimeSet()) {
    //timeClient.setTimeOffset(10800);
    display.println("ntp OK");
  } else {
    display.println("ntp error. Skip sync");
  }
  
  display.println("init RTC");
  
  Wire.begin(SDA_RTC, SCL_RTC);
  rtc.begin();

  timeClient.update();

  if (!rtc.isrunning() && !timeClient.isTimeSet()) {
    display.println("RTC is NOT running!");
    display.println("set build time");
    rtc.adjust(DateTime(__DATE__, __TIME__)); // set build time
  } else if (timeClient.isTimeSet()) {
    
    display.println("Sync RTC");
    rtc.adjust(timeClient.getEpochTime());
    rtc.adjust(rtc.now().unixtime() + 10800);
  }
  
  delay(500);
  display.fillRect(0, 0, 128, 128, BLACK);

}

void loop() {
  
  DateTime now = rtc.now();

  display.setCursor(0, 0);
  display.print("IP: ");
  display.println(WiFi.localIP());

  time_t epochTime = now.unixtime();
  struct tm *ptm = gmtime ((time_t *)&epochTime); 
  
  GFXcanvas1 canvas(128, 19); 
  canvas.setFont(&FreeMono12pt7b);
  canvas.setCursor(0, 18);
  canvas.printf("%02d:%02d:%02d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
  display.drawBitmap(6, 24, canvas.getBuffer(), 128, 19, WHITE, BLACK); 
  
  String weekDay = weekDays[now.dayOfWeek()];

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  String currentMonthName = months[currentMonth-1];
  int currentYear = ptm->tm_year+1900;

  //String currentDate = sptintf("%02d:%02d:%02d", monthDay, currentMonth, currentYear);//String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear);

  display.setCursor(40, 50);
  
  display.println(weekDay);  
  display.println();
  
  display.setTextSize(2);
  display.setCursor(4, 70);
  display.printf("%02d:%02d:%02d", monthDay, currentMonth, currentYear);

  display.setTextSize(1);
  display.setCursor(34, 118);
  display.setTextColor(0x07FF, BLACK);
  display.println(epochTime);
  display.setTextColor(0x07E0, BLACK);

}

void lcdTestPattern(void)
{
  static const uint16_t PROGMEM colors[] =
    { 0xF800, 0xFFE0, 0x07E0, 0x07FF, 0x001F, 0xF81F, BLACK, WHITE };

  for(uint8_t c=0; c<8; c++) {
    display.fillRect(0, display.height() * c / 8, display.width(), display.height() / 8,
      pgm_read_word(&colors[c]));
  }
}