/*
 * CheerLights for the ESP8266 using the Arduino IDE
 * 
                 .--._.--.--.__.--.--.__.--.--.__.--.--._.--.
               _(_      _Y_      _Y_      _Y_      _Y_      _)_
              [___]    [___]    [___]    [___]    [___]    [___]
              /:' \    /:' \    /:' \    /:' \    /:' \    /:' \
             |::   |  |::   |  |::   |  |::   |  |::   |  |::   |
             \::.  /  \::.  /  \::.  /  \::.  /  \::.  /  \::.  /
         jgs  \::./    \::./    \::./    \::./    \::./    \::./
               '='      '='      '='      '='      '='      '=' 
 * Single RGB by jfrmilner
 * Demo - https://twitter.com/jfrmilner/status/674316344968486912
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

ESP8266WiFiMulti WiFiMulti;

const int redPin = D6; //12
const int greenPin = D7; //13
const int bluePin = D8; //15

static unsigned long lastColour = 0;

void setcolour(int red, int green, int blue) {
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}


void setup() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);

    setcolour(1023,1023,1023);

    Serial.begin(115200);
   // Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFiMulti.addAP("SSID01", "password");
    WiFiMulti.addAP("SSID02", "password");
}

void loop() {
    // wait for WiFi connection
    if((WiFiMulti.run() == WL_CONNECTED)) {

        HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        // configure traged server and url
        http.begin("http://api.thingspeak.com/channels/1417/field/2/last.txt"); //HTTP

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);

            // good response
            if(httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                Serial.print("hex: ");
                Serial.println(payload);
                unsigned long colour;
                
                if ((payload[0] == '#') && (payload.length() == 7)) {
                    int r, g, b;
                    Serial.println(payload);
                    colour = strtoul(payload.c_str()+1, NULL, 16);
                      // Update LEDs only when colour changes
                      if (colour != lastColour) {
                          lastColour = colour;
                          Serial.print(">#");
                          Serial.print(colour, HEX);
                          Serial.println('<');
                          r = (colour & 0xFF0000) >> 16;
                          g = (colour & 0x00FF00) >>  8;
                          b = (colour & 0x0000FF);
                          r = map(r, 0, 255, 1023, 0); //The values of the colours are mapped
                          g = map(g, 0, 255, 1023, 0); //the opposite way because this RGB LED
                          b = map(b, 0, 255, 1023, 0); //is "Common-Anode". Also the PWM on an ESP is 10bit.
                          Serial.println(r);
                          Serial.println(g);
                          Serial.println(b);
                          setcolour(r, g, b);
                      }
                }
            
            }
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }

    delay(15000);
}

