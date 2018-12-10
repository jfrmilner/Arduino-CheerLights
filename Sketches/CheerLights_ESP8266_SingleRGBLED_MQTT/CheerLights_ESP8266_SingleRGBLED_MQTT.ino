/*
 * CheerLights for the ESP8266 using the Arduino IDE
 * Protocol: MQTT
 * Setup: Edit the WiFiMulti.addAPP with your SSID and Password, and add your ThingSpeak MQTT_APIKEY (Tip: Search for the text CHANGEME)
                 .--._.--.--.__.--.--.__.--.--.__.--.--._.--.
               _(_      _Y_      _Y_      _Y_      _Y_      _)_
              [___]    [___]    [___]    [___]    [___]    [___]
              /:' \    /:' \    /:' \    /:' \    /:' \    /:' \
             |::   |  |::   |  |::   |  |::   |  |::   |  |::   |
             \::.  /  \::.  /  \::.  /  \::.  /  \::.  /  \::.  /
         jgs  \::./    \::./    \::./    \::./    \::./    \::./
               '='      '='      '='      '='      '='      '=' 
 * Single RGB by jfrmilner
 * https://github.com/jfrmilner/Arduino-CheerLights
 * Demo - https://twitter.com/jfrmilner/status/674316344968486912
 */

// libraries
#include <ESP8266WiFi.h> //https://github.com/esp8266/Arduino
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h> //https://github.com/knolleary/pubsubclient

// esp8266 client
ESP8266WiFiMulti WiFiMulti;
WiFiClient espClient;

// mqtt client
PubSubClient client(espClient);
/*  
 *   ThingSpeak™ MQTT requires an MQTT API Key to subscribe to channel updates for both private and public channels. 
 *   Supply any unique user name and your MQTT API Key as a password parameter to the ThingSpeak MQTT broker when connecting. 
 *   Choose Account > My Profile to see your MQTT API key. https://uk.mathworks.com/help/thingspeak/subscribetoachannelfieldfeed.html
 *   
 */
const char* MQTT_SERVER = "mqtt.thingspeak.com";
const char* MQTT_CLIENTNAME = String(ESP.getChipId()).c_str();
const char* MQTT_USERNAME = String(ESP.getChipId()).c_str();
const char* MQTT_APIKEY = "CHANGEME";

// LED Pins
const int RED_PIN = 12; // aka D6
const int GREEN_PIN = 13; // aka D7
const int BLUE_PIN = 15; // aka D8
// colour history
unsigned long lastColour = 0;

// set LED colour function
void setcolour(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

// connection management function
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(MQTT_CLIENTNAME, MQTT_USERNAME, MQTT_APIKEY)) {
      Serial.println("connected");
      digitalWrite(LED_BUILTIN, LOW);
      //subscribe (topic, [qos])
      client.subscribe("channels/1417/subscribe/fields/field2", 0);
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
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
      colourCStr[i+1] = '\0';
    }
  }

  if ((colourCStr[0] == '#') && (strlen(colourCStr) == 7)) {
      int r, g, b;
      unsigned long colour = strtoul(colourCStr +1, NULL, 16); 
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
          b = map(b, 0, 255, 1023, 0); //is "Common-Anode". Also, the PWM on an ESP is 10bit.
          Serial.println(r);
          Serial.println(g);
          Serial.println(b);
          setcolour(r, g, b);
      }
  }
}

void setup() {
  // configure LEDs
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  setcolour(1023,1023,1023);

  // serial
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // wifi
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
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // process mqtt tasks
  delay(10);
}



