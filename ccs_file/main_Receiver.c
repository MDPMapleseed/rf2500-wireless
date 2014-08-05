// Software is Code Composer Studio 5.5.0
// Address 78, could be changed 

#include <string.h>
#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "virtual_com_cmds.h"

static      linkID_t sLinkID2;
static 		uint8_t	sSemaphore;
uint8_t 	msg[8], len;
static void linkFrom(void);

/* Rx callback handler */
static uint8_t sRxCallback(linkID_t);

int main (void)
{
	BSP_Init();
	SMPL_Init(sRxCallback);
	COM_Init();
	NWK_DELAY(500);
	linkFrom();

}

static void linkFrom()
{
	while (SMPL_SUCCESS != SMPL_LinkListen(&sLinkID2))
	{
		/* blink red LED, until we link successfully */
		BSP_TOGGLE_LED2();
		NWK_DELAY(100);
		TXString("Configuring the Wireless!\n\r",sizeof("Configuring the Wireless!\n\r"));
	}

	BSP_TURN_OFF_LED2();

	/* turn on RX. default is RX off. */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

	TXString("Configuration Done!\n\r",sizeof("Configuration Done!\n\r"));
	while (1)
	{
		/* Wait for a frame to be received. The Rx handler, which is running in
		 * ISR thread, will post to this semaphore allowing the application to
		 * send the reply message in the user thread.
		 */
		if (sSemaphore)
		{
		   sSemaphore = 0;
		   //TXString("This is data with ID='t' \n\r",sizeof("This is data with ID='t' \n\r"));
		}
	}
}



/* handle received messages */
static uint8_t sRxCallback(linkID_t port)
{

	/* is the callback for the link ID we want to handle? */
	if (port == sLinkID2)
	{
		/* yes. go get the frame. we know this call will succeed. */
		if ((SMPL_SUCCESS == SMPL_Receive(sLinkID2, msg, &len)) && len)
		{
			/* Check the application sequence number to detect
			 * late or missing frames...
			 */
			sSemaphore = 1;
			TXString((char *)msg,8);
			//Blink red led for successfully getting message
			BSP_TOGGLE_LED1();
			/* drop frame. we're done with it. */
			return 1;
		}
	}
	/* keep frame for later handling */
	return 0;
}
