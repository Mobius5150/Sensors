/*
* CAN Bus - Fuel Cell Slave Node
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
int need_to_ack = 0;
int times_recv = 0;
const unsigned long int cyclesMax = 500000;

int sensorData[8][3] =
    {{DATA_FC_POWER, 0x00, 0x00},
    {DATA_FC_TEMP, 0x00, 0x00},
    {DATA_TEMPTRUNK, 0x00, 0x00},
    {DATA_FC_CURR, 0x00, 0x00},
    {DATA_FC_VSTACK, 0x00, 0x00},
    {DATA_FC_VBATT, 0x00, 0x00},
    {DATA_FC_PTANK, 0x00, 0x00},
    {DATA_FC_STATUS, 0x00, 0x00}};


void main( void )
{
    int i; long int result;
    char TimeoutData[8];
    unsigned long int cycleCounter;
    
    
    char DATAH = 0x00;
    char DATAL = 0x00;

    InitEcoCar();
    
    J1939_Initialization( TRUE );

    ADC_Init();

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
                if(Msg.Data[0] == DATA_FC_CURR)
                {
                    times_recv++;
                }

                Process_and_Send_Data(&Msg, Msg.Data[0]);

                if(times_recv > 3)
                {
                        // Acquire some serial data.
                        cycleCounter = SerialListen();
                        if(cycleCounter <= cyclesMax)
                        {
                            ParseSerial();
                            StoreSerial();
                        }else{
                            ClearSerial();
                        }
                        times_recv = 0;
                }
            }
        }

        if(need_to_ack == 1)
        {
            need_to_ack = 0;
            
            //cycleCounter = SerialListen();

            //if(cycleCounter <= cyclesMax)
            //{
                // No timeouts encountered. Continue:
                
                // The data from the fuel cell is outputted in a continuous
                // sequence. By listening for special "anchor" bytes, we can
                // sort out which bytes belong to which sensor.

                //ParseSerial();

                // parsedBuffer now contains the sorted data (8 bytes).

                /* First byte = Fuel Cell Status:
                0 - NORMAL
                1 - BATTERY LOW
                2 - FC VOLTAGE LOW
                3 - H2 HIGH
                4 - CURRENT HIGH
                5 - TEMPERATURE HIGH
                20 - MANUAL TURN OFF
                */
                //StoreSerial();
                //cycleCounter = 0;
            //}else{
            //    ClearSerial(); // Fuel cell shut off.
            //}

            // Collect pressure sensor data:
            ConvertADC();           //Read from ADC
            while(BusyADC());       //Wait for ADC to finish conversion
            result = ReadADC();     // Read result

            // TODO: Apply scale factor to convert voltage to pressure.
            
            // Separate MSB and LSB:
            DATAH = (result>>8);    // Separate low and high bits
            DATAL = ((result<<8)>>8);
            
            sensorData[6][1] = DATAH;
            sensorData[6][2] = DATAL;

            Broadcast_Data(&Msg, ACK_DONE, MsgData);
        }
        
    }
}

void ADC_Init()
{
   // Analog initialization for Fuel Cell
   // We will be reading the pressure from AN0, so we
    ADRESH = 0x00;              //Set the ADC variables to 0x00 initially
    ADRESL = 0x00;


    ADCON0bits.ADON = 0; // Disable A/D module
    ADCON0bits.CHS0 = 0; // Select channel 0 (AN0)
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS2 = 0;
    ADCON0bits.CHS3 = 0;
    ADCON1bits.VCFG1 = 0; // Use VSS for Vref- source
    ADCON1bits.VCFG0 = 0; // Use VDD for Vref+ source

    ADCON1bits.PCFG0 = 0; // Make AN0 pin analog and all others digital
    ADCON1bits.PCFG1 = 1;
    ADCON1bits.PCFG2 = 1;
    ADCON1bits.PCFG3 = 1;
    ADCON2bits.ADFM = 1; // A/D result is right justified

    ADCON2bits.ACQT0 = 1; // Acquisition time
    ADCON2bits.ACQT1 = 0;
    ADCON2bits.ACQT2 = 0;

    ADCON2bits.ADCS0 = 0; // A/D conversion clock
    ADCON2bits.ADCS1 = 1;
    ADCON2bits.ADCS2 = 1;
    TRISAbits.TRISA0 = 1; // input
    ADCON0bits.ADON = 1; // Enable A/D module

    Delay10TCYx(5);             //Delay while the ADC is activated
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
                sensorData[7][1] = 0x00;
                sensorData[7][2] = parsedBuffer[0];

                if (sensorData[7][2] == 20)
                {
                    sensorData[0][1] = 0x00;
                    sensorData[0][2] = 0x00;
                }
                else
                {
                    sensorData[0][1] = 0xFF;
                    sensorData[0][2] = 0xFF;
                }

                // Second Byte = Ambient Temperture, 2x value
                sensorData[2][1] = 0x00;
                sensorData[2][2] = parsedBuffer[1];

                // Third Byte = Stack Voltage, 3x value
                sensorData[4][1] = 0x00;
                sensorData[4][2] = parsedBuffer[2];

                // Fourth Byte = H2 Voltage, no value (0x00, used as anchor)

                // Fifth Byte = Stack Temperature, 2x value
                sensorData[1][1] = 0x00;
                sensorData[1][2] = parsedBuffer[4];

                // Sixth & Seventh Bytes = Stack Current, 5x value
                // The MSB of the stack current should always be 0x00, and
                // is used a the second anchor byte.
                sensorData[3][1] = parsedBuffer[5];
                sensorData[3][2] = parsedBuffer[6];

                // Eighth Byte = Battery Voltage, 10x value
                sensorData[5][1] = 0x00;
                sensorData[5][2] = parsedBuffer[7];

}
void ClearSerial()
{

                // Fuel cell is probably off, since we're getting timeouts.
                // Clear in-memory values.

                sensorData[0][1] = 0x00;
                sensorData[0][2] = 0x00;

                sensorData[7][1] = 0x00;
                sensorData[7][2] = 20;  // Assume manual turn off?

                sensorData[2][1] = 0x00;
                sensorData[2][2] = 0x00;

                sensorData[4][1] = 0x00;
                sensorData[4][2] = 0x00;

                sensorData[1][1] = 0x00;
                sensorData[1][2] = 0x00;

                sensorData[3][1] = 0x00;
                sensorData[3][2] = 0x00;

                sensorData[5][1] = 0x00;
                sensorData[5][2] = 0x00;
            
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