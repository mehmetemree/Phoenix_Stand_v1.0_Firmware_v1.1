#include "gd32f10x.h"
#ifndef PHOENIX
#define PHOENIX
void Pho_init(void);
//! Configuration function call from here like RCC,GPIO,ADC,DMA etc.
/**
@return uint16_t temp_volt_all is return ADC value as voltage in array
*/
uint16_t *Pho_run(void);
//! Received ADC data come with DMA collapse and can be taken if it is necessary.
/**
@param[in] uint16_t* temp_volt = to hold adc value from get_adc().
@param[in] uint16_t temp_volt_all[8] = written adc value(voltage) and return array.
@return temp_volt_all
*/
int Pho_GetComboxMBAddr();
//! Address of master information can reach from here
/**
@param[in] int ComBoxMBAddress = Master address and return parameter.
@return ComBoxMBAddress
*/
void Pho_SetComboxMBAddr(int Addr);
/**
@param[in] int Addr =new value for ComBoxMBAddress
*/
//! Set Modbus slave device addres.
void Pho_ReadCurrentValueSwitches();
//! COMBOX_MBAdrress is based on swtiches.
void SetRelay1(uint8_t newValue1);
//! Configuration of Relay 1
/**
@param uint8_t relay1_cond = current condition of relay 1.
*/
uint8_t GetRelay1();
//!it will be used to modbus write command,etc.
/**
@return relay1_cond 
*/
void SetRelay2(uint8_t newValue2);
//! Configuration of Relay 2
/**
@param uint8_t relay2_cond = current condition of relay 2.
*/
uint8_t GetRelay2();
//!it will be used to modbus write command,etc.
/**
@return relay2_cond 
*/
void  LCD_MSleep(uint16_t mSecs);
//!Provide time interval for LCD initialization
void TIMER4_IRQHandler(void);
void TIMER3_IRQHandler(void);
void DelayUs( uint16_t uSecs );
//!Delay function
#endif