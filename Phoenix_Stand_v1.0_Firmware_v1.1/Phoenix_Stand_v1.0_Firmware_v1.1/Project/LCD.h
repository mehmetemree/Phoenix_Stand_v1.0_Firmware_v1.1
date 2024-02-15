#ifndef LCD_WH1602_04_H
#define LCD_WH1602_04_H

#include "gd32f10x.h"

#define lcd_port				GPIOB
#define lcd_pin_RS			GPIO_PIN_9
#define lcd_pin_E				GPIO_PIN_8	
#define lcd_pins_data		GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12
#define lcd_RCC					RCU_GPIOB 


#define lcd_RS_1 gpio_bit_set(lcd_port, lcd_pin_RS);
#define lcd_E_1  gpio_bit_set(lcd_port, lcd_pin_E);
#define lcd_RS_0 gpio_bit_reset(lcd_port, lcd_pin_RS);
#define lcd_E_0  gpio_bit_reset(lcd_port, lcd_pin_E);


/**
@param[in] double* Lcd_Temp() = data is written Lcd_Temp as celcius.
@param[in] double array_lcd_temp() = Temprature data divided to array element.
@param[in] lcd_SetLCDPosition(2,row) = set text to center
@param[in] lcd_ShowChar(48+queue) = 48+queue= 0,1,2..,7 \n
if(addr_temp_value>0){lcd_SetLCDPosition(9,row);}else{lcd_SetLCDPosition(8,row);} equalize starting digit. \n
sprintf(addr_temp_value,"%3.1f",array_lcd_temp[queue]); only one digit is taken after the point.
*/
void Lcd(void);

void lcdFastClear(void);
void lcd_WriteByte(unsigned char data);
void lcd_ClearLCD(void);
void command(unsigned char data);
void lcd_SetLCDPosition(char x, char y);
void lcd_ShowChar(unsigned char c);
void lcd_ShowStr(unsigned char *s);
void lcd_init_pins(void);
void lcd_init(void);

#endif