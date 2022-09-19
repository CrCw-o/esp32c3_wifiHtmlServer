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
const int np = 8;       // pin for Neopixels
const int npCount = 25; // number of pixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(npCount, np, NEO_GRB + NEO_KHZ800);

// the XML array size needs to be bigger that your maximum expected size. 2048 is way too big for this example
char XML[2048];

// just some buffer holder for char operations
char buf[32];
bool  LED0 = true;


// build a simple HTML page, as a string
// buttons to turn status LED on and off, or run a rainbow cycle
char html[] = "<html><head><title>ESP32-C3FH4-RGB</title>"
              "<meta name=\"viewport\" content=\"width=device-width initial-scale=1\">"
              "<style>html{font-family: Helvetica; display:inline-block; margin: 0px auto; text-align: center;} "
              "h1{color: #0F3376;}p{font-size: 1.3rem;}"
              ".button{display: inline-block; background-color: #36ba59; border: none;"
              "color: white; padding: 8px 30px; text-decoration: none; font-size: 20px; cursor: pointer;}"
              ".button2{background-color: #ba4536;} .button3{background-color: #ce4efc;}</style></head>"
              "<body><h1>ESP32-C3FH4-RGB Demo</h1>"
              "<p><a href=\"/H\"><button class=\"button\">led on</button></a></p>"
              "<p><a href=\"/L\"><button class=\"button button2\">led off</button></a></p>"
              "<p><a href=\"/R\"><button class=\"button button3\">rainbow</button></a></p>"
              "</body></html>";

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
  for(x=0; x< strip.numPixels(); x++) {
    strip.setPixelColor(x, 0, 0, 0, 0);
  }
  strip.show();
  
}



// web server path handlers
void handleRoot() {
  server.sendHeader("Server", "Demo (ESP32-C3FH4-RGB)");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", html);
}

void handleHighLed() {
  digitalWrite(led, HIGH);
  handleRoot();
}

void handleLowLed() {
  digitalWrite(led, LOW);
  handleRoot();
}

void handleRainbow() {
  rainbowCycle(3);
  handleRoot();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
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

// end o// function managed by an .on method to handle slider actions on the web page
// this example will get the passed string called VALUE and conver to a pwm value
// and control the fan speed
void UpdateSlider() {
  String t_state = server.arg("VALUE");

  // conver the string sent from the web page to an int
  int FanSpeed = t_state.toInt();
  Serial.print("UpdateSlider"); Serial.println(FanSpeed);
  // now set the PWM duty cycle
  //ledcWrite(0, FanSpeed);


  // YOU MUST SEND SOMETHING BACK TO THE WEB PAGE--BASICALLY TO KEEP IT LIVE

  // option 1: send no information back, but at least keep the page live
  // just send nothing back
  // server.send(200, "text/plain", ""); //Send web page

  // option 2: send something back immediately, maybe a pass/fail indication, maybe a measured value
  // here is how you send data back immediately and NOT through the general XML page update code
  // my simple example guesses at fan speed--ideally measure it and send back real data
  // i avoid strings at all caost, hence all the code to start with "" in the buffer and build a
  // simple piece of data
  long FanRPM = map(FanSpeed, 0, 255, 0, 2400);
  strcpy(buf, "");
  sprintf(buf, "%d", FanRPM);
  sprintf(buf, buf);

  // now send it back
  server.send(200, "text/plain", buf); //Send web page

}

void ProcessButton_0() {
  LED0 = !LED0;
  digitalWrite(led, LED0);
  Serial.print("Button 0 "); Serial.println(LED0);
  server.send(200, "text/plain", ""); //Send web page
}

void ProcessButton_1() {
  ProcessButton_0();
}

void SendWebsite() {
  Serial.println("sending web page");
  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/html", PAGE_MAIN);
}

// code to send the main web page
// I avoid string data types at all cost hence all the char mainipulation code
void SendXML() {

  // Serial.println("sending xml");

  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  // send bitsA0
  sprintf(buf, "<B0>%d</B0>\n", 1);
  strcat(XML, buf);
  // send Volts0
  sprintf(buf, "<V0>%d.%d</V0>\n", (int) (3.14), abs((int) (3.14 * 10)  - ((int) (3.14) * 10)));
  strcat(XML, buf);

  // send bits1
  sprintf(buf, "<B1>%d</B1>\n", 27);
  strcat(XML, buf);
  // send Volts1
  sprintf(buf, "<V1>%d.%d</V1>\n", (int) (2.718), abs((int) (2.718 * 10)  - ((int) (2.718) * 10)));
  strcat(XML, buf);

  // show led0 status
  if (LED0) {
    strcat(XML, "<LED>1</LED>\n");
  }
  else {
    strcat(XML, "<LED>0</LED>\n");
  }
  bool SomeOutput = true;  //normally set if some values changed
  if (SomeOutput) {
    strcat(XML, "<SWITCH>1</SWITCH>\n");
  }
  else {
    strcat(XML, "<SWITCH>0</SWITCH>\n");
  }

  strcat(XML, "</Data>\n");
  // wanna see what the XML code looks like?
  // actually print it to the serial monitor and use some text editor to get the size
  // then pad and adjust char XML[2048]; above
  Serial.println(XML);

  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/xml", XML);


}



//end of functions from esp32_webserver --------------
// init code, do all the basics
void setup(void) {
  pinMode(led, OUTPUT);

  strip.begin();           // INITIALIZE NeoPixel strip object
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(25);

  Serial.begin(115200);
  sleep(15);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32C3
    // if the board has issues connecting, try uncommenting this
    WiFi.setTxPower(WIFI_POWER_8_5dBm);  // required for ESP32-C3FH4-RGB - WIFI_POWER_5dBm  others say 8_5_dBm
  #endif
  Serial.println("waiting for wifi connection");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // set hostname and advertise on the network (http://esp32rgb.local)
  if (MDNS.begin("esp32rgb")) {
    Serial.println("mDNS responder started");
  }

  // specify how to handle different URL paths
    server.on("/", SendWebsite);

  // upon esp getting /XML string, ESP will build and send the XML, this is how we refresh just parts of the web page
  server.on("/xml", SendXML);
  server.on("/UPDATE_SLIDER", UpdateSlider);
  server.on("/BUTTON_0", ProcessButton_0);
  server.on("/BUTTON_1", ProcessButton_1);

  server.onNotFound(handleNotFound);

  server.begin();
  blinkLed(); // quickly blink - we have an IP address, now ready
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2); // allow the cpu to switch to other tasks
}