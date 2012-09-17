//***************************************************************************************
//  MSP430 SPI daisy chain demo - SLAVE
//
//
//                MSP430G2553
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          | |             XOUT|-
// Master---+-|RST              |
//            |             P1.1|<- Data In (UCA0SOMI)
//            |                 |
//            |             P1.2|-> Data Out (UCA0SIMO)
//            |                 |
//            |             P1.4|<- Serial Clock In (UCA0CLK)
//            |             P1.5|<- Transmit Enable (UCA0STE)
//
//***************************************************************************************
#define SIMO_BIT			BIT2
#define MISO_BIT			BIT1
#define SCLK_BIT			BIT4
#define STE_BIT 			BIT5


#include <msp430.h>

void msp_init();
void Initialize_SPI(void);
void Delay(volatile unsigned long a);

char rx_data;
char tx_data;


int main(void) {
	WDTCTL = WDTPW + WDTHOLD;		// Stop watchdog timer
	
	// Init Msp
	msp_init();

	// Port init
	P1DIR |= BIT0 + BIT6;			// Set P1.0 and P1.6 to output direction - LED
	P1DIR &= ~(BIT3);				// Set P1.3 to input direction - S2
	P1SEL &= ~(BIT3);
	P1SEL2 &= ~(BIT3);
	P1REN |= (BIT3);
	P1OUT |= (BIT3);

	// Init SPI
	Initialize_SPI();

	_BIS_SR(GIE);


	for(;;) {
	}
}


void msp_init() {

  P1SEL = 0x00;
  P1SEL2 = 0x00;
  P1DIR = 0x00;
  P1OUT = 0x00;

  P2SEL = 0x00;
  P2SEL2 = 0x00;
  P2DIR = 0x00;
  P2OUT = 0x00;
}

void Initialize_SPI(void)
{
	// disable the module if enabled previously to make register configurations
	UCA0CTL1 = UCSWRST;

	// setup pin directions
	P1SEL  |= SIMO_BIT + MISO_BIT + SCLK_BIT;
	P1SEL2  |= SIMO_BIT + MISO_BIT + SCLK_BIT;
	P1DIR &= ~STE_BIT;
	P1IE |= STE_BIT;
	P1IES &= ~STE_BIT;

	UCA0CTL0 |= UCCKPH + UCMSB + UCSYNC; // MSB + Slave + 3-wire

	// enable the module
	UCA0CTL1 &= ~UCSWRST;

	// Interrupt enable
	IE2 |= UCA0RXIE;
}

void Delay(volatile unsigned long a) { while (a!=0) a--; }



#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIA0RX_ISR (void)
{
	while(!(IFG2&UCB0TXIFG)); 		    // Make sure last character went out.
	UCA0TXBUF = UCA0RXBUF;			    // Echo RXed character
}


#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR (void)
{
	P1IFG = 0x00; 					    // Clear interrupt flag
	rx_data = UCA0RXBUF;

	if ((rx_data & 0x01) == 0x01)
		P1OUT ^= 0x01;				    // Toggle LED1
	if ((rx_data & 0x02) == 0x02)
		P1OUT ^= 0x40;				    // Toggle LED2

	if ((P1IN & 0x08) == 0x00)
		UCA0TXBUF = 0x01;               // Send S2 status
	else
		UCA0TXBUF = 0x00;
}
