#include <MAX_31855.h>
#include <SPI.h>


//=======================================================       
//           MAX31855 
//          -----_----
//    GND  | 1      8 |  DNC         
//     T-  | 2      7 |  SO      (Arduino UNO SPI_PIN_12)
//     T+  | 3      6 |  ~CS     (Arduino UNO SPI_PIN_10)
//    VCC  | 4      5 |  SCK     (Arduino UNO SPI_Pin_13)
//          ----------
//    Petrus Prins 12 August 2023
//=======================================================


MAX_31855 SPI_Thermo_Couple(SCK, SS, MISO, MOSI);

double TC_Temp      = 0.0;
double IC_Temp      = 0.0;
bool   Open_Circuit = true;
bool   Short_To_GND = true;
bool   Short_To_VCC = true;


//================================
//            SETUP
//================================
void setup() 
{
    Serial.begin(57600);
    SPI.begin();
}

//================================
//            LOOP
//================================
void loop() 
{    
    delay(500);
    if (SPI_Thermo_Couple.Read_All(&TC_Temp, &IC_Temp, &Open_Circuit, &Short_To_GND, &Short_To_VCC))
    {
        Serial.print(TC_Temp); Serial.print(" TCdegC ");Serial.print("\t");
        Serial.print(IC_Temp); Serial.print(" ICdegC ");Serial.print("\t");
        Serial.println(" No Faults ");
    } 
    else
    {
        Serial.print(TC_Temp); Serial.print(" TCdegC ");Serial.print("\t");
        Serial.print(IC_Temp); Serial.print(" ICdegC ");Serial.print("\t");
        Serial.print(Open_Circuit   ? " Open Circuit ":" ");
        Serial.print(Short_To_GND   ? " Short To GND ":" ");
        Serial.println(Short_To_VCC ? " Short To VCC ":" ");   
    }
    Serial.println("---");
}