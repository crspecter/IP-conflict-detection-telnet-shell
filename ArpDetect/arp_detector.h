#ifndef __ARP_DETECOTR_H__
#define __ARP_DETECOTR_H__
#include "stream_buffer.h"
#include "log_file.h"
#include "arp_decoder.h"
#include "eth_decoder.h"
#include <vector>
#include <map>
#include "arp_detector.h"
#include "thread.h"
#include "ydx_condition.h"
#include "pfring.h"
#include "ydx_atomic.h"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>



using namespace ydx;

class ArpDetector : boost::noncopyable
{
public:
	ArpDetector(const std::string dev);
	~ArpDetector()
	{

	}

	void StartUp();
	
private:
	void thread_process(); // ��kfifo�ж�ȡarp������ip��⴦��
	void thread_pfring();	//������ץȡ�������뻺����
	void ParseData(uint8_t* buf, int buflen);
	boost::scoped_ptr<eth_decoder>       eth_decoder_;		//arp������
	boost::scoped_ptr<ArpDecoder>        arp_decoder_;		//arp������
	boost::scoped_ptr<StreamBuffer> stream_buffer_; 	//����������
	boost::scoped_ptr<LogFile>      log_file_; 		//��⵽��ͻipд���ļ�
	std::string dev_name_;
	Thread thread_process_;
	Thread thread_pfring_;
	mutable MutexLock mutex_;
	Condition cond_;
	pfring *ring_;
	AtomicInt64 atomic_stat_;
	std::map<std::vector<uint8_t>, std::vector<uint8_t> > buffer_;
};

#endif