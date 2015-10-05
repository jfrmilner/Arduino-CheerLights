/*
  Christmas Internet of Things Tree Project by https://en.gravatar.com/jfrmilner

  This sketch connects to api.thingspeak.com and queries for the latest 
  CheerLights colour (using an Arduino Wifi shield) which is then applied
  to an LED strip. 
  Effects include colour wheel transistions, ie Green>Red = Green-Yellow-Red.
  Plus two waiting effects, Twinkle and Sparkle.    
  
  This sketch is based on three example sketchs:
  1. Arduino\WiFiWebClientRepeating
       created 23 April 2012
       modified 31 May 2012
       by Tom Igoe
       modified 13 Jan 2014
       by Federico Vanzati
     http://arduino.cc/en/Tutorial/WifiWebClientRepeating
  2. FastLED\ColorPallete 
  3. FastLED\Cylon
     https://github.com/FastLED/FastLED

 Circuit:
 * WiFi Shield-compatible Arduino board (Tested on Uno)
 * WiFi shield attached to pins SPI pins and pin 7
 * RGB LED strip (Tested with 25*WS2801) - many options here, 
   see FastLED link above for supported options. I have included a few 
   in Setup(), just comment out yours there.

 This code is in the public domain.
 */

#include <SPI.h>
#include <WiFi.h>
#include "FastLED.h"

char ssid[] = "ssid";      //  your network SSID (name) 
char pass[] = "changeme";   // your network password
//int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Initialize the Wifi client library
WiFiClient client;

// CheerLights API server address:
char server[] = "api.thingspeak.com";
unsigned long lastConnectionTime = 0;            // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10L * 1000L; // delay between updates, in milliseconds

//Debug RAM check function
int freeRamCheck = freeRam();

//handle_input function global vars
char token[30];
uint8_t input_len = 0;
boolean xMLmatch = false;
 
//fastLED custom functions vars

// How many leds in your strip?
#define NUM_LEDS 25

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 3
#define CLOCK_PIN 9

// Define the array of leds
CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;
TBlendType    currentBlending = LINEARBLEND;
uint8_t startIndex = 0;
CRGB newC = CRGB::Black;
CRGB oldC = CRGB::Black;
boolean tDirection = true;
boolean transitionComplete;
boolean colourChangeRequired;
String strMatchPrevious = "black";
int noChangeCount;
boolean effectSelection = true;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print(F("Attempting to connect to SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 3 seconds for connection:
    delay(3000);
  }

  // you're connected now, so print out the status:
  printWifiStatus();
  
  // Uncomment/edit one of the following lines for your leds arrangement.
  // FastLED.addLeds<TM1803, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1804, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<TM1809, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<UCS1903B, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205, DATA_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<GW6205_400, DATA_PIN, RGB>(leds, NUM_LEDS);
  
  // FastLED.addLeds<WS2801, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SM16716, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD8806, RGB>(leds, NUM_LEDS);

  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<SM16716, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
  // FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);

}



void loop() {
  // Run once Startup function to clear LEDs
  startup();
  
  //Debug - Check RAM usage
  if (freeRam() != freeRamCheck) {
    Serial.print(F("FreeRam: "));
    Serial.println(freeRam());
    freeRamCheck = freeRam();
  }
  
  // Clear XML Match Check for next loop 
  boolean xMLmatch = false;
 
  while (client.available()) {
    char c = client.read();
    // if there's incoming data from the net connection.
    // send it out the serial port.  This is for debugging
    // purposes only:
    //Serial.write(c);
    
    //read char by char HTTP request until XML Colour match
    if (xMLmatch == false) {
      handle_input(c);
    }
  }

  // if ten seconds have passed since your last connection,
  // then connect again and send data:
  if (millis() - lastConnectionTime > postingInterval) {
    httpRequest();
    Serial.print(F("No Change Count: "));
    Serial.println(noChangeCount);
  }

  // if a colour change is detected it will be processed here
  if (colourChangeRequired) {
    Serial.println(F("colourChangeRequired true"));
    // Change Colour
    transitionBlend(oldC, newC, tDirection);
    colourChangeRequired = false;
    // Toggle Transition Direction
    tDirection = !tDirection;
    // Update Old/Previous Colour
    oldC = newC;
  }
  
  // trigger effect
  if (noChangeCount >= 2) {
    unsigned long endTime = millis() + postingInterval;

    if (effectSelection) { Serial.println(F("run twinkle")); }
    else {Serial.println(F("run sparkle"));}

    while ( millis() <= endTime) {
      if (effectSelection) { effectTwinkle(); }
      else { effectSparkle();}
    }
    noChangeCount = 0;
    // Toggle effect
    effectSelection = !effectSelection;
  }
  
}

// this method makes a HTTP connection to the server:
void httpRequest() {
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 80)) {
    Serial.println(F("Checking thingspeak API for latest colour.."));
    // send the HTTP GET request to api.thingspeak.com:
    client.println(F("GET /channels/1417/field/1/last.xml HTTP/1.1"));
    client.println(F("Host: api.thingspeak.com"));
    client.println(F("User-Agent: arduino-jfmilner.co.uk"));
    client.println(F("Connection: close"));
    client.println();

    // note the time that the connection was made:
    lastConnectionTime = millis();
  }

  else {
    // if you couldn't make a connection:
    Serial.println(F("connection failed"));
  }

}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print(F("IP Address: "));
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print(F("signal strength (RSSI):"));
  Serial.print(rssi);
  Serial.println(F(" dBm"));
}


void strMatchToCRGB(String strMatch) {

  if (strMatch == strMatchPrevious) {
    Serial.println(F("No Change"));
    noChangeCount++;
    return;
  }

  if      (strMatch == "black")                                 {Serial.println("black");              newC = CRGB::Black;}
  else if (strMatch == "red")                                   {Serial.println("red");                newC = CRGB::Red;}
  else if (strMatch == "green")                                 {Serial.println("green");              newC = CRGB::Green;}
  else if (strMatch == "blue")                                  {Serial.println("blue");               newC = CRGB::Blue;}
  else if (strMatch == "cyan")                                  {Serial.println("cyan");               newC = CRGB::Cyan;}
  else if (strMatch == "white")                                 {Serial.println("white");              newC = CRGB::White;}
  else if (strMatch == "warmwhite" || strMatch == "oldlace" )   {Serial.println("warmwhite/oldlace");  newC = CRGB::OldLace; }
  else if (strMatch == "purple")                                {Serial.println("purple");             newC = CRGB::Purple;}
  else if (strMatch == "magenta")                               {Serial.println("magenta");            newC = CRGB::Magenta;}
  else if (strMatch == "yellow")                                {Serial.println("yellow");             newC = CRGB::Yellow;}
  else if (strMatch == "orange")                                {Serial.println("orange");             newC = CRGB::OrangeRed;}
  else if (strMatch == "pink")                                  {Serial.println("pink");               newC = CRGB::DeepPink;}

  strMatchPrevious = strMatch;
  colourChangeRequired = true;
  noChangeCount = 0;
}

//Show freeRam - http://playground.arduino.cc/Code/AvailableMemory
int freeRam () {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void handle_input(uint8_t c){

  String colour;
  char red [] = "red";
  char green [] = "green";
  char blue [] = "blue";
  char cyan [] = "cyan";
  char white [] = "white";
  char warmwhite [] = "warmwhite";
  char purple [] = "purple";
  char magenta [] = "magenta";
  char yellow [] = "yellow";
  char orange [] = "orange";
  char pink [] = "pink";
  char oldlace [] = "oldlace";


  /* looking for xml code in the response - <field1>warmwhite</field1>
  * c = '>', then a new tag sequence is starting
  */ 
  if (c == '>') {
      input_len = 0;
      return;
  }

  /* c = '>', a tag sequence is complete.
  * Check if we have a colour match
  */
  if (c == '<') {
    //Serial.println("End");
    token[input_len] = 0;

    if (strncmp(token, red, 3) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, green, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, blue, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, cyan, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, white, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, warmwhite, 9) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, purple, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, magenta, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, yellow, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, orange, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, pink, 10) == 0) {
      xMLmatch = true;
    }
    else if (strncmp(token, oldlace, 10) == 0) {
      xMLmatch = true;
    }
    else {
      return;
    }

    if (xMLmatch) {
      colour = token;
      //Serial.print("token:");
      //Serial.println(strlen (token) );
      Serial.print("Got colour: ");
      Serial.println(colour);
      strMatchToCRGB(colour);
    }
  }

  /* 
  otherwise we have an input character
  */
  if (input_len <= 10 - 1)
     token[input_len++] = c;
}


void startup() {
  //Startup Routine
  static boolean startup;
  if (startup == 0) {
    Serial.println(F("Running Startup Routine"));
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::White;
      FastLED.delay(30);
    }
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      FastLED.delay(30);
      startup = true;
    }
  }
}

// Transition function
void transitionBlend(CRGB oldC, CRGB newC, boolean tDirection) {
  transitionComplete = false;
  startIndex = 0;
  fill_gradient_RGB (currentPalette, 16, oldC, oldC, newC, newC);
  while (transitionComplete == false) {
    startIndex = startIndex + 1; /* motion speed */
    if (tDirection == true)       {    FFillLEDsFromPaletteColors(startIndex, true); }
    else if (tDirection == false) {    RFillLEDsFromPaletteColors(startIndex, true); }
    FastLED.delay(50);
  }
}

// Forward/Top Down Transition
void FFillLEDsFromPaletteColors(uint8_t colorIndex, boolean limit){
  uint8_t brightness = 255;
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 1;
    //Limit colourIndex to prevent overlap
    if (colorIndex == 255-NUM_LEDS && limit == true) {
      transitionComplete = true;
    }
  }
}

// Reverse/Bottom Up Transition
void RFillLEDsFromPaletteColors(uint8_t colorIndex, boolean limit){
  uint8_t brightness = 255;
  for(int i = NUM_LEDS-1; i >= 0; i--) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 1;
    //Limit colourIndex to prevent overlap
    if (colorIndex == 255-NUM_LEDS && limit == true) {
      transitionComplete = true;
    }
  }
}


/*
Effect function Twinkle
This effect creates a colour palette of the latest colour with black stripes.
It will run for the duration of the postingInterval var (10 Seconds by default) and
then fill all LEDs with the latest colour to prevent LEDs being left Black.
*/
void effectTwinkle() {
  fill_solid( currentPalette, 16, newC);
  // and set every fourth one to Black.
  currentPalette[0] = CRGB::Black;
  currentPalette[4] = CRGB::Black;
  currentPalette[8] = CRGB::Black;
  currentPalette[12] = CRGB::Black;

  unsigned long endTime = millis() + postingInterval;

  while ( millis() <= endTime) {
    startIndex = startIndex + 1; /* motion speed */
    FFillLEDsFromPaletteColors(startIndex, true);
    FastLED.delay(50);
  }
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to the latest colour
    leds[i] = newC;
  }
  FastLED.show();
}

/*
Effect function Sparkle
This effect creates a brighter led of the latest colour and cylons
back and forth across the stip.
*/
void effectSparkle() {
  // First slide the led in one direction
  for(int i = 0; i < NUM_LEDS; i++) {
    // Set the i'th led to the latest colour with extra brightness
    leds[i] = newC;
    leds[i] += 25;
    // Show the leds
    FastLED.delay(50);
    // Set the i'th led to the latest colour
    leds[i] = newC;
    // Show the leds and loop
    FastLED.delay(50);
  }

  // Now go in the other direction.  
  for(int i = NUM_LEDS; i >= 0; i--) {
    // Set the i'th led to the latest colour with extra brightness 
    leds[i] = newC;
    leds[i] += 25;
    // Show the leds
    FastLED.delay(50);
    // Set the i'th led to the latest colour
    leds[i] = newC;
    // Show the leds and loop
    FastLED.delay(50);
  }
}

//// Random colour generator which was used for offline testing
//CRGB randomColour() {
//static int newC;
//static int oldC;
//  
//while (newC == oldC) {
//newC = random(2, 12);
//}
//
//oldC = newC;
//
//if      (newC == 1)   {Serial.println("black");              return CRGB::Black;}
//else if (newC == 2)   {Serial.println("red");                return CRGB::Red;}
//else if (newC == 3)   {Serial.println("green");              return CRGB::Green;}
//else if (newC == 4)   {Serial.println("blue");               return CRGB::Blue;}
//else if (newC == 5)   {Serial.println("cyan");               return CRGB::Cyan;}
//else if (newC == 6)   {Serial.println("white");              return CRGB::White;}
//else if (newC == 7)   {Serial.println("warmwhite/oldlace");  return CRGB::OldLace; }
//else if (newC == 8)   {Serial.println("purple");             return CRGB::Purple;}
//else if (newC == 9)   {Serial.println("magenta");            return CRGB::Magenta;}
//else if (newC == 10)  {Serial.println("yellow");             return CRGB::Yellow;}
//else if (newC == 11)  {Serial.println("orange");             return CRGB::OrangeRed;}
//else if (newC == 12)  {Serial.println("pink");               return CRGB::DeepPink;}
//}
