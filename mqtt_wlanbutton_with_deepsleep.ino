// MQTT Button

#include <esp_deep_sleep.h>
#include <esp_sleep.h>
#include <WiFiClientSecure.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <SPI.h>
#include <Wire.h>

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       ""
#define WLAN_PASS       ""

/************************* MQTT Setup *********************************/

#define MQTT_SERVER      ""
#define MQTT_SERVERPORT  8883                   // use 8883 for SSL
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD    ""
#define MQTT_TOPIC       ""

/************************* Button Config  *********************************/

#define button_A    14  // There is the Button on GPIO 14
#define button_B    15  // There is the Button on GPIO 15
#define button_C    32  // There is the Button on GPIO 32
#define button_D    33  // There is the Button on GPIO 33

/************************* Input Config  *********************************/

#define bat_voltage A13

/************ Global State (you don't need to change this!) ******************/

RTC_DATA_ATTR int bootCount = 0;

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
//WiFiClient client;
// or... use WiFiFlientSecure for SSL
WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.

Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

/****************************** Feeds ***************************************/

// Setup a feed for publishing.
Adafruit_MQTT_Publish buttonPub = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME MQTT_TOPIC);

const char buttonPub_FEED[] PROGMEM = MQTT_USERNAME MQTT_TOPIC;

/*************************** Code ************************************/

void MQTT_connect();

void print_wakeup_reason() {
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (esp_sleep_get_wakeup_cause()) {
        case ESP_SLEEP_WAKEUP_EXT1: {
            uint64_t wakeup_pin_mask = esp_sleep_get_ext1_wakeup_status();
            Serial.println("Cause:");
            if (wakeup_pin_mask != 0) {
                int pin = __builtin_ffsll(wakeup_pin_mask) - 1;
                printf("Wake up from GPIO %d\n", pin);
                switch (pin) {
                  case 14: {
                    //Button AsendUpdate("1");
                    sendUpdate("status");
                    Serial.println("A Enter Sleep");
                    //esp_deep_sleep_start();
                  }
                  break;
                  case 15: {
                    //Button B
                    sendUpdate("B");
                    Serial.println("B Enter Sleep");
                    //esp_deep_sleep_start();
                  }
                  break;
                  case 32: {
                    // Button C
                    sendUpdate("C");
                    Serial.println("C Enter Sleep");
                    //esp_deep_sleep_start();
                  }
                  break;
                  case 33: {
                    //Button D
                    sendUpdate("D");
                    String bat_level = String(((analogRead(bat_voltage))*2)/4095+3.3*1.1) + "V";
                    sendUpdate(bat_level.c_str());
                    Serial.println("D Enter Sleep");
                    //esp_deep_sleep_start();
                  }
                  break;
                }
            } else {
                printf("Wake up from GPIO\n");
            }
            break;
        }
        case ESP_SLEEP_WAKEUP_TIMER: {
            Serial.println("Wake up from timer.");
            break;
        }
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            Serial.printf("Not a deep sleep reset\n");
    }
}

void print64(uint64_t number) {
  for (int i = 0; i < 64; i++) {
    bool bitt = number & 0x8000000000000000;
    Serial.print(bitt);
    number = number << 1;
  }
}

void setup() {
  digitalWrite(13, HIGH); // turn the LED on
  pinMode(button_A, INPUT_PULLDOWN);
  pinMode(button_A, INPUT);
  pinMode(button_B, INPUT_PULLDOWN);
  pinMode(button_B, INPUT);
  pinMode(button_C, INPUT_PULLDOWN);
  pinMode(button_C, INPUT);
  pinMode(button_D, INPUT_PULLDOWN);
  pinMode(button_D, INPUT);
  pinMode(13, OUTPUT);

  Serial.begin(115200);
  delay(10);

  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));

  Serial.println(F("MQTT Emergency Button"));

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);
  
  int rounds = 1;
  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    rounds++;
    if (rounds == 5) {
      ESP.restart();
    }
    }
  
  rounds = 1;
  Serial.println();

  Serial.println("WiFi connected");

  //delay(1000);

  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  //delay(1000);

  // Send HWaked up notification
   MQTT_connect();

  // Now we can publish stuff!
  Serial.print(F("\nSending Waked up!"));
  Serial.print("...");
  if (! buttonPub.publish("Waked up!")) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  //Print the wakeup reason for ESP32
  print_wakeup_reason();


  const int ext_wakeup_pin_1 = 14;
  const uint64_t ext_wakeup_pin_1_mask = 1ULL << ext_wakeup_pin_1;
  const int ext_wakeup_pin_2 = 15;
  const uint64_t ext_wakeup_pin_2_mask = 1ULL << ext_wakeup_pin_2;
  const int ext_wakeup_pin_3 = 32;
  const uint64_t ext_wakeup_pin_3_mask = 1ULL << ext_wakeup_pin_3;
  const int ext_wakeup_pin_4 = 33;
  const uint64_t ext_wakeup_pin_4_mask = 1ULL << ext_wakeup_pin_4;

  pinMode(ext_wakeup_pin_1, INPUT_PULLDOWN);
  pinMode(ext_wakeup_pin_2, INPUT_PULLDOWN);
  pinMode(ext_wakeup_pin_3, INPUT_PULLDOWN);
  pinMode(ext_wakeup_pin_4, INPUT_PULLDOWN);

  Serial.printf("Enabling EXT1 wakeup on pins GPIO%d, GPIO%d, GPIO%d, GPIO%d\n", ext_wakeup_pin_1, ext_wakeup_pin_2, ext_wakeup_pin_3, ext_wakeup_pin_4);

  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

  Serial.println(esp_sleep_enable_ext1_wakeup(ext_wakeup_pin_1_mask | ext_wakeup_pin_2_mask | ext_wakeup_pin_3_mask | ext_wakeup_pin_4_mask, ESP_EXT1_WAKEUP_ANY_HIGH));
   
  digitalWrite(13, LOW); // turn the LED off
  esp_deep_sleep_start();
}

void led_blink(uint32_t delay_ms) {
  digitalWrite(13, LOW); // turn the LED on
  delay(delay_ms); // wait for a second
  digitalWrite(13, HIGH); // turn the LED off
}

void sendUpdate(const char* msg) {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.

  MQTT_connect();

  // Now we can publish stuff!
  Serial.print(F("\nSending button press!"));
  Serial.print("...");
  if (! buttonPub.publish(msg)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
    //led_blink(500);
  }

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
    if(! mqtt.ping()) {
    mqtt.disconnect();
    }
  */
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

void loop() {

  MQTT_connect();

}

