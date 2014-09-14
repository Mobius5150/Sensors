/*
 * File:   purgeCodeV2-5.c
 * Author: sdamkjar@ualberta.ca
 *
 * Created on MARCH 28, 2014, 12:39 PM
 *
 * Version Notes: This working code uses Timer0 and outputs a periodic 5V signal
] *                to pin 4 on the PIC18F2685. Timing can be configured in the
 *                preamble below. Subsequent versions should use Timer1.
 */

////////////////////////////////////////////////////////////////////////////////
/////////////////////////// TIMING CONFIGURATION ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////
/* Enter the timing intervals here in milliseconds. For each cycle, the purge
 * valve will remain closed for WAIT milliseconds and then open for PURGE
 * milliseconds. Note that the minimum allowed interval is 20ms */
#define WAITtime 20000 /*ms (originally 1s)*/
#define PURGEtime 50 /*ms (originally 0.1s)*/
#define SENDtime 105   /*ms*/
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/////////////////////// SOME THINGS TO CONSIDER ////////////////////////////////

/* Documentation for the PIC18F2685 is located on the Google drive at:
 *   "C:\Users\sdamkjar\Google Drive\EcoCar\EE Team\Sensors\Device Documentation
 *       \IC Documentation"
 */

/* Always (especially if you have multiple projects) define the project you want
 * to compile by right-clicking that project and selecting "Set as Main Project"
 */

/* Unless you are using an external power supply to power the PIC while you
 * program it, you can set the PICKIT3 to power the PIC from your computer.
 *      To do this: Right-click your project and select "Properties"
 *         Under "Conf: [default]" in "Categories" select "PICkit3"
 *         Select "Power" from the "Option Categories" drop-down menu at the top
 *         Enable "Power target circuit from PICkit3"
 *         **** Make sure to select your voltage (usually 5V) from the
 *              "Voltage Level" drop down menu.
 */
////////////////////////////////////////////////////////////////////////////////

////////////////////// INPUT/OUPUT CONFIGURATION NOTES /////////////////////////

// Look at the pin diagram on page 4 of PIC18F2685 Documentation.pdf

/*  The input and output of the PIC is controlled by the following registers

 ** TRISA ( Data Direction Register )
 *    ( high or '1' for input and low or '0' for output )
 *    Example: To configure port RA0 as an output, use "TRISAbits.RA0 = 0;"

 ** LATA  => Latch Register
 *    ( used for sending or "latching" OUTPUT )
 *    Example: To set port RA0, use "LATAbits.LATA0 = 1;"

 ** PORTA => Port Register
 *    ( used for reading INPUT )
 *

 */
////////////////////////////////////////////////////////////////////////////////

///////////// INCLUDE THIS PREAMBLE IN EVERY PROJECT ///////////////////////////
#include <stdio.h>
#include <stdlib.h>

/* This header file includes important functions we need for the PIC18F2685 */
#include <p18cxxx.h>

/* precompiler directive configures the oscillator which is, whose factory
 * default frequency is 8 MHz */
#pragma config OSC = IRCIO67

/* Disable "watch-dog-timer". If enabled, the watch-dog timer needs to be reset
 * periodically otherwise the PIC will reset  */
#pragma config WDT = OFF
////////////////////////////////////////////////////////////////////////////////

/* If you want to use some timing, we need this header. Keep in mind that the
 * PIC sees time in numbers of clock cycles*/
#include <delays.h>

/* These are just to make the timing code (in the ISR) more readable. */
#define WAIT 0
#define PURGE 1


#include "J1939.h"
#include "ecocar.h"
#include <usart.h>
#include <adc.h>    // Analog

#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software

void Recv_FuelCell(void);
void ParseSerial(void);
void ADC_Init(void);
void StoreSerial(void);
void ClearSerial(void);
long int  SerialListen(void);
// Global Sensor Data:
char buffer[16];
// Serial data buffer
char parsedBuffer[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

char MsgData[];

J1939_MESSAGE Msg;
volatile int need_to_send_data = 0;
const unsigned long int cyclesMax = 500000;

void Process_and_Send_Data(J1939_MESSAGE*, int);

#define DATA_LEN 8
int sensorLatestData[DATA_LEN][DATA_ITEM_LENGTH] = {
    {DATA_FC_POWER, 0x00, 0x00},
    {DATA_FC_TEMP, 0x00, 0x00},
    {DATA_TEMPTRUNK, 0x00, 0x00},
    {DATA_FC_CURR, 0x00, 0x00},
    {DATA_FC_VSTACK, 0x00, 0x00},
    {DATA_FC_VBATT, 0x00, 0x00},
    {DATA_FC_PTANK, 0x00, 0x00},
    {DATA_FC_STATUS, 0x00, 0x00}
};

int sensorLastSentData[DATA_LEN][DATA_ITEM_LENGTH] = {
    {DATA_FC_POWER, 0x00, 0x00},
    {DATA_FC_TEMP, 0x00, 0x00},
    {DATA_TEMPTRUNK, 0x00, 0x00},
    {DATA_FC_CURR, 0x00, 0x00},
    {DATA_FC_VSTACK, 0x00, 0x00},
    {DATA_FC_VBATT, 0x00, 0x00},
    {DATA_FC_PTANK, 0x00, 0x00},
    {DATA_FC_STATUS, 0x00, 0x00}
};


/* The main function should always be 'void'.
 *** Make sure there is no return statement! */
void main( void )
{
    long long int result;
    int i = 0;
    int first_run = 1;
    int times_recv = 0;

    InitEcoCar();
    
    /* This line configures the RA0 port (pin 2 on the PIC) as an output pin.
     * This will control the purge valve. */
    TRISAbits.RA2 = 0;
    /* This ensures the purge valve is closed while the program initializes */
    LATAbits.LATA2 = 0;

    /* Disables the TIMER0 FLAG ( this is enabled at the start of the INTERRUPT
     * SERVICE ROUTINE and disabled at the end ) */
    INTCONbits.TMR0IF = 0;

    /* Enables the interrupt for TIMER0 overflow.*/
    INTCONbits.TMR0IE = 1;

    /* This is the TIMER0 CONTROL REGISTER
    * (Page 149 of PIC18F2685 Documentation.pdf defines each bit)
    * Here, we are using an 8-bit, internal, rising edge timer with 1:256
    * prescaling (this counts once every 256 clock cycles)
    */
    T0CON = 0b11000100;

    /* This disables interrupt priority */
    RCONbits.IPEN = 0;

    /* Setting this register to 1 allows interrupts. Otherwise, all interrupts
     * will be disabled */
    INTCONbits.GIE = 1;
    
    J1939_Initialization( TRUE );

//    ADC_Init();

    // Open USART:
    OpenUSART( USART_TX_INT_OFF &
    USART_RX_INT_OFF &
    USART_ASYNCH_MODE &
    USART_EIGHT_BIT &
    USART_CONT_RX &
    USART_BRGH_HIGH, 207 );

    // 207 = 9600 bps at 32 MHz
    BAUDCONbits.BRG16 = 0;  // Disable 16-bit SPBRG (low speed!)

    RCSTAbits.SPEN = 1; // Enable USART pin.

    // Check for CAN address collisions:
    while (J1939_Flags.WaitingForAddressClaimContention)
            J1939_Poll(5);

    /* This prevents the program from reaching the end of the main function */
    while (1) {
        J1939_Poll(5);
        
        // Broadcast the data as ordered by the master:
        if ( SerialListen() ) {
            ParseSerial();
            StoreSerial();
        }

        result = (sensorLatestData[3][1] << 8) | sensorLatestData[3][2];
        if ( need_to_send_data ) {
            result += 100;
            if ( result > 30000 ) {
                result = 0;
            }
        }

        sensorLatestData[3][1] = (result >> 8) & 0xFF;
        sensorLatestData[3][2] = result & 0xFF;

        if( need_to_send_data == 1 ) {
            need_to_send_data = 0;
            for (; i < DATA_LEN; ++i ) {
                if ( NeedToSendData( sensorLastSentData, sensorLatestData, i ) || first_run ) {
                    Process_and_Send_Data( &Msg, i );
                    J1939_Poll(10);
                }
            }

            if ( DATA_LEN == i ) {
//                first_run = 0;
                i = 0;
            }
        }
    }
}

//////////////////THIS IS OUR INTERRUPT SERVICE ROUTINE/////////////////////////
/* SOME THINGS TO KEEP IN MIND :
    Comments on the same line as the pragma statements here can cause problems*/
#pragma interrupt isr
void isr(void){

    /* With 1:1 prescaling, this will increment every 4 milliseconds. With a
     * LONG variable, this should be able to except a maximum interval of
     * roughly 100 days (this is probably excessive but the next smallest data
     * type (UNSIGNED INT) would have a maximum interval of only 4 minutes). */
    static long counter = 0;
    static long data_counter = 0;

    /* This is used to toggle interval duration. */
    static int mode = WAIT;

    /* The code should get here ever 4 milliseconds */
    if ( INTCONbits.TMR0IF ) {

	/* Increment the counter */
	counter++;
        data_counter++;

	/* If the purge valve is closed and the timer finishes the WAIT
	 * interval then open the valve, reset the timer, and switch to PURGE
	 * mode. The interval is in milliseconds or 1/4 timer cycles. */
	if (mode == WAIT && counter > WAITtime) {
            /* Open the purge valve */
            LATAbits.LATA2 = 1;
            /* Reset the timer */
            counter = 0;
            /* Switch to PURGE mode */
            mode = PURGE;
	}

	/* If the purge valve is closed and the timer finishes the PURGE
	 * interval then open the valve, reset the timer, and switch to WAIT
	 * mode. The interval is in milliseconds or 1/4 timer cycles.*/
	else if (mode == PURGE && counter > PURGEtime) {
            /* Close the purge valve */
            LATAbits.LATA2 = 0;
            /* Reset the timer */
            counter = 0;
            /* Switch to WAIT mode */
            mode = WAIT;
        }

        if ( data_counter > SENDtime ) {
            data_counter = 0;
            need_to_send_data = 1;
        }

	/* This resets the TIMER1 interrupt flag to get ready for next time*/
        INTCONbits.TMR0IF = 0;

    }
}

//////////////////THIS IS OUR INTERRUPT/////////////////////////
// This calls the interrupt service routine when the interrupt is called
#pragma code high_vector = 0x08

/* this 'function' can only be 8 bytes in length. This is why the code we want
 * to run with our interrupt will be put in the interrupt service routine   */
void high_interrupt(void){

    /* This is an assembly instruction. This efficiently calls our interrupt
     * service routine */
    _asm GOTO isr _endasm
}

/* There is actually a space at the end of this line and a line break after...
 * This is important for some strange reason */
#pragma code

//
//void ADC_Init()
//{
//   // Analog initialization for Fuel Cell
//   // We will be reading the pressure from AN0, so we
//    ADRESH = 0x00;              //Set the ADC variables to 0x00 initially
//    ADRESL = 0x00;
//
//
//    ADCON0bits.ADON = 0; // Disable A/D module
//    ADCON0bits.CHS0 = 0; // Select channel 0 (AN0)
//    ADCON0bits.CHS1 = 0;
//    ADCON0bits.CHS2 = 0;
//    ADCON0bits.CHS3 = 0;
//    ADCON1bits.VCFG1 = 0; // Use VSS for Vref- source
//    ADCON1bits.VCFG0 = 0; // Use VDD for Vref+ source
//
//    ADCON1bits.PCFG0 = 0; // Make AN0 pin analog and all others digital
//    ADCON1bits.PCFG1 = 1;
//    ADCON1bits.PCFG2 = 1;
//    ADCON1bits.PCFG3 = 1;
//    ADCON2bits.ADFM = 1; // A/D result is right justified
//
//    ADCON2bits.ACQT0 = 1; // Acquisition time
//    ADCON2bits.ACQT1 = 0;
//    ADCON2bits.ACQT2 = 0;
//
//    ADCON2bits.ADCS0 = 0; // A/D conversion clock
//    ADCON2bits.ADCS1 = 1;
//    ADCON2bits.ADCS2 = 1;
//    TRISAbits.TRISA0 = 1; // input
//    ADCON0bits.ADON = 1; // Enable A/D module
//
//    Delay10TCYx(5);             //Delay while the ADC is activated
//}
//
void Process_and_Send_Data(J1939_MESSAGE *MsgPtr, int i) {
    char data[8];
    data[1] = sensorLatestData[i][1]; // MSB
    data[0] = sensorLatestData[i][2]; // LSB
    Broadcast_Data(MsgPtr, sensorLatestData[i][0], data);
}

void ParseSerial()
{
    // This function processes the serial data stream coming from the
    // fuel cell.

    int i; int j;

    // To extract the correct 8 bytes, we look at a 16-byte stream for
    // special "anchor" bytes (0x00) in the 4th and 6th positions.

    // Examine the 16-byte array for the anchor bits (0x00):
    for(i=3;i<=11;i++)
    {
        if(buffer[i] == 0x00 && buffer[i+2] == 0x00)
        {
            // 00 2F 75 00 6A 00 44 89
            //          ^^
            // i indicates position #4 (index 3) in series
            for(j=0;j<=7;j++)
            {
                // Copy the bytes over into the parsedBuffer array.
                parsedBuffer[j] = buffer[i-3+j];
            }
        }
    }
}

void StoreSerial()
{
    long int result = 0;
    sensorLatestData[7][1] = 0x00;
    sensorLatestData[7][2] = parsedBuffer[0];

    if (sensorLatestData[7][2] == 20)
    {
        sensorLatestData[0][1] = 0x00;
        sensorLatestData[0][2] = 0x00;
    }
    else
    {
        sensorLatestData[0][1] = 0xFF;
        sensorLatestData[0][2] = 0xFF;
    }

    // Second Byte = Ambient Temperture, 2x value
    result = parsedBuffer[1] * 500;
    sensorLatestData[2][1] = ( result >> 8 );
    sensorLatestData[2][2] = result & 0xFF;

    // Third Byte = Stack Voltage, 3x value
    result = ( parsedBuffer[2] / 3 ) * 1000;
    sensorLatestData[4][1] = ( result >> 8 );
    sensorLatestData[4][2] = result & 0xFF;

    // Fourth Byte = H2 Voltage, no value (0x00, used as anchor)

    // Fifth Byte = Stack Temperature, 2x value
    result = parsedBuffer[1] * 500;
    sensorLatestData[1][1] = ( result >> 8 );
    sensorLatestData[1][2] = result & 0xFF;

    // Sixth & Seventh Bytes = Stack Current, 5x value
    // The MSB of the stack current should always be 0x00, and
    // is used a the second anchor byte.
    result = ( (parsedBuffer[6])) * 200; // 200 = 1000/5
    sensorLatestData[3][1] = ( result >> 8 );
    sensorLatestData[3][2] = result & 0xFF;

    // Eighth Byte = Battery Voltage, 10x value
    result = parsedBuffer[7] * 100;
    sensorLatestData[5][1] = ( result >> 8 );
    sensorLatestData[5][2] = result & 0xFF;

}
void ClearSerial()
{
    // Fuel cell is probably off, since we're getting timeouts.
    // Clear in-memory values.

    sensorLatestData[0][1] = 0x00;
    sensorLatestData[0][2] = 0x00;

    sensorLatestData[7][1] = 0x00;
    sensorLatestData[7][2] = 20;  // Assume manual turn off?

    sensorLatestData[2][1] = 0x00;
    sensorLatestData[2][2] = 0x00;

    sensorLatestData[4][1] = 0x00;
    sensorLatestData[4][2] = 0x00;

    sensorLatestData[1][1] = 0x00;
    sensorLatestData[1][2] = 0x00;

    sensorLatestData[3][1] = 0x00;
    sensorLatestData[3][2] = 0x00;

    sensorLatestData[5][1] = 0x00;
    sensorLatestData[5][2] = 0x00;

}

long int SerialListen()
{
    int i;
    unsigned long int cycleCounter;

            // Clear errors if they exist.
            if (RCSTAbits.OERR || RCSTAbits.FERR)
            {
                RCSTAbits.CREN = 0; /* clear any errors */
                Nop();
                RCSTAbits.CREN = 1; /* re-enable reception? */
            }

            // Wait for serial data from Fuel Cell.
            // Need to receive 8 bytes before it can proceed.
            for(i=0;i<8;i++)
            {
                cycleCounter = 0; // Reset timeout counter.
                while (!DataRdyUSART())
                {
                    cycleCounter++;
                    if(cycleCounter > cyclesMax){ break; }; // Timeout!
                }
                if(cycleCounter > cyclesMax){ break; }; // Timeout!
                buffer[i] = ReadUSART();
                buffer[i+8] = buffer[i];
            }

    return cycleCounter;
}
