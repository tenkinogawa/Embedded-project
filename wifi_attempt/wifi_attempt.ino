#include <WiFiNINA.h>
#include "arduino_secrets.h"
#include <DFRobot_DHT11.h>
#include <OneWire.h>
#include <DFRobot_CCS811.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>

OneWire  ds(7); 
DFRobot_DHT11 DHT;
#define DHT11_PIN 2
DFRobot_CCS811 CCS811;

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;     // the Wifi radio's status

void printData() {
  Serial.println("Board Information:");
  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  Serial.println();
  Serial.println("Network Information:");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.println(rssi);

}

void temperature_outside() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    Serial.println("DS18B20 (Temperature sensor) not found!");
    return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();

  }


  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
//  fahrenheit = celsius * 1.8 + 32.0;
  Serial.println("DS18B20");
  Serial.print("  Temperature outside = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.println();
  Serial.println();
//  Serial.print(fahrenheit);
//  Serial.println(" Fahrenheit");

  ds.reset_search();
}

void photoresistor() {
  int odczytana = 0;
  Serial.println("Photo");
  odczytana = analogRead(A5);
  Serial.print("Day/Night: ");
  Serial.println(odczytana);
  Serial.println();

//  if (odczytana <200) {
//    Serial.println("Obecnie mamy noc");
//  }
//  else if (odczytana > 850) {
//    Serial.println("Obecnie mamy dzień");
//  }
}

void temperature_and_humidity() {
  DHT.read(DHT11_PIN);
  Serial.println("DHT11");
  Serial.print("temp:");
  Serial.print(DHT.temperature);
  Serial.print("  humi:");
  Serial.println(DHT.humidity);
  Serial.println();
//  delay(1000);
}

void pressure_sensor(){
  Adafruit_BMP280 bmp280;
  if (!bmp280.begin(0x76)){ 
    Serial.println("BMP280 (Pressure sensor) not found");
    while(1);
  }
  Wire.beginTransmission(0x76);
  float temperature = bmp280.readTemperature(); 
  float pressure    = bmp280.readPressure();    
  float altitude_   = bmp280.readAltitude(pressure);
  Serial.println("BMP280");
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println("°C");
  // 2:  ciśnienie
  Serial.print("Pressure    = ");
  Serial.print(pressure/100);
  Serial.println("hPa");
  // 3:  wysokość
  Serial.print("Altitude = ");
  Serial.print(altitude_);
  Serial.println("m");
    
  Serial.println();  // nowa linie
  delay(1000);       // czekamy 2 sekundy  
  Wire.endTransmission(0x76);
}

void TVOC_CO_sensor(){
//  if(!CCS811.begin()){
//      Serial.println("CSS811 (Air Quality Sensor) not found!");
//      delay(1000);
//    }
  Wire.beginTransmission(0x5B);
  
  if(CCS811.checkDataReady() == true){
      Serial.println();
      Serial.print("CO2: ");
      Serial.print(CCS811.getCO2PPM());
      Serial.print("ppm, TVOC: ");
      Serial.print(CCS811.getTVOCPPB());
      Serial.println("ppb");
      Serial.println();

    } else {
        Serial.println("Data is not ready!");
    }
    CCS811.writeBaseLine(0x447B);
    //delay cannot be less than measurement cycle
    delay(1000);

    Wire.endTransmission(0x5B);
}
//--------------------------------SETUP-------------------------------------
void setup() {
  // put your setup code here, to run once:
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial);
  
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to network: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }

  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");

  Serial.println("----------------------------------------");
  printData();
  Serial.println("----------------------------------------");

}
//--------------------------------LOOP-------------------------------------
void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
  Serial.println("loop------------------------------------");
  printData();
  temperature_outside();
  photoresistor();
  temperature_and_humidity();
  pressure_sensor();
  TVOC_CO_sensor();
  Serial.println("end-of-loop------------------------------------");
}
