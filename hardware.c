#include <em_device.h>
#include "hardware.h"

DMA_DESCRIPTOR_TypeDef DMA_C0 __attribute__((aligned (256))); // allocate DMA structure

void CMU_setup(void)									// configure clock
{
	CMU->HFCORECLKEN0 |= CMU_HFCORECLKEN0_LE;			// required to enable RTC
	CMU->OSCENCMD = CMU_OSCENCMD_LFRCOEN;				// enable LFRCO
	CMU->LFCLKSEL = CMU_LFCLKSEL_LFA_LFRCO;				// set LFRCO as LFA
}

void GPIO_setup(void)									// configure GPIO ports
{
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_GPIO;			// enable clock to GPIO
	(GPIO->P[2]).MODEH = GPIO_P_MODEH_MODE13_PUSHPULL +	// PWM outputs
						 GPIO_P_MODEH_MODE14_PUSHPULL;
}

void RTC_setup(void)									// configure RTC
{
	CMU->LFACLKEN0 |= CMU_LFACLKEN0_RTC; 				// enable LFA to RTC
	RTC->COMP0 = 2000;									// 0.0625 sec period setup
	RTC->IEN = RTC_IEN_COMP0;							// enable interrupt
	RTC->CTRL = RTC_CTRL_COMP0TOP + RTC_CTRL_EN;		// start RTC
	NVIC_EnableIRQ(RTC_IRQn);
}

void Timer1_setup(void)
{
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_TIMER1; 		// enable clock to timer	
	TIMER1->TOP = 4375;									// 4375 = 50 Hz
	TIMER1->CTRL = TIMER_CTRL_PRESC_DIV64;				// run timer at 14MHz / 64 KHz
	TIMER1->ROUTE = TIMER_ROUTE_CC0PEN+					// enable timer1 output pins
					TIMER_ROUTE_CC1PEN+
				    TIMER_ROUTE_LOCATION_LOC0;
	(TIMER1->CC[0]).CTRL = TIMER_CC_CTRL_MODE_PWM+		// select PWM mode
						   TIMER_CC_CTRL_CUFOA_SET; 	// configure PWM mode 
	(TIMER1->CC[0]).CCV = INIT_DUTY;							// init duty cycle
	(TIMER1->CC[1]).CTRL = TIMER_CC_CTRL_MODE_PWM+		// select PWM mode
						   TIMER_CC_CTRL_CUFOA_SET; 	// configure PWM mode 
	(TIMER1->CC[1]).CCV = INIT_DUTY;							// init duty cycle 
	TIMER1->CMD = TIMER_CMD_START;						// start timer
}

void ADC_setup(void)
{
	CMU->HFPERCLKEN0 |= CMU_HFPERCLKEN0_ADC0;			// enable clock to ADC
	ADC0->CTRL = ADC_CTRL_OVSRSEL_X4 +					// enable 4x oversampling
				 ADC_CTRL_TIMEBASE_DEFAULT +			// select warm-up time
				 (1 << 8) + 							// set 1:2 clock prescaler
				 ADC_CTRL_LPFMODE_RCFILT; 				// use input RC-filter	
	ADC0->SINGLECTRL =  									// enable PRS Ch0 trigger
				 ADC_SINGLECTRL_AT_16CYCLES + 			// setup acquisition time
				 ADC_SINGLECTRL_REF_VDD; 				// select VDD as reference	
/*	ADC0->SCANCTRL =  ADC_SCANCTRL_PRSEN +				// enable PRS Ch0 trigger
				 ADC_SCANCTRL_AT_16CYCLES + 			// setup acquisition time
				 ADC_SCANCTRL_REF_VDD + 				// select VDD as reference
				 ADC_SCANCTRL_INPUTMASK_CH4 +			// select ADC inputs
				 ADC_SCANCTRL_INPUTMASK_CH5 +
				 ADC_SCANCTRL_INPUTMASK_CH6 +
				 ADC_SCANCTRL_INPUTMASK_CH7;
*/	
	ADC0->IEN = ADC_IEN_SINGLE;							// enable ADC interrupt
	NVIC_EnableIRQ(ADC0_IRQn);							// enable ADC interrupt in NVIC	
}
