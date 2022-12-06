/*
 * CheerLights for the ESP8266 using the Arduino IDE
 * Protocol: MQTT
 * Setup: Edit the WiFiMulti.addAPP() with your SSID and Password. Configure FastLED for your LEDs (Tip: Search for the text CHANGEME)
                 .--._.--.--.__.--.--.__.--.--.__.--.--._.--.
               _(_      _Y_      _Y_      _Y_      _Y_      _)_
              [___]    [___]    [___]    [___]    [___]    [___]
              /:' \    /:' \    /:' \    /:' \    /:' \    /:' \
             |::   |  |::   |  |::   |  |::   |  |::   |  |::   |
             \::.  /  \::.  /  \::.  /  \::.  /  \::.  /  \::.  /
         jgs  \::./    \::./    \::./    \::./    \::./    \::./
               '='      '='      '='      '='      '='      '='
 * Multi RGB LED (FastLED) version by jfrmilner
 * https://github.com/jfrmilner/Arduino-CheerLights
 * Demo - https://twitter.com/jfrmilner/status/1072287635396153346
 */

// libraries
#include <FastLED.h> //https://github.com/FastLED/FastLED
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient

// esp8266 client
ESP8266WiFiMulti WiFiMulti;
WiFiClient espClient;

// mqtt client
PubSubClient client(espClient);
/*
 *   CheerLights with MQTT and fancy colour transition effects!
 *   MQTT allows for real-time subscription to the CheerLights feed. When someone changes the colour, the colour gets published
 *   immediately over the “HEX” topic. MQTT also allows for your devices to stay connected versus polling the API for changes.
 */
String chipId = String(ESP.getChipId(), HEX);
const char* MQTT_SERVER = "mqtt.cheerlights.com";
const char* MQTT_CLIENTNAME = chipId.c_str();

// FastLED Setup CHANGEME
#define DATA_PIN    2
#define CLOCK_PIN   4
#define COLOR_ORDER RGB
#define CHIPSET     WS2801
#define NUM_LEDS    25
#define BRIGHTNESS  96

CRGB leds[NUM_LEDS];

// Transition effects
// colour history
unsigned long lastColour = 0;
// current/new colour
CRGB colourNew = CRGB::Black;

// connection management function
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Client:");
    Serial.println(chipId);
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENTNAME)) {
      Serial.println("connected");
      // subscribe (topic, [qos])
      client.subscribe("hex", 0);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// mqtt callback function
void callback(char* topic, byte* payload, unsigned int length) {
  // create colour c string
  char colourCStr[length];
  for (int i = 0; i < length; i++) {
    if (payload[i] != 10) {
      colourCStr[i] = (char)payload[i];
      colourCStr[i + 1] = '\0';
    }
  }

  if ((colourCStr[0] == '#') && (strlen(colourCStr) == 7)) {
    unsigned long colour = strtoul(colourCStr + 1, NULL, 16);
    // Update LEDs only when colour changes
    if (colour != lastColour) {
      lastColour = colour;
      Serial.print(">#");
      Serial.print(colour, HEX);
      Serial.println('<');
      CRGB col = colour;
      colourNew = col;

      // Run Colour Transition (Old to New)
      // randomTransition
      int var = random(2, 4);
      //          int var = 2; //Static Transition - Debug
      Serial.print("Transition:");
      Serial.println(var);
      switch (var) {
        case 1:
          instantColor(colourNew);
          break;
        case 2:
          fadeTowardColor(colourNew);
          break;
        case 3:
          colourWipe(colourNew);
          break;
        case 4:
          shuffleIn(colourNew);
          break;
        default:
          break;
      }
    }
  }
}

////Transitions
// instantColor - Instantly change to new colour
void instantColor(CRGB colourNew) {
  fill_solid(leds, NUM_LEDS, colourNew);
  FastLED.show();
}
// instantColor end

/*
 * shuffleIn - Shuffles in a new colour one pixel at a time over ~4 seconds
 * Uses Fisher–Yates shuffle http://en.wikipedia.org/wiki/Fisher-Yates_shuffle
 */
void shuffleArray(int* t, int n) {
  randomSeed(RANDOM_REG32);
  while (--n >= 2) {
    // n is now the last pertinent index
    int k = random(n);  // 0 <= k <= n - 1
    // Swap
    int temp = t[n];
    t[n] = t[k];
    t[k] = temp;
  }  // end of while
}

void shuffleIn(CRGB colourNew) {
  unsigned long timeStart = millis();
  CRGB black = CRGB::Black;

  // create array
  int t[NUM_LEDS];
  for (int i = 0; i < NUM_LEDS; i++) {
    t[i] = i;
  }

  // shuffle LED array
  shuffleArray(t, NUM_LEDS);

  // show results
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[(t[i])] = colourNew;
    //  Serial.println (t[i]);
    FastLED.delay(3950 / NUM_LEDS);
  }

  unsigned long timeend = millis();
  Serial.print("shuffleIn - time:");
  unsigned long timespan = (timeend - timeStart);
  Serial.println(timespan);
}
// shuffleIn end

/*
 * colourWipe - Fades LEDs to Black then populates the new colour either
 * bottom up or top down.
 */
void colourWipe(CRGB colourNew) {
  unsigned long timeStart = millis();
  CRGB black = CRGB::Black;
  do {
    fadeToBlackBy(leds, NUM_LEDS, 1);  // 8 bit, 1 = slow, 255 = fast
    FastLED.delay(10);
  } while (leds[10] != black);

  // calculate delay interval
  // dark colours fadeToBlack faster and the more NUM_LEDS take longer to colour
  unsigned long timeEndFade = millis();
  unsigned long timespanEndFade = (timeEndFade - timeStart);
  unsigned long timeRemaining = 3950 - timespanEndFade;
  unsigned long delayInterval = timeRemaining / NUM_LEDS;

  //  Serial.print("timespanEndFade:");
  //  Serial.println(timespanEndFade);
  //  Serial.print("timeRemaining:");
  //  Serial.println(timeRemaining);
  //  Serial.print("delayInterval:");
  //  Serial.println(delayInterval);

  if ((int)random(2) == 1) {
    // bottom up
    Serial.println(F("colourWipe bottom up"));
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = colourNew;
      FastLED.delay(delayInterval);
    }
  } else {
    // top down
    Serial.println(F("colourWipe top down"));
    for (int i = NUM_LEDS - 1; i >= 0; i--) {
      leds[i] = colourNew;
      FastLED.delay(delayInterval);
    }
  }

  unsigned long timeend = millis();
  Serial.print("colourWipe - time:");
  unsigned long timespan = (timeend - timeStart);
  Serial.println(timespan);
}
// colourWipe end

//fadeTowardColor - Fades the RGB colour toward a new RGB colour. Speed is dependant on how close current colour is to new colour (1-4seconds)
//Uses code by Mark Kriegsman https://gist.github.com/kriegsman/d0a5ed3c8f38c64adcb4837dafb6e690
void fadeTowardColor(CRGB colourNew) {
  // Blend one CRGB color toward another CRGB color
  unsigned long timeStart = millis();
  do {
    // fade all existing pixels toward colourNew by "1" (out of 255)
    fadeTowardColorArray(leds, NUM_LEDS, colourNew, 1);
    FastLED.delay(15);
  } while (leds[10] != colourNew);

  unsigned long timeend = millis();
  Serial.print("fadeTowardColor - time:");
  unsigned long timespan = (timeend - timeStart);
  Serial.println(timespan);
}
// Blends one uint8_t toward another by a given amount
void nblendU8TowardU8(uint8_t& cur, const uint8_t target, uint8_t amount) {
  if (cur == target)
    return;

  if (cur < target) {
    uint8_t delta = target - cur;
    delta = scale8_video(delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video(delta, amount);
    cur -= delta;
  }
}
// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor(CRGB& cur, const CRGB& target, uint8_t amount) {
  nblendU8TowardU8(cur.red, target.red, amount);
  nblendU8TowardU8(cur.green, target.green, amount);
  nblendU8TowardU8(cur.blue, target.blue, amount);
  return cur;
}
// Fade an entire array of CRGBs toward a given new color by a given amount
// This function modifies the pixel array in place.
void fadeTowardColorArray(CRGB* L,
                          uint16_t N,
                          const CRGB& colourNew,
                          uint8_t fadeAmount) {
  for (uint16_t i = 0; i < N; i++) {
    fadeTowardColor(L[i], colourNew, fadeAmount);
  }
}
// fadeTowardColor end

void setup() {
  // configure LEDs
  pinMode(LED_BUILTIN, OUTPUT);

  // serial
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // wifi
  // CHANGEME SSID, Password
  WiFiMulti.addAP("CHANGEME", "CHANGEME");
  Serial.print("WiFi Connecting.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  // mqtt
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  // fastled
  // FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(0xFFAFAF);
  FastLED.setBrightness(BRIGHTNESS);

  // clear LEDs
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();  // process mqtt tasks

  delay(10);
}
