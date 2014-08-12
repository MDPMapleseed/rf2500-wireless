//Address 79
//MAX_APP_PAYLOAD=40
//MAX_NWK_PAYLOAD=40 (Theoretical Maximum should be 50???)

#include <msp430f2274.h>
#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"

//Address for Communication
static linkID_t sLinkID1;

//Wireless Link Initialization function
static void Link_Init(void);

//UART Initialization function
//Minimum Voltage for the GPS should be 3.3V, otherwise stop working
static void UART_Init(void);

//GPS Data processing
static char Index = 0;
//Editing this function to parse data, right now this one is just for getting current time infomation
static void ProcessingData(void);
static void Clean_MSG(void);
//Storing GPS Data, could be larger than 40. We need to pick out the useful infomation
static char MSG[200];

void main (void)
{
	BSP_Init();
	SMPL_Init(NULL);
	UART_Init();
	NWK_DELAY(100);
	Link_Init();
}

static void Link_Init()
{
	while (SMPL_SUCCESS != SMPL_Link(&sLinkID1))
	{
		// Blink red LED, until we link successfully
		BSP_TOGGLE_LED2();
		NWK_DELAY(100);
	}
	BSP_TURN_OFF_LED2();

	// Turn on RX. default is RX off.
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

	// Maximum Length = 40 bits
	unsigned char str[] = "Testing Maximum package = 39 ........\n\r";
	int i = 0;
	BSP_TURN_ON_LED1();
	for (i = 0; i < 5; i++)
	{
		NWK_DELAY(100);
		BSP_TOGGLE_LED1();
		SMPL_Send(sLinkID1, str, sizeof(str));
	}
	BSP_TURN_OFF_LED1();
}

static void UART_Init(void)
{
	  P3SEL |= 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
	  UCA0CTL1 = UCSSEL_2;                      // SMCLK

	#if (BSP_CONFIG_CLOCK_MHZ_SELECT == 1)
	  UCA0BR0 = 104;                            // 9600 from 1Mhz
	  UCA0BR1 = 0;
	  UCA0MCTL = UCBRS_1;
	#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 2)
	  UCA0BR0 = 0xDA;                           // 9600 from 2Mhz
	  UCA0BR1 = 0x0;
	  UCA0MCTL = UCBRS_6;
	#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 4)
	  UCA0BR0 = 0xA0;                           // 9600 from 4Mhz
	  UCA0BR1 = 0x1;
	  UCA0MCTL = UCBRS_6;
	#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 6)
	  UCA0BR0 = 0x7B;                           // 9600 from 6Mhz
	  UCA0BR1 = 0x2;
	  UCA0MCTL = UCBRS_3;
	#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 8)
	  UCA0BR0 = 0x41;                           // 9600 from 8Mhz
	  UCA0BR1 = 0x3;
	  UCA0MCTL = UCBRS_2;
	#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 10)
	  UCA0BR0 = 0x79;                           // 9600 from 10Mhz
	  UCA0BR1 = 0x4;
	  UCA0MCTL = UCBRS_7;
	#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 12)
	  UCA0BR0 = 0xE2;                           // 9600 from 12Mhz
	  UCA0BR1 = 0x4;
	  UCA0MCTL = 0;
	#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 16)
	  UCA0BR0 = 0x82;                           // 9600 from 16Mhz
	  UCA0BR1 = 0x6;
	  UCA0MCTL = UCBRS_6;
	#else
	#error "ERROR: Unsupported clock speed.  Custom clock speeds are possible. See comments in code."
	#endif

	  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
	  __bis_SR_register(GIE);
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	// Blink red LED after RX_UART
	BSP_TOGGLE_LED2();
	char buffer = UCA0RXBUF;
	if (buffer == '$')
	{
		ProcessingData();
		Clean_MSG();
		Index = 0;
	}
	else
	{
		MSG[Index] = buffer;
		Index++;
	}

}

void ProcessingData(void)
{
	unsigned char Fg_GPGGA = 1;
	char GPGGA[] = "GPGGA";
	unsigned char i = 0;

	for(i = 0; i < 5; i++)
	{
		if(!(GPGGA[i] == MSG[i]))
		{
			Fg_GPGGA = 0;
		}
	}

	if(Fg_GPGGA)
	{
		char output[] = "Current Time:\n\rhh:hh\tmm:mm\tss:ss\n\r";
		output[18] = MSG[6];
		output[19] = MSG[7];
		output[24] = MSG[8];
		output[25] = MSG[9];
		output[30] = MSG[10];
		output[31] = MSG[11];
		SMPL_Send(sLinkID1, output, sizeof(output));
		// Blink green led after sending msg
		BSP_TOGGLE_LED1();
	}
}

static void Clean_MSG(void)
{
	int i = 0;
	for (i = 0; i < 200; i++)
	{
		MSG[i] = 0;
	}
}
