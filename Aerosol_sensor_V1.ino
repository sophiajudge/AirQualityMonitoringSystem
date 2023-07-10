//This code is used for the 2023 BME summer camp activities for detecting environmental aerosols. 
//updated on Jun 26, 2023.
//The MCU is M5Stick-C
//The particle sensor is PMSA003I. The sensor manufactured by PlantTower. The breakboard and the library by Adafruit.
//The gas sensor is BME688. The sensor manufactured by Bosch. The breakboard and the library by Adafruit.


#include <M5StickC.h>
#include <SPI.h>          //to be used in future for wring data
#include <Wire.h>            //Used for IC2 communication

#include <Adafruit_GFX.h>   //Library for the graphics, the base class for the OLED display.
#include "Adafruit_PM25AQI.h" //library for the particle counter
#include <Adafruit_Sensor.h> //library needed for BME680 sensor
#include "Adafruit_BME680.h"
#define BUTTON_B 37      //Button to restart the board

#define SEALEVELPRESSURE_HPA (1013.25);


Adafruit_PM25AQI PMSA003; //object for particle sensor
PM25_AQI_Data AerosolData; //object for acquired data from particle sensor
Adafruit_BME680 bme;  //object for BME688 sensor. 
float airT, VOC, RH;
 
bool readState=HIGH;
int countDisconnect=0;

int PC03_05, PC05_10, PC10_25, PC25_50, PC50_100, PC_100_INF; //particle concentrations in 6 channels. Units is particles/Liter.


//interrupt service routine (ISR) must have IRAM_ATTR attribute and need to appear before it is included in the function.
void IRAM_ATTR RestartMCU() 
{
   ESP.restart();
}


void setup() {
  M5.begin();  //default: enable LCD, Power management, Serial with 115200bps.
  pinMode(BUTTON_B, INPUT_PULLUP);  
  //Button B pin to trigger interrupt to restart the board.
  attachInterrupt(BUTTON_B, RestartMCU, FALLING);  


  M5.Lcd.setRotation(1);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(0, 10);

  
  while (! PMSA003.begin_I2C(&Wire)) 
  { //check if the connection is successful
    Serial.println("Could not find particle sensor!");
    delay(500);
  } 
  Serial.println("Particle sensor connected!");
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,10);
  M5.Lcd.print(" Particle sensor connected");
  delay(1000);

  while (! bme.begin()) 
  { //check if the connection is successful
    Serial.println("Could not find BME688 sensor!");
    delay(500);
  } 
  Serial.println("BME sensor connected!");
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0,10);
  M5.Lcd.print(" BME688 sensor connected");
  delay(1000);
  
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}


void loop() {

do{
 readState=PMSA003.read(&AerosolData); //The readState to tell the connection status
 countDisconnect++;   //count number of disconnections
 delay(50);   //each time delay 50ms before connecting again

 if (countDisconnect>=100)  //If retried for 100 times or more, restart the board.
      { Serial.println("Restart in 5 seconds");
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,10);
        M5.Lcd.print("Restart in 5s");
        delay(5000);   //wait for 5 seconds to restart.
        ESP.restart();//restart the board
      }
} while(!readState);  //If connection failed, re-do the loop.


 countDisconnect=0; //reset the count

   PC03_05=(AerosolData.particles_03um-AerosolData.particles_05um)*10; //first channel 0.3 - 0.5 um particle, converted into concentration of particles/Liter
   PC05_10=(AerosolData.particles_05um-AerosolData.particles_10um)*10; //second channel 0.5 - 1.0 um particle
   PC10_25=(AerosolData.particles_10um-AerosolData.particles_25um)*10; //third channel 1.0 - 2.5 um particle
   PC25_50=(AerosolData.particles_25um-AerosolData.particles_50um)*10; //fourth channel 2.5 - 5.0 um particle
   PC50_100=(AerosolData.particles_50um-AerosolData.particles_100um)*10;//fifth channel 5.0 um - 10.0 um particle
   PC_100_INF=AerosolData.particles_100um*10;//sixth channel > 10.0 um particle


 
do{
 readState=bme.performReading(); //The readState to tell the connection status
 countDisconnect++;   //count number of disconnections
 delay(50);   //each time delay 50ms before connecting again

 if (countDisconnect>=100)  //If retried for 100 times or more, restart the board.
      { Serial.println("Restart in 5 seconds");
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0,10);
        M5.Lcd.print("Restart in 5s");
        delay(5000);   //wait for 5 seconds to restart.
        ESP.restart();//restart the board
      }
} while(!readState);  //If connection failed, re-do the loop.
  countDisconnect=0;
  
  airT=bme.temperature; //air temperature in Celcius
  VOC=bme.gas_resistance/1000.0;//voc resistance in KOhm
  RH=bme.humidity; //relative humidity

  Disp_TFT_Page();
  
}

void Disp_TFT_Page()
{   //display all results. Three frames, each frame for 4 seconds.
    M5.Lcd.setRotation(1);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 10);
    M5.Lcd.print("  0.3 - 0.5um:  "); M5.Lcd.println(PC03_05);
    M5.Lcd.print("  0.5 - 1.0um:  "); M5.Lcd.println(PC05_10);
    M5.Lcd.print("  1.0 - 2.5um:  "); M5.Lcd.println(PC10_25);
    M5.Lcd.print("  2.5 - 5.0um:  "); M5.Lcd.println(PC25_50);
    M5.Lcd.print("  5.0 - 10.0um: "); M5.Lcd.println(PC50_100);
    M5.Lcd.print("       >10.0um: "); M5.Lcd.println(PC_100_INF);
    M5.Lcd.print("\n  unit: particles/Liter");
    delay(4000);
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 10);
    M5.Lcd.print("    PM1.0: "); M5.Lcd.println(AerosolData.pm10_standard); //units of PM measurement is microgram/cubic meter
    M5.Lcd.println(" ");
    M5.Lcd.print("    PM2.5: "); M5.Lcd.println(AerosolData.pm25_standard);
    M5.Lcd.println(" ");
    M5.Lcd.print("     PM10: ");  M5.Lcd.println(AerosolData.pm100_standard);
    M5.Lcd.print("\n     unit: ug/m3");
    delay(4000);
    
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 15);
    M5.Lcd.print("      Temp:  "); M5.Lcd.print(int(airT));M5.Lcd.println(" C");    
    M5.Lcd.println(" ");            
    M5.Lcd.print("  Humidity:  "); M5.Lcd.print(int(RH));M5.Lcd.println(" %"); 
    M5.Lcd.println(" ");
    M5.Lcd.print("       Voc:  "); M5.Lcd.print(int(VOC)); M5.Lcd.println(" kOhm"); 
    M5.Lcd.println(" ");   
    delay(4000);
}
