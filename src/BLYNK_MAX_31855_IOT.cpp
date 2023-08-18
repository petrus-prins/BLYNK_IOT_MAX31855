#define BLYNK_TEMPLATE_ID "TMPL2vRSlnEg9"
#define BLYNK_TEMPLATE_NAME "UNO ESP8266"
#define BLYNK_AUTH_TOKEN "73T2MPrMq94_IUX29b3Ph7kuWSqbqOSC"
#define BLYNK_PRINT Serial


// look at esplib
// reboot
// kick
// 

#include <Arduino.h>
#include <SPI.h>
#include <SoftwareSerial.h>
#include <MAX_31855.h>
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <TimerOne.h>
#include <arduino-timer.h>
#include <avr/wdt.h>


//Original_Blynk_Begin
char ssid[] = "WIFI1";
char pass[] = "00000000000000000000";

// Custom_Blynk_Begin
const char* ssid_list[] = {"WIFI1","WIFI2","WIFI3"};                                                   // WIFI LIST
const char* pass_list[] = {"00000000000000000000","00000000000000000000","00000000000000000000"};      // Password List


SoftwareSerial EspSerial(8, 9);  // RX, TX   ESP_WROOM_02_IO_13_RX connects to UNO_PIN9_TX (38400bps SW_Serial)
#define ESP8266_BAUD 38400       //          ESP_WROOM_02_IO_15_TX connects to UNO_PIN8_RX (38400bps SW_Serial)

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


void Reboot_Arduino()
{
    wdt_disable();
    wdt_enable(WDTO_15MS);
    while (1) {}
}

//==========================================
//     DEBUG TO CAPTURE A REBOOT COMMAND
//==========================================
BLYNK_WRITE(InternalPinDBG) 
{
    if (String(param.asStr()) == "reboot") 
    {
        Serial.println("Reboot CMD Received From Server...");
        Reboot_Arduino();
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
    if ((TC_Fault == false) && (Blynk.connected() == true ))
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
     Timer1.initialize(2000000);                   // 1 second
     Timer1.attachInterrupt(Read_TC_Data);      // capture data ever 1s
     Timer_Post_Blynk_Data.every(2000000, Upload_Blynk_Data);
}



//=========================================================================
//               CUSTOM BLYNK 2.0 BEGIN TO FIND LOCAL ACCESS POINTS
//=========================================================================
void Custom_Blynk_Begin(const char*  _auth,
                        ESP8266&     _esp8266,
                        const char*  _domain = BLYNK_DEFAULT_DOMAIN,
                        uint16_t     _port   = BLYNK_DEFAULT_PORT)

{
	int ssid_index=0;
	int ssid_list_size = sizeof(ssid_list) / sizeof(ssid_list[0]);         // calculate number of entries in SSID list (3)
	Blynk.config(_esp8266, _auth, _domain, _port);
	do 
    {	
        if (Blynk.connectWiFi(ssid_list[ssid_index], pass_list[ssid_index])) 
		{
			Serial.println("CONNECTED TO : " + String(ssid_list[ssid_index]));
        }
        else
        {
           Serial.println("Could Not Connect To: "+ String(ssid_list[ssid_index]));
           ssid_index++; 
        }
        
        if ((ssid_index+1) >= ssid_list_size) {ssid_index  = 0;}          // loop infinitely until connected.
    }
    while ((Blynk.connect() == false));
}




//============================
//          SETUP
//============================
void setup()
{
    pinMode(8, INPUT);
    pinMode(9, OUTPUT);
    Serial.begin(57600);                                                 // Debug console
    Serial.println("System Booting...");
    SPI.begin();
    EspSerial.begin(ESP8266_BAUD);                                       // Set ESP8266 baud rate
    delay(10);
    INIT_Timers();                                                       // Start Running NON Bynk Arduino Code
    Serial.println("Starting Blynk...");
    Custom_Blynk_Begin(BLYNK_AUTH_TOKEN, wifi, "blynk.cloud", 80);       // Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass, "blynk.cloud", 80); 
}



//====================
//     MAIN LOOP
//====================
int Blynk_Fail_Count = 0;
void loop()
{
    if (Blynk.run() == false)              // Falls through every 5 seconds when failing
    {
        Blynk_Fail_Count++;
        Serial.print("Blynk_Fail_Count: "); Serial.println(Blynk_Fail_Count);
    };
    
    if (Blynk_Fail_Count >= 30)           // restart everybody
    {
        wifi.restart();
        Reboot_Arduino();
    }
    Timer_Post_Blynk_Data.tick();            // will only upload if Blynk.Connected == True;
}