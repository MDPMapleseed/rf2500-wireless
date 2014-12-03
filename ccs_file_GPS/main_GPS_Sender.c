//Address 79
//MAX_APP_PAYLOAD=40
//MAX_NWK_PAYLOAD=40 (Theoretical Maximum should be 50???)
//Exclude "virtual_com_cmds.c" for compile
//Minimum Voltage for the GPS should be 3.3V, otherwise stop working

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
	for (i = 0; i < 2; i++)
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
	unsigned char Fg_GPRMC = 1;
	char GPGGA[] = "GPGGA";
	char GPRMC[] = "GPRMC";
	unsigned char i = 0;

	for(i = 0; i < 5; i++)
	{
		if(!(GPGGA[i] == MSG[i]))
		{
			Fg_GPGGA = 0;
		}
		if(!(GPRMC[i] == MSG[i]))
		{
			Fg_GPRMC = 0;
		}
	}

	if(Fg_GPGGA)
	{

		NWK_DELAY(100);

		char output[] = "Time:\n\rhh:hh\tmm:mm\tss:ss\n\r";
		output[10] = MSG[6];
		output[11] = MSG[7];
		output[16] = MSG[8];
		output[17] = MSG[9];
		output[22] = MSG[10];
		output[23] = MSG[11];
		SMPL_Send(sLinkID1, output, sizeof(output));

		NWK_DELAY(100);

		char output2[] = "La:aadbb.ccccc'd\n\r";
		// Latitude
		output2[3] = MSG[16];
		output2[4] = MSG[17];
		output2[6] = MSG[18];
		output2[7] = MSG[19];
		output2[9] = MSG[21];
		output2[10] = MSG[22];
		output2[11] = MSG[23];
		output2[12] = MSG[24];
		output2[13] = MSG[25];
		output2[15] = MSG[27];
		SMPL_Send(sLinkID1, output2, sizeof(output2));

		NWK_DELAY(100);

		char output3[] = "Long:aaadbb.ccccc'd\n\r";
		// Longitude
		output3[5] = MSG[29];
		output3[6] = MSG[30];
		output3[7] = MSG[31];
		output3[9] = MSG[32];
		output3[10] = MSG[33];
		output3[12] = MSG[35];
		output3[13] = MSG[36];
		output3[14] = MSG[37];
		output3[15] = MSG[38];
		output3[16] = MSG[39];
		output3[18] = MSG[41];
		SMPL_Send(sLinkID1, output3, sizeof(output3));

		NWK_DELAY(100);

		char output4[] = "Altitude:aaaaM\n\r";
		// Altitude
		output4[9] = MSG[54];
		output4[10] = MSG[55];
		output4[11] = MSG[56];
		output4[12] = MSG[57];
		SMPL_Send(sLinkID1, output4, sizeof(output4));

		NWK_DELAY(100);

		char output5[] = "S_Used:aa\n\r";
		// # of satellite used
		output5[7] = MSG[45];
		output5[8] = MSG[46];
		SMPL_Send(sLinkID1, output5, sizeof(output5));

		NWK_DELAY(100);

		// Blink green led after sending msg
		BSP_TOGGLE_LED1();
	}

/*	if(Fg_GPRMC)
	{

		NWK_DELAY(100);

		char output6[] = "Speed:aaabbb\n\r";
		output6[6] = MSG[45];
		output6[7] = MSG[46];
		output6[8] = MSG[47];
		output6[9] = MSG[48];
		output6[10] = MSG[49];
		output6[11] = MSG[50];
		SMPL_Send(sLinkID1, output6, sizeof(output6));

		NWK_DELAY(100);

		// Blink green led after sending msg
		BSP_TOGGLE_LED1();
	}
	*/
}

static void Clean_MSG(void)
{
	int i = 0;
	for (i = 0; i < 200; i++)
	{
		MSG[i] = 0;
	}
}
