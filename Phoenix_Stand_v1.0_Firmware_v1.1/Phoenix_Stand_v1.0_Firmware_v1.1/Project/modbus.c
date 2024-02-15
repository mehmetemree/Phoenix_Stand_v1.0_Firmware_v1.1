#include "modbus.h"
#include "phoenixConfig.h"
#include "ringbuffer.h"
#include "phoenix.h"
#include "systick.h"
#include "volt2temp.h"
#include "gd32f10x.h"
//!To collect the data to be sent in a single array.
unsigned char MB_txFrame[FRAME_SIZE];

unsigned char MB_div_txFrame[FRAME_DIV_SIZE];
//!To receive data from Uart.
unsigned char MB_receivedData;
//!To collect all received data in a single array.
unsigned char MB_rxBuffer[16];
//!To sort the received data in an array.
unsigned char MB_rxBufferIndex = 0;
//!The number of bytes to receive.
unsigned char MB_noOfDataBytes;
//!To store the received data in a new array and process it.
unsigned char MB_receivedModbusFrame[16];
//!//!To sort the received data in an array.
unsigned char MB_receivedModbusFrameIndex = 0;
MB_receive_Typedef MB_receiveStatus = searchForSlaveAddress;
//!To hold the calculated CRC value for Modbus data security.
unsigned short CalculatedCRC;
//! To hold the received CRC value for Modbus data security.
unsigned short ReceivedCRC;
ring_buffer_t ring_buffer;
char buf;
int cnt;
double* for_mb_temp;
uint16_t MBSensorStatus;
double array_mb_temp[MAXIMUM_SENSOR];
unsigned char* chr_mb_temp;
int m=0;

// remove it


void UART3_IRQHandler(void)
{
		usart_flag_clear(UART3, USART_FLAG_RBNE);		
		MB_Process();
}

ring_buffer_t ring_buffer;
void MB_Init(){
	
	USART_Configuration();
	MB_ReadEnable();

	nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x0);
  nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	nvic_irq_enable(UART3_IRQn,0,0);

	RELAY1_OFF;
	RELAY2_OFF;
	ring_buffer_init(&ring_buffer);
	
}


void MB_Process()
{
	MB_receivedData =  usart_data_receive(UART3);// rs485 RX pc11				
	ring_buffer_queue(&ring_buffer, MB_receivedData);
	while(usart_flag_get(UART3, USART_FLAG_TBE) == RESET){} 	
	cnt = ring_buffer_num_items(&ring_buffer);	
		
	if(cnt>=MINMBLENGHT)
	{
		ring_buffer_peek(&ring_buffer, &buf, cnt-MINMBLENGHT);
		if(buf== Pho_GetComboxMBAddr())
		{
			unsigned char i = 0 ; 
			for(i=0;i<MINMBLENGHT;i++)
			{
				ring_buffer_peek(&ring_buffer, &buf, cnt-MINMBLENGHT+i);
				MB_rxBuffer[i]=buf;
			}
			if(MB_isReceivedFrameValid(MB_rxBuffer, MINMBLENGHT))	/* received data appropriate or not*/
			{
				memcpy(MB_receivedModbusFrame, MB_rxBuffer, 16);
				while(ring_buffer_dequeue(&ring_buffer, &buf)){}
				MB_Job();	
			}
		}
	
	}
}

void MB_Job()
{
	unsigned int Register;
	unsigned int Numberofreg;
	DelayUs(10);
	Register    = ((MB_receivedModbusFrame[2] << 8)  | MB_receivedModbusFrame[3]);
	Numberofreg = ((MB_receivedModbusFrame[4] << 8)  | MB_receivedModbusFrame[5]);
	if(	MB_receivedModbusFrame[1] == MB_FUNCTION_READ)
	{
		
		if(Register > MB_REGISTER_ADC_MIN && Register < MB_REGISTER_ADC_MAX )
		{
			MB_ReadHoldingRegister(Pho_GetComboxMBAddr(), MB_FUNCTION_READ,Register,Numberofreg);
		}
		else if(Register== Pho_REGISTER_TEST_HEADER_ON)
		{
			SetRelay1(1);
			SetRelay2(1);
			gpio_bit_set(GPIOC,GPIO_PIN_0);
			gpio_bit_set(GPIOC,GPIO_PIN_1);	
			MB_ReadHoldingRegister(Pho_GetComboxMBAddr(), MB_FUNCTION_READ,Register,Numberofreg);
		}
		else if(Register== Pho_REGISTER_TEST_HEADER_OFF)
		{
			SetRelay1(0);
			SetRelay2(0);
			gpio_bit_reset(GPIOC,GPIO_PIN_0);
			gpio_bit_reset(GPIOC,GPIO_PIN_1);	
			MB_ReadHoldingRegister(Pho_GetComboxMBAddr(), MB_FUNCTION_READ,Register,Numberofreg);
		}
		else if(Register== Pho_REGISTER_TEST_FAN_ON)
		{
			SetRelay1(1);
			gpio_bit_set(GPIOC,GPIO_PIN_0);
			MB_ReadHoldingRegister(Pho_GetComboxMBAddr(), MB_FUNCTION_READ,Register,Numberofreg);
		}
		else if(Register== Pho_REGISTER_TEST_FAN_OFF)
		{
			SetRelay1(0);
			gpio_bit_reset(GPIOC,GPIO_PIN_0);
			MB_ReadHoldingRegister(Pho_GetComboxMBAddr(), MB_FUNCTION_READ,Register,Numberofreg);
		}
		else if(Register == STATUS_FAULT_FLAG_REGISTER)
		{				
			MB_ReadHoldingRegister(Pho_GetComboxMBAddr(), MB_FUNCTION_READ,Register,Numberofreg);
		}
		
	}
	else if(	MB_receivedModbusFrame[1] == MB_FUNCTION_WRITE)
	{
		if(Register==Pho_REGISTER_RELAY_1)
		{
			if(( (MB_receivedModbusFrame[4] << 8)  | MB_receivedModbusFrame[5]) ==1)
			{
				SetRelay1(1);
				MB_Write(Pho_GetComboxMBAddr(), MB_FUNCTION_WRITE,Pho_REGISTER_RELAY_1);
			}
			else
			{
				SetRelay1(0);
				MB_Write(Pho_GetComboxMBAddr(), MB_FUNCTION_WRITE,Pho_REGISTER_RELAY_1);
			}
			
		}
		else if(Register== Pho_REGISTER_RELAY_2)
		{
			if(( (MB_receivedModbusFrame[4] << 8)  | MB_receivedModbusFrame[5]) ==1)
			{
				SetRelay2(1);
				MB_Write(Pho_GetComboxMBAddr(), MB_FUNCTION_WRITE,Pho_REGISTER_RELAY_2);
			}
			else
			{
				SetRelay2(0);
				MB_Write(Pho_GetComboxMBAddr(), MB_FUNCTION_WRITE,Pho_REGISTER_RELAY_2);
			}
		}
		
	}
}
void MB_WriteEnable(){
	gpio_bit_set(GPIOC, GPIO_PIN_12);
}
// Read enable pin.
void MB_ReadEnable(){
	gpio_bit_reset(GPIOC, GPIO_PIN_12);
}

void MB_SendDataArray(unsigned char str[], unsigned char strl){
	unsigned char u;
	
	MB_WriteEnable();
	DelayUs(1000);
	for(u=0;u<strl;u++)
	{
		while(usart_flag_get(UART3, USART_FLAG_TBE) == RESET){}           
		usart_data_transmit(UART3, str[u]);
		usart_data_transmit(USART0, str[u]);
	}
	while(usart_flag_get(UART3, USART_FLAG_TC) == RESET){} 
	DelayUs(3000); // wait for last byte sent
	MB_ReadEnable();
}
unsigned short MB_calculateCRC(unsigned char frame[], unsigned char frameSize){

  unsigned int temp_crc, temp_crc2, flag;
  unsigned char i,j;
	
  temp_crc = 0xFFFF;
  for (i = 0; i < frameSize; i++)
  {
    temp_crc = temp_crc ^ frame[i];	
    for (j = 1; j <= 8; j++)
    {
      flag = temp_crc & 0x0001;
      temp_crc >>= 1;
      if (flag)
        temp_crc ^= 0xA001;
    }
  }
  temp_crc2 = temp_crc >> 8;
  temp_crc = (temp_crc << 8) | temp_crc2;
  temp_crc &= 0xFFFF;

  return temp_crc;
}
unsigned short crc16;
unsigned int ReadHoldingRegister;
uint8_t mbSNO;
void MB_ReadHoldingRegister(unsigned char slaveAddress, unsigned int functionCode,unsigned int Register,unsigned int numberofreg)
{

	for( mbSNO=0; mbSNO < MAXIMUM_SENSOR ; mbSNO++ )
	{
		array_mb_temp[mbSNO] = GetSensorTemperature(mbSNO);
	}
	MBSensorStatus = SensorFaultStatus();
	if(	functionCode == MB_FUNCTION_READ)
	{
		if(numberofreg>=125 || numberofreg <= 0)
		{//0x0001 <= number of register  >= 0x007D .
			MB_Exception(functionCode,3);
		}
		else
		{
			int i=0;
			for( i= 0 ; i < numberofreg*2; i = i + 2)
			{
				ReadHoldingRegister=Register+(i/2);
				MB_txFrame[0] = slaveAddress;
				MB_txFrame[1] = MB_FUNCTION_READ;
				if(ReadHoldingRegister==Pho_REGISTER_ADC_1_1)
				{
					chr_mb_temp = (unsigned char*) &array_mb_temp[0];/*64-bit double value divided to 8-bit each value*/
					chr_mb_temp = chr_mb_temp + 7;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_1_2)
				{
					chr_mb_temp = (unsigned char*) &array_mb_temp[0];
					chr_mb_temp = chr_mb_temp + 5;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
			  else if(ReadHoldingRegister==Pho_REGISTER_ADC_1_3)
				{
					chr_mb_temp = (unsigned char*) &array_mb_temp[0];
					chr_mb_temp = chr_mb_temp + 3;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_1_4)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[0];
					chr_mb_temp=chr_mb_temp+1;	
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_2_1)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[1];
					chr_mb_temp=chr_mb_temp+7;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_2_2)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[1];
					chr_mb_temp=chr_mb_temp+5;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
			  else if(ReadHoldingRegister==Pho_REGISTER_ADC_2_3)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[1];
					chr_mb_temp=chr_mb_temp+3;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_2_4)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[1];
					chr_mb_temp=chr_mb_temp+1;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_3_1)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[2];
					chr_mb_temp=chr_mb_temp+7;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_3_2)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[2];
					chr_mb_temp=chr_mb_temp+5;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
			  else if(ReadHoldingRegister==Pho_REGISTER_ADC_3_3)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[2];
					chr_mb_temp=chr_mb_temp+3;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_3_4)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[2];
					chr_mb_temp=chr_mb_temp+1;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_4_1)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[3];
					chr_mb_temp=chr_mb_temp+7;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_4_2)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[3];
					chr_mb_temp=chr_mb_temp+5;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
			  else if(ReadHoldingRegister==Pho_REGISTER_ADC_4_3)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[3];
					chr_mb_temp=chr_mb_temp+3;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_4_4)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[3];
					chr_mb_temp=chr_mb_temp+1;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_5_1)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[4];
					chr_mb_temp=chr_mb_temp+7;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_5_2)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[4];
					chr_mb_temp=chr_mb_temp+5;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
			  else if(ReadHoldingRegister==Pho_REGISTER_ADC_5_3)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[4];
					chr_mb_temp=chr_mb_temp+3;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_5_4)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[4];
					chr_mb_temp=chr_mb_temp+1;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_6_1)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[5];
					chr_mb_temp=chr_mb_temp+7;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_6_2)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[5];
					chr_mb_temp=chr_mb_temp+5;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
			  else if(ReadHoldingRegister==Pho_REGISTER_ADC_6_3)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[5];
					chr_mb_temp=chr_mb_temp+3;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_6_4)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[5];
					chr_mb_temp=chr_mb_temp+1;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_7_1)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[6];
					chr_mb_temp=chr_mb_temp+7;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_7_2)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[6];
					chr_mb_temp=chr_mb_temp+5;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
			  else if(ReadHoldingRegister==Pho_REGISTER_ADC_7_3)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[6];
					chr_mb_temp=chr_mb_temp+3;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_7_4)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[6];
					chr_mb_temp=chr_mb_temp+1;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_8_1)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[7];
					chr_mb_temp=chr_mb_temp+7;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_8_2)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[7];
					chr_mb_temp=chr_mb_temp+5;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
			  else if(ReadHoldingRegister==Pho_REGISTER_ADC_8_3)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[7];
					chr_mb_temp=chr_mb_temp+3;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(ReadHoldingRegister==Pho_REGISTER_ADC_8_4)
				{
					chr_mb_temp=(unsigned char*) &array_mb_temp[7];
					chr_mb_temp=chr_mb_temp+1;
					MB_txFrame[3+i] = (*chr_mb_temp);
					chr_mb_temp--;
					MB_txFrame[4+i] = (*chr_mb_temp);
					chr_mb_temp--;
				}
				else if(Register==Pho_REGISTER_RELAY_1)
				{
					MB_txFrame[3+i] = 0x00;
					MB_txFrame[4+i] = Pho_REGISTER_RELAY_1;
				}
				else if(Register==Pho_REGISTER_RELAY_2)
				{
					MB_txFrame[3+i] = 0x00;
					MB_txFrame[4+i] = Pho_REGISTER_RELAY_2;
				}
				else if(Register==Pho_REGISTER_TEST_HEADER_ON)
				{
					MB_txFrame[3+i] = 0x00;
					MB_txFrame[4+i] = Pho_REGISTER_TEST_HEADER_ON;
				}
				else if(Register==Pho_REGISTER_TEST_HEADER_OFF)
				{
					MB_txFrame[3+i] = 0x00;
					MB_txFrame[4+i] = Pho_REGISTER_TEST_HEADER_OFF;
				}
				else if(Register==Pho_REGISTER_TEST_FAN_ON)
				{
					MB_txFrame[3+i] = 0x00;
					MB_txFrame[4+i] = Pho_REGISTER_TEST_FAN_ON;
				}
				else if(Register==Pho_REGISTER_TEST_FAN_OFF)
				{
					MB_txFrame[3+i] = 0x00;
					MB_txFrame[4+i] = Pho_REGISTER_TEST_FAN_OFF;
				}
				else if(Register==STATUS_FAULT_FLAG_REGISTER)
				{
					MB_txFrame[3+i] = ((STATUS_FAULT_FLAG & MBSensorStatus) >> 8) & 0xFF; 		
					MB_txFrame[4+i] =	(STATUS_FAULT_FLAG & MBSensorStatus) & 0xFF;
				}
			}
		}
		MB_txFrame[2] = numberofreg * 2;
		crc16 = MB_calculateCRC(MB_txFrame, 3 + (numberofreg *2));
		MB_txFrame[3 + (numberofreg *2)] = crc16 >> 8;
		MB_txFrame[4 + (numberofreg *2)] = crc16 &0xFF;
		MB_SendDataArray(MB_txFrame, 5 + (numberofreg *2));
	}
}

void MB_Write(unsigned char slaveAddress, unsigned int functionCode,unsigned int Register)
{
	unsigned short crc16;
	
	if(Register==Pho_REGISTER_RELAY_1)
	{
		MB_txFrame[0] = slaveAddress;
		MB_txFrame[1] = functionCode;
		MB_txFrame[2] = (Pho_REGISTER_RELAY_1 >> 8) & 0xFF; 		
		MB_txFrame[3] = Pho_REGISTER_RELAY_1 & 0xFF;
		MB_txFrame[4] = 0x00;// MB_txFrame[5] = GetRelay1;
		MB_txFrame[4] = 0x00;// MB_txFrame[5] = GetRelay1;
		MB_txFrame[6] = crc16 >> 8; 	
		MB_txFrame[7] = crc16 & 0xFF;
		MB_SendDataArray(MB_txFrame, 8);
	}
	if(Register==Pho_REGISTER_RELAY_2)
	{
		MB_txFrame[0] = slaveAddress;
		MB_txFrame[1] = functionCode;
		MB_txFrame[2] = (Pho_REGISTER_RELAY_2 >> 8) & 0xFF; 		
		MB_txFrame[3] = Pho_REGISTER_RELAY_2 & 0xFF;
		MB_txFrame[4] = 0x00;// MB_txFrame[5] = GetRelay2;
		MB_txFrame[4] = 0x00; //MB_txFrame[5] = GetRelay2;
		MB_txFrame[6] = crc16 >> 8; 	
		MB_txFrame[7] = crc16 & 0xFF;
		MB_SendDataArray(MB_txFrame, 8);
	}
}
void MB_Exception( unsigned int functionCode,unsigned int Exception_Code ){
	unsigned short crc16;
	MB_txFrame[0] = Pho_GetComboxMBAddr();
	MB_txFrame[1] = functionCode+0x40;// function code  + 0x40 (modbus protocol)
	MB_txFrame[2] = Exception_Code;
	crc16 = MB_calculateCRC(MB_txFrame, 3);
	MB_txFrame[3] = crc16 >> 8; 	
	MB_txFrame[4] = crc16 & 0xFF;
	MB_SendDataArray(MB_txFrame, 5);
}

static unsigned char MB_isReceivedFrameValid(unsigned char *frame, unsigned char frameSize){

	CalculatedCRC = MB_calculateCRC(frame, frameSize - 2);

	ReceivedCRC = ((frame[frameSize - 2] << 8) | frame[frameSize - 1]);

	if (CalculatedCRC == ReceivedCRC)
		return 1;
	else
		return 0;
}

