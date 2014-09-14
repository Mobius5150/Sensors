/*
* USART wireless transmission based off node D (Passive Listener)
* Authors: Tim Ho, Michael Blouin
*
*/

#include <p18cxxx.h>	// Contains core functions for the PIC 18F series microcontroller.
#include "J1939.H"	// Header file for CANbus J1939 routines.
#include <usart.h>	// Contains code for USART communications
#include "ecocar.h"

#pragma config OSC = IRCIO67	// This configures the PIC's internal oscillator to be used, instead of an external one.
#pragma config WDT = OFF        // Watchdog Timer disabled (control is placed on the SWDTEN bit)
#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software

J1939_MESSAGE Msg;
void main( void ) {
    // Pins C6 and C7 are used for UART TX and RX respectively
    InitEcoCar();
    TRISCbits.RC4 = 0;

    // Open USART:
    OpenUSART( USART_TX_INT_OFF &
        USART_RX_INT_OFF &
        USART_ASYNCH_MODE &
        USART_EIGHT_BIT &
        USART_CONT_RX &
        USART_BRGH_HIGH, 207 );     // 9600 bps baud rate

    RCSTAbits.SPEN = 1;		// Enable USART on pins C6, C7
    BAUDCONbits.BRG16 = 0;	// 8-bit BR Generator
    
    J1939_Initialization( TRUE );

    while (J1939_Flags.WaitingForAddressClaimContention)
        J1939_Poll(5);
    
    // CANbus is now initialized and we can now loop while we check
    // our message receive buffer for new CANbus messages (where all received messages are put).
    while (1) {
        //Receive Messages
        J1939_Poll(10);
        while (RXQueueCount > 0) {

            J1939_DequeueMessage( &Msg );
            if ( Msg.GroupExtension >= 0xA0 && Msg.GroupExtension <= 0xA7 )
                LATCbits.LATC4 = 1;
            // Currently, only broadcast messages are repeated
//            if( Msg.PDUFormat == PDU_BROADCAST )
                putSerialData(Msg.GroupExtension, Msg.Data[0], Msg.Data[1]);
            
            if ( J1939_Flags.ReceivedMessagesDropped )
                J1939_Flags.ReceivedMessagesDropped = 0;

            LATCbits.LATC4 = 0;
        }
    }
}