//Address 78
//MAX_APP_PAYLOAD=40
//MAX_NWK_PAYLOAD=40

#include <string.h>
#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "virtual_com_cmds.h"

static linkID_t sLinkID2;
static uint8_t	sSemaphore;

// Larger than Maximum Value = 40
static uint8_t msg[50], len;

static void Link_From(void);
static void Clean_MSG(void);

/* Rx callback handler */
static uint8_t sRxCallback(linkID_t);

int main (void)
{
	BSP_Init();
	SMPL_Init(sRxCallback);
	COM_Init();
	NWK_DELAY(500);
	Link_From();
}

static void Link_From()
{
	while (SMPL_SUCCESS != SMPL_LinkListen(&sLinkID2))
	{
		// Blink red LED, until we link successfully
		BSP_TOGGLE_LED2();
		NWK_DELAY(50);
		TXString("Configuring the Wireless!\n\r",sizeof("Configuring the Wireless!\n\r"));
	}
	BSP_TURN_OFF_LED2();

	// Turn on RX. Default is RX off
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

	TXString("Configuration Done!\n\r", sizeof("Configuration Done!\n\r"));
	BSP_TURN_ON_LED1();

	while (1)
	{
		/* Wait for a frame to be received. The Rx handler, which is running in
		 * ISR thread, will post to this semaphore allowing the application to
		 * send the reply message in the user thread.
		 */
		if (sSemaphore)
		{
		   sSemaphore = 0;
		   // Do something here after you get the msg
		}
	}
}

// Handle received messages
static uint8_t sRxCallback(linkID_t port)
{
	// Check if the callback for the link ID we want to handle
	if (port == sLinkID2)
	{
		Clean_MSG();
		// Get the frame. we know this call will succeed.
		if ((SMPL_SUCCESS == SMPL_Receive(sLinkID2, msg, &len)) && len)
		{
			sSemaphore = 1;
			TXString((char *)msg, len);
			// Blink green led for successfully getting message
			BSP_TOGGLE_LED1();
			return 1;
		}
	}
	return 0;
}

static void Clean_MSG(void)
{
	int i = 0;
	for (i = 0; i < 50; i++)
	{
		msg[i] = 0;
	}
}
