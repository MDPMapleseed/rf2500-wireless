//Address 79
#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"

//Clock Configuration
#define CALDCO_8MHZ 0x8B
#define CALBC1_8MHZ 0x8D

//ADC Configuration
#define ADC_INPUT_BIT_MASK 0x10
#define ADC_INCH INCH_4

//Address for Communication
static linkID_t sLinkID1;

//Wireless Linkto function
static void linkTo(void);

//ADC functions
void ADC_Init(void);
void Start_Conversion(void);
int Get_Result(void);

//Converting an integer to a string
void itos(char string[], int number);

void main (void)
{
	// 8Mhz calibration for clock
	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL  = CALDCO_8MHZ;

	ADC_Init();
	BSP_Init();
	SMPL_Init(NULL);

	//ADC PIN Configuration
	P1REN |= 0x04;
	P1IE |= 0x04;

	NWK_DELAY(100);
	//__bis_SR_register(GIE);
	linkTo();

}

static void linkTo()
{
	uint8_t  msg[8]; //ID+sign+1023+\n\r
	int Result;

	while (SMPL_SUCCESS != SMPL_Link(&sLinkID1))
	{
		/* blink red LED, until we link successfully */
		BSP_TOGGLE_LED2();
		NWK_DELAY(100);
	}

	BSP_TURN_OFF_LED2();

	/* turn on RX. default is RX off. */
	SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);

	/* Message to Send */
	msg[0] = 't';
	while (1)
	{
		NWK_DELAY(100);
		Start_Conversion();
		Result = Get_Result();
		itos(msg+1,Result);
		SMPL_Send(sLinkID1, msg, sizeof(msg));
		//blink green for successfully sending message
		BSP_TOGGLE_LED1();
	}
}

//basic ADC operations
void Start_Conversion()
{
	ADC10CTL0 |= ADC10SC;
}

void ADC_Init()
{
	ADC10CTL1= ADC_INCH	//input channel bit field
			+SHS_0 //use ADC10SC bit to trigger sampling
			+ADC10DIV_4 // ADC10 clock/5
			+ADC10SSEL_0 // Clock Source=ADC10OSC
			+CONSEQ_0; // single channel, single conversion

	ADC10AE0=ADC_INPUT_BIT_MASK; // enable A4 analog input
	ADC10CTL0= SREF_0	//reference voltages are Vss and Vcc
			+ADC10SHT_3 //64 ADC10 Clocks for sample and hold time (slowest)
			+ADC10ON	//turn on ADC10
			+ENC;	 //enable (but not yet start) conversions

}

int Get_Result()
{
	int delay_hits=0;
	while (ADC10CTL1 & ADC10BUSY) {++delay_hits;}// busy wait for busy flag off
	return ADC10MEM;
}

void itos(char string[], int number)
{
	uint8_t BIT_STRING = 7; //sign+1023+\n\r
	int i;
	int num = number;
	int flag = 1;
	if (number < 0)
	{
		flag = 0;
		num = -number;
	}
	for (i = 0; i<BIT_STRING; i++) string[i] = 0;
	string[BIT_STRING-1] = '\n';
	string[BIT_STRING-2] = '\r';
	i = BIT_STRING-3;
	while(num != 0)
	{
		string[i] = num % 10 + 48;
		num = num / 10;
		i--;
	}
	if (flag == 0) string[i] = '-';

}
