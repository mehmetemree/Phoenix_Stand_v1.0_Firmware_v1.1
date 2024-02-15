#ifndef VOLT2TEMP
#define VOLT2TEMP

#include "phoenixConfig.h"
#include "gd32f10x.h"

#define MAXIMUM_SENSOR 			4
#define SENSOR1							0
#define SENSOR2							1
#define SENSOR3							2
#define SENSOR4							3
#define SENSOR5							4
#define SENSOR6							5
#define SENSOR7							6
#define SENSOR8							7

#define CUT_OFF_TEMP_H 			45
#define CUT_OFF_TEMP_L 			5
#define CUT_OFF_TEMP_L2 		10
#define CUT_OFF_TEMP_H2 		43
#define MINIMUM_TEMP 				-60
#define	MAXIMUM_TEMP 				150
#define MINUMUM_SAMPLE_CNT 	0
#define MAXIMUM_SAMPLE_CNT 	100
#define HEATER_STATUS_FAULT 0x01
#define FAN_STATUS_FAULT    0x02

/**
volt => Celcius

temp_volt_pho_run()=Pho_run()  taking voltage value from pins


if( valueof_temp_volt()==0) if value equal zero, process of the below will be undefined.

@param[in] double resistance()= include each sensor resistance
@param[in] double TempKelvin()= at the begin values obtain as Kelvin.
@param[in] double temp()= transformation from Kelvin to Celcius.
@param[in] double Temperatures() = data is celcius.
*/
double *Temperature(void);
static bool IsSensorOK(double value);//whether observed value of sensors are normal or not normal
//!It aimed to if the temprature exceed defined value,relays will be open.
void SensorEvents(void);
uint16_t SensorFaultStatus();
double GetSensorTemperature(uint8_t sno);

#endif