/*
* CAN Bus - Motors Slave Node
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
void Process_and_Send_Data(J1939_MESSAGE*, int);
void ADC_Init(void);

volatile int need_to_send_data = 0;

int sensorData[8][3] = {
    {DATA_PWR_VBUCK, 0x00, 0x00},
    {DATA_PWR_MOTORON, 0x00, 0x00},
    {DATA_PWR_CURRMOT1, 0x00, 0x00},
    {DATA_PWR_CURRMOT2, 0x00, 0x00},
    {DATA_PWR_SPEED, 0x00, 0x00},
    {DATA_PWR_DIR, 0x00, 0x00},
    {DATA_PWR_CRUISEON, 0x00, 0x00}
};

void main( void )
{
    long int result;
    int j = 0;

    InitEcoCar();
    
    J1939_Initialization( TRUE );

    ADC_Init(); // Activate AN0-3 for the 4 analog inputs

    // We also need to read cruise and the direction from the
    // motors controller. Set RA7 and RA6 as digital inputs.
    // Note that these ports do not support analog, so the ADCON
    // registers in ADC_Init() do not affect RA6 and RA7.
    TRISAbits.TRISA6 = 1; //  Direction (fwd/rev)
    TRISAbits.TRISA7 = 1; //  Cruise on/off

    // Check for CAN address collisions:
    while (J1939_Flags.WaitingForAddressClaimContention)
        J1939_Poll(5);

    while (1) {
        // This node does not currently accept messages
        J1939_Poll(10);
        while (RXQueueCount > 0)
            J1939_DequeueMessage( &Msg );

        // Acquire the buck converter voltage from AN0
        result = ReadAnalog(0);
        sensorData[0][1] = (result>>8);
        sensorData[0][2] = result & 0xFF;

        // Acquire the motor current from AN1:
        // TODO: Apply scale factor to motor current
        result = ReadAnalog(1);
        sensorData[2][1] = (result>>8);
        sensorData[2][2] = result & 0xFF;

        // Acquire the motor current AN2:
        // TODO: Apply scale factor to the voltage to 10 x motor current
        result = ReadAnalog(2);
        sensorData[3][1] = (result>>8);
        sensorData[3][2] = result & 0xFF;

        // Read speed from AN3
        result = ReadAnalog(3);
        sensorData[4][1] = (result>>8);
        sensorData[4][2] = result & 0xFF;

        // Done acquisition of analog data.

        if(PORTAbits.RA7 == 1) {
            // Cruise control on.
            sensorData[6][1] = 0xFF;
            sensorData[6][2] = 0xFF;
        } else {
            // Cruise control off.
            sensorData[6][1] = 0x00;
            sensorData[6][2] = 0x00;
        }

        if(PORTAbits.RA6 == 1) {
            // Motor direction forward
            sensorData[5][1] = 0x00;
            sensorData[5][2] = 0x00;
        } else {
            // reverse direction:
            sensorData[5][1] = 0xFF;
            sensorData[5][2] = 0xFF;
        }

        // Check if motor currents are non-zero:
        if(sensorData[2][2] > 0x00 || (sensorData[2][1] > 0x00 && sensorData[2][2] == 0x00) || sensorData[3][2] > 0x00 || (sensorData[3][1] > 0x00 && sensorData[3][2] == 0x00)) {
            // There is a motor current. Turn motor power toggle on.
            sensorData[1][1] = 0xFF;
            sensorData[1][2] = 0xFF;
        } else {
            // No motor current. Turn motor power toggle off.
            sensorData[1][1] = 0x00;
            sensorData[1][2] = 0x00;
        }

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

void ADC_Init()
{
   // We are reading in buck conv. voltage, motor currents, and speed with
    // the analog inputs.
    // Open up AN0-3 for analog read
    
    ADRESH = 0x00;              //Set the ADC variables to 0x00 initially
    ADRESL = 0x00;

    ADCON0bits.ADON = 0; // Disable A/D module
    ADCON0bits.CHS0 = 0; // Select channel 0 (AN0)
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS2 = 0;
    ADCON0bits.CHS3 = 0;
    ADCON1bits.VCFG1 = 0; // Use VSS for Vref- source
    ADCON1bits.VCFG0 = 0; // Use VDD for Vref+ source

    ADCON1bits.PCFG0 = 1; // Make AN0-2 pins analog, AN3 digital
    ADCON1bits.PCFG1 = 1;
    ADCON1bits.PCFG2 = 0;
    ADCON1bits.PCFG3 = 1;
    ADCON2bits.ADFM = 1; // A/D result is right justified

    ADCON2bits.ACQT0 = 1; // Acquisition time
    ADCON2bits.ACQT1 = 0;
    ADCON2bits.ACQT2 = 0;

    ADCON2bits.ADCS0 = 0; // A/D conversion clock
    ADCON2bits.ADCS1 = 1;
    ADCON2bits.ADCS2 = 1;
    TRISAbits.TRISA0 = 1; // input
    TRISAbits.TRISA1 = 1; // input
    TRISAbits.TRISA2 = 1; // input
    TRISAbits.TRISA3 = 1; // input
    TRISAbits.TRISA6 = 1; // input
    TRISAbits.TRISA7 = 1; // input
    
    ADCON0bits.ADON = 1; // Enable A/D module

    Delay10TCYx(5);             //Delay while the ADC is activated
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