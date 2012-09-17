//***************************************************************************************
//   SPI daisy chain - SLAVE
//
//                    MSP430G2xx3
//                 -----------------
//             /|\|              XIN|-
//              | |                 |
//              --|RST          XOUT|-
//                |                 |
//                |             P1.7|-> Data Out (UCB0SIMO)
//                |                 |
//          LED <-|P1.0         P1.6|<- Data In (UCB0SOMI)
//                |                 |
//  Slave Select<-|P1.4         P1.5|-> Serial Clock Out (UCB0CLK)
//
//	Matthias Bichuniak
//***************************************************************************************

#define SIMO_BIT			BIT2
#define MISO_BIT			BIT1
#define SCLK_BIT			BIT4
#define STE_BIT 			BIT5

#define Number_Slaves		2


#include <msp430.h>


// Prototypes
void msp_init();
void Delay(volatile unsigned long a);
void SPI_RX_TX();
void Initialize_SPI(void);

char i_data = 0;
char i;
char rx_data[2];
char tx_data[4][2] = {
		{0x02, 0x00},
		{0x01, 0x00},
		{0x00, 0x02},
		{0x00, 0x01}
};


int main(void) {
	WDTCTL = WDTPW + WDTHOLD;		// Stop watchdog timer

	// Init msp
	msp_init();

	Delay(100);						// Delay for slaves

	// Initialize ports
	P1DIR = BIT0+BIT6;				// Output: LED1 and LED2

	Delay(100);						// Delay for slaves

	Initialize_SPI();



	for(;;)
	{
		if (i_data == 3)
			i_data = 0;
		else i_data++;

		SPI_RX_TX();               // send and receive data

		if (rx_data[0] == 0x01)    // slave
			P1OUT |= BIT0;
		else
			P1OUT &= ~BIT0;
		if (rx_data[1] == 0x01)
			P1OUT |= BIT6;
		else
			P1OUT &= ~BIT6;

		Delay(15000);
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

void Delay(volatile unsigned long a) { while (a!=0) a--; }

void Initialize_SPI(void)
{
	// disable the module if enabled previously to make register configurations
	UCA0CTL1 = UCSWRST;

	// setup pin directions
	P1SEL  |= SIMO_BIT + MISO_BIT + SCLK_BIT;
	P1SEL2  |= SIMO_BIT + MISO_BIT + SCLK_BIT;
	P1DIR |= STE_BIT;
	P1OUT |= STE_BIT;

	UCA0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; // MSB + Master + SPI-Mode
	UCA0CTL1 |= UCSSEL_2;                        // SMCLK
	
	// enable the module
	UCA0CTL1 &= ~UCSWRST;

	// Interrupt enable
	IE2 |= UCA0RXIE;
}



void SPI_RX_TX() {

    P1OUT &= ~STE_BIT;						// Slave select
    Delay(100);								// Delay for Slaves

    for(i = 0; i < Number_Slaves; i++)
    {
    	UCA0TXBUF = tx_data[i_data][Number_Slaves-1-i];    // send data

    	while (!(IFG2 & UCA0RXIFG));
    	rx_data[Number_Slaves-1-i] = UCA0RXBUF;            // receive data
    }

	P1OUT |= STE_BIT;						// end of slave select
}

