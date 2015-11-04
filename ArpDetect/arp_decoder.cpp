#include "arp_decoder.h"
#include <arpa/inet.h>
bool ArpDecoder::Decode(uint8_t * pcode, uint32_t uiLen)
{
	if((NULL==pcode) || (0==uiLen))
	{
		return false;
	}
	init();
	LPARPHEADER pheader = (LPARPHEADER)pcode;
	
	Hardware_type = ntohs(pheader->Hardware_type);
	Protocol_type = ntohs(pheader->Protocol_type);
	Hardware_size = pheader->Hardware_size;
	Protocol_size = pheader->Protocol_size;
	Opcode = ntohs(pheader->Opcode);
	memcpy(Sender_Mac, pheader->Sender_Mac, 6);
	memcpy(Sender_IP,  pheader->Sender_IP, 4);
	memcpy(Target_Mac, pheader->Target_Mac, 6);
	memcpy(Target_IP,  pheader->Target_IP, 4);
	return true;
}