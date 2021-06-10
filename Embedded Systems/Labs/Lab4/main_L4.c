#include <msp.h>
#include <stdbool.h>

void PORT1_IRQHandler(void);
void TA0_N_IRQHandler(void);

static bool redLedSelected = true;

void TA0_N_IRQHandler( void ){
	TIMER_A0->CTL &= (uint16_t)(~BIT0);
	if(redLedSelected){
		P1OUT ^= (uint8_t)(BIT0);
	}else if((P1OUT & (BIT0 | BIT1 | BIT2)) == (BIT0 | BIT1 | BIT2)){
		P2OUT &= (uint8_t)(~(BIT0 | BIT1 | BIT2));
	} else {
		P2OUT += 1;
	}
}


void PORT1_IRQHandler( void ){
	if (P1IFG & BIT1){
		P1IFG &= (uint8_t)(~ BIT1);
		redLedSelected = !redLedSelected;
	} else if (P1IFG & BIT4) {
		P1IFG &= (uint8_t)(~ BIT4);
		TIMER_A0->CTL ^= (uint16_t)(BIT5);
	}
}


int main(void){
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
	
	P1SELC &= (uint8_t)(~ (BIT0 | BIT1 | BIT4));
	P2SELC &= (uint8_t)(~ (BIT0 | BIT1 | BIT2));
	
	P1DIR &= (uint8_t)(~ (BIT1 | BIT4));
	P1DIR |= (uint8_t)(BIT0);
	P2DIR |= (uint8_t)(BIT0 | BIT1 | BIT2);
	
	P1REN |= (uint8_t)(BIT1 | BIT4);
	
	P1OUT |= (uint8_t)(BIT1 | BIT4);
	
	P1OUT &= (uint8_t)(~ BIT0);
	P2OUT &= (uint8_t)(~ (BIT0 | BIT1 | BIT2));
	
	P1IES |= (uint8_t) (BIT1 | BIT4);
	P1IFG	&= (uint8_t) ~(BIT1 | BIT4);
	P1IE |= (uint8_t) (BIT1 | BIT4);

	TIMER_A0->CTL &= (uint16_t)(~(BIT5 | BIT4));
	TIMER_A0->CTL &= (uint16_t)(~BIT0);
	TIMER_A0->CTL |= (uint16_t)(BIT8);
	TIMER_A0->CTL |= (uint16_t)(BIT1);
	
	NVIC_SetPriority(PORT1_IRQn , 2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);
	
	NVIC_SetPriority(TA0_N_IRQn , 3);
	NVIC_ClearPendingIRQ(TA0_N_IRQn);
	NVIC_EnableIRQ(TA0_N_IRQn);
	
	__ASM("CPSIE I");
	
	TIMER_A0->CTL |= (uint16_t)(BIT5);
	
	while(1){
    __WFI();
  }
}
