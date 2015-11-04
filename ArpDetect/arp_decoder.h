#ifndef __ARP_DECODER_H__
#define __ARP_DECODER_H__
#include <stdint.h>
#include <string.h>
#pragma pack(1)
typedef struct _ARPHEADER
{
		uint16_t Hardware_type;
		uint16_t Protocol_type;
		uint8_t  Hardware_size;
		uint8_t  Protocol_size;
		uint16_t Opcode;
		uint8_t 	 Sender_Mac[6];
		uint8_t     Sender_IP[4];
		uint8_t	 	Target_Mac[6];
		uint8_t 	 Target_IP[4];
}ARPHEADER, *LPARPHEADER;

#pragma pack()

class ArpDecoder
{
public:
	void init()
	{
		memset(this, 0, sizeof(*this));
	}
	bool Decode(uint8_t *pcode, uint32_t uiLen);
	
	uint16_t Hardware_type;
	uint16_t Protocol_type;
	uint8_t  Hardware_size;
	uint8_t  Protocol_size;
	uint16_t Opcode;
	uint8_t 	 Sender_Mac[6];
	uint8_t     Sender_IP[4];
	uint8_t	 	Target_Mac[6];
	uint8_t 	 Target_IP[4];
};


#endif
