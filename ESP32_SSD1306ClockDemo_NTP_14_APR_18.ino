#include <WiFi.h>
#include <TimeLib.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include "OLEDDisplayUi.h"
#include <NTPClient.h>

#define NTP_OFFSET  2  * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "time.nist.gov"

/*  Configuration values to be filled in as needed  */
#define SSID  "..";
#define PASS  "..";
#define WSHOST  "..";
#define WSPORT  81;
#define MQTT_CLIENT ESP.getChipId()
#define MQTT_SERVER "...";
#define MQTT_PORT 17332;
#define MQTT_USER "...";
#define MQTT_PASS "..";

#include <init.h> //  I use this library file to redefine the configuration values - remove if config data are defined abowe

const char *ssid = SSID;
const char *pass = PASS;
char* mqtt_client = "ESP123"; //String(ESP.getChipId());
char* mqtt_host = MQTT_SERVER;
uint16_t mqtt_port = MQTT_PORT;
char* mqtt_user = MQTT_USER;
char* mqtt_pass = MQTT_PASS;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
SSD1306Wire  display(0x3c, 5, 4);
OLEDDisplayUi ui ( &display );

int screenW = 128;
int screenH = 64;
int clockCenterX = screenW / 2;
int clockCenterY = ((screenH) / 2) ; // top yellow part is 16 px height
int clockRadius = 31;

void initNTP() {
  timeClient.begin();
  timeClient.setTimeOffset(NTP_OFFSET);
  timeClient.forceUpdate();
}

bool tryGetTime() {
  if (timeClient.update()) {
    setTime(timeClient.getEpochTime());
    return true;
  }
  return false;
}

// utility function for digital clock display: prints leading 0
String twoDigits(int digits) {
  if (digits < 10) {
    String i = '0' + String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  for ( int z = 0; z < 360; z = z + 30 ) {
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }
  float angle = second() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  angle = minute() * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  angle = hour() * 30 + int( ( minute() / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = String(hour()) + ":" + twoDigits(minute()) + ":" + twoDigits(second());
  String datenow = twoDigits(day()) + "-" + twoDigits(month()) + "-" + String(year());
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x , clockCenterY + y+2, timenow );
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x , clockCenterY + y-18, datenow );
}

FrameCallback frames[] = { analogClockFrame, digitalClockFrame };

int frameCount = 2;

OverlayCallback overlays[] = { clockOverlay };
int overlaysCount = 1;

void setup() {
  Serial.begin(115200);
  Serial.println();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) delay(500), Serial.print("."); Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  ui.setTargetFPS(60);
  ui.setIndicatorPosition(RIGHT);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.init();
  display.flipScreenVertically();
  initNTP();
  while (!tryGetTime()) Serial.print('.'); Serial.println();
  Serial.println(timeClient.getFormattedTime());
}

void loop() {
  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }
}
