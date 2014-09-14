/*
* Eco-Car - CAN Bus Master Node
* Based on CAN Bus Node A Demonstration
*/

#include <p18cxxx.h>
#include "J1939.h"
#include "ecocar.h"
#include <delays.h>
#pragma config OSC = IRCIO67    // Oscillator Selection Bit
#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software
#pragma config WDT = OFF        // Watchdog Timer disabled (control is placed on the SWDTEN bit)

J1939_MESSAGE Msg;


J1939_MESSAGE Msg;
void main( void ) {
    

    // Pins C6 and C7 are used for UART TX and RX respectively
    InitEcoCar();
    J1939_Initialization( TRUE );

    TRISC = 0;

    while (J1939_Flags.WaitingForAddressClaimContention)
        J1939_Poll(5);

    // CANbus is now initialized and we can now loop while we check
    // our message receive buffer for new CANbus messages (where all received messages are put).
    while (1) {
        //Receive Messages
        J1939_Poll(10);
        while (RXQueueCount > 0) {
            J1939_DequeueMessage( &Msg );
            LATCbits.LATC5 = 1;

            // Currently, only broadcast messages are repeated
//            if( Msg.PDUFormat == PDU_BROADCAST )
//                putSerialData(Msg.GroupExtension, Msg.Data[0], Msg.Data[1]);

            if ( J1939_Flags.ReceivedMessagesDropped )
                J1939_Flags.ReceivedMessagesDropped = 0;
        }
    }
}