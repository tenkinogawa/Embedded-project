#include <DFRobot_DHT11.h>
#include "DFRobot_CCS811.h"
#include <Adafruit_BMP280.h>
#include "Nextion.h"

DFRobot_CCS811 CCS811; //TVOC + CO2 sensor
DFRobot_DHT11 DHT;     //Humidity + Temperature sensor
#define DHT11_PIN 6

//CO sensor variables declaration
#define TIME_OF_BURNING 90000
#define TIME_OF_ACCUMULATING 60000
#define mosfetPIN 8
#define analogIN A2

//---------------------------------- Display variables----------------------------------------------
// Declare your Nextion objects - Example (page id = 0, component id = 1, component name = "b0") 
NexText tTempIn = NexText(1, 3, "tTempIn");
NexText tHum = NexText(1, 4, "tHum");
NexText tTVOC = NexText(2, 2, "tTVOC");
NexText tCO2 = NexText(2, 3, "tCO2");
NexText tCO = NexText(2, 4, "tCO");
NexText tPress = NexText(3, 1, "tPress");
NexText tAlt = NexText(3, 2, "tAlt");
NexText tLight = NexText(3, 6, "tLight");

// Register a button object to the touch event list.  
NexTouch *nex_listen_list[] = {
  NULL
};

//loop variables
unsigned long t_now_loop,t_previous_loop, t_diff_loop;

//global variables------------------
unsigned long time_now,time_previous,time_difference;
bool burn;
float ppmvalue,sensor_volt,RS_gas,ratio,R0;
int ppmvalue_int,sensorValue;

//char array for display communication
char buff[20];

//----------------------------------- Burnoff function ----------------------------------------------
void burnoff(unsigned long time_burn){
  time_now=millis();
  time_difference=time_now-time_previous; 
  
  sensorValue = analogRead(A1); 
  sensor_volt = ((float)sensorValue / 1024) * 4.8; 

  R0 = (998-998*sensor_volt)/sensor_volt;
  R0 =700;
  RS_gas = (3.6/sensor_volt-1)*996; // Depend on RL on yor module
  ratio = RS_gas / R0; // ratio = RS/R0 

  ppmvalue=595*pow(ratio,-2.24);
  ppmvalue_int=round(ppmvalue);
  ppmvalue_int= ppmvalue_int-1;
  sprintf (buff, "CO: %d", ppmvalue_int);
  strcat(buff," ppm");
  tCO.setText(buff);
  memset(buff, 0, sizeof(buff)); 
  
  if (time_difference > time_burn) {
    digitalWrite(mosfetPIN,HIGH);
    time_previous=millis();
    burn=false;
  }}

//---------------------------------- Accumulate function ----------------------------------------------
void accumulate(unsigned long time_accumulate){
  time_now=millis();
  time_difference=time_now-time_previous;
  if (time_difference > time_accumulate) {
    digitalWrite(mosfetPIN,LOW);
    time_previous=millis();
    burn=true;
  }}

//----------------------------------- Photoresistor function ----------------------------------------------
void photoresistor() {
  int read_value = 0;
  read_value = analogRead(A5);

  if (read_value <= 75) {
    tLight.setText("Exposure: None");
    memset(buff, 0, sizeof(buff)); 
  }
  else if (read_value > 75 && read_value < 350){
    tLight.setText("Exposure: Weak");
    memset(buff, 0, sizeof(buff)); 
  }
  else {
    tLight.setText("Exposure: Strong");
    memset(buff, 0, sizeof(buff)); 
  }
}

//----------------------------------- Temperature and humidity function ----------------------------------------------
void temperature_and_humidity() {
  DHT.read(DHT11_PIN);
  
  float temperatureDHT = DHT.temperature;
  float humidityDHT = DHT.humidity;

  if (humidityDHT == 0){
    strcat(buff,"Indoor: No data");
    tTempIn.setText(buff);
    memset(buff, 0, sizeof(buff)); 

    strcat(buff,"Humidity: No data");
    tHum.setText(buff);
    memset(buff, 0, sizeof(buff));
    }
  else {
    sprintf (buff, "Indoor: %.2f", temperatureDHT);
    strcat(buff," *C");
    tTempIn.setText(buff);
    memset(buff, 0, sizeof(buff)); 
    
    sprintf (buff, "Humidity: %.2f", humidityDHT);
    strcat(buff," %");
    tHum.setText(buff);
    memset(buff, 0, sizeof(buff));
  }
}

//----------------------------------- Pressure and altitude function ----------------------------------------------
void pressure_sensor(){
  Adafruit_BMP280 bmp280;
  bool error = false;
  if (!bmp280.begin(0x76)){ 
    error = true;
  }
  
  Wire.beginTransmission(0x76);

  //Read values
  float pressure    = bmp280.readPressure()/100 + 35;    
  float altitude   = bmp280.readAltitude(1013.25) + 40;

  if (error == true) {
    strcat(buff,"Pressure: No data");
    tPress.setText(buff);
    memset(buff, 0, sizeof(buff));
    strcat(buff,"Altitude: No data");
    tAlt.setText(buff);
    memset(buff, 0, sizeof(buff));
    }
  else {
    sprintf (buff, "Pressure: %.2f", pressure);
    strcat(buff," hPa");
    tPress.setText(buff);
    memset(buff, 0, sizeof(buff));

    sprintf (buff, "Altitude: %.2f", altitude);
    strcat(buff," m");
    tAlt.setText(buff);
    memset(buff, 0, sizeof(buff));
      
    Serial.println();  // nowa linie
  }

  Wire.endTransmission(0x76);
}

//----------------------------------- TVOC and CO2 function ----------------------------------------------
void TVOC() {
  
  if(CCS811.checkDataReady() == true){
        float CO2 = CCS811.getCO2PPM();
        sprintf (buff, "eCO2: %.2f", CO2);
        strcat(buff," pmm");
        tCO2.setText(buff);
        memset(buff, 0, sizeof(buff));
        
        float TVOC = CCS811.getTVOCPPB();
        sprintf (buff, "TVOC: %.2f", TVOC);
        strcat(buff," ppb");
        tTVOC.setText(buff);
        memset(buff, 0, sizeof(buff));
    } else {
        strcat(buff,"TVOC: No data");
        tTVOC.setText(buff);
        memset(buff, 0, sizeof(buff));
        
        strcat(buff,"CO2: No data");
        tCO2.setText(buff);
        memset(buff, 0, sizeof(buff));
    }
    
    CCS811.writeBaseLine(0x447B);
    delay(1000);
  
  }
  
void setup(void)
{
    Serial1.begin(9600);
    nexInit();
    if(CCS811.begin() != 0){
        delay(1000);
    }
    t_now_loop=millis();
    t_previous_loop = t_now_loop;
}
void loop() 
{
    t_now_loop=millis();
    t_diff_loop=t_now_loop-t_previous_loop; 
    if(t_diff_loop >= 5000)
    {
      t_previous_loop = millis();
      TVOC();
      pressure_sensor();
      temperature_and_humidity();
      photoresistor();
      if (burn==true) burnoff(TIME_OF_BURNING);
      if (burn==false) accumulate(TIME_OF_ACCUMULATING);
      
  }
}
