#include "master_cluster.h"
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include "arp_detector.h"
#include "logging.h"
#include "daemon.h"
void master_cluster::start_up()
{

	config_.parse_file();
	int dev_num = config_.get_int("detector", "detector_num", 1);
	if(dev_num > 16)
	{
		printf("%d device set,device number must lower than 16\n" , dev_num);
		exit(1);
	}
		
	std::string dev_list = config_.get_string("detector", "device", "eth0");

	char *src = (char*)dev_list.c_str();
	char *dev = NULL;
	char *inner_ptr = NULL;
	while((dev = strtok_r(src, ",", &inner_ptr)) != NULL )
	{
		DETECT_BUF_MEM_PTR detector(new ArpDetector(dev));
		detector_buf_.push_back(detector.release());
		src = NULL;
	}

	if(detector_buf_.empty())
	{
		printf("no Arpdetector create\n");
		exit(1);
	}
	
	for(uint32_t i = 0; i < detector_buf_.size(); i++)
	{
		detector_buf_[i].StartUp();
	}
	LOG_INFO << "Arpdetector start work...";
	
}


int main(int argc, char **argv)
{

	init_dae  dae;
	dae.parse_cmd_line(argc, argv);
	master_cluster cluster;
	cluster.start_up();
	while(1)
	{
		sleep(1);
	}
	return 0;
}