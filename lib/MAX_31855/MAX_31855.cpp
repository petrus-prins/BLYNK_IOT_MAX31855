#include <MAX_31855.h>
#include <SPI.h>


//==================================
//      MAX31855 Constructor
//==================================
MAX_31855::MAX_31855(int SCK_Pin, int CS_Pin, int MISO_Pin, int MOSI_Pin)      // SCK, SS, MISO, MOSI
{
    _sck_pin = SCK_Pin;
    _cs_pin = CS_Pin;
    _so_pin = MISO_Pin;
    _si_pin = MOSI_Pin;
    pinMode(_cs_pin, OUTPUT);
    pinMode(_sck_pin, OUTPUT);
    pinMode(_so_pin, OUTPUT);
    pinMode(_si_pin, INPUT);
    digitalWrite(_cs_pin, HIGH);
}



//========================================
//        MAX31855 READ ALL 32 BITS 
//========================================
void MAX_31855::SPI_Read_32_Bits(int *TC_Data, int *IC_Data) 
{
    *TC_Data = 0;
    *IC_Data =  0;
    digitalWrite(_cs_pin, LOW);
    SPI.beginTransaction( SPISettings(1000000, MSBFIRST, SPI_MODE0));
        *TC_Data = SPI.transfer(0);           // TC 16 Bits               
        *TC_Data <<= 8;
        *TC_Data |= SPI.transfer(0);          // IC 16 Bits
        *IC_Data = SPI.transfer(0);
        *IC_Data <<= 8;                                 
        *IC_Data |= SPI.transfer(0);                                   
    SPI.endTransaction();
    digitalWrite(_cs_pin, HIGH);
}



//============================================
//        MAX31855 DECODE ALL 32 BITS 
//============================================
bool MAX_31855::Read_All(double *TC_Temp, double *IC_Temp, bool *Open_Circuit, bool *Short_To_GND, bool *Short_To_VCC)
{
    int local_TC_Temp = 0;
    int local_IC_Temp = 0;
    bool Status_OK = false;

    SPI_Read_32_Bits(&local_TC_Temp, &local_IC_Temp);

        if (local_TC_Temp & 0x1)                                           // Fault Present Bit 
        {
            Status_OK = false;
            *TC_Temp = -9999;
        } 
        else 
        {
            local_TC_Temp &= 0xfffc;                                 // Keep top 14 Temperature Bits  (14 Bit Value)                           
            *TC_Temp = local_TC_Temp / 16.0;                         // Divide by 4 to discard bottom two bits, then div by 4 again as 1 bit = 0.25 degC
            Status_OK = true;
        }

        *Open_Circuit = (local_IC_Temp & 0x1) ? true : false  ;       // D0 
        *Short_To_GND = ((local_IC_Temp>>1) & 0x1) ? true : false;    // D1
        *Short_To_VCC = ((local_IC_Temp>>2) & 0x1) ? true : false;    // D2
        local_IC_Temp = local_IC_Temp & 0xfff0;                       //  Keep Top 12 bits (12 Bit Value)               
        *IC_Temp = local_IC_Temp / 256.0;                             //  Div by 16 to drop bottom 4 bits. Div by 16 again as 1 bit = 0.0625 degC

        return Status_OK;
}

