#include "gd32f10x.h"
#ifndef PHOENIXCONFIG
#define PHOENIXCONFIG

#define RELAY1_ON		gpio_bit_set(GPIOC, GPIO_PIN_0)
#define RELAY1_OFF	gpio_bit_reset(GPIOC, GPIO_PIN_0)
#define RELAY2_ON		gpio_bit_set(GPIOC, GPIO_PIN_1) 
#define RELAY2_OFF	gpio_bit_reset(GPIOC, GPIO_PIN_1)
void Pho_RCC_Configuration(void);
//! System Clocks Configuration
void TIM3Configuration(void);
//! TIM3 Configuration 
void TIM4Configuration(void);
//! TIM4 Configuration
void NVIC_Configuration(void);
void Pho_GPIOConfiguration(void);
//! GPIO Configuration
void RTC_IRQHandler(void);
/**
@param[in] LCD() loop in here each 1 second
*/
void nvic_configuration(void);
void USART_Configuration(void);
void rtc_configuration(void);
//! Real Time Clock Configuration for LCD
void ADC_Config(void);
//! ADC Configuration
void DMA_Config(void);
//! DMA Configuration
//! Able to sending initialized and obtained ADC values 
uint16_t *get_adc(void);
//! First funciton include ADC value, using "adc_value" that first step and variable for ADC with DMA
/** 
@return Unsigned 16-bit *adc_value .
*/
#define RS485_ADDR0  	  gpio_input_bit_get(GPIOC, GPIO_PIN_6)
#define RS485_ADDR1    	gpio_input_bit_get(GPIOC, GPIO_PIN_7)
#endif