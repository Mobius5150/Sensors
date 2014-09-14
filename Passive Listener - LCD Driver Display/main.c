/*
* CAN Bus - Passive Listener (LCD)
*/

#include <p18cxxx.h>
#include "J1939.h"
#include "ecocar.h"
#include <usart.h>
#include <delays.h>

#pragma config OSC = IRCIO67    // Oscillator Selection Bit
#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software
#pragma config WDT = OFF        // Watchdog Timer disabled (control is placed on the SWDTEN bit)

J1939_MESSAGE Msg;

unsigned char DataList [21][3] =
    {{DATA_FC_POWER, 0x00, 0x00},
    {DATA_FC_TEMP, 0x00, 0x00},
    {DATA_FC_AMBTEMP, 0x00, 0x00},
    {DATA_FC_CURR, 0x00, 0x00},
    {DATA_FC_VSTACK, 0x00, 0x00},
    {DATA_FC_VBATT, 0x00, 0x00},
    {DATA_FC_PTANK, 0x00, 0x00},
    {DATA_FC_STATUS, 0x00, 0x00},
    {DATA_PWR_VBUCK, 0x00, 0x00},
    {DATA_PWR_MOTORON, 0x00, 0x00},
    {DATA_PWR_CURRMOT1, 0x00, 0x00},
    {DATA_PWR_CURRMOT2, 0x00, 0x00},
    {DATA_PWR_SPEED, 0x00, 0x00},
    {DATA_PWR_DIR, 0x00, 0x00},
    {DATA_PWR_CRUISEON, 0x00, 0x00},
    {DATA_TEMPTRUNK, 0x00, 0x00},
    {DATA_TEMPCABIN, 0x00, 0x00},
    {DATA_TEMPOUTSIDE, 0x00, 0x00},
    {DATA_VACCESSORY, 0x00, 0x00},
    {DATA_BKPALARM, 0x00, 0x00},
    {ERR_TIMEOUT, 0x00, 0x00}};

unsigned char TimeoutLog = 0x00;

int NEW_CYCLE = 0;   // Flag for receiving data from begnning of cycle

void main( void )
{
    int i;
    unsigned int counter;
    
    InitEcoCar();

    TRISCbits.TRISC1 = 1;   // Listen on this pin for LCD

    RCSTAbits.SPEN = 1;
    TRISCbits.RC4 = 0;

    J1939_Initialization( TRUE );

    // Open USART:
    OpenUSART( USART_TX_INT_OFF &
    USART_RX_INT_OFF &
    USART_ASYNCH_MODE &
    USART_EIGHT_BIT &
    USART_CONT_RX &
    USART_BRGH_HIGH, 34 );

    // 34 = 57,600 bps at 32 MHz
    BAUDCONbits.BRG16 = 0;  // Enable 16-bit SPBRG (high speed!)

    RCSTAbits.SPEN = 1; // Enable USART pin.

    // Check for address collisions:
    while (J1939_Flags.WaitingForAddressClaimContention)
            J1939_Poll(5);

    LATCbits.LATC0 = 1;     // PICIN to LCD
    TRISCbits.RC4 = 0;

    while (1)
    {
        J1939_Poll(10);
        while (RXQueueCount > 0)
        {
            LATCbits.LATC4 = 1;
            J1939_DequeueMessage( &Msg );
            putSerialData(Msg.PDUSpecific, Msg.Data[1], Msg.Data[0]);
//            for(i=0;i<sizeof(DataList)/sizeof(DataList[0]);i++)
//            {
//                if ( DataList[i][0] == Msg.PDUSpecific ) {
//                    DataList[i][1] = Msg.Data[1];
//                    DataList[i][2] = Msg.Data[0];
//                }
//
//                putSerialData(DataList[i][0], DataList[i][1], DataList[i][2]);
//            }
            LATCbits.LATC4 = 0;
        }
    }
}