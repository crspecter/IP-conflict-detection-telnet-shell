#ifndef __UPS_SERVER_H__
#define __UPS_SERVER_H__

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <set>
#include "msg_define.h"
#include "receiver.h"
#include "buffer.h"
#include "epoller.h"
#include <stdint.h>
#include "ydx_timer_set.h"
#include "ydx_timerid.h"
#include "tcp_connection.h"

using namespace ydx;

class UPsServer : boost::noncopyable
{
public:



typedef boost::shared_ptr<TcpConnection> TcpConnectionPtr;
public:
	UPsServer();
	~UPsServer();
	void setup();
	void data_tansfor();

private:
	void timer_do();
	void optOnMessage(const TcpConnectionPtr& conn, Buffer* buf);
	void optOnConnect(const TcpConnectionPtr& conn);
	void optConnect(TcpConnectionPtr conn, int opt);
	
	void onCommandConnection(const TcpConnectionPtr& conn);
	void onCommandMessage(const TcpConnectionPtr& conn, Buffer* buf);		
	
	std::set<TcpConnectionPtr> conn_set;
	
	boost::scoped_ptr<Receiver>   	cli_server_;
	boost::scoped_ptr<Receiver>   	cmd_server_;
	boost::scoped_ptr<TimerSet>  	timer_set_;
	boost::scoped_ptr<response_message>  res_msg_;
	boost::shared_ptr<EPollPoller> 	cli_epoller_;
	boost::shared_ptr<EPollPoller> 	cmd_epoller_;
};

#endif
