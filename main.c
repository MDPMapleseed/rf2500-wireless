#include "mrfi.h"
#include "radios/family1/mrfi_spi.h"

/* 11-21-10
This is an example of using the ADC to covert a single
analog input. The external input is to ADC10 channel A4.
This example uses a function to start conversion and
a separate function get_result that busy-waits for the result.
*/
// Roscoe's calibration for near 8Mhz
#define CALDCO_8MHZ 0x8B
#define CALBC1_8MHZ 0x8D

#define ADC_INPUT_BIT_MASK 0x10
#define ADC_INCH INCH_4

/* a simple function to read the single value from the ADC.
 * There are actually a pair of functions:
 * (1) start_conversion(), which initiates a single conversion
 * (2) get_result() which returns a number from 0 t0 1023
 *  	get_result WAITS for a value to be ready.
 */

 unsigned delay_hits=0;	/* counter to estimate the busy wait time */
 /* global variable for result (for debugging) */
volatile int latest_result;
volatile uint32_t myInt;
 
 /* declarations of functions */
 void init_adc(void);
 void start_conversion(void);
 int get_result(void);


mrfiPacket_t packetToSend;
void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;

  BCSCTL1 = CALBC1_8MHZ;			// 8Mhz calibration for clock
  DCOCTL  = CALDCO_8MHZ;

  init_adc();

  BSP_Init();
  
  // Timer initialization
//  TBCCTL0 |= CCIE;
//  TBCCR0 = 1000;
//  TBCTL |= MC_1+TBSSEL_1;
  //end
  
  P1REN |= 0x04;
  P1IE |= 0x04;  
  MRFI_Init();
  mrfiSpiWriteReg(PATABLE, 0xBB);
  mrfiSpiWriteReg(PKTCTRL0, 0x41);
  P3SEL |= 0x30;

  UCA0CTL1 = UCSSEL_2;
  UCA0BR0 = 0x41;
  UCA0BR1 = 0x3;
  UCA0MCTL = UCBRS_2;
  UCA0CTL1 &= ~UCSWRST;
  MRFI_WakeUp();
  MRFI_RxOn();
  
  __bis_SR_register(GIE);

  long long count;
  while(1) 
  {
  for(count=0; count<200000; count++){}

  start_conversion();
  latest_result=get_result();
  
  mrfiPacket_t packet;
  packet.frame[9] = latest_result>>24;
  packet.frame[10] = latest_result>>16;
  packet.frame[11] = latest_result>>8;
  packet.frame[12] = latest_result; 
  
  packet.frame[0] = 28;
  MRFI_Transmit(&packet, MRFI_TX_TYPE_FORCED);
  P1OUT ^= 0x01;

  }

}

void MRFI_RxCompleteISR()
{
  uint8_t i = 0;
  int count;
  P1OUT ^= 0x02;
  mrfiPacket_t packet;
  MRFI_Receive(&packet);
  char output[6];
  
  myInt = packet.frame[12] + (packet.frame[11]<<8) + (packet.frame[10]<<16) + (packet.frame[9]<<24); // big endian

  while(myInt!=0) 
  {
    output[3-i] = myInt % 10 +48;
    myInt /= 10;
    i++;
  }
  for(count=0; count<4-i; count++) 
  {
    output[count] = 0;
  }
  output[4] = '\r';
  output[5] = '\n';
  TXString(output, (sizeof output));
}

//#pragma vector = TIMERB0_VECTOR
//__interrupt void Timer_B (void)
//{
//}

/* basic adc operations */
void start_conversion(){
	ADC10CTL0 |= ADC10SC;
}

int get_result(){
	delay_hits=0;
	while (ADC10CTL1 & ADC10BUSY) {++delay_hits;}// busy wait for busy flag off
	return ADC10MEM;
}

void init_adc(){
 	ADC10CTL1= ADC_INCH	//input channel bit field
			  +SHS_0 //use ADC10SC bit to trigger sampling
			  +ADC10DIV_4 // ADC10 clock/5
			  +ADC10SSEL_0 // Clock Source=ADC10OSC
			  +CONSEQ_0; // single channel, single conversion
			  ;
	ADC10AE0=ADC_INPUT_BIT_MASK; // enable A4 analog input
	ADC10CTL0= SREF_0	//reference voltages are Vss and Vcc
	          +ADC10SHT_3 //64 ADC10 Clocks for sample and hold time (slowest)
	          +ADC10ON	//turn on ADC10
	          +ENC		//enable (but not yet start) conversions
	          ;
}
