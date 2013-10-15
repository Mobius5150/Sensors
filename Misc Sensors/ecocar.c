/*
 * File:   ecocar.c
 *
 * Helper functions for EcoCar Project
 * Includes broadcast, data request, and other common routines.
 * Last edited: Feb. 22, 2013
 */

#include "ecocar.h"
#include "J1939.H"
#include <delays.h>
#include <usart.h>

void Broadcast_Data(J1939_MESSAGE *MsgPtr, unsigned char DataType, unsigned char MsgData[])
{
	// Broadcasts data to all nodes on the CAN network. Performed by slave nodes only.
	int i;

	MsgPtr->DataPage    =   0;
	MsgPtr->Priority    =   J1939_CONTROL_PRIORITY;
	MsgPtr->DestinationAddress	= NODE_BROADCAST;	// Use the global broadcast address (there is no actual specific destination)
	MsgPtr->GroupExtension  =   DataType;	// This is the data type, as defined in ecocar.h
	MsgPtr->DataLength  =   sizeof(MsgData);		// This is arbitrary, depending on the type of data we're sending.
	MsgPtr->PDUFormat   =   PDU_BROADCAST;	// It is a broadcast-type message.

	for(i=0; i < sizeof(MsgData); i++)
	{
            MsgPtr->Data[i] = MsgData[i];
	}
	while (J1939_EnqueueMessage( MsgPtr ) != RC_SUCCESS)
        {
            J1939_Poll(5);
        }
}

void Request_Data(J1939_MESSAGE *MsgPtr, unsigned int DestAddr, unsigned int DataType)
{
	// Requests specific data from a specific slave node. Only used by the master node.

	MsgPtr->DataPage    =   0;
	MsgPtr->Priority    =   J1939_CONTROL_PRIORITY;
	MsgPtr->DestinationAddress = DestAddr;	// Destination address is the target slave node.
	MsgPtr->DataLength  =   1;			// Data is only one byte long -- contains the type of data we would like.
	MsgPtr->PDUFormat = PDU_REQUEST;	// It is a request-type message.
	MsgPtr->Data[0] = DataType;		// Set the data to be the type of data we want broadcasted.
	while (J1939_EnqueueMessage( MsgPtr ) != RC_SUCCESS)
        {
            J1939_Poll(5);
        }
}

void Set_Oscillator()
{
    // Run internal oscillator at 8 MHz
    OSCCONbits.IRCF2 = 1;
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;

    // Multiply frequency using PLL to 4* = 32 MHz
    OSCTUNEbits.PLLEN = 1;

    // Wait a bit for PLL to stabilize.
    Delay10KTCYx(10);
}

void InitEcoCar()
{
    // Returns registers of chip to original state.

    // Initialize registers:
    ADCON1 = 0x0F;                  // Disable analog inputs on all A ports.
    TRISA = 0b00000010;             // Set A0 to output, A1 to input
    TRISB = 0b00001000;             // Set B2 (CANTX) to output, B3 (CANRX) to input
    TRISC = 0b10000000;				// Set C to all output
    LATC = 0b10000000;				// Reset C latches to low

    //LATCbits.LATC6 = 1;              // TX should be held high when not transmitting (for wireless)

    Set_Oscillator();               // Set speed of oscillator to 32 MHz?
}

void putUSART(int i)
{
	while(BusyUSART()) ;
	putcUSART(i);
}

void putSerialData(char DataType, char DataMSB, char DataLSB)
{
    putUSART(0xFF);
    putUSART(DataType);
    putUSART(DataMSB);
    putUSART(DataLSB);
}
