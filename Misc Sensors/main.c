/*
* CAN Bus - Misc. Sensors Slave Node
* Based on CAN Bus Demo B
*/

#include <p18cxxx.h>
#include "J1939.h"
#include "ecocar.h"
#include <usart.h>
#include <delays.h>
#include "AnalogHelper.h"

#pragma config OSC = IRCIO67    // Oscillator Selection Bit
#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software
#pragma config WDT = OFF        // Watchdog Timer disabled (control is placed on the SWDTEN bit)

J1939_MESSAGE Msg;
volatile int need_to_send_data = 0;

void Process_and_Send_Data(J1939_MESSAGE*, int);
void ADC_Init(void);

int sensorData[4][3] = {
    {DATA_TEMPOUTSIDE, 0x00, 0x00},
    {DATA_TEMPCABIN, 0x00, 0x00},
    {DATA_VACCESSORY, 0x00, 0x00},
    {DATA_BKPALARM, 0x00, 0x00}
};

void main( void ) {
    long int result;
    long int corr;
    int j = 0;

    InitEcoCar();

    J1939_Initialization( TRUE );

    ADC_Init();

    // Setup Timer0
    INTCONbits.TMR0IE = 1; // Enable the timer
    INTCONbits.TMR0IF = 0; // Clear the flag
    T0CON = 0b10000101; // 1:64 , 16 bits

    // Enable interrupts and disable priorities
    RCONbits.IPEN = 0;
    INTCONbits.GIE = 1;

    // Check for CAN address collisions:
    while (J1939_Flags.WaitingForAddressClaimContention)
        J1939_Poll(5);

    while (1) {
        // This node currently does nothing with received messages
        J1939_Poll(10);
        while (RXQueueCount > 0)
            J1939_DequeueMessage( &Msg );
        
        //---- Aquire distance from backup sensor --------------------------
        // Read from analog channel 2 (AN2)
        result = ReadAnalog(2); // Convert to cm and take average (10x)
        sensorData[3][1] = (result>>8);        // DATAH
        sensorData[3][2] = result & 0xFF;      // DATAL ((result<<8)>>8);

        //---- Acquire the cabin temperature: ------------------------------
        // Read from analog channel 1 (AN1)
        result = ReadAnalog(1);

        // Calibration: 22.6 deg. C = 163 (0.800V)
        //  500/1023 deg. C/ticks
        // Separate MSB and LSB:
        corr = 30 + 22.6 + (result - 1660)*488/1024;
        corr = 30 + 22.6 + corr/10;
        sensorData[1][1] = (corr >> 8) & 0xFF;
        sensorData[1][2] = corr & 0xFF;

        //---- Acquire the outside temperature: ----------------------------
        // Select analog channel 0 (AN0)
        result = ReadAnalog(0);

        // Calibration: 22.6 deg. C = 163 (0.800V)
        //  500/1023 deg. C/ticks
        // Separate MSB and LSB:
        corr = 30 + 22.6 + (result - 1660)*488/1024;
        corr = 30 + 22.6 + corr/10;
        sensorData[0][1] = (corr >> 8) & 0xFF;
        sensorData[0][2] = corr & 0xFF;

        //---- Acquire the auxillary battery voltage: ----------------------
        // Read from channel 3 (AN3)
        result = ReadAnalog(3) * 4;
        sensorData[2][1] = (result >> 8);   // DATAH;
        sensorData[2][2] = result & 0xFF;  // DATAL;
        
        // Check if its time to send the next data item
        // Do this here rather than the ISR to make sure we've completed a full
        // cycle of data collection
        if( need_to_send_data == 1 ) {
            need_to_send_data = 0;
            Process_and_Send_Data( Msg, j );
            j = ++j % 4;
        }
    }
}

#pragma interrupt isr
void isr(void) {
    if (INTCONbits.TMR0IF) {
        // Every timer overflow mark that we should send data
        need_to_send_data = 1;
        INTCONbits.TMR0IF = 0;
    }
}

#pragma code high_vector = 0x08
void high_interrupt(void) {
    _asm GOTO isr _endasm
}
#pragma code

void Process_and_Send_Data(J1939_MESSAGE *MsgPtr, int i) {
    char data[8];
    data[0] = sensorData[i][1]; // MSB
    data[1] = sensorData[i][2]; // LSB
    Broadcast_Data(MsgPtr, sensorData[i][0], data);
}

void ADC_Init()
{
    // Analog initialization for Misc. Sensors board
    // We will be reading outside, cabin, and trunk temperatures
    // as well as the auxillary battery voltage. (AN0-3)

    ADRESH = 0x00;              //Set the ADC variables to 0x00 initially
    ADRESL = 0x00;

    ADCON0bits.ADON = 0; // Disable A/D module
    ADCON0bits.CHS0 = 0; // Select channel 0 (AN0)
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS2 = 0;
    ADCON0bits.CHS3 = 0;
    ADCON1bits.VCFG1 = 0; // Use VSS for Vref- source
    ADCON1bits.VCFG0 = 0; // Use VDD for Vref+ source

    ADCON1bits.PCFG0 = 0; // Make AN0-3 pins analog
    ADCON1bits.PCFG1 = 0;
    ADCON1bits.PCFG2 = 0;
    ADCON1bits.PCFG3 = 0;
    ADCON2bits.ADFM = 1; // A/D result is right justified

    ADCON2bits.ACQT0 = 1; // Acquisition time
    ADCON2bits.ACQT1 = 1;
    ADCON2bits.ACQT2 = 0;

    ADCON2bits.ADCS0 = 0; // A/D conversion clock
    ADCON2bits.ADCS1 = 1;
    ADCON2bits.ADCS2 = 0;
    TRISAbits.TRISA0 = 1; // input
    TRISAbits.TRISA1 = 1; // input
    TRISAbits.TRISA2 = 1; // input
    TRISAbits.TRISA3 = 1; // input
    ADCON0bits.ADON = 1; // Enable A/D module

    Delay10TCYx(5);             //Delay while the ADC is activated
}