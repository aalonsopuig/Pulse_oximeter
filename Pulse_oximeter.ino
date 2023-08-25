/* Program Pulse Oximeter
 * by: Alejandro Alonso Puig
 * 2023
 * Version: 0.1
 * 
 * This program uses a MAX30102 sensor to read heartbeat pulse and oxigen concentration in blood.
 * The SBC used is a Wemos D1 Mini, although it might be compatible with othe versions and Arduino.
 * The Display is a SSD1306 128x64
 */

#include <Wire.h>     //I2C library
#include "MAX30105.h" //Sensor library  https://github.com/sparkfun/MAX30105_Breakout
#include "heartRate.h" // https://github.com/sparkfun/MAX30105_Breakout

#include <U8g2lib.h>  //OLED display library
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); //Display specification

MAX30105 particleSensor;


const byte RATE_SIZE = 8; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;


void displayHeader() {
  //Display the header at the display
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
  u8g2.drawStr(5,15,"%SpO2");  // write something to the internal memory
  u8g2.drawStr(70,15,"bpmPR");  // write something to the internal memory    
  u8g2.sendBuffer();          // transfer internal memory to the display
}



//##############   SETUP ###############

void setup() {
  Serial.begin(115200);
  u8g2.begin(); //Display start up

  //Initial title screen
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_ncenB10_tr); // choose a suitable font
  u8g2.drawStr(0,30,"PULSE");  // write something to the internal memory
  u8g2.drawStr(40,45,"OXIMETER");  // write something to the internal memory
    
  u8g2.sendBuffer();          // transfer internal memory to the display
  delay(2000);  

  // Initialize sensor
  if (particleSensor.begin(Wire, I2C_SPEED_FAST) == false) //Use default I2C port, 400kHz speed
  {
    u8g2.clearBuffer();          // clear the internal memory
    u8g2.setFont(u8g2_font_ncenB08_tr); // choose a suitable font
    u8g2.drawStr(0,30,"ERROR 01");  // write something to the internal memory
    u8g2.drawStr(0,45,"SENSOR FAILURE");  // write something to the internal memory
      
    u8g2.sendBuffer();          // transfer internal memory to the display
    while (1);
  }

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x2A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

  displayHeader();  //Display the header
}


//##############   LOOP  ###############


void loop()
{

   
  
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }


  if (irValue < 50000)
  {
    u8g2.setFont(u8g2_font_ncenB10_tr); // choose a suitable font
    u8g2.drawStr(20,40,"Finger out");  // write something to the internal memory
//    u8g2.sendBuffer();          // transfer internal memory to the display
  }
  else 
  {
    u8g2.setFont(u8g2_font_ncenB18_tr);
    u8g2.setCursor(70, 45);
    u8g2.print(beatAvg);
//  Serial.print(", BPM=");
  Serial.print(beatsPerMinute);
//  Serial.print(", Avg BPM=");
//  Serial.print(beatAvg);
  Serial.println();

  }
  
  u8g2.sendBuffer();          // transfer internal memory to the display    
 
  

}
