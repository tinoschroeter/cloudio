// esp8266 community version: 2.5.2
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <FirebaseArduino.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <AsyncDelay.h>
#include "DHTesp.h"


#define FIREBASE_HOST "tino-test-41bd3.firebaseio.com"
#define FIREBASE_AUTH "++++++++++++++++++++++++"
#define WIFI_SSID "++++++++"
#define WIFI_PASSWORD "++++++++"

AsyncDelay delay_sensors;
AsyncDelay delay_led;

DHTesp dht;

String stringHumidity, stingTemperature;

const int numReadings = 30;
int readings[numReadings];
int readIndex = 0;   
int total = 0;    
int average = 0;
bool startReading = false;

int sensor = A0; // Analog Port
int air, airMap, gasStart, gasEnd;
int startDelay = 0;
String stringAir;
String ntp;

//const int WlanLED = 4;
//const int DataLED = 6;

const int ledPin = LED_BUILTIN;
int ledStatus;

// ntp offset fuer Berlin +1H
const long utcOffsetInSeconds = 3600;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, 'de.pool.ntp.org', utcOffsetInSeconds);

void setup()
{
    // Start counting
    delay_sensors.start(1000, AsyncDelay::MILLIS);
    delay_led.start(500, AsyncDelay::MILLIS);

    // Debug console
    Serial.begin(115200);
    // connect to wifi.
    pinMode(ledPin, OUTPUT);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());

    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
    timeClient.begin();

    // setzt die "Datenbank" initial
    /*
       Firebase.setString("Data/tmp", "0"); // Temperatur
       Firebase.setString("Data/gas", "0"); // Luft Qualitaet
       Firebase.setString("Data/gas_sensor", "0"); // Luft Qualitaet
       Firebase.setString("Data/hum", "0"); // Luftfeuchtigkeit

       Firebase.setString("Data/time", "0"); // ZeitStempel
       Firebase.setString("Data/led", "0"); // LED

       Firebase.setString("Data/slack_hook", "not set"); // slack url
       Firebase.setString("Data/slack_hook", "false"); // slack alarm

       Firebase.setString("Data/gas_start", "49"); // bester Wert
       Firebase.setString("Data/gas_end", "90"); // schrlechtester Wert
       Firebase.setString("Data/gas_alarm", "20"); // wenn schlechter als der Wert gibt es ein Alarm/Slack Nachricht

       Firebase.setString("Data/tmp_alarm_up", "26"); // Alarm wenn Temperatur zu hoch
       Firebase.setString("Data/tmp_alarm_down", "17"); // Alarm wenn Temperatur zu niedrig

       Firebase.setString("Data/hum_alarm_up", "70"); // Alarm wenn Luftfeuchte zu hoch
       Firebase.setString("Data/hum_alarm_down", "10"); // Alarm wenn Luftfeuchte zu niedrig
     */

    dht.setup(5, DHTesp::DHT11); // GPIO 5 ist D1
    pinMode(sensor , INPUT); // GPIO 16 ist D0

      // initialize all the readings to 0:
      for (int thisReading = 0; thisReading < numReadings; thisReading++) {
        readings[thisReading] = 0;
     }
}

void loop()
{
    if (delay_sensors.isExpired()) {
        timeClient.update();
        Firebase.setString("Data/time", String(timeClient.getFormattedTime()));
        total = total - readings[readIndex];
        readings[readIndex] = analogRead(sensor);
        total = total + readings[readIndex];
        readIndex = readIndex + 1;
          if (readIndex >= numReadings) {
          readIndex = 0;
          startReading = true;
        }
         // Durchschnitt Luftqualitaetssensor
         if(startReading) {
           air = total / numReadings;
         }
        // map die Messwerte auf eine art Prozent 100% ist top
        gasStart = Firebase.getInt("Data/gas_start");
        Serial.print("getStart: ");
        Serial.println(gasStart);

        gasEnd = Firebase.getInt("Data/gas_end");
        Serial.print("getEnd: ");
        Serial.println(gasEnd);

        airMap = map(air, gasEnd, gasStart, 0, 100);
        Serial.print("airMap: ");
        Serial.println(airMap);
        stringAir = String(airMap);
        if(air > 0 && airMap < 110 && startDelay > 0) {
            Firebase.setString("Data/gas", stringAir);
            stringAir = String(air);
            Firebase.setString("Data/gas_sensor", stringAir);
        }
        // Temperatur
        delay(dht.getMinimumSamplingPeriod());

        int humidity = dht.getHumidity();
        Serial.print("humidity: ");
        Serial.println(humidity);

        float temperature = dht.getTemperature();
        Serial.print("temperature: ");
        Serial.println(temperature);
        
        if(temperature > 0) {
            stringHumidity = String(humidity);
            stingTemperature = String(temperature);
            Firebase.setString("Data/hum", stringHumidity);
            Firebase.setString("Data/tmp", stingTemperature);
            delay_sensors.repeat();
        }
        startDelay++;
    }

    if (delay_led.isExpired()) {
        // LED Status
        ledStatus = !Firebase.getInt("Data/led");
        if (ledStatus >= 0) {
            digitalWrite(ledPin, ledStatus);
        }
        delay_led.repeat();
    }
}
