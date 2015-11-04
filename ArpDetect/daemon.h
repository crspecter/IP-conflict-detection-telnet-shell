#ifndef __DAEMON_H__
#define __DAEMON_H__

#include <stdio.h>

//#include "writelog.h"



class init_dae
{
public:
	init_dae();
	FILE* check_process();
	void parse_cmd_line(int argc, char *argv[]);
	void write_pid_log(FILE *fp);
private:
	void init_daemon(char* pname);
	int status_;
	
	//write_log logger;
};

#endif