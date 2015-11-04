
#include "ups_server.h"
#include "daemon.h"
#include "config.h"
#include "logging.h"
#include <stdint.h>
#include <boost/bind.hpp>



enum CONNECTOPT
{
	ADD = 0x01,
	DEC = 0x02,
};



UPsServer::UPsServer()
	:cli_epoller_(new EPollPoller),
	 cmd_epoller_(new EPollPoller)
{
	res_msg_.reset(new response_message);
	res_msg_->init();
}

UPsServer::~UPsServer()
{

}


void UPsServer::optOnConnect(const TcpConnectionPtr& conn)
{
	  LOG_INFO << "UPsServer - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
	  if(conn->connected())
	  	optConnect(conn, ADD);
	  else
	  	optConnect(conn, DEC);  
}


void UPsServer::optOnMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
	LOG_INFO << "UPsServer optMessage - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
	(void*)buf;
}

void UPsServer::optConnect(TcpConnectionPtr conn, int opt)
{
	LOG_INFO << "UPsServer optConnect - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
	if (opt == ADD)
	{
		conn_set.insert(conn);
	}
	else if(opt == DEC)
	{
		conn_set.erase(conn);
	}
}

struct msg
{
	char command[256];
};


#define WELCOME "welcome to ups_sever \r\n"

#define USEAGE  "\r\n1--------low input valtage\r\n" \
				"2--------low output valtage\r\n" \
				"3---------set valtage status 0x01\r\n" \
				"4---------set battary status 0x03\r\n" \
				"5-------reset the whole status\r\n" \
				"6------show current status\r\n" \
				"0-------show all commands\r\n"

#define SHELL "\r\nups command shell@linux> "
#define COMMAND_SUCC "command success."
#define COMMAND_FAILED "command failed."
#define LIV 	"1"
#define LOV 	"2"
#define EV  	"3"
#define BV  	"4"
#define RESET 	"5"
#define SHOW 	"6"
#define HELP	"0"
#define ENTER   "\r\n"
#define OK		"\r\nOK!\r\n"

void UPsServer::onCommandConnection(const TcpConnectionPtr& conn)
{
	  LOG_INFO << "command Server - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");

	  if(conn->connected())
	  {
		 conn->send((char*)WELCOME, strlen(WELCOME));
		 conn->send((char*)USEAGE, strlen(USEAGE));
		 conn->send((char*)SHELL, strlen(SHELL));
	  }

}

void formatShowString(char *buf, response_message* msg)
{
	char iv[4] = {0};
	iv[0] = msg->input_voltage[2];
	iv[1] = msg->input_voltage[1];
	iv[2] = msg->input_voltage[0];
	char oi[4] = {0};
	oi[0] = msg->output_voltage[2];
	oi[1] = msg->output_voltage[1];
	oi[2] = msg->output_voltage[0];
	msg->command =(msg->command >> 8 & 0x0f)|(msg->command << 8 & 0xf0); 

	snprintf(buf, 1024, "\r\nhead----0x%x\r\ncommand--0x%x\r\n"\
	                    "input_voltage--%s\r\niv_status--%u\r\n"\
	                    "output_voltage--%s\r\nbattary_status--%u\r\n" \
	                    "tail----0x%x\r\n",
	                    msg->head, msg->command, iv, msg->iv_status,\
	                    oi, msg->battary_status, msg->tail);
}

void UPsServer::onCommandMessage(const TcpConnectionPtr& conn, Buffer* buf)
{
	  LOG_INFO << "onCommandMessage Server - " << conn->peerAddress().toIpPort() << " -> "
           << conn->localAddress().toIpPort() << " is "
           << (conn->connected() ? "UP" : "DOWN");
	struct msg message;//接受消息
	
	while(buf->readable_bytes() != 0)
	{
		 
		
		::memcpy(&message.command, buf->peek(), buf->readable_bytes() < 256? buf->readable_bytes():256);
		//对命令处理的过程
		if(memcmp(&message.command, LIV, strlen(LIV)) == 0)
		{
			res_msg_->opt_iv();
			conn->send((char*)OK, strlen(OK));
		}
		//////////////
 		else if(memcmp(&message.command, LOV, strlen(LOV)) == 0)
 		{
 			res_msg_->opt_ov();
			conn->send((char*)OK, strlen(OK));
		}
		//////////////
		else if(memcmp(&message.command, EV, strlen(EV)) == 0)
		{
			res_msg_->opt_vstatus(0x01);
			conn->send((char*)OK, strlen(OK));
		}
		//////////////
		else if(memcmp(&message.command, BV, strlen(BV)) == 0)
		{
			res_msg_->opt_bstatus(0x03);
			conn->send((char*)OK, strlen(OK));
		}
		//////////////
		else if(memcmp(&message.command, RESET, strlen(RESET)) == 0)
		{
			res_msg_->init();
			conn->send((char*)OK, strlen(OK));
		}
		//////////////
		else if(memcmp(&message.command, SHOW, strlen(SHOW)) == 0)
		{
			char show_info[1024]={0};
			formatShowString(show_info, res_msg_.get());
			conn->send(show_info, strlen(show_info));
		}
		//////////////
		else if(memcmp(&message.command, HELP, strlen(HELP)) == 0)
		{
			conn->send((char*)USEAGE, strlen(USEAGE));
			
		}		
		else
		{
			
		}
		conn->send((char*)SHELL, strlen(SHELL));
		buf->retrieve(buf->readable_bytes());

 
	}	
	
}

void UPsServer::data_tansfor()
{
	//确定超时时间
	Timestamp time(addTime(Timestamp::now(), 0.5));
	//加入定时器
	TimerId id = timer_set_->addTimer(boost::bind(&UPsServer::timer_do, this), time, 0.5);
	cli_server_->start();
	cmd_server_->start();
}

void UPsServer::timer_do()
{
	std::set<TcpConnectionPtr>::iterator it;
	for (it = conn_set.begin(); it != conn_set.end();)
	{
		if((*it)->connected())
		{
			(*it)->send(res_msg_.get(), sizeof(*res_msg_.get()));
			it++;
		}
		else
		{
			conn_set.erase(it++);
		}
	}
}

void UPsServer::setup()
{
	Config_parser conf;
	conf.parse_file();
	/////////////////////监听客户端连接//////////////////////////////
	uint32_t cli_listen = conf.get_int("server", "listen_port", 6500);
	InetAddress  server_addr("0.0.0.0", cli_listen);
	cli_server_.reset(new Receiver(server_addr, cli_epoller_));
	timer_set_.reset(new  TimerSet(cli_epoller_.get()));
	cli_server_->setConnectionCallback(boost::bind(&UPsServer::optOnConnect, this, _1));
	cli_server_->setMessageCallback(boost::bind(&UPsServer::optOnMessage, this, _1, _2));
	/////////////////////监听命令行连接//////////////////////////////
	uint32_t cmd_listen = conf.get_int("server", "cmd_port", 6501);
	InetAddress  cmd_addr("0.0.0.0", cmd_listen);
	cmd_server_.reset(new Receiver(cmd_addr, cmd_epoller_));
	cmd_server_->setConnectionCallback(boost::bind(&UPsServer::onCommandConnection, this, _1));
	cmd_server_->setMessageCallback(boost::bind(&UPsServer::onCommandMessage, this, _1, _2));
}

int main(int argc, char **argv)
{

	init_dae  dae;
	dae.parse_cmd_line(argc, argv);
	UPsServer server;
	server.setup();
	server.data_tansfor();
	while(1)
	{
		sleep(1);
	}
	return 0;
}

