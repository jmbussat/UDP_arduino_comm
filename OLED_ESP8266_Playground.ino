/* Text Display for the Arduino v2 for SSD1306 or SH1106 OLED modules

   David Johnson-Davies - www.technoblogy.com - 31st August 2016
   Arduino Uno or ATmega328
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <Time.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <I2Cdev.h>       // Needed by MPU60X0.h
#include <SPI.h>          // Needed by I2Cdev.h even though there is no SPI device
#include <MPU60X0.h>

MPU60X0 imu(0,0x68);
#define OUTPUT_BINARY_ACCELGYRO

#include "NTP_Defs.h"
#include "OLED_Display_Defs.h"

char buffer[80];                            // For OLED display
char udpBuffer[UDP_TX_PACKET_MAX_SIZE];  // For UPD communication

int16_t ax, ay, az;   // IMU
int16_t gx, gy, gz;
char IMU_data[13];
int8_t cmd;
int8_t counter;
IPAddress remote;

// OTA update
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

WiFiUDP udp_data;

/* Display date and time on last line of display (line 7)
 */
void dispDateTime() {
  char *p;
  uint8_t c;
  time_t t;
  
  t=now();

  sprintf(buffer,"%02d:%02d:%02d %02d/%02d/%d",hour(t),minute(t),second(t),month(t),day(t),year(t));
  p=buffer;
  c=0;
  while(*p){
    PlotChar(*p++,7,c++);
  }
}

void dispWordHex(uint8_t l, uint8_t c, int16_t x){
  char str[5];
  int16_t var=x;
  int8_t n;
  for(uint8_t i=0;i<4;i++){
    n=var & 0xF;
    if(n>9){
      n=n-10+0x41;
    } else {
      n=n+0x30;
    }
    str[3-i]=n;
    var=var>>4;  
  }
  str[4]='\0';
  DisplayStrXY(str,l,c,1,1);
}

void setup () {
  char spin[4]={'|','/','-','\\'};
  uint8_t i=0;
  IPAddress ipaddr;
  
  InitDisplay();

  // If this isn't added, OTA update won't work
  // OTA update also requires a hard reset (power cycling) after the 
  // first serial upload. Otherwise, it will hang during reboot
  Serial.begin(115200);

  // Connect to WiFi network
  WiFi.begin(ssid, pass);
  DisplayStrXY("Connecting",0,0,1,1);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    PlotChar(spin[i++],0,12);
    if(i>3){
      i=0;
    }
  }

  // OTA Updates
  httpUpdater.setup(&httpServer);
  httpServer.begin(); 

  // Once connected, display IP address on first line of display
  ipaddr=WiFi.localIP();
  sprintf(buffer,"%d.%d.%d.%d",ipaddr[0],ipaddr[1],ipaddr[2],ipaddr[3]);
  ClearLine(0,1);
  DisplayStrXY(buffer,0,0,1,1);

  // Initialization for NTP functions
  udp.begin(localPort);
  gotNTPTime=0;

  // IMU
  imu.initialize();
  delay(100);
  sprintf(buffer,"%s",imu.testConnection() ? "MPU6050 OK" : "MPU6050 ERR");
  DisplayStrXY(buffer,1,0,1,1);
  udp_data.begin(4998);
  counter=0;
}

void loop () {
  time_t t;
  
  dispDateTime();

  // Synchronize once a day
  t=now();
  if((hour(t)==23) && (minute(t)==59) && (second(t)==59)){
    gotNTPTime=0;
  }
  
  if(!gotNTPTime){
    setTimeNTP(); 
    gotNTPTime=1; 
  } else {
    UDP_Comm();
  }

  delay(50);
  httpServer.handleClient();
}

