#ifndef __MSG_DEFINE_H__
#define __MSG_DEFINE_H__

#include <string.h>
#include <stdint.h>
#pragma pack (1) 
struct response_message
{
	response_message()
	{
		memset(this, 0, sizeof(*this));
	}
	
    void init()
    {
		head = 0xfefe;
		command = 0x0390;
		input_voltage[0] = '0';
		input_voltage[1] = '2';
		input_voltage[2] = '2';	
		iv_status = 0x00;
		output_voltage[0] = '0';
		output_voltage[1] = '2';
		output_voltage[2] = '2';
		battary_status = 1;
		tail = 0x3333;
	}
	//输入电压修改
	void opt_iv()
	{
		input_voltage[0] = '0';
		input_voltage[1] = '8';
		input_voltage[2] = '1';			
	}
	//输出电压修改
	void opt_ov()
	{
		output_voltage[0] = '0';
		output_voltage[1] = '8';
		output_voltage[2] = '1';			
	}	
	//电压状态调整
	void opt_vstatus(const uint8_t v)
	{
		iv_status = v;
	}
	//电池状态调整
	void opt_bstatus(const uint8_t v)
	{
		battary_status = v;
	}
	
	uint16_t  	head;
	uint16_t  	command;
	char 	  	input_voltage[3];	 //输入电压
	uint8_t 	iv_status;
	char 		output_voltage[3];  //输出电压
	uint8_t 	battary_status;  //电池状态
	uint16_t  	tail;
};
#pragma pack () 

#endif