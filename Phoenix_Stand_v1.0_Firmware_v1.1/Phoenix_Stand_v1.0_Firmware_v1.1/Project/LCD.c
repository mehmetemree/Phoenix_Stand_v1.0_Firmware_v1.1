#include "LCD.h"
#include "phoenix.h"
#include "volt2temp.h"
#include "phoenixConfig.h"
#include <stdio.h>
#include <stdlib.h>
unsigned char lcd_Address, lcd_Line;
const unsigned char lcd_addLUT[4] = {0x80, 0xC0, 0x90, 0xD0};	
const unsigned char russian[]={ 0x41, 0xA0, 0x42, 0xA1, 0xE0, 0x45,
0xA3, 0xA4, 0xA5,0xA6, 0x4B, 0xA7, 0x4D, 0x48, 0x4F, 0xA8, 0x50,0x43,
0x54, 0xA9, 0xAA, 0x58, 0xE1, 0xAB, 0xAC, 0xE2, 0xAD,0xAE, 0x62,
0xAF, 0xB0, 0xB1, 0x61, 0xB2, 0xB3, 0xB4, 0xE3, 0x65, 0xB6, 0xB7,
0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0x6F, 0xBE, 0x70, 0x63,0xBF,
0x79, 0xE4, 0x78, 0xE5, 0xC0, 0xC1, 0xE6, 0xC2,0xC3, 0xC4, 0xC5,
0xC6, 0xC7 };

// remove it

double Lcd_Temp[MAXIMUM_SENSOR];
unsigned char *chr_lcd_temp;
char* str;
char str_temp[5] = "TEMP";
char addr_temp_value[8];
int queue = 0;
int row;
uint8_t sensNo = 0;
void Lcd()
{
	//lcdFastClear();
	/*data is written Lcd_Temp as celcius*/
	
	// for loop?
	for(sensNo = 0 ; sensNo < MAXIMUM_SENSOR ; sensNo++)
	{
		Lcd_Temp[sensNo] = GetSensorTemperature(sensNo);
	}
	row = 0;
	
	lcd_SetLCDPosition(2,row);/*set text to center*/
	//lcd_ShowStr(str_temp);
	lcd_ShowChar(48+queue);/*48+queue= 0,1,2..,7*/
	lcd_ShowChar(58);/*" : "*/
	
	/*equalize starting digit */
	if(addr_temp_value>0){lcd_SetLCDPosition(9,row);}
	else{lcd_SetLCDPosition(8,row);}
	
	/*only one digit is taken after the point*/
	sprintf(addr_temp_value,"%3.1f",Lcd_Temp[queue]);
	//lcd_ShowStr(addr_temp_value);
	
	row++;
	
	lcd_SetLCDPosition(2,row);
	//lcd_ShowStr(str_temp);
	lcd_ShowChar(48+queue+1);
	lcd_ShowChar(58);
	
	if(addr_temp_value>0){lcd_SetLCDPosition(9,row);}
	else{lcd_SetLCDPosition(8,row);}
	
	sprintf(addr_temp_value,"%3.1f",Lcd_Temp[queue+1]);
	//lcd_ShowStr(addr_temp_value);
	
	
	queue=queue+2;
	if(queue>=8){ queue=0;}
	

}

void lcd_init_pins(void)
{
	rcu_periph_clock_enable(lcd_RCC);
	rcu_periph_clock_enable(RCU_GPIOC);
	gpio_init(lcd_port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, lcd_pin_E);
	gpio_init(lcd_port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, lcd_pins_data);
	gpio_init(lcd_port, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, lcd_pin_RS);
	gpio_init(GPIOC		, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
	
}
void lcd_WriteNibble(unsigned char data) {

  if (((data >> 3) & 0x01) == 1) gpio_bit_set(GPIOC, GPIO_PIN_6); //d7
  else gpio_bit_reset(GPIOC, GPIO_PIN_6);
  if (((data >> 2) & 0x01) == 1) gpio_bit_set(lcd_port, GPIO_PIN_12); //d6
  else gpio_bit_reset(lcd_port, GPIO_PIN_12);
  if (((data >> 1) & 0x01) == 1) gpio_bit_set(lcd_port, GPIO_PIN_11); //d5
  else gpio_bit_reset(lcd_port, GPIO_PIN_11);
  if (((data >> 0) & 0x01) == 1) gpio_bit_set(lcd_port, GPIO_PIN_5); //d4
  else gpio_bit_reset(lcd_port, GPIO_PIN_5);

  LCD_MSleep(100);
  lcd_E_1;
	LCD_MSleep(100);
  lcd_E_0;

}

void lcd_WriteByte(unsigned char data)
{
	lcd_WriteNibble(data >> 4);
	lcd_WriteNibble(data & 0x0F);
}
/*
LCD send command
*/
void command(unsigned char data){
	lcd_WriteNibble(data >> 4);
} 

void lcd_GoToLine(char LineNum)
{
	lcd_RS_0;
	lcd_Address = lcd_addLUT[LineNum-1];
	lcd_WriteByte(lcd_Address);
	lcd_RS_1;
	lcd_Address = 0;
	lcd_Line = LineNum;
}

/*
LCD clear.
*/
void lcd_ClearLCD(void)
{
	lcd_RS_0;
	lcd_WriteByte(0x01);
	LCD_MSleep(50);
	lcd_RS_1;
	lcd_GoToLine(1);
}

/*
LCD position settings
*/
void lcd_SetLCDPosition(char x, char y)
{
	lcd_RS_0;
	lcd_Address = lcd_addLUT[y] + x;
	lcd_WriteByte(lcd_Address);
	lcd_RS_1;
	lcd_Line = y+1;
}

void lcd_ShowChar(unsigned char c)
{

	lcd_RS_1;
	DelayUs(2500);
	if(c>=192) lcd_WriteByte(russian[c-192]);
	else lcd_WriteByte(c);
	lcd_Address++;
	switch (lcd_Address)
	{
		case 16: 	lcd_SetLCDPosition(0,1); break;
		case 0xD0: lcd_SetLCDPosition(0,0); break;
		case 0x90: 	lcd_SetLCDPosition(0,1); break;
	}
}

/*
LCD show string
*/
void lcd_ShowStr(unsigned char *s)
{
	while (*s != 0) lcd_ShowChar(*s++);
}
void lcdFastClear(){
lcd_SetLCDPosition(0,0);
//lcd_ShowStr("                                ");

}
void lcd_init(void)
{
	
	LCD_MSleep(50);
	lcd_E_0;
	LCD_MSleep(15);
	lcd_RS_0;
	command(0x38);
	LCD_MSleep(7);
	command(0x38);
	LCD_MSleep(1);
	command(0x38);
	LCD_MSleep(1);
	command(0x28);
	LCD_MSleep(1);
	lcd_WriteByte(0x29);
	LCD_MSleep(7);
	lcd_WriteByte(0x14);
	LCD_MSleep(7);
	lcd_WriteByte(0x71);
	LCD_MSleep(7);
	lcd_WriteByte(0x5D);
	LCD_MSleep(7);
	lcd_WriteByte(0x6C);
	LCD_MSleep(7);
	lcd_WriteByte(0x08);
	LCD_MSleep(7);
	lcd_WriteByte(0x0C); // for blink 0x0d
	LCD_MSleep(7);
	lcd_WriteByte(0x06); 
	LCD_MSleep(7);
	lcd_RS_1;
	lcd_ClearLCD();
}
