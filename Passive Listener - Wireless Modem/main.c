/*
* USART wireless transmission based off node D (Passive Listener)
* Tim Ho Feb 4/13
*/

// These are the C libraries that this project requires to run:
#include <p18cxxx.h>	// Contains core functions for the PIC 18F series microcontroller.
#include "J1939.H"	// Header file for CANbus J1939 routines.
#include <delays.h>	// Contains code used to delay the microcontroller (as a pause)
#include <usart.h>	// Contains code for USART communications
#include "ecocar.h"
#pragma config OSC = IRCIO67	// This configures the PIC's internal oscillator to be used, instead of an external one.
#pragma config WDT = OFF        // Watchdog Timer disabled (control is placed on the SWDTEN bit)
#pragma config BOREN = OFF      // Brown-out Reset disabled in hardware and software

// Note: If the watchdog timer is not disabled, the microcontroller will automatically reset. For
//	long running firmware, this is not good.

J1939_MESSAGE Msg;	// An object (of type "J1939_Message", defined in J1939.h) where we store the CANbus messages.

//DataList contains data received in type, high byte, low byte
//{DATATYPE, DATAHIGH, DATALOW}
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
int NEW_CYCLE = 0;  // Flag to check if we are receving a full cycle of data

void main( void )
{
    int i;
    // TRIS registers store whether the pin is an input (1) or output (0).
    // PORT registers contains the voltage level that each pin is at (read this when reading from an input).
    // LAT registers store what the output latch should be set to (modify this for writing to an output).
    // Pins C6 and C7 are used for UART TX and RX respectively
    InitEcoCar(); 

    // Open USART:
    OpenUSART( USART_TX_INT_OFF &
    USART_RX_INT_OFF &
    USART_ASYNCH_MODE &
    USART_EIGHT_BIT &
    USART_CONT_RX &
    USART_BRGH_HIGH, 207 );     // 9600 bps baud rate

    RCSTAbits.SPEN = 1;		// Enable USART on pins C6, C7
    BAUDCONbits.BRG16 = 0;	// 8-bit BR Generator
    
    J1939_Initialization( TRUE );	// This routine initializes various registers for CANbus operation. Not too necessarily to know what goes on in here.
		
    // This code checks for any nodes claiming the same address on our network:
    // If there's address contention, we've designed our network wrong.
    while (J1939_Flags.WaitingForAddressClaimContention)
            J1939_Poll(5);
    
    // CANbus is now initialized and we can now loop while we check
    // our message receive buffer for new CANbus messages (where all received messages are put).
    LATCbits.LATC0 = 1; // Testing LED
    
    while (1)
    {
        //Receive Messages
        J1939_Poll(10);
//        putSerialData(DATA_VACCESSORY, 0, 0);
//        Delay1KTCYx(100);
        while (RXQueueCount > 0)
        {
            //Testing delete later
            J1939_DequeueMessage( &Msg );
            if(Msg.PDUFormat == PDU_BROADCAST && Msg.GroupExtension != ERR_TIMEOUT && Msg.GroupExtension != CYCLE_COMPLETE)
            {
                putSerialData(Msg.GroupExtension, Msg.Data[1], Msg.Data[0]);
            }
            continue;
            
            J1939_DequeueMessage( &Msg );
            if (Msg.PDUFormat == PDU_BROADCAST && NEW_CYCLE == 1 && Msg.GroupExtension != CYCLE_COMPLETE)
            {
                // Store data broadcasted by slaves if we have been listening from the beginning of the cycle
                // MSB = Msg.Data[0], DataList[i][1]
                // LSB = Msg.Data[1], DataList[i][2]
                for(i=0;i<(sizeof(DataList)/sizeof(DataList[0]));i++)
                {
                    if(DataList[i][0] == ERR_TIMEOUT && TimeoutLog == 0x00)
                    {
                        // Clear the timeout info that is currently stored in memory.
                        DataList[i][1] = 0x00;
                        DataList[i][2] = 0x00;
                    }

                    if (Msg.GroupExtension == DataList[i][0])
                    {
                        // Handle timeout logging:
                        if(Msg.GroupExtension == ERR_TIMEOUT)
                        {
                            TimeoutLog = Msg.Data[0];   // Set timeout log to current failing node.
                        }
                        else if(Msg.GroupExtension == TimeoutLog)
                        {
                            TimeoutLog = 0x00;  // Obviously, the node is responding now.
                        }
                        
                        DataList[i][1] = Msg.Data[0];
                        DataList[i][2] = Msg.Data[1];
                    }
                }
            }
            
            if (Msg.PDUFormat == PDU_BROADCAST && Msg.GroupExtension == CYCLE_COMPLETE && NEW_CYCLE == 1)
            {
                // Master is finished asking for data and we have been listening
                // since the beginning, send data now
                NEW_CYCLE = 0;      // Reset cycle flag

                for(i=0;i<sizeof(DataList)/sizeof(DataList[0]);i++)
                {
                    putSerialData(DataList[i][0], DataList[i][1], DataList[i][2]);
                }
            }
            else if (Msg.PDUFormat == PDU_BROADCAST && Msg.GroupExtension == CYCLE_COMPLETE && NEW_CYCLE == 0)
            {
		// This is the beginning of a new data cycle, start listening now
                LATCbits.LATC1 = 1;
                NEW_CYCLE = 1;   // Set 'beginning of cycle' flag
            }
        }
        J1939_Poll(10);
    }
    
}
