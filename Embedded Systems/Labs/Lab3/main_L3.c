#include <msp.h>
#include <stdint.h>
#define Red 0
#define RGB 1

static uint8_t led;

static void configure(void)
{
	P1SEL0 &=~ (uint8_t)0x12;
	P1SEL1 &=~ (uint8_t)0x12;
	P1DIR &=~ (uint8_t)0x12; // Set P1.1 and P1.4 as intput
	P1DIR  |= (uint8_t)0x01; // Set P1.0 as output 
	P2DIR |= (uint8_t)0x07; // Set P2.0, P2.1, P2.2 as output
	P1OUT |= (uint8_t)0x12;
	P1REN |= (uint8_t)0x12; // Enable pull-up resistors on P1.1, P1.4
	P1OUT &=~ (uint8_t)0x01;
	P2OUT &=~ (uint8_t)0x07;
	// Ensure that high drive strength is disabled
  P1->DS &= ~(1<<0);
  P2->DS &= ~((1<<0) | (1<<1) | (1<<2));
	
	P1IE &=~ 0x12; //ensure interrupts are disabled
	// Ensure that interrupts are disabled
  P1->IE &= ~((1<<0));
  P1->IE &= ~((1<<0) | (1<<1) | (1<<2));
}

void PORT1_IRQHandler(void);
	
void PORT1_IRQHandler(void){

	if ((P1IFG & 0x02) != 0) {
		P1IFG &= ~0x02;		// clear flag1, acknowledge
		P1OUT &=~ 0x01;
		P2OUT &=~ 0x07;
		led = (led == Red) ? RGB : Red;
  }

	if ((P1IFG & 0x10) != 0) {
		P1IFG &= ~0x10;		// clear flag4, acknowledge
		
		if(led == Red){
			P2OUT &=~ 0x07;
			P1OUT ^= 0x01;   // toggle LED1
		}
		if(led == RGB){
			P1OUT &=~ 0x01;
			
			if(P2OUT == 0x07)
			{
				P2OUT = 0x00;  
			} 
			else
			{
				P2OUT += 0x01;
			}
		}
  }
}

int main()
{
	__ASM("CPSID I");
	
	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
	
	configure();
	
	led = Red;
	
	P1IES |= (uint8_t)0x12;                   // P1.4 and P1.1 are falling edge event
	P1IFG &=~ (uint8_t)0x12;                  // clear flag4 and flag1 (reduce possibility of extra interrupt)
	P1IE |= (uint8_t)0x12;                    // arm interrupt on P1.4 and P1.1
	
	NVIC_SetPriority(PORT1_IRQn, 2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);
	
	__ASM("CPSIE I");
	
	for(;;)
	{
		__WFI();
	}
}
