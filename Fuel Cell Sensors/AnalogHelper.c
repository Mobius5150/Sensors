/*
 * File: AnalogHelper.c
 * Author: Mike Blouin
 *
 * Holds a helper function for reading analog data.
 */

#include <adc.h>
#include <delays.h>

int ReadAnalog(int analogInput) {
    int result = 0;
    int intermediate = 0;
    int j = 0;

    ADCON0 = 0x3C & (analogInput << 2);
    ADCON1 = 0x0F & (0xE - analogInput); // Set the analog input to use
    ADCON2 = 0b10001001;

    ADCON0bits.ADON = 1; // Re-enable A/D module

    Delay10TCYx(5); // Wait for analog input to read

    for(;j<10;j++) {
        ConvertADC();           //Read from ADC
        while(BusyADC());       //Wait for ADC to finish conversion
        result += ReadADC();     // Read result
    }

    // Disable analog inputs
    ADCON0 = 0x0F;

    return result/2;
}