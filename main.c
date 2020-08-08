#include <em_device.h>							// header for all micro registers
#include "hardware.h"							// custom headers
#define MIN_DUTY 140
#define MAX_DUTY 480

//uint16_t VOL_buf[4];
//uint16_t avt, avb, avr, avl;
//int16_t dver, dhor;
uint16_t ADC_val[4];

float VOL_buf[4];
float avt, avb, avr, avl;
float dver, dhor;

uint32_t channel;
uint16_t htol;									// tolerance value
uint16_t vtol;
int16_t dutyV, dutyH, dutyVold, dutyHold;

int main()
{	
	CMU_setup();								// configure hardware			
	GPIO_setup();
	Timer1_setup();
	RTC_setup();
	ADC_setup();
		
	dutyV = dutyH = INIT_DUTY;
	htol = 500;
	vtol = 500;

	for (uint32_t i = 0; i < 100000; i++){}		// put servos to default position
	TIMER1->CMD = TIMER_CMD_STOP;
				
	while(1)
	{			
		__WFI();
		
		for (channel=4; channel<=7; channel++)	// measure light
		{
			ADC0->SINGLECTRL &= ~0xF00;
			ADC0->SINGLECTRL |= (channel << 8); 
			ADC0->CMD = ADC_CMD_SINGLESTART;
			__WFI();
			ADC_val[channel-4] = ADC0->SINGLEDATA;
		}
		VOL_buf[0] = (ADC_val[0]-11)*4095.0/(3980-11);	// equialize sensors sensitivity
		VOL_buf[1] = (ADC_val[1]-41)*4095.0/(3916-41);
		VOL_buf[2] = (ADC_val[2]-11)*4095.0/(3873-11);
		VOL_buf[3] = (ADC_val[3]- 0)*4095.0/(3962-0);
		
		avt = (VOL_buf[0] + VOL_buf[3])/2;      // average value top
		avb = (VOL_buf[1] + VOL_buf[2])/2;      // average value bottom
		avr = (VOL_buf[2] + VOL_buf[3])/2;      // average value right
		avl = (VOL_buf[0] + VOL_buf[1])/2;      // average value left		
		dver = avt - avb;						// difference top and bottom
		dhor = avr - avl;						// diference right and left
	
		if (dver < -vtol || dver > vtol)      	// check vertical measurements 
		{
			if (avt < avb)						// if top value is greater than bottom value increase duty 
			{
				dutyV += 10;				
				if (dutyV > MAX_DUTY) 			// check if duty is already MAX_DUTY
					dutyV = MAX_DUTY;	
			}
			else          		    			// if top value is smaller than bottom value decrease duty
			{
				dutyV -= 10;
				if (dutyV < MIN_DUTY) 			// check if duty is already MIN_DUTY
					dutyV = MIN_DUTY;
			}
			(TIMER1->CC[0]).CCV = dutyV;		// move servo vertically
		}
			
		if(dhor < -htol || dhor > htol)      	// check horizontal measurements 
		{
			if (avr > avl)                 		// if right value is greater than left value increase duty
			{			
				dutyH += 10;				
				if (dutyH > MAX_DUTY) 			// check if duty is already MAX_DUTY
					dutyH = MAX_DUTY;	
			}
			else if (avr < avl)					// if right value is smaller than left value increase duty
			{
				dutyH -= 10;
				if (dutyH < MIN_DUTY) 			// check if duty is already MIN_DUTY
					dutyH = MIN_DUTY;
			}
			(TIMER1->CC[1]).CCV = dutyH;		// move servo horizontally
		}
		
		if ((dutyV != dutyVold) || (dutyH != dutyHold))	// check if the move is needed
		{
			TIMER1->CMD = TIMER_CMD_START;		// turn on PWM 
			dutyVold = dutyV;
			dutyHold = dutyH;
			for (uint32_t i = 0; i < 100000; i++){}	// wait for servos to move
			TIMER1->CMD = TIMER_CMD_STOP;		// turn OFF PWM to reduce servo's noise
		}
	}
}

void RTC_IRQHandler(void)
{
	RTC->IFC = RTC_IFC_COMP0;					// clear RTC flag						
}

void ADC0_IRQHandler(void)
{
	ADC0->IFC = ADC_IFC_SINGLE;					// clear ADC interrupt
}
