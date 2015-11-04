#ifndef __MASTER_CLUSTER_H__
#define __MASTER_CLUSTER_H__

#include <vector>
#include "config.h"
#include "arp_detector.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

class ArpDetector;

class master_cluster : boost::noncopyable
{
public:

void start_up();

private:
	typedef boost::ptr_vector<ArpDetector> DETECT_BUF;
	typedef DETECT_BUF::auto_type DETECT_BUF_MEM_PTR;
	Config_parser 	config_;
	DETECT_BUF 		detector_buf_;
};

#endif