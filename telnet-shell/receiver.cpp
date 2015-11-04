#include "receiver.h"
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp> 
#include <iostream>
#include <stdio.h>
#include "stream_buffer.h"
#include "tcp_server.h"
#include "epoller.h"
#include "logging.h"
using namespace ydx;


Receiver::Receiver(InetAddress& serverAddr,  
					  boost::shared_ptr<EPollPoller>& ep,
					  std::string name)
	:epoller_(ep),
	 stream_buffer_(new StreamBuffer()),
	 thread_poll_(boost::bind(&Receiver::thread_start_epoll, this, serverAddr, name)),
	 cond_(mutex_),
	 msg_count_(0)
	
{
	stream_buffer_->alloc();
	connectionCallback_ = boost::bind(&Receiver::onConnection, this, _1);
	messageCallback_ = boost::bind(&Receiver::onMessage, this, _1, _2);
}


Receiver::~Receiver()
{

}



void Receiver::onConnection(const TcpConnectionPtr& conn)
{
	  LOG_INFO << "command Server - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");


}

void Receiver::onMessage(const TcpConnectionPtr& conn, Buffer* buf)
{

	
}


bool Receiver::receive(void* pData, int& nSize)
{
	bool bRet = false;

	MutexLockGuard lock(mutex_);
	if(msg_count_ == 0)
	{
		cond_.wait();
	}

	bRet = stream_buffer_->read_buffer(pData, nSize);
	if(bRet)
	{
		--msg_count_; 
	}
	
	return bRet;	
}

void Receiver::start()
{
	thread_poll_.start();
}

void Receiver::thread_start_epoll(InetAddress& serverAddr, 
									   std::string &name)

{
	server_  = boost::make_shared<TcpServer>(epoller_.get(), serverAddr, name);	
	//server_->setConnectionCallback(boost::bind(&Receiver::onConnection, this, _1));
	//server_->setMessageCallback(boost::bind(&Receiver::onMessage, this, _1, _2));
	server_->setConnectionCallback(connectionCallback_);
	server_->setMessageCallback(messageCallback_);
	server_->start();
	epoller_->poll();
}