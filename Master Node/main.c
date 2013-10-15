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

// Define node-data pairs so we can request the data
// from the correct slave node:
int nodeList[19][2] = {{NODE_SLAVE_FC, DATA_FC_POWER},
                    {NODE_SLAVE_FC, DATA_FC_TEMP},
                    {NODE_SLAVE_FC, DATA_FC_CURR},
                    {NODE_SLAVE_FC, DATA_FC_VSTACK},
                    {NODE_SLAVE_FC, DATA_FC_VBATT},
                    {NODE_SLAVE_FC, DATA_FC_PTANK},
                    {NODE_SLAVE_FC, DATA_FC_STATUS},
                    {NODE_SLAVE_FC, DATA_TEMPTRUNK},
                    {NODE_SLAVE_MOTORS, DATA_PWR_VBUCK},
                    {NODE_SLAVE_MOTORS, DATA_PWR_MOTORON},
                    {NODE_SLAVE_MOTORS, DATA_PWR_CURRMOT1},
                    {NODE_SLAVE_MOTORS, DATA_PWR_CURRMOT2},
                    {NODE_SLAVE_MOTORS, DATA_PWR_SPEED},
                    {NODE_SLAVE_MOTORS, DATA_PWR_DIR},
                    {NODE_SLAVE_MOTORS, DATA_PWR_CRUISEON},
                    {NODE_SLAVE_MISC, DATA_TEMPOUTSIDE},
                    {NODE_SLAVE_MISC, DATA_TEMPCABIN},
                    {NODE_SLAVE_MISC, DATA_BKPALARM},
                    {NODE_SLAVE_MISC, DATA_VACCESSORY}};

int SLAVE_FC_READY = 0;
int SLAVE_MOTORS_READY = 0;
int SLAVE_MISC_READY = 0;

void main( void )
{
    // Define our list indicies and the total number of data types:
    int typeIndex = 0;
    int typeMax = sizeof(nodeList)/sizeof(nodeList[0]) - 1;
    long int cyclesElapsed = 0;
    int receiveFlag = 0;
    int i;
    
    InitEcoCar();

    J1939_Initialization( TRUE );

    // Check for address collisions:
    while (J1939_Flags.WaitingForAddressClaimContention)
    {
        J1939_Poll(5);
    }
    for(i=0;i<10;i++)
    {
        Delay10KTCYx(255);
    }
    LATCbits.LATC0 = 1;

    
    while (1)
    {
        // Request the data type from the slave node:
        Request_Data(&Msg, nodeList[typeIndex][0], nodeList[typeIndex][1]);

        // Wait for a response from the slave node:
        cyclesElapsed = 0;
        LATCbits.LATC1 = 0;
        LATCbits.LATC2 = 0;
        
        while (cyclesElapsed <= 500)
        {
            J1939_Poll(10);
            while (RXQueueCount > 0)
            {
                LATCbits.LATC1 = 1;
                J1939_DequeueMessage( &Msg );
                if(Msg.PDUFormat == PDU_BROADCAST && Msg.SourceAddress == nodeList[typeIndex][0])
                {
                    receiveFlag = 1;	// Yep, received a reply back from this node.
                }
            }

            if(receiveFlag)	
            {
                // Get out of the loop if we got a reply back.
                receiveFlag = 0;
                break;
            }	
            
            cyclesElapsed++;
        }

        if(cyclesElapsed > 500)
        {
            // Generate a timeout message.
            char MsgData[8];
            MsgData[0] = nodeList[typeIndex][0];
            MsgData[1] = nodeList[typeIndex][1];
            Broadcast_Data(&Msg, ERR_TIMEOUT, MsgData);	// Broadcast that a timeout error occured.
            LATCbits.LATC2 = 1;

        }

        // Start requesting each piece of data from nodeList:
        if(typeIndex == typeMax)
        {
            char MsgData[];
            typeIndex = 0;	// Reset the index counter since we're doing a new cycle.
            Broadcast_Data(&Msg, CYCLE_COMPLETE, MsgData); // Broadcast to everyone that the cycle has ended.
            J1939_Poll(5);
            
            SLAVE_FC_READY = 0;
            SLAVE_MOTORS_READY = 0;
            SLAVE_MISC_READY = 0;

            cyclesElapsed = 0;

            // Wait for the slaves to respond to the cycle_complete:
            while(!SLAVE_FC_READY && !SLAVE_MOTORS_READY && !SLAVE_MISC_READY)
            {
                while (RXQueueCount > 0)
                {
                    J1939_DequeueMessage( &Msg );
                    if(Msg.PDUFormat == PDU_BROADCAST && Msg.GroupExtension == ACK_DONE)
                    {
                        // Which slave is acknowledging?
                        if(Msg.SourceAddress == NODE_SLAVE_FC){ 
                            SLAVE_FC_READY = 1;
                        }
                        else if(Msg.SourceAddress == NODE_SLAVE_MOTORS){
                            SLAVE_MOTORS_READY = 1;
                        }
                        else if(Msg.SourceAddress == NODE_SLAVE_MISC){
                            SLAVE_MISC_READY = 1;
                        };
                    }
                }
                J1939_Poll(5);

                if(0)//cyclesElapsed > 50000)
                {
                    char MsgData[8];
                    cyclesElapsed = 0;
                    
                    // Generate a timeout message.
                    
                    MsgData[0] = NODE_MASTER;
                    MsgData[1] = SLAVE_FC_READY || SLAVE_FC_READY * 2 || SLAVE_FC_READY * 4;

                    Broadcast_Data(&Msg, ERR_TIMEOUT, MsgData);	// Broadcast that a timeout error occured.
                    break;

                }

                //cyclesElapsed++;
            }
        }
        else
        {
            typeIndex++;	// Move to the next type of data.
        }
        J1939_Poll(5);
    }
}
