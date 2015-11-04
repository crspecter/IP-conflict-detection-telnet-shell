#include "arp_detector.h"
#include <boost/bind.hpp>
#include <string>
#include "Pcap.h"
#include "config.h"
#define PF_CAP_LEN 1600

ArpDetector::ArpDetector(const std::string dev)
				:eth_decoder_(new eth_decoder()),
				 arp_decoder_(new ArpDecoder()),
				 stream_buffer_(new ydx::StreamBuffer()),
				 log_file_(new ydx::LogFile(5 * 1024 * 1024, "ip_conflict")),
				 dev_name_(dev),
				 thread_process_(boost::bind(&ArpDetector::thread_process, this), "arp_processor"),
				 thread_pfring_(boost::bind(&ArpDetector::thread_pfring, this), "arp_pfring"),
		         mutex_(),
	             cond_(mutex_)
{	
	stream_buffer_->alloc();
	ring_ = pfring_open(dev_name_.c_str(), PF_CAP_LEN, PF_RING_PROMISC | PF_RING_TIMESTAMP);
	if(NULL == ring_)
	{
		printf("open %s device failed\n", dev_name_.c_str());
		exit(1);
	}
}

void ArpDetector::StartUp()
{
	thread_process_.start();
	thread_pfring_.start();
}

enum CAP
{
	CAP_NETWORK_DEVICE = 0,
	READ_PCAP_FILE 	   = 1,	
};

void ArpDetector::thread_pfring()
{

	Config_parser conf;
	conf.parse_file();
	
	uint8_t cap_status = conf.get_int("detector", "cap_status", 0);
	
	if( CAP_NETWORK_DEVICE == cap_status )
	{
		int len = 2048;
	    u_char *buffer = new u_char[len];
	    struct pfring_pkthdr *hdr = (pfring_pkthdr *)buffer;
	    u_char* lpPkt = buffer + sizeof(struct pfring_pkthdr);

	    int waitable = 1;           /* 0-->non-blocking, otherwise blocking*/
	    //u_int8_t level = 4;         /* 2 of OSI-7*/
	    //u_int8_t addtimestamp = 1;  /* 1 --> add timetamp */
	    //u_int8_t addhash = 0;       /* 0 --> don't add hash */
		
		pfring_enable_ring(ring_);

		for(;;)
		{
			int ret = pfring_recv(ring_, (u_char**)&lpPkt, PF_CAP_LEN, hdr, waitable);
			if( ret > 0 )
			{
				MutexLockGuard lock(mutex_);
				stream_buffer_->write_buffer((char*)lpPkt, hdr->caplen);
			}
			if(atomic_stat_.getAndAdd(1) == 0)
			{
				cond_.notify();
			}
		}
	}
	
	else if( READ_PCAP_FILE == cap_status )
	{
		std::string file = conf.get_string("detector", "file_name", "test.pcap");
		Pcap pcap;
		pcap.open_offline(file.c_str());

		int len = 2048;
	    u_char *buffer = new u_char[len];
	    struct pfring_pkthdr *hdr = (pfring_pkthdr *)buffer;
	    u_char* lpPkt = buffer + sizeof(struct pfring_pkthdr);
		lpPkt = const_cast<u_char*>(pcap.next_packet(hdr));	
		
		while(NULL != lpPkt)
		{
			
			{
				MutexLockGuard lock(mutex_);
				stream_buffer_->write_buffer((char*)lpPkt, hdr->caplen);
			}	
			if(atomic_stat_.getAndAdd(1) == 0)
			{
				cond_.notify();
			}			
			lpPkt = const_cast<u_char*>(pcap.next_packet(hdr));	
		}
		printf("read pcap file end..\n");
		exit(0);
	}
	
}


void ArpDetector::thread_process()
{
	uint8_t *buf = new uint8_t[2048];
	int len = 2048;
	while(1)
	{
		bool bRet = false;	
		{
			MutexLockGuard lock(mutex_);
			if(atomic_stat_.get() == 0)
			{
				cond_.wait();
			}
			bRet = stream_buffer_->read_buffer(buf, len);
		}

		if(bRet)
		{
			atomic_stat_.decrement(); 
		}	
		///////结束临界区操作/////
		ParseData(buf, len);
	}
}

void ArpDetector::ParseData(uint8_t* buf, int buflen)
{
	bool ret = false;
	
	if((ret = eth_decoder_->Decode(buf, buflen)) != true) 
		return;
	//非arp包直接返回
	if (eth_decoder_->packet_type != EETHPLYTPE_ARP)
		return;

	if((ret = arp_decoder_->Decode(eth_decoder_->payload_data_offset, 
				eth_decoder_->payload_data_len)) != true)
		return;
	//对解码结果进行匹配
	//只对应答包做分析
	if(arp_decoder_->Opcode != 0x02) //0x01请求 0x02应答
		return;
	
	std::vector<uint8_t> ip(arp_decoder_->Sender_IP, arp_decoder_->Sender_IP + 4);
	std::vector<uint8_t> mac(arp_decoder_->Sender_Mac, arp_decoder_->Sender_Mac + 6);
	std::map<std::vector<uint8_t>, std::vector<uint8_t> >::const_iterator it;
	/*
	{
		char output[512];
		snprintf(output, 512, "device : %02x:%02x:%02x:%02x:%02x:%02x,"
			  " IP:%d.%d.%d.%d\n", 
			  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], 
			  ip[0], ip[1], ip[2], ip[3]);
		log_file_->append(output, strlen(output));
		log_file_->flush();
	}
	*/	
	if((it = buffer_.find(ip)) == buffer_.end())
	{		
		buffer_.insert(std::move(std::pair<std::vector<uint8_t>, 
					std::vector<uint8_t> >(std::move(ip), std::move(mac))));
	}
	
	else
	{
		//ip相同而mac不同时，判断ip冲突，写入文件中
		if(mac != it->second)
		{
			char output[512];
			snprintf(output, 512, "device A: %02x:%02x:%02x:%02x:%02x:%02x,"
				  " device B: %02x:%02x:%02x:%02x:%02x:%02x  IP:%d.%d.%d.%d\n", 
				  it->second[0], it->second[1],it->second[2], it->second[3], it->second[4], it->second[5], 
				  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], 
				  ip[0], ip[1], ip[2], ip[3]);
			log_file_->append(output, strlen(output));
			log_file_->flush();
			///////////////screen print/////////////////////////
			printf("%s", output);
		}
	}
}