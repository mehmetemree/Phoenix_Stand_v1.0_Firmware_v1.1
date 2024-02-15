#include "gd32f10x.h"
#include "phoenixConfig.h"
#include "phoenix.h"
#include "modbus.h"

int ComBoxMBAddress=0;
int State=0;
uint8_t relay1_cond=0;
uint8_t relay2_cond=0;
void Pho_init(){
  // System Clocks Configuration
  Pho_RCC_Configuration();
  // GPIO Configuration 
  Pho_GPIOConfiguration();
	// TIM3 Configuration 
	TIM3Configuration();
	// TIM4 Configuration 
	TIM4Configuration();
	NVIC_Configuration();
	
	rtc_configuration();
  //DMA Configuration
	DMA_Config();
	//ADC Configuration
	ADC_Config();
	// Selection of Master Address
	Pho_ReadCurrentValueSwitches();
	
}

uint16_t* temp_volt;
// MAXIMUM SENSOR CNT
uint16_t temp_volt_all[MAXIMUM_SENSOR];
uint16_t* Pho_run(){

	/*adc_regular_software_startconv_flag_get(ADC1);*/
	int i=0;
	temp_volt = get_adc();
	for(i=0;i<MAXIMUM_SENSOR;i++){
		temp_volt_all[i]=*(temp_volt+i);
	}	
	
	return temp_volt_all;
}

void Pho_ReadCurrentValueSwitches() 
{


// COMBOX_MBAdrress is based on swtiches.
/*	if(!RS485_ADDR0)
	{
		if(RS485_ADDR1) ComBoxMBAddress=0x40;
		else ComBoxMBAddress=0x41;
	}
	else
	{
		if(RS485_ADDR1) ComBoxMBAddress=0x42;   
		else ComBoxMBAddress=0x43;
	}
*/	
	ComBoxMBAddress	= 0x43;
}
int Pho_GetComboxMBAddr()
{
	return ComBoxMBAddress;
}
void Pho_SetComboxMBAddr(int Addr)
{
	 ComBoxMBAddress = Addr;
}
/* Set and get condition od relays*/
void SetRelay1(uint8_t newValue1)
{
	relay1_cond = newValue1;
}

uint8_t GetRelay1()
{
	return relay1_cond;
}

void SetRelay2(uint8_t newValue2)
{
	relay2_cond = newValue2;
}
uint8_t GetRelay2()
{
	return relay2_cond;
}

void DelayUs( uint16_t uSecs ) 
{
  uint16_t start = TIMER_CNT(TIMER3);
  /* use 16 bit count wrap around */
  while((uint16_t)(TIMER_CNT(TIMER3) - start) <= uSecs);
}

void LCD_MSleep(uint16_t mSecs)
{
  
  uint16_t u,v;
	
    while(mSecs--){
			fwdgt_counter_reload();
      for(u=0;u<2;u++)
        for(v=0;v<285;v++);
		}
}

