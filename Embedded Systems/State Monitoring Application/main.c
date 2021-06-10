#include <msp.h>
#include <stdint.h>

static uint8_t counter, RED = 0x01, GREEN = 0x02, BLUE = 0x04, OFF = 0x00, CYAN = 0x06, state;

void Port1_IRQ_Handler();
uint8_t Update_State();
static void Port_configure();
static void uart_configure();

static void Port_configure()
{
	P1SEL0 &=~ (uint8_t)0x12; 									//Initialize GPIO pins
	P1SEL1 &=~ (uint8_t)0x12; 									//Init GPIO pins
	P1DIR &=~ (uint8_t)0x12; 										//Set P1.1 and P1.4 as intput
	P2DIR |= (uint8_t)0x07; 										//Set P2.0, P2.1, P2.2 as output
	P1OUT |= (uint8_t)0x12; 
	P1REN |= (uint8_t)0x12; 										//Enable pull-up resistors on P1.1, P1.4
	P2OUT &=~ (uint8_t)0x07;
  P2->DS &= ~((1<<0) | (1<<1) | (1<<2)); 			//Ensure that high drive strength is disabled
}

static void uart_configure()
{
	P1SEL0 |= 0x0C;
	
	EUSCI_A0->CTLW0 |= EUSCI_A_CTLW0_SWRST; 		// Put eUSCI in reset
  EUSCI_A0->CTLW0 = EUSCI_A_CTLW0_SWRST | 		// Remain eUSCI in reset
										EUSCI_B_CTLW0_SSEL__SMCLK;// Configure eUSCI clock source for SMCLK
	EUSCI_A0->BRW = 78;                     		// 12000000/16/9600
  EUSCI_A0->MCTLW = (2 << EUSCI_A_MCTLW_BRF_OFS) |
													EUSCI_A_MCTLW_OS16;

  EUSCI_A0->CTLW0 &= ~EUSCI_A_CTLW0_SWRST; 		// Initialize eUSCI
  EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG;    		// Clear eUSCI RX interrupt flag
  EUSCI_A0->IE |= EUSCI_A_IE_RXIE;        		// Enable USCI_A0 RX interrupt
}	

void PORT1_IRQHandler(void)
{
	if ((P1IFG & 0x02) != 0) 
	{
		P1IFG &= ~0x02;														//clear flag1, acknowledge
		counter++;
		if(counter > 4 || counter < 0)
		{
			counter = 0;
		}
		state = Update_State();
		EUSCI_A0->TXBUF = counter;
		P2OUT &=~ (uint8_t)0x07;
		P2OUT |= (uint8_t)state;
  }
	else if ((P1IFG & 0x10) != 0)
	{
		P1IFG &= ~0x10;														//clear flag4, acknowledge
		counter--;
		if(counter > 4 || counter < 0)
		{
			counter = 4;
		}
		state = Update_State();
		EUSCI_A0->TXBUF = counter;
		P2OUT &=~ (uint8_t)0x07; 									//Clear previous values
		P2OUT |= (uint8_t)state; 									//Set state
	}
}

uint8_t Update_State()
{
	if (counter == 0)
	{
		state = OFF;
	}
	else if (counter == 1)
	{
		state = RED;
	}
	else if (counter == 2)
	{
		state = GREEN;
	}
	else if (counter == 3)
	{
		state = BLUE;
	}
	else if (counter == 4)
	{
		state = CYAN;
	}
	return state;
}
// UART interrupt service routine
void EUSCIA0_IRQHandler(void)
{
	if (EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG)
	{
		// Check if the TX buffer is empty first
		while(!(EUSCI_A0->IFG & EUSCI_A_IFG_TXIFG));
		/***switch(EUSCI_A0->TXBUF = EUSCI_A0->RXBUF)
		{
			case 'i':
				P1IFG |= (uint8_t)0x02;
			case 'd':
				P1IFG |= (uint8_t)0x10;
			case 'o':
				counter = 3;
				P1IFG |= (uint8_t)0x02; ...and so on but gave many unwanted bugs
		}***/
		if(EUSCI_A0->RXBUF == 0x69)
		{
			P1IFG |= (uint8_t)0x02;
		}
		if(EUSCI_A0->RXBUF == 0x64)
		{
			P1IFG |= (uint8_t)0x10;
		}
		if(EUSCI_A0->RXBUF == 0x6F)
		{
			counter = 4;
			P1IFG |= (uint8_t)0x02;
		}
		if(EUSCI_A0->RXBUF == 0x71)
		{
			counter = 4;
			P1IFG |= (uint8_t)0x02;
		}
	}
}

int main()
{
	char c;
	__ASM("CPSID I");
	
	WDTCTL = WDTPW | WDTHOLD; 									// Stop watchdog timer
	
	CS->KEY = CS_KEY_VAL;                   		// Unlock CS module for register access
	CS->CTL0 = 0;                           		// Reset tuning parameters
	CS->CTL0 = CS_CTL0_DCORSEL_3;           		// Set DCO to 12MHz (nominal, center of 8-16MHz range)
	CS->CTL1 = CS_CTL1_SELA_2 |             		// Select ACLK = REFO
								CS_CTL1_SELS_3 |              // SMCLK = DCO
								CS_CTL1_SELM_3;               // MCLK = DCO
	CS->KEY = 0;                            		// Lock CS module from unintended accesses
	
	Port_configure();
	uart_configure();
	
	counter = 0;
	state = OFF;
	
	P1IES |= (uint8_t)0x12;                   	// P1.4 and P1.1 are falling edge event
	P1IFG &=~ (uint8_t)0x12;                  	// clear flag4 and flag1 (reduce possibility of extra interrupt)
	P1IE |= (uint8_t)0x12;                    	// arm interrupt on P1.4 and P1.1
	
	NVIC_SetPriority(PORT1_IRQn, 2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);
	
	__ASM("CPSIE I");
	
  SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk;				// Enable sleep on exit from ISR

  __enable_irq();															// Enable global interrupt

  NVIC->ISER[0] = 1 << ((EUSCIA0_IRQn) & 31);	// Enable eUSCIA0 interrupt in NVIC module

  __sleep();																	// Enter LPM0
  __no_operation();                       		// For debugger
	
	for(;;)
	{
		__WFI();
	}
}
