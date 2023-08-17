#ifndef MAX_31855_h
#define MAX_31855_h


#include <Arduino.h>


class MAX_31855 
{
    public:
        MAX_31855(int SCK_Pin, int CS_Pin, int SO_Pin, int SI_Pin);
        bool Read_All(double *TC_Temp, double *IC_Temp, bool *Open_Circuit, bool *Short_To_GND, bool *Short_To_VCC);
    
    private:
        int _sck_pin;
        int _cs_pin; 
        int _so_pin;
        int _si_pin;
        void SPI_Read_32_Bits(int *TC_Data, int *IC_Data);
};

#endif