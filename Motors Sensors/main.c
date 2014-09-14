/*
* CAN Bus - Motors Slave Node
* Based on CAN Bus Demo B
*/

#include <p18cxxx.h>
#include "J1939.h"
#include "ecocar.h"
#include <usart.h>
#include <delays.h>
#include <timers.h>
#include "AnalogHelper.h"
#include "speed_detector.h"

#pragma config OSC = IRCIO67    // Oscillator Selection Bit
#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software
#pragma config WDT = OFF        // Watchdog Timer disabled (control is placed on the SWDTEN bit)
#define DATA_LEN 8

// Minimum time between CANbus messages
#define CANBUS_SEND_INTERVAL 20

// Time between overamperage step-offs in ms
#define BACKOFF_STEP_TIME 100

// Number of millivolts to backoff in each step
#define BACKOFF_MV 100

///// WARNING: We rely on this condition: FC_WARN_CURRENT < FC_EMRG_CURRENT
#define FC_WARN_CURRENT 28000
#define FC_EMRG_CURRENT 35000

J1939_MESSAGE Msg;
void Process_and_Send_Data(J1939_MESSAGE*, int);
void ADC_Init(void);

volatile char can_backoff = 0;
volatile char need_to_send_data = 0;
volatile char backing_off = 0;

int MPedal = 0x0000;
unsigned char     Set = 0;

typedef enum backoff {
    NOT_BACKING_OFF = 0,
    BACKING_OFF = 1,
    EMERG_BACKOFF = 2,
} backoff_t;

typedef enum cruise {
    CRUISE_OFF = 0,
    CRUISE_ON = 1
} cruise_t;

cruise_t Cruise = CRUISE_OFF;
backoff_t BackOffStatus = NOT_BACKING_OFF;

void set_cruise(cruise_t new_value);
int sensorLatestData[DATA_LEN][DATA_ITEM_LENGTH] = {
    {DATA_PWR_VBUCK, 0x00, 0x00},
    {DATA_PWR_MOTORON, 0x00, 0x00},
    {DATA_PWR_CURRMOT1, 0x00, 0x00},
    {DATA_PWR_CURRMOT2, 0x00, 0x00},
    {DATA_PWR_SPEED, 0x00, 0x00},
    {DATA_PWR_DIR, 0x00, 0x00},
    {DATA_PWR_CRUISEON, 0x00, 0x00}
};

int sensorLastSentData[DATA_LEN][DATA_ITEM_LENGTH] = {
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
    long long int result = 0;
    int first_run = 1;
    int i = 0;

    InitEcoCar();
    
    J1939_Initialization( TRUE );

    ADCON0 = 0x01;                  // Initialize the ADC Registers
    ADCON1 = 0x0E;
    ADCON2 = 0x8E;

    TRISA = 0xFF;             // Set A0 to input, others to output
    TRISB = 0b00001011;             // Set B2 (CANTX) to output, B3 (CANRX) to input
    TRISC = 0b00000000;
    LATCbits.LATC3 = 0;

    OpenTimer2(T1_SOURCE_CCP);
    OpenPWM1(0xFF);                 //Turn on PWM capabilities

//    ADC_Init(); // Activate AN0-3 for the 4 analog inputs

    // We also need to read cruise and the direction from the
    // motors controller. Set RA7 and RA6 as digital inputs.
    // Note that these ports do not support analog, so the ADCON
    // registers in ADC_Init() do not affect RA6 and RA7.
    TRISAbits.TRISA6 = 1; //  Direction (fwd/rev)
    TRISAbits.TRISA7 = 1; //  Cruise on/off
    TRISCbits.TRISC0 = 1; // Brake

    // Setup Timer0
    INTCONbits.TMR0IE = 1; // Enable the timer
    INTCONbits.TMR0IF = 0; // Clear the flag
//    T0CON = 0b10000100; // 1:64 , 16 bits
    T0CON = 0b11000111;

    // Setup interrupt on port b
//    TRISBbits.RB4 = 1; // Setup portb0 as an input
    INTCONbits.RBIE = 1; // Enable Interrupt0
    INTCONbits.RBIF = 0; // Clear the Interrupt0 flag

    // Enable interrupts and disable priorities
    RCONbits.IPEN = 0;
    INTCONbits.GIE = 1;

    // Check for CAN address collisions:
    while (J1939_Flags.WaitingForAddressClaimContention)
        J1939_Poll(5);

    while (1) {
        J1939_Poll(10);
        if (RXQueueCount > 0)
            J1939_DequeueMessage( &Msg );

        // Check the FC current status
        if ( DATA_FC_CURR == Msg.GroupExtension ) {
            unsigned long int fc_curr = (Msg.Data[1] << 8) | Msg.Data[0];
            if ( fc_curr > FC_WARN_CURRENT ) {
                // Only engage cruise if we switch from not backing off to backing off
                if ( BackOffStatus == NOT_BACKING_OFF ) {
                    Cruise = CRUISE_ON;
                }

                if ( fc_curr > FC_EMRG_CURRENT ) {
                    // We're above the emergency current threshold
                    BackOffStatus = EMERG_BACKOFF;
                } else {
                    // We're above the warning current threshold
                    BackOffStatus = BACKING_OFF;
                }
            } else {
                // We're at a safe current level
                BackOffStatus = NOT_BACKING_OFF;
            }
        }

        // The brake can turn off the cruise
        if ( PORTCbits.RC0 == 1 ) {
            MPedal = 0;
            Cruise = CRUISE_OFF;
        } else if ( BackOffStatus = NOT_BACKING_OFF ) {
            // The brake is not engaged and we aren't backing off. Read cruise status
            if ( PORTAbits.RA7 == 1 && Set == 1 ) {
                Set = 0;
                if(Cruise == CRUISE_OFF){
                    set_cruise( CRUISE_ON );
                } else if (Cruise == CRUISE_ON){
                    set_cruise( CRUISE_OFF );
                }
            } else if ( PORTAbits.RA7 == 0 ) {
                Set = 1;
            }
        }

        // Only allow the driver to control speed if we aren't backing off
        if ( BackOffStatus == NOT_BACKING_OFF  && Cruise == CRUISE_OFF ) {
            MPedal = ReadAnalog(4) * 0.20;;
        } else if ( BackOffStatus == BACKING_OFF && can_backoff ) {
            can_backoff = 0;
            if ( MPedal > BACKOFF_MV ) {
                MPedal -= BACKOFF_MV;
            } else {
                MPedal = 0;
            }
        } else if ( BackOffStatus == EMERG_BACKOFF ) {
            MPedal = 0;
        }

        SetDCPWM1(MPedal);
//            SetDCPWM1(0);
        // Acquire the buck converter voltage from AN0
        result = ReadAnalog(0);
        sensorLatestData[0][1] = (result>>8);
        sensorLatestData[0][2] = result & 0xFF;

        // Acquire the motor current from AN1:
        // TODO: Apply scale factor to motor current
        result = ReadAnalog(1);
        result = (result*47808)-123149;
        sensorLatestData[2][1] = (result>>8);
        sensorLatestData[2][2] = result & 0xFF;

        // Acquire the motor current AN2:
        // TODO: Apply scale factor to the voltage to 10 x motor current
        result = ReadAnalog(2);
        sensorLatestData[3][1] = (result>>8);
        sensorLatestData[3][2] = result & 0xFF;

        // Read speed from AN3
        // (km/h)/(e/s)
        result = get_average_speed();
//        result = ReadAnalog(3);
//        result = (result * 12523);
//        result /= 1000;
        sensorLatestData[4][1] = (result>>8);
        sensorLatestData[4][2] = result & 0xFF;
//        if ( need_to_send_data == 1 ) {
//            sensorLatestData[4][1] += 1;
//            sensorLatestData[4][2] += 1;
//        }
//        if ( sensorLatestData[4][2] > 80 )
//            sensorLatestData[4][2] = 0;
//        if ( sensorLatestData[4][1] > 80 )
//            sensorLatestData[4][1] = 0;

        // Done acquisition of analog data.

        if(Cruise == CRUISE_ON) {
            // Cruise control on.
            sensorLatestData[6][1] = 0xFF;
            sensorLatestData[6][2] = 0xFF;
        } else {
            // Cruise control off.
            sensorLatestData[6][1] = 0x00;
            sensorLatestData[6][2] = 0x00;
        }

        if(PORTAbits.RA6 == 1) {
            // Motor direction forward
            sensorLatestData[5][1] = 0x00;
            sensorLatestData[5][2] = 0x00;
        } else {
            // reverse direction:
            sensorLatestData[5][1] = 0xFF;
            sensorLatestData[5][2] = 0xFF;
        }

        // Check if motor currents are non-zero:
        if(sensorLatestData[2][2] > 0x00 || (sensorLatestData[2][1] > 0x00 && sensorLatestData[2][2] == 0x00) || sensorLatestData[3][2] > 0x00 || (sensorLatestData[3][1] > 0x00 && sensorLatestData[3][2] == 0x00)) {
            // There is a motor current. Turn motor power toggle on.
            sensorLatestData[1][1] = 0xFF;
            sensorLatestData[1][2] = 0xFF;
        } else {
            // No motor current. Turn motor power toggle off.
            sensorLatestData[1][1] = 0x00;
            sensorLatestData[1][2] = 0x00;
        }

        // Check if its time to send the next data item
        // Do this here rather than the ISR to make sure we've completed a full
        // cycle of data collection
        if( need_to_send_data == 1 ) {
            need_to_send_data = 0;
            for (; i < DATA_LEN; ++i ) {
                if ( NeedToSendData( sensorLastSentData, sensorLatestData, i ) || first_run ) {
                    Process_and_Send_Data( &Msg, i );
                    J1939_Poll(10);
                }
            }

            if ( DATA_LEN == i ) {
                first_run = 0;
                i = 0;
            }
        }
    }
}

void set_cruise( cruise_t new_value ) {
    Cruise = new_value == CRUISE_ON;
//    LATCbits.LATC3 = new_value == CRUISE_ON;
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
    static unsigned long long int time = 0;
    static unsigned long long int speed = 0;
    static int last_canbus_send = 0;
    int last_backoff_time = 0;
    int last_speed_tick = 0;

    if (INTCONbits.TMR0IF) {
        ++time;
        ++last_canbus_send;
        ++last_backoff_time;
        ++last_speed_tick;

        // Every timer overflow mark that we should send data
        if ( last_canbus_send > CANBUS_SEND_INTERVAL ) {
            need_to_send_data = 1;
            last_canbus_send = time;
        }

        if ( last_backoff_time > BACKOFF_STEP_TIME ) {
            can_backoff = 1;
            last_backoff_time = time;
        }

        INTCONbits.TMR0IF = 0;
    } else if (INTCONbits.RBIF) {
        if ( last_speed_tick == 0 ) {
            return;
        }

        // Determine the instantaneous speed
        speed = (1000/last_speed_tick) * 540;

        // Log the speed
        log_speed(speed/1000);

        // Mark the tick
        last_speed_tick = 0;

        // Reset the timer
        INTCONbits.RBIF = 0;
    }
}

#pragma code high_vector = 0x08
void high_interrupt(void) {
    _asm GOTO isr _endasm
}
#pragma code

void Process_and_Send_Data(J1939_MESSAGE *MsgPtr, int i) {
    char data[8];
    data[1] = sensorLatestData[i][1]; // MSB
    data[0] = sensorLatestData[i][2]; // LSB
    Broadcast_Data(MsgPtr, sensorLatestData[i][0], data);
}

