/**
 * This was put together using the example code
 * from Adafruit's mqtt library
 */

/*************************** The LED stuff ****************************/

#include "FastLED.h"

//The number of LED's in the strip
#define NUM_LEDS 100

//The data pin for the LED's
#define DATA_PIN 14

/**
 * The array of leds
 */
CRGB leds[NUM_LEDS];

/**
 * Used to tell which state the lights are in
 */
int current = 1;

/***************************************************
  This is for a thermostat using the ESP8266
  A special thanks to Adafruit for their
  tutorials, libraries and sample code
  This code only uploads data from the DHT22 sensor
  to Adafruits MQTT server
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
 
/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "YOUR WIFI NAME"
#define WLAN_PASS       "YOUR WIFI PASSWORD"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "YOUR ADAFUIT NAME"
#define AIO_KEY         "YOUR ADAFRUIT KEY, SHOULD BE ALFANUMERIC LIKE SO: 12ghf3hfk39f93nf93nf93jfn3"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff");

/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  Serial.begin(115200);
  delay(10);

  Serial.println(F("Adafruit MQTT demo"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff
  mqtt.subscribe(&onoffbutton);
}


void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here
 
  Adafruit_MQTT_Subscribe *subscription;
  Serial.println("right above while loop");
  while ((subscription = mqtt.readSubscription(5000))) {
    // Check if its the onoff button feed
    Serial.println("right after while loop");
    if (subscription == &onoffbutton) {
      char *value = (char *)&onoffbutton.lastread;
      current = atoi(value);
      Serial.print(F("On-Off button: "));
      Serial.println(current);
      
      if (current == 0) {
        for(int i = 0; i < NUM_LEDS; i++){
          leds[i] = 0;
        }
        FastLED.show();
        Serial.println("I am turing off");
      }
      if (current == 1) {
        for(int i = 0; i < NUM_LEDS; i++){
          leds[i] = CRGB(255, 244, 229);
        }
        FastLED.show();
        Serial.println("I am turning on");
      }
      if (current == 3){
        for(int i = 0; i < NUM_LEDS; i++){
          leds[i] = CRGB(0, 140, 140);
        }
        FastLED.show();
      }
      if (current == 4){
        for(int i = 0; i < NUM_LEDS; i++){
          if(i%2 == 0){
            leds[i] = CRGB(160, 0, 0);
          }
          else
          {
            leds[i] = CRGB(0, 160, 0);
          }
        }
        FastLED.show();
      }
    }

  }
  
  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
