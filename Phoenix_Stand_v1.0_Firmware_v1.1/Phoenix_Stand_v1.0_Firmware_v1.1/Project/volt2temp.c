#include "gd32f10x.h"
#include "phoenix.h"
#include "volt2temp.h"
#include <math.h>
#define REFERENCE_RESISTANCE  	6650.0
//#define NOMINAL_RESISTANCE  		4700.0
#define NOMINAL_RESISTANCE  		10000.0
#define NOMINAL_TEMPERATURE  		298.15//25 + 273.15
#define B_VALUE						  		4145.0
#define STM32_ANALOG_RESOLUTION 4095.0

static uint16_t desired_value = 0;
double Temperatures[MAXIMUM_SENSOR];// value of temperature by celsius
double resistance[MAXIMUM_SENSOR];// resistance value of adc pins
double TempKelvin[MAXIMUM_SENSOR];//value of temperature by kelvin
double temp[MAXIMUM_SENSOR];
uint16_t valueof_temp_volt[MAXIMUM_SENSOR];//temprature value in voltage
uint16_t *temp_volt_pho_run;//pho_run function values
uint16_t sensor_statusflag=0x00;// which sensor normal or fault
double *temporaryValue;// in order to take the temperature value 
double currentTemp[MAXIMUM_SENSOR];
uint8_t sample_cnt=MINUMUM_SAMPLE_CNT;
uint8_t currentTempNo;
uint8_t numofsens;
double totalTemp[MAXIMUM_SENSOR];
double avarageTemp[MAXIMUM_SENSOR];


double *Temperature()
{
	temp_volt_pho_run=Pho_run();/*taking voltage value from pins*/
	for( currentTempNo = 0 ; currentTempNo < MAXIMUM_SENSOR ; currentTempNo++ )
	{
		valueof_temp_volt[currentTempNo] = *(temp_volt_pho_run+currentTempNo);
		
		if(valueof_temp_volt[currentTempNo] == 0)/*if value equal zero, process of the below will be undefined. */
		{
			return 0;
		}
		resistance[currentTempNo] = REFERENCE_RESISTANCE /  ((STM32_ANALOG_RESOLUTION /  valueof_temp_volt[currentTempNo]) -1);
			TempKelvin[currentTempNo] = log(resistance[currentTempNo] / NOMINAL_RESISTANCE);
		/*
		https://www.allaboutcircuits.com/industry-articles/how-to-obtain-the-temperature-value-from-a-thermistor-measurement/  
	  https://mehmettopuz.net/stm32-ve-ntc-sensoru-ile-sicaklik-olcme/.html
		*/
		TempKelvin[currentTempNo] /= B_VALUE;
		TempKelvin[currentTempNo] += (1 / NOMINAL_TEMPERATURE);
		TempKelvin[currentTempNo] = 1 / TempKelvin[currentTempNo];
		temp[currentTempNo] = TempKelvin[currentTempNo] - 273.15;
		Temperatures[currentTempNo] = temp[currentTempNo];
	}
	return  Temperatures;
}

void SensorEvents()
{
	temporaryValue = Temperature();
	
	for( numofsens=0 ; numofsens < MAXIMUM_SENSOR ; numofsens++ )
	{
		currentTemp[numofsens] = *(temporaryValue + numofsens);
		totalTemp[numofsens] += currentTemp[numofsens];
	}
	sample_cnt++;
	if(sample_cnt == MAXIMUM_SAMPLE_CNT)
	{
	for( numofsens=0 ; numofsens < MAXIMUM_SENSOR ; numofsens++ )
	{
		totalTemp[numofsens] = totalTemp[numofsens] / MAXIMUM_SAMPLE_CNT;
		avarageTemp[numofsens] = totalTemp[numofsens];
		totalTemp[numofsens]=0;
	}
		if(IsSensorOK(avarageTemp[SENSOR1])== TRUE)
		{
			sensor_statusflag= sensor_statusflag & FAN_STATUS_FAULT;
			if (avarageTemp[SENSOR1] <= CUT_OFF_TEMP_L || (avarageTemp[SENSOR1] <= CUT_OFF_TEMP_L2 && gpio_input_bit_get(GPIOC,GPIO_PIN_1)))
			{
				gpio_bit_set(GPIOC,GPIO_PIN_0);
				gpio_bit_set(GPIOC,GPIO_PIN_1);		
			}
			else 
			{
				gpio_bit_reset(GPIOC,GPIO_PIN_0);
				gpio_bit_reset(GPIOC,GPIO_PIN_1);
			}
		}
		else
		{
			gpio_bit_reset(GPIOC,GPIO_PIN_0);
			gpio_bit_reset(GPIOC,GPIO_PIN_1);
			sensor_statusflag |= HEATER_STATUS_FAULT;
		}	
		if(IsSensorOK(avarageTemp[SENSOR2]) == TRUE)
		{
			sensor_statusflag= sensor_statusflag & HEATER_STATUS_FAULT;
			if(avarageTemp[SENSOR2] > CUT_OFF_TEMP_H || (avarageTemp[SENSOR2] >= CUT_OFF_TEMP_H2 && gpio_input_bit_get(GPIOC,GPIO_PIN_2)))
			{
				gpio_bit_set(GPIOC,GPIO_PIN_2);
			}
			else
			{	
				gpio_bit_reset(GPIOC,GPIO_PIN_2);
			}
		}
		else
		{
			gpio_bit_reset(GPIOC,GPIO_PIN_2);
			sensor_statusflag |= FAN_STATUS_FAULT;
		}
		sample_cnt = MINUMUM_SAMPLE_CNT;
		
		// Average other sensors???
	}

}

static bool IsSensorOK(double value)
{
	bool ret = FALSE;

	if((MINIMUM_TEMP < value) && (value < MAXIMUM_TEMP))
	{
		ret = TRUE;
	}
	
	return ret;
}

uint16_t SensorFaultStatus()
{
	return sensor_statusflag;
}

double GetSensorTemperature(uint8_t sno)
{
	return avarageTemp[sno];
}

