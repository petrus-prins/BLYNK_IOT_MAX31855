
#define BLYNK_TEMPLATE_ID "TdadMdqPL32v2fRS4dlnEg39fd"
#define BLYNK_TEMPLATE_NAME "UNO ESP8266"
#define BLYNK_AUTH_TOKEN "7a23aT23MPrM4dq94_IdUfX259b3P5h7kuf6WSqbdqOSdC"

/* Comment this out to disable prints and save space */

#define BLYNK_PRINT Serial
#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <MAX_31855.h>
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <TimerOne.h>
#include <arduino-timer.h>
#include <avr/wdt.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "WIFI1";
char pass[] = "00000000000000000000";


//ESP_WROOM_02_IO_13_RX connects to UNO_PIN9_TX (38400bps SW_Serial)
//ESP_WROOM_02_IO_15_TX connects to UNO_PIN8_RX (38400bps SW_Serial)
SoftwareSerial EspSerial(8, 9);  // RX, TX
#define ESP8266_BAUD 38400                    // ESP WROOM-02 SW BAUDRATE

ESP8266 wifi(&EspSerial);


//  TYPE K THERMO COUPLE
MAX_31855 SPI_Thermo_Couple(SCK, SS, MISO, MOSI);
double TC_Temp      = 0.0;
double IC_Temp      = 0.0;
bool   Open_Circuit = true;
bool   Short_To_GND = true;
bool   Short_To_VCC = true;
bool   TC_Fault     = true;

// TIMER
Timer<1, micros> Timer_Post_Blynk_Data; // create a timer with 1 task and microsecond resolution


//==============================
//     ON UPDATE EVENT FOR V0
//==============================
BLYNK_WRITE(V0)
{

  int value = param.asInt();               // Set incoming value from pin V0 to a variable
  if (value == 0) 
  {  
    //digitalWrite(13, LOW);  
  }
  else
  {
    //digitalWrite(13, HIGH);  
  }
}



//====================================================
//     UPDATE ALL VIRTUAL PINS STATES WHEN CONNECTED
//====================================================
BLYNK_CONNECTED() 
{
   Blynk.syncAll();
}


//==========================================
//     DEBUG TO CAPTURE A REBOOT COMMAND
//==========================================
BLYNK_WRITE(InternalPinDBG) 
{
    if (String(param.asStr()) == "reboot") 
    {
        Serial.println("Reboot CMD Received From Server...");
        wdt_disable();
        wdt_enable(WDTO_15MS);
        while (1) {}
    }
}

    

//==================================
//           READ TC DATA
//==================================

void Read_TC_Data()
{
    if (SPI_Thermo_Couple.Read_All(&TC_Temp, &IC_Temp, &Open_Circuit, &Short_To_GND, &Short_To_VCC) && TC_Temp != -9999)
    {
        Serial.print(TC_Temp); Serial.print(" TCdegC ");Serial.print("\t");
        Serial.print(IC_Temp); Serial.print(" ICdegC ");Serial.println("");
        TC_Fault = false;
    } 
    else
    {
        TC_Fault = true;
        Serial.print(TC_Temp == -9999 ? "TC Faulty : ":" ");
        Serial.print(Open_Circuit   ? " Open Circuit ":" ");
        Serial.print(Short_To_GND   ? " Short To GND ":" ");
        Serial.println(Short_To_VCC ? " Short To VCC ":" "); 
    }
}



//===================================
//          UPLOAD DATA TO BLYNK
//===================================
bool Upload_Blynk_Data(void *)
{
    if (TC_Fault == false)
    {
        Blynk.virtualWrite(V1, TC_Temp);     // Update  Graph
        Serial.println("Blynk Data Posted...");
    }
    return true; 
}



//============================
//          INIT TIMERS
//============================
void INIT_Timers()
{
     Timer1.initialize(1000000);                   // 1 second
     Timer1.attachInterrupt(Read_TC_Data);      // capture data ever 1s
     Timer_Post_Blynk_Data.every(2000000, Upload_Blynk_Data);
}



//============================
//          SETUP
//============================
void setup()
{
    pinMode(8, INPUT);
    pinMode(9, OUTPUT);
    Serial.begin(57600);     // Debug console
    Serial.println("System Booting...");
    SPI.begin();
    EspSerial.begin(ESP8266_BAUD);   // Set ESP8266 baud rate
    delay(10);
    INIT_Timers();
    Serial.println("Starting Blynk...");
    Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass, "blynk.cloud", 80);           // Blynk 2.0    
}



//====================
//     MAIN LOOP
//====================
void loop()
{
    Blynk.run();
    Timer_Post_Blynk_Data.tick();
}