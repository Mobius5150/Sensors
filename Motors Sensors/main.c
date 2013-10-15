/*
* CAN Bus - Motors Slave Node
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
int counting_speed = 0;
int speed_prev_state = 0;
long int elapsed_time = 0x00000000;
unsigned int cycles = 0;
unsigned long int speed = 0;

int sensorData[8][3] =
    {{DATA_PWR_VBUCK, 0x00, 0x00},
    {DATA_PWR_MOTORON, 0x00, 0x00},
    {DATA_PWR_CURRMOT1, 0x00, 0x00},
    {DATA_PWR_CURRMOT2, 0x00, 0x00},
    {DATA_PWR_SPEED, 0x00, 0x00},
    {DATA_PWR_DIR, 0x00, 0x00},
    {DATA_PWR_CRUISEON, 0x00, 0x00}};

void main( void )
{
    int i; long int result;
    int j;
    long int tmp;
    int backup;
    int curr_val;
    
    char DATAH = 0x00;
    char DATAL = 0x00;

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
            // Acquire the cabin temperature:
            ADCON0bits.ADON = 0; // Disable A/D module
            ADCON0bits.CHS0 = 0; // Select channel 0 (AN0)
            ADCON0bits.CHS1 = 0;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS3 = 0;
            ADCON0bits.ADON = 1; // Re-enable A/D module
            Delay10TCYx(5);             //Delay while the ADC is activated

            result = 0;
            for(j=0;j<10;j++)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                result += ReadADC();     // Read result
            }
            // TODO: Apply scale factor to the data to convert voltage divider
            // voltage to 10 x buck converter voltage
                        
            // Separate MSB and LSB:
            DATAH = (result>>8);
            DATAL = ((result<<8)>>8);

            sensorData[0][1] = DATAH;
            sensorData[0][2] = DATAL;

            // Acquire the motor current (1):
            ADCON0bits.ADON = 0; // Disable A/D module
            ADCON0bits.CHS0 = 1; // Select channel 1 (AN1)
            ADCON0bits.CHS1 = 0;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS3 = 0;
            ADCON0bits.ADON = 1; // Re-enable A/D module
            Delay10TCYx(5);             //Delay while the ADC is activated

            result = 0;
            for(j=0;j<10;j++)
            {
                ConvertADC();          //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                result += ReadADC();     // Read result
            }
            // x ticks * 4.88 mV/tick * 1A/15mV = tmp
            // TODO: Apply scale factor to the voltage to 10 x motor current
            tmp = result;
            if(tmp < 0)
            {
                result = 0x0000;
            }else{
                result = tmp;
            }

            // Separate MSB and LSB:
            DATAH = (result>>8);
            DATAL = ((result<<8)>>8);

            sensorData[2][1] = DATAH;
            sensorData[2][2] = DATAL;

            // Acquire the motor current (2):
            ADCON0bits.ADON = 0; // Disable A/D module
            ADCON0bits.CHS0 = 0; // Select channel 2 (AN2)
            ADCON0bits.CHS1 = 1;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS3 = 0;
            ADCON0bits.ADON = 1; // Re-enable A/D module
            Delay10TCYx(5);             //Delay while the ADC is activated

            result = 0;
            for(j=0;j<10;j++)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                result += ReadADC();     // Read result
            }

            // TODO: Apply scale factor to the voltage to 10 x motor current
            tmp = result;
            if(tmp < 0)
            {
                result = 0x0000;
            }else{
                result = tmp;
            }
            
            // Separate MSB and LSB:
            DATAH = (result>>8);
            DATAL = ((result<<8)>>8);

            sensorData[3][1] = DATAH;
            sensorData[3][2] = DATAL;
            
            // SPEED
            T0CON = 0b00000110;
            TMR0H = 0x00;
            TMR0L = 0x00;
            
            ADCON0bits.ADON = 0; // Disable A/D module
            ADCON0bits.CHS0 = 1; // Select channel 4 (AN3)
            ADCON0bits.CHS1 = 1;
            ADCON0bits.CHS2 = 0;
            ADCON0bits.CHS3 = 0;
            ADCON0bits.ADON = 1; // Re-enable A/D module
            Delay10TCYx(5);             //Delay while the ADC is activated
/*
            T0CON = 0b10000110;
            if(1)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                curr_val = ReadADC();     // Read result
                
                if(curr_val > 900){
                    speed_prev_state = 1;
                }else if(curr_val < 200){
                    speed_prev_state = 0;
                }

            //speed_prev_state = PORTAbits.RA3;
            cycles = 0;
            while(cycles < 50000)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                curr_val = ReadADC();     // Read result
                
                if(curr_val > 900){
                    curr_val = 1;
                }else if(curr_val < 100){
                    curr_val = 0;
                }

                if(curr_val == 0 && counting_speed == 0 && curr_val != speed_prev_state)
                {
                    // Pin is low, not counting, and state changed.
                    // Start counting.
                    T0CON = 0b00000110; // Turn off timer.
                    TMR0H = 0x00;   // Reset timer.
                    TMR0L = 0x00;
                    T0CON = 0b10000110; // Turn on timer.
                    counting_speed = 1;
                    Delay1KTCYx(255);

                }else if(curr_val == 1 && counting_speed == 1 && curr_val != speed_prev_state)
                {
                    // Pin has transitioned from zero to one, and we are counting.
                    // End timer.
                    elapsed_time = (TMR0H*256 + TMR0L);
                    counting_speed = 0;
                    break;
                }

                speed_prev_state = curr_val; // Update previous speed to be the current value.
                cycles += 1;
                Delay10TCYx(1);
            }
            }

            // Assuming circumference = 1m
            if(cycles < 50000)
            {
                speed = elapsed_time;
            }else{
                speed = 0;
            }
*/
            result = 0;
            for(j=0;j<10;j++)
            {
                ConvertADC();           //Read from ADC
                while(BusyADC());       //Wait for ADC to finish conversion
                result += ReadADC();     // Read result
            }
            
            result = result/10/2;
            result -= 1; // Subtract the offset from the speed.
            
            if(result <= 1)
            {
                result = 0x0000; // Speed too low to be trusted. Set to zero.
            }


            // Send 16-bit elapsed_time to display as speed.
            sensorData[4][1] = (result>>8);
            sensorData[4][2] = ((result<<8)>>8);

            // Done acquisition of analog/digital data.
            // Now acquire the digital data:

            if(PORTAbits.RA7 == 1)
            {
                // Cruise control on.
                sensorData[6][1] = 0xFF;
                sensorData[6][2] = 0xFF;

            }else{
                // Cruise control off.
                sensorData[6][1] = 0x00;
                sensorData[6][2] = 0x00;
            }
            backup = PORTAbits.RA6;
            
            if(backup == 1)
            {
                // Motor direction forward
                sensorData[5][1] = 0x00;
                sensorData[5][2] = 0x00;
            }else{
                // reverse direction:
                sensorData[5][1] = 0xFF;
                sensorData[5][2] = 0xFF;
            }

            // Check if motor currents are non-zero:
            if(sensorData[2][2] > 0x00 || (sensorData[2][1] > 0x00 && sensorData[2][2] == 0x00) || sensorData[3][2] > 0x00 || (sensorData[3][1] > 0x00 && sensorData[3][2] == 0x00))
            {
                // There is a motor current. Turn motor power toggle on.
                sensorData[1][1] = 0xFF;
                sensorData[1][2] = 0xFF;
            }else{
                // No motor current. Turn motor power toggle off.
                sensorData[1][1] = 0x00;
                sensorData[1][2] = 0x00;
            }

            // Done analog and digital data acquisition.
            // Signal to the master that we are done:
            Broadcast_Data(&Msg, ACK_DONE, MsgData);
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