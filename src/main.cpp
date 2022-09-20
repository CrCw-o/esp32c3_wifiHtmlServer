#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Adafruit_NeoPixel.h>
#include "webpagecontent.h"
#include "mySecrets.h"
#include "Secrets.h"
 
const char* ssid = YOUR_SSID;    // Wifi network
const char* password = YOUR_WIFIPASSWORD; // Wifi password

WebServer server(80);   // port for web server (80, default for HTTP)

const int led = ONBOARDLED;     // pin for the status LED
const int np = NEOPIXELPIN;       // pin for Neopixels
const int npCount = NUMNEOPIXELS; // number of pixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(npCount, np, NEO_GRB + NEO_KHZ800);


bool SomeOutput = false;  //normally set if some values changed

// just some buffer holder for char operations
char buf[32];
bool  LED0 = true;

void paintItBlack(){
  for(int x=0; x< strip.numPixels(); x++) {
    strip.setPixelColor(x, 0, 0, 0, 0);
  }
  strip.show();
}


uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

// NeoPixel colour functions
void rainbowCycle(uint8_t wait) {
  uint16_t i, j, x;
 
  for(j=0; j<256*3; j++) { // 3 cycles
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }

  // clear all pixels
  paintItBlack();
  
}

int StrToHex(char str[])
{
  return (int) strtol(str, 0, 16);
}

void handleNotFound() {
  String message = "............................\n Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message += "............................\n";
  server.sendHeader("Server", "Demo (ESP32-C3FH4-RGB)");
  server.send(404, "text/plain", message);
}

// function to rapidly blink the status LED
void blinkLed() {
  for (int i = 0; i <= 5; i++) {
  digitalWrite(led, HIGH);
  delay(100);                       
  digitalWrite(led, LOW);    
  delay(100); 
  }
}

//functions from esp32_webserver --------------

void UpdateLED() {

  /* String message = "The update led request\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: \nCount : ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  Serial.println(message);
  Serial.println("------------------------");
  */
  String t_Number = server.arg("VALUE");
  int LedIndex = t_Number.substring(1).toInt();
  // Serial.print("UpdateLED number is "); Serial.println(LedIndex);
  String t_Color = server.arg("COLOR");
  // Serial.print("UpdateLED color ->"); Serial.print(t_Color);Serial.println("<-");
  
  long long number = strtoll( t_Color.c_str() , NULL, 16);
  // Split them up into r, g, b values
  long long r = number >> 16;         //we shift so far that green and blue drop off
  long long g = number >> 8 & 0xFF;
  long long b = number & 0xFF;
  /* Serial.print("Rot: ");   Serial.println(r);
  Serial.print("Gruen: "); Serial.println(g);
  Serial.print("Blau: ");  Serial.println(b);
  */
  strip.setPixelColor((uint16_t)LedIndex,( uint8_t) r, ( uint8_t)g, ( uint8_t)b);
  strip.show();
  // YOU MUST SEND SOMETHING BACK TO THE WEB PAGE--BASICALLY TO KEEP IT LIVE

  // option 1: send no information back, but at least keep the page live
  // just send nothing back
  server.send(200, "text/plain", ""); //Send web page

  // option 2: send something back immediately, maybe a pass/fail indication, maybe a measured value
  // here is how you send data back immediately and NOT through the general XML page update code
  // my simple example guesses at fan speed--ideally measure it and send back real data
  // i avoid strings at all caost, hence all the code to start with "" in the buffer and build a
  // simple piece of data
  //long FanRPM = map(FanSpeed, 0, 255, 0, 2400);
  //strcpy(buf, "");
  //sprintf(buf, "%d", FanRPM);
  //sprintf(buf, buf);

  // now send it back
  //server.send(200, "text/plain", buf); //Send web page

}


void SendWebsite() {
  Serial.println("sending web page");
  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/html", PAGE_MAIN);
  paintItBlack();
}


//end of functions from esp32_webserver --------------
// init code, do all the basics
void setup(void) {
  pinMode(led, OUTPUT);

  strip.begin();           // INITIALIZE NeoPixel strip object
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(25);
  paintItBlack();

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32C3
    // if the board has issues connecting, try uncommenting this
    WiFi.setTxPower(WIFI_POWER_8_5dBm);  // required for ESP32-C3FH4-RGB - WIFI_POWER_5dBm  others say 8_5_dBm
  #endif
  sleep(10);
  Serial.println("waiting for wifi connection");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.print("\nConnected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // set hostname and advertise on the network (http://esp32rgb.local)
  if (MDNS.begin(MDNSNAME)) {
    Serial.print("mDNS responder started. Name is ");Serial.println(MDNSNAME);
  }

  // specify how to handle different URL paths
  server.on("/", SendWebsite);
  server.on("/UPDATE_LED", UpdateLED);
  server.onNotFound(handleNotFound);

  server.begin();
  blinkLed(); // quickly blink - we have an IP address, now ready
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2); // allow the cpu to switch to other tasks
}