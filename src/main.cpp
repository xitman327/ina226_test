#include <Arduino.h>
#include "INA226.h"
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include "ESPTelnet.h"
#include <WiFiManager.h>

INA226 sensor(0x40);

ESPTelnet telnet;
uint16_t  port = 23;

int sensor_int;

void setup() {
  Serial.begin(115200);
  WiFi.setHostname("ESP8266_meter");
  //WiFi.begin("TP-Link_CE3F", "81335657");
  WiFiManager wifiManager;
  wifiManager.autoConnect();

  uint32_t notConnectedCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      Serial.println("Wifi connecting...");
      notConnectedCounter++;
      if(notConnectedCounter > 150) { // Reset board if not connected after 5s
          Serial.println("Resetting due to Wifi not connecting...");
          ESP.restart();
      }
  }
  Serial.print("Wifi connected, IP address: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.begin();
  telnet.begin(port);

  delay(1000);
   sensor_int = sensor.begin(5, 4);
   if(sensor_int){
     Serial.println("Sensor Found");
     if(sensor.isCalibrated()){
       Serial.println("Calibration OK");
     }else{
       Serial.println("NO Calibration, trying to calibrate with known parameters!");
       sensor_int = sensor.setMaxCurrentShunt(200.0, 0.000375);// modified to suit the abomination
       sensor_int |= !sensor.setAverage(5);
       //sensor_int = sensor.setMaxCurrentShunt(20.0, 0.1);
       if(sensor_int){
         Serial.print("Calibration Error: ");
         Serial.println(sensor_int, HEX);
       }else{
         Serial.println("Calibration OK");
         sensor_int = 0;
       }
     }
   }else{
     Serial.println("Sensor ERROR");
     //while(true) {}
   }
}

uint32_t tm0;
void loop() {
  ArduinoOTA.handle();
  telnet.loop();

  if(!sensor_int){
    if(millis() - tm0 > 200){
      tm0 = millis();
      //Serial.println(sensor.getCurrent());
      if(telnet.isConnected()){
        // telnet.println(sensor.getCurrent());
        telnet.printf("Voltage: %2.2f Current: %3.2f Energy: %4.2f \r", sensor.getBusVoltage(), sensor.getCurrent(), sensor.getPower());
      }
      
    }
  }
}
