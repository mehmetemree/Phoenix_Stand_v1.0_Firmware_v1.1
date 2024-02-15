#include "phoenix.h"
#include "gd32f10x.h"
#include "volt2temp.h"
#ifndef MODBUSH
#define MODBUSH


#define	Version	0x04

//! Modbus function code Readholdingregister .
#define MB_FUNCTION_READ 										0x03
//! Modbus function code  single data write .
#define MB_FUNCTION_WRITE										0x06

#define MB_REGISTER_ADC_MIN 			0x0019
#define MB_REGISTER_ADC_MAX			0x0040

#define Pho_REGISTER_ADC_1				0x0010									
#define Pho_REGISTER_ADC_2				0x0011		
#define Pho_REGISTER_ADC_3				0x0012
#define Pho_REGISTER_ADC_4				0x0013
#define Pho_REGISTER_ADC_5				0x0014
#define Pho_REGISTER_ADC_6				0x0015
#define Pho_REGISTER_ADC_7				0x0016
#define Pho_REGISTER_ADC_8				0x0017

#define Pho_REGISTER_ADC_1_1			0x0020
#define Pho_REGISTER_ADC_1_2			0x0021	
#define Pho_REGISTER_ADC_1_3			0x0022	
#define Pho_REGISTER_ADC_1_4			0x0023	

#define Pho_REGISTER_ADC_2_1			0x0024	
#define Pho_REGISTER_ADC_2_2			0x0025	
#define Pho_REGISTER_ADC_2_3			0x0026	
#define Pho_REGISTER_ADC_2_4			0x0027	

#define Pho_REGISTER_ADC_3_1			0x0028	
#define Pho_REGISTER_ADC_3_2			0x0029	
#define Pho_REGISTER_ADC_3_3			0x002A	
#define Pho_REGISTER_ADC_3_4			0x002B

#define Pho_REGISTER_ADC_4_1			0x002C	
#define Pho_REGISTER_ADC_4_2			0x002D
#define Pho_REGISTER_ADC_4_3			0x002E	
#define Pho_REGISTER_ADC_4_4			0x002F	

#define Pho_REGISTER_ADC_5_1			0x0030	
#define Pho_REGISTER_ADC_5_2			0x0031	
#define Pho_REGISTER_ADC_5_3			0x0032	
#define Pho_REGISTER_ADC_5_4			0x0033

#define Pho_REGISTER_ADC_6_1			0x0034	
#define Pho_REGISTER_ADC_6_2			0x0035	
#define Pho_REGISTER_ADC_6_3			0x0036	
#define Pho_REGISTER_ADC_6_4			0x0037

#define Pho_REGISTER_ADC_7_1			0x0038	
#define Pho_REGISTER_ADC_7_2			0x0039	
#define Pho_REGISTER_ADC_7_3			0x003A	
#define Pho_REGISTER_ADC_7_4			0x003B

#define Pho_REGISTER_ADC_8_1			0x003C
#define Pho_REGISTER_ADC_8_2			0x003D	
#define Pho_REGISTER_ADC_8_3			0x003E	
#define Pho_REGISTER_ADC_8_4			0x003F

#define Pho_REGISTER_RELAY_1 			0x0018
#define Pho_REGISTER_RELAY_2 			0x0019

#define Pho_REGISTER_TEST_HEADER_ON		0x0070
#define Pho_REGISTER_TEST_HEADER_OFF	0x0071
#define Pho_REGISTER_TEST_FAN_ON			0x0072
#define Pho_REGISTER_TEST_FAN_OFF			0x0073



#define STATUS_HEATER_SENSOR_FAULT 0x0001
#define STATUS_FAN_SENSOR_FAULT 	 0x0002
#define STATUS_FAULT_FLAG	 				 0x0003

#define STATUS_FAULT_FLAG_REGISTER	 0x0074

//! Modbus register for charge box name.
#define MB_REGISTER_CHARGE_BOX_NAME 				0x0064
//! Modbus register for charge box model.
#define MB_REGISTER_CHARGE_BOX_MODEL 				0x0078
//! Modbus register for charge box manufacturer.
#define MB_REGISTER_MANUFACTURER 			    	0x008C
//! Modbus register for stm32 serial number.

#define MB_REGISTER_HARDWARE_REVISION 			0x00AC
//! Modbus register for chargebox firmware versin.
#define MB_REGISTER_FIRMWARE_VERSION 		  	0x00B1


//! Modbus register for general status.

#define MB_REGISTER_GENERAL_STATUS_ADDITION 0x0FAB

#define MINMBLENGHT     8

//! Data lenght
#define DATA_LENGHT 	0x0E
#define ERROR					0xFF
//! Frame Size 
#define FRAME_SIZE 		32
#define FRAME_DIV_SIZE 6
typedef	enum {
	searchForSlaveAddress	=	0,
	functionNumber        =	1,
	getData 				      =	2,
	getCRCHigh						=	3,
	getCRCLow							=	4
} MB_receive_Typedef;
//!  UART Interrupt.When the data is received, it executes the MB_Process() function.
void USART2_IRQHandler(void);
//! Modbus init.
void MB_Init(void);
//! Recive  and processing of incoming data.
void MB_Process(void);
//! Parsing and processing of incoming data.
void MB_Job(void);
//! Modbus write enable function.
void MB_WriteEnable(void);
//! Modbus read enable function.
void MB_ReadEnable(void);
//!  Modbus send data.
//void delay_1ms(uint32_t count);
/**

@param[in] unsigned char str[] = data to be sent .
@param[in] unsigned char strl[] = data lenght.
*/
void MB_SendDataArray(unsigned char str[], unsigned char strl);
//! Calculate crc for modbus data.
/**
@return unsigned short crc value
@param[in] unsigned char frame[]= The data frame whose CRC is to be calculated.
@param[in] unsigned char frameSize= Frame size.
*/
unsigned short MB_calculateCRC(unsigned char frame[], unsigned char frameSize);
//!Send a response to incoming data.
/**
@param[in] unsigned char slaveAddress = Modbus device slave address.
@param[in] unsigned int functionCode  = Modbus function code. Read or write.
@param[in] unsigned int Register = Modbus register value.
*/
void divided_mb(unsigned int first_slave,double *array); 
void MB_sendFrame(unsigned char slaveAddress, unsigned int functionCode,unsigned int Register);

//!Case where the incoming data is incorrect.
/**
@param[in] unsigned int functionCode  = Modbus function code. Read or write.
@param[in] unsigned int Exception_Code = Modbus error exception code.
*/
void MB_Exception( unsigned int functionCode,unsigned int Exception_Code );
static unsigned char MB_isReceivedFrameValid(unsigned char *frame, unsigned char frameSize);
unsigned char MB_findStartByteIndex(unsigned char buffer[32], unsigned char lastIdx, unsigned char chr);
//!  Modbus readholdingregister function. It is used to read multiple data.
/**
@param[in] unsigned char slaveAddress = Modbus device slave address.
@param[in] unsigned int functionCode  = Modbus function code. Read or write.
@param[in] unsigned int Register = Modbus register value.
@param[in] unsigned int count = Number of register.
*/
void MB_ReadHoldingRegister(unsigned char slaveAddress, unsigned int functionCode,unsigned int Register,unsigned int count);
/**
@param struct ChargeboxMBSystem
Contains information for Modbus about Chargebox .
@param [in] double* for_mb_temp = Written temprature information as Celcius.
@param [in] double array_mb_temp()= Each temprature sensor data written an element in the array.
@param [in] unsigned char *chr_mb_temp = 64-bit(double) data divide 8-bit. 
//! Each char_mb_temp include one array_mb_temp element as 8 element 1 byte. In o
*/
void MB_Write(unsigned char slaveAddress, unsigned int functionCode,unsigned int Register);
struct ChargeboxMBSystem{
	//! ChargeBox name .
	uint8_t name[20];
	//! ChargeBox model.
	uint8_t model[20];
	//! ChargeBox manufacturer.
	uint8_t manufacturer[20];
	//! Stm32  unique ID.
	uint8_t serialnumber[12];
	//! Hardware revision.
	uint8_t hardware_revision[5];
	//! Firmware version.
	uint8_t firmware_version[7];
	//! MCU temperature.
	uint8_t mc_temperature[2];
};
void UART3_IRQHandler(void);
#endif