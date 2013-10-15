/*
* CAN Bus - Misc. Sensors Slave Node
* Based on CAN Bus Demo B
*/

#include <p18cxxx.h>
#include "J1939.h"
#include "ecocar.h"
#include <usart.h>
#include <delays.h>
#include <stdio.h>
#include <adc.h>    // Analog
#include <stdlib.h> // Analog

#pragma config OSC = IRCIO67    // Oscillator Selection Bit
#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software
#pragma config WDT = OFF        // Watchdog Timer disabled (control is placed on the SWDTEN bit)

J1939_MESSAGE Msg;
void Process_and_Send_Data(J1939_MESSAGE *MsgPtr, unsigned char DataType);
void ADC_Init(void);

// Global Sensor Data:
char buffer[16];
// Serial data buffer
char parsedBuffer[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

char MsgData[];
int need_to_ack = 0;

int sensorData[4][3] =
    {{DATA_TEMPOUTSIDE, 0x00, 0x00},
    {DATA_TEMPCABIN, 0x00, 0x00},
    {DATA_VACCESSORY, 0x00, 0x00},
    {DATA_BKPALARM, 0x00, 0x00}};

void main( void )
{
    long int result;
    long int corr;
    char DATAH = 0x00;
    char DATAL = 0x00;
    int j;

    InitEcoCar();

    J1939_Initialization( TRUE );

    ADC_Init();

    // Check for CAN address collisions:
    while (J1939_Flags.WaitingForAddressClaimContention)
            J1939_Poll(5);

    while (1)
    {
        LATCbits.LATC1 = 0;     // Turn indicator LED off
        LATCbits.LATC0 = 1;     // Turn indicator LED on

        // Listen in for a signal from the master wanting our data.
        J1939_Poll(10);
        while (RXQueueCount > 0)
        {
            J1939_DequeueMessage( &Msg );
            if(Msg.PDUFormat == PDU_BROADCAST && Msg.GroupExtension == CYCLE_COMPLETE)
            {
                need_to_ack = 1;
            }
            else if(Msg.PDUFormat == PDU_REQUEST && Msg.SourceAddress == NODE_MASTER)
            {
                // Broadcast the data as ordered by the master:
                LATCbits.LATC1 = 1;     // Turn indicator LED on to signal end of cycle
                Process_and_Send_Data(&Msg, Msg.Data[0]);
            }
        }

        if(need_to_ack == 1)
        {
            need_to_ack = 0;
            // Skipping acquisition of outside temperature.
            // Already acquired by the fuel cell.

            // Aquire distance from backup sensor
            ADCON0bits.ADON = 0; // Disable A/D module
            ADCON0bits.CHS0 = 0; // Select channel 2 (AN2)
            ADCON0bits.CHS1 = 1;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS3 = 0;
            ADCON0bits.ADON = 1; // Re-enable A/D module
            Delay10TCYx(10);             //Delay while the ADC is activated

            result = 0;
            for(j=0;j<10;j++)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                result += ReadADC();     // Read result
            }

            // Calibration: 
            //              

            corr = result/10; // Convert to cm and take average (10x)
            DATAH = (corr>>8);
            DATAL = ((corr<<8)>>8);

            sensorData[3][1] = DATAH;
            sensorData[3][2] = DATAL;

            // Acquire the cabin temperature:
            ADCON0bits.ADON = 0; // Disable A/D module
            ADCON0bits.CHS0 = 1; // Select channel 1 (AN1)
            ADCON0bits.CHS1 = 0;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS3 = 0;
            ADCON0bits.ADON = 1; // Re-enable A/D module
            Delay10TCYx(10);             //Delay while the ADC is activated

            result = 0;
            for(j=0;j<10;j++)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                result += ReadADC();     // Read result
            }

            // Calibration: 22.6 deg. C = 163 (0.800V)
            //  500/1023 deg. C/ticks
            // Separate MSB and LSB:
            DATAH = (result>>8);
            DATAL = ((result<<8)>>8);

            sensorData[1][1] = 0;

            corr = (result - 1660)*488/1024;
            sensorData[1][2] = 30 + 22.6 + corr/10;

            // Acquire the outside temperature:
            ADCON0bits.ADON = 0;   // Disable A/D module
            ADCON0bits.CHS0 = 0;   // Select channel 0 (AN0)
            ADCON0bits.CHS1 = 0;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS3 = 0;
            ADCON0bits.ADON = 1;   // Re-enable A/D module
            Delay10TCYx(10);        //Delay while the ADC is activated


            result = 0;
            for(j=0;j<10;j++)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                result += ReadADC();     // Read result
            }

            // Calibration: 22.6 deg. C = 163 (0.800V)
            //  500/1023 deg. C/ticks
            // Separate MSB and LSB:
            DATAH = (result>>8);
            DATAL = ((result<<8)>>8);

            sensorData[0][1] = 0;

            corr = (result - 1660)*488/1024;
            sensorData[0][2] = 30 + 22.6 + corr/10;

            // Acquire the auxillary battery voltage:
            ADCON0bits.ADON = 0; // Disable A/D module
            ADCON0bits.CHS0 = 1; // Select channel 3 (AN3)
            ADCON0bits.CHS1 = 1;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS3 = 0;
            ADCON0bits.ADON = 1; // Re-enable A/D module
            Delay10TCYx(10);             //Delay while the ADC is activated

            result = 0;
            for(j=0;j<10;j++)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                result += ReadADC();     // Read result
            }

            result = result/47;

            DATAH = (result>>8);
            DATAL = ((result<<8)>>8);



            sensorData[2][1] = DATAH;
            sensorData[2][2] = DATAL;

            // Done analog data acquisition. No digital data, so
            // tell the master we're done:
            Broadcast_Data(&Msg, ACK_DONE, MsgData);
        }

    }
}

void Process_and_Send_Data(J1939_MESSAGE *MsgPtr, unsigned char DataType)
{
    char SensorData[8];
    int i;

    for(i=0;i<sizeof(sensorData)/sizeof(sensorData[0]);i++)
    {
        if(DataType == sensorData[i][0])
        {
            // Broadcast the data:
            char data[8];
            data[0] = sensorData[i][1]; // MSB
            data[1] = sensorData[i][2]; // LSB
            Broadcast_Data(MsgPtr, DataType, data);
        }
    }



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