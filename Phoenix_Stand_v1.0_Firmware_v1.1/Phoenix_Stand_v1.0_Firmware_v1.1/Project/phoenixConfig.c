#include "gd32f10x.h"
#include "systick.h"
#include "phoenixConfig.h"
#include "phoenix.h"
#include "LCD.h"
#include "volt2temp.h"

uint16_t adc_value[MAXIMUM_SENSOR];


uint16_t *get_adc(){
	return adc_value;
}

void Pho_RCC_Configuration(void)
{
	
		// RCC system reset(for debug purpose) 
	rcu_deinit();
	rcu_osci_bypass_mode_enable(RCU_HXTAL);
	// activate HSI (Internal High Speed oscillator) 
	rcu_osci_on(RCU_IRC8M);
	while (rcu_flag_get(RCU_FLAG_IRC8MSTB) == RESET);

	// Enable Prefetch Buffer 
//	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

	// Flash 2 wait state 
//	FLASH_SetLatency(FLASH_Latency_2);

	// HCLK = SYSCLK 
	rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV1);//24 MHz

	// PCLK2 = HCLK 
	rcu_apb2_clock_config(RCU_APB2_CKAHB_DIV1);//24 MHz

	// PCLK1 = HCLK/4
	rcu_apb1_clock_config(RCU_APB1_CKAHB_DIV2);//12 MHz

	// PLLCLK = 4MHz * 6 = 24 MHz 
	rcu_pll_config(RCU_PLLSRC_IRC8M_DIV2, RCU_PLL_MUL6);//HSI kullanildiginda max. frekans 36 MHz
	// Enable PLL 
	//RCC_PLLCmd(ENABLE);
	rcu_osci_on(RCU_PLL_CK);
	// Wait till PLL is ready 
	while (rcu_flag_get(RCU_FLAG_PLLSTB) == RESET)
	{}

	// Select PLL as system clock source 
	rcu_system_clock_source_config(RCU_CKSYSSRC_PLL);
	// Wait till PLL is used as system clock source 
	while (rcu_system_clock_source_get() != RCU_SCSS_PLL)
	{}

	//ADC Clock Configuration
	rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV2);//12 Mhz
	// TIM3 and TIM2 clock enable 
	rcu_periph_clock_enable(RCU_TIMER5);
	rcu_periph_clock_enable(RCU_TIMER4);
	rcu_periph_clock_enable(RCU_TIMER3);
	rcu_periph_clock_enable(RCU_TIMER2);// pwm için pb0
	rcu_periph_clock_enable(RCU_TIMER1);
	rcu_periph_clock_enable(RCU_TIMER0);	
	rcu_periph_clock_enable(RCU_USART2);
	rcu_periph_clock_enable(RCU_UART3);// modbus uart
	rcu_periph_clock_enable(RCU_USART0);	// uart 
	// Interrupt, ADC1, GPIOA, GPIOB and GPIOC clock enable 
	rcu_periph_clock_enable(RCU_USART1);
	rcu_periph_clock_enable(RCU_GPIOA );
	rcu_periph_clock_enable(RCU_GPIOB);
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_AF);
	rcu_periph_clock_enable(RCU_ADC1);	
	rcu_periph_clock_enable(RCU_ADC0);
	rcu_periph_clock_enable(RCU_DMA0);

}
void USART_Configuration()
{
	   usart_baudrate_set(USART0, 115200U);
		 usart_word_length_set(USART0,USART_WL_8BIT);
		 usart_stop_bit_set(USART0,USART_STB_1BIT);
		 usart_parity_config(USART0,USART_PM_NONE);
		 usart_hardware_flow_rts_config(USART0,USART_RTS_DISABLE);
		 usart_hardware_flow_cts_config(USART0,USART_CTS_DISABLE);
		 usart_receive_config(USART0, USART_RECEIVE_ENABLE);
	   usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
		 usart_enable(USART0);
	
	  #ifdef HARDWARE_BUTTON_MODE
						
		#else
		 usart_deinit(UART3);
		 #ifdef MODBUS_BAUDRATE_19200U
		 usart_baudrate_set(UART3, 19200U);
		 #else
		 usart_baudrate_set(UART3, 19200U);
		 #endif
		 usart_word_length_set(UART3,USART_WL_9BIT);
		 usart_stop_bit_set(UART3,USART_STB_1BIT);
		 usart_parity_config(UART3,USART_PM_EVEN);
		 usart_hardware_flow_rts_config(UART3,USART_RTS_DISABLE);
		 usart_hardware_flow_cts_config(UART3,USART_CTS_DISABLE);
		 usart_receive_config(UART3, USART_RECEIVE_ENABLE);
	   usart_transmit_config(UART3, USART_TRANSMIT_ENABLE);
	   usart_interrupt_enable(UART3,USART_INT_RBNE);
		 usart_enable(UART3);
			
		#endif

}
void TIM3Configuration()
{// delayus

	 /* Time base configuration */
	timer_parameter_struct TIM_TimeBaseStructure3;
  TIM_TimeBaseStructure3.period 				= 23999;//65535;
  TIM_TimeBaseStructure3.prescaler 		= 0;//23;
  TIM_TimeBaseStructure3.clockdivision = 0;
  TIM_TimeBaseStructure3.counterdirection 	= TIMER_COUNTER_UP;
	
	timer_init(TIMER3, &TIM_TimeBaseStructure3);
		/* Enable Auto Reload */
	timer_auto_reload_shadow_enable(TIMER3);
	
	timer_update_source_config(TIMER3,TIMER_UPDATE_SRC_REGULAR);
  timer_interrupt_enable(TIMER3, TIMER_INT_UP);
	timer_enable(TIMER3);

}

void TIM4Configuration()
{
	timer_parameter_struct TIM_TimeBaseStructure4;
  TIM_TimeBaseStructure4.period 				= 47999;
  TIM_TimeBaseStructure4.prescaler 		= 0;
  TIM_TimeBaseStructure4.clockdivision = 0;
  TIM_TimeBaseStructure4.counterdirection 	= TIMER_COUNTER_UP;
	
	timer_init(TIMER4, &TIM_TimeBaseStructure4);
		/* Enable Auto Reload */
	timer_auto_reload_shadow_enable(TIMER4);
	
	timer_update_source_config(TIMER4,TIMER_UPDATE_SRC_REGULAR);
  timer_interrupt_enable(TIMER4, TIMER_INT_UP);
	timer_enable(TIMER4);
}
void Pho_GPIOConfiguration(void)
{
	gpio_pin_remap_config(GPIO_USART2_FULL_REMAP , ENABLE);
	
  // RELAY_1, RELAY_2, 
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
	

	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_2);
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_3);
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4);
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
	gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
	
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
  /* Configure RS485 UART4 Rx as input floating */
	gpio_init(GPIOC, GPIO_MODE_IPU  , GPIO_OSPEED_50MHZ, GPIO_PIN_11);

  
  /* Configure RS485 UART4  Tx as alternate function push-pull */
	gpio_init(GPIOC, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	
	// Led0, Led1, Led2 
	gpio_init(GPIOC,  GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5);
	gpio_init(GPIOB,  GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	gpio_init(GPIOB,  GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);


	
	/* Configure USART1 Rx as input floating */
	gpio_init(GPIOB, GPIO_MODE_IPU , GPIO_OSPEED_50MHZ, GPIO_PIN_7);
  /* Configure USART1 Tx as alternate function push-pull */
	gpio_init(GPIOB, GPIO_MODE_AF_PP , GPIO_OSPEED_50MHZ, GPIO_PIN_6);
	
  gpio_pin_remap_config(GPIO_USART0_REMAP, ENABLE);
	// Socket in or not
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_12);
	// Open Lock
	gpio_init(GPIOA, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
	
	//RS485_ADDR0
	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_15);
	//RS485_ADDR1
	gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_14);
	
}
void ADC_Config(void)
{
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE);
    /* ADC data alignment config */
		adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
		adc_special_function_config(ADC1, ADC_CONTINUOUS_MODE, ENABLE);
    /* ADC scan mode disable */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
		adc_special_function_config(ADC1, ADC_SCAN_MODE, ENABLE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
		adc_data_alignment_config(ADC1, ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 8U);
    adc_data_alignment_config(ADC1, ADC_DATAALIGN_RIGHT);
    /* ADC channel length config */
		adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC1, ADC_REGULAR_CHANNEL, 8U);
	
	  /* ADC regular channel config */
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_0, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_1, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_2, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_3, ADC_SAMPLETIME_55POINT5);
		adc_regular_channel_config(ADC0, 4, ADC_CHANNEL_4, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 5, ADC_CHANNEL_5, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 6, ADC_CHANNEL_6, ADC_SAMPLETIME_55POINT5);
    adc_regular_channel_config(ADC0, 7, ADC_CHANNEL_7, ADC_SAMPLETIME_55POINT5);
	
    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE); 
		adc_external_trigger_source_config(ADC1, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE); 
	
    /* ADC external trigger config */
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
		adc_external_trigger_config(ADC1, ADC_REGULAR_CHANNEL, ENABLE);
		
		/* enable ADC interface */
    adc_enable(ADC0);
		adc_enable(ADC1);
		
		DelayUs(1000);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
		adc_calibration_enable(ADC1);
		adc_dma_mode_enable(ADC0);
		adc_dma_mode_enable(ADC1);
		adc_software_trigger_enable(ADC0,ADC_REGULAR_CHANNEL);
		adc_software_trigger_enable(ADC1,ADC_REGULAR_CHANNEL);
		DelayUs(1000);
		
}
void DMA_Config(void)
{
    /* ADC_DMA_channel configuration */
    dma_parameter_struct dma_data_parameter;
    
    /* ADC DMA_channel configuration */
    dma_deinit(DMA0, DMA_CH0);
    
    /* initialize DMA single data mode */
    dma_data_parameter.periph_addr  = (uint32_t)(&ADC_RDATA(ADC0));
    dma_data_parameter.periph_inc   = DMA_PERIPH_INCREASE_DISABLE;
    dma_data_parameter.memory_addr  = (uint32_t)(&adc_value);/*data exist in here '&adc_value'*/
    dma_data_parameter.memory_inc   = DMA_MEMORY_INCREASE_ENABLE;
    dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
    dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
    dma_data_parameter.direction    = DMA_PERIPHERAL_TO_MEMORY;
    dma_data_parameter.number       = 8U;
    dma_data_parameter.priority     = DMA_PRIORITY_HIGH;
    dma_init(DMA0, DMA_CH0, &dma_data_parameter);
    dma_circulation_enable(DMA0, DMA_CH0);
  
    /* enable DMA channel */
    dma_channel_enable(DMA0, DMA_CH0);
}

void NVIC_Configuration(void)
{
	
	
#ifdef  VECT_TAB_RAM
  /* Set the Vector Table base location at 0x20000000 */
  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
#else  /* VECT_TAB_FLASH  */
  /* Set the Vector Table base location at 0x08000000 */
  nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x0);
#endif
	
	 /* Configure two bits for preemption priority and two bits for subpriority*/
 
  
  nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	nvic_irq_enable(UART3_IRQn,0,0);
	
	//nvic_irq_enable(TIMER4_IRQn,2,0);
	
	nvic_priority_group_set(NVIC_PRIGROUP_PRE1_SUB3);
  nvic_irq_enable(RTC_IRQn,1,0);
	
}

void rtc_configuration(void)
{
    /* enable PMU and BKPI clocks */
    rcu_periph_clock_enable(RCU_BKPI);
    rcu_periph_clock_enable(RCU_PMU);
    /* allow access to BKP domain */
    pmu_backup_write_enable();

    /* reset backup domain */
    bkp_deinit();

    /* enable LXTAL */
    rcu_osci_on(RCU_IRC40K );
    /* wait till LXTAL is ready */
    rcu_osci_stab_wait(RCU_IRC40K );
    
    /* select RCU_LXTAL as RTC clock source */
    rcu_rtc_clock_config(RCU_RTCSRC_IRC40K);

    /* enable RTC Clock */
    rcu_periph_clock_enable(RCU_RTC);

    /* wait for RTC registers synchronization */
    rtc_register_sync_wait();

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* enable the RTC second interrupt*/
    rtc_interrupt_enable(RTC_INT_SECOND);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();

    /* set RTC prescaler: set RTC period to 1s */
    rtc_prescaler_set(40000);

    /* wait until last write operation on RTC registers has finished */
    rtc_lwoff_wait();
}
int g = 0 ;
void RTC_IRQHandler(void)
{
    if (rtc_flag_get(RTC_FLAG_SECOND) != RESET){
        /* clear the RTC second interrupt flag*/
        rtc_flag_clear(RTC_FLAG_SECOND);
		}
		g++;
		Lcd();
}