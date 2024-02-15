#include "gd32f10x.h"
#include <stdio.h>
#include "phoenix.h"
#include "modbus.h"
#include "volt2temp.h"
#include "LCD.h"

int main (void)
{
	Pho_init();
	MB_Init();
	lcd_init_pins();
	lcd_init();
	lcdFastClear();
	while(1)
	{
		SensorEvents();
	}
}
