#include "msp.h"
#include <stdint.h>
#define Delay_Value 10000

static void configure(void)
{
	//Set GPIO of P1.1, P1.4, P1.0 to 0
	P1SEL0 &= (uint8_t)(~((1<<1) | (1<<4) | (1<<0)));
	P1SEL1 &= (uint8_t)(~((1<<1) | (1<<4)) | (1<<0));
	//P2.0, P2.1, P2.2
	P2SEL0 &= (uint8_t)(~((1<<2) | (1<<1) | (1<<0)));
	P2SEL1 &= (uint8_t)(~((1<<2) | (1<<1) | (1<<0)));
	//Set Direction as INPUT/Output
	P1DIR &= (uint8_t)(~((1<<4) | (1<<1)));
	P1DIR |= (uint8_t)(1<<0);
	P2DIR |= (uint8_t)(((1<<2) | (1<<1) | (1<<0)));
	//Set Drive: 1 for Active Low, 0 for Active High
	P1OUT &= ~(uint8_t)(1<<0);	
	P2OUT &= ~(uint8_t)(((1<<2) | (1<<1) | (1<<0)));
	//Enable internal pull-up resistors
	P1OUT |= (uint8_t)(~((1<<4) | (1<<1)));
	P1REN |= (uint8_t)(~((1<<4) | (1<<1)));
}

	int which_led = 0;
	int i = Delay_Value;
	
	//Disabling the watchdog timer
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
	
	configure();

	while(1)
	{
		
		//If both pins are on
		
		
		i = Delay_Value;
		//at least one pin is low (hopefully)
		if(P1IN & 0x10)
		{
			if(which_led)
			{
				which_led = 0;
			}
			else which_led = 1;
		}
		else if(P1IN & 0x02)
		{
			if(which_led)
			{
				P1OUT ^= 0x00;
				P2OUT &= 0x00;
			}
			else 
			{
				P1OUT &= 0x00;
				
				if(P2OUT == 0x07)
				{
					P2OUT &= ˜0x07;
				}
				else
				{
					P2OUT += 0x01;
				}
			}
		}

	while(i>0){i--;}
	}
}
