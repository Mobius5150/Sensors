/* 
 * File:   main.c
 * Author: Matt Amyotte
 *
 * This code is desinged to control the voltage signal running into the motor
 * controller pedal input.  It increases the input voltage from the pedal, to
 * creat a proper input range for the controller.  Additionally, it handles
 * the cruise control function, which sets the voltage to be constant.  Cruise
 * control is enabled through a button press interrupt, and turned off by the
 * same button, or the application of the brakes.
 *
 * Created on March 21, 2013, 8:19 PM
 */

#include <p18cxxx.h>
#include <delays.h>
#include <pwm.h>
#include <adc.h>
#include <timers.h>

#pragma config OSC = IRCIO67    // Oscillator Selection Bit
#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software
#pragma config WDT = OFF        // Watchdog Timer disabled (control is placed on the SWDTEN bit)

unsigned long int    Pedal = 0x0000;
unsigned long int    MPedal = 0x0000;
unsigned char   Cruise = 0;
unsigned char   Set = 0;



void main(void) {

    ADCON0 = 0x01;                  // Initialize the ADC Registers
    ADCON1 = 0x0E;
    ADCON2 = 0x8E;

    TRISA = 0b00000001;             // Set A0 to input, others to output
    TRISB = 0b00001011;             // Set B2 (CANTX) to output, B3 (CANRX) to input
    TRISC = 0b00000000;
    LATCbits.LATC3 = 0;
    
    INTCONbits.GIE = 0; //global interrupt enable ON
    OpenTimer2(T1_SOURCE_CCP);
    OpenPWM1(0xFF);                 //Turn on PWM capabilities

    while(1){
        if(Cruise == 0){
            ConvertADC();           //Read from ADC
            while(BusyADC());       //Wait for ADC to finish conversion
            Pedal = ReadADC();      //Read the ADC result
        }
        
        if(PORTBbits.RB0 == 1){
            Cruise = 0;
            LATCbits.LATC3 = 0;
        }

        while(PORTBbits.RB1 == 1){
            Set = 1;
        }
        if(Set == 1 && Cruise == 0){
            Cruise = 1;
            LATCbits.LATC3 = 1;
            Set = 0;
        }
        else if (Set == 1 && Cruise == 1){
            Cruise = 0;
            LATCbits.LATC3 = 0;
            Set = 0;
        }

        MPedal = Pedal;
        SetDCPWM1(Pedal);
    }
}

