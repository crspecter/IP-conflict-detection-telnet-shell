#include "daemon.h"
#include <unistd.h>
#include <stdlib.h>
#include <cstring>
#include <sys/param.h> 
#include <sys/wait.h>
#include <sys/types.h> 
#include <sys/stat.h> 
#include <stdio.h>
#include <getopt.h>
#include <string>
#include "time.h"
//#include <syslog.h>
#include <signal.h>
#include "backtrace.h"

#include <boost/scoped_ptr.hpp>
#include "async_logging.h"
#include "logging.h"

using namespace ydx;

boost::scoped_ptr<ydx::AsyncLogging> g_async_log;

void asyncOutput(const char* msg, int len)
{
  	g_async_log->append(msg, len);
}

init_dae::init_dae()
{

}

FILE* init_dae::check_process()
{

    FILE *pstr, *fpid ; char cmd[128]={0}, buff[512]={0}, pid_buff[24] ={0}, *p;

	
	int pid_n, pid_now;
	if((fpid = fopen("./pid.log", "a+")) == NULL)
	{
		printf("fopen error \n");
		return NULL;		
	}

    fread(pid_buff, sizeof(pid_buff), 1, fpid); 
	
	pid_n = atoi(pid_buff);	

	if( pid_n == 0 )  return fpid;




	do
	{
		sprintf(cmd, "ps -ef|grep %d ", pid_n);
		pstr=popen(cmd, "r");
	    if(pstr==NULL)
	    {
	    	exit(1); 
		}
	    memset(buff,0,sizeof(buff));
	    fgets(buff,512,pstr);
	    p=strtok(buff, " ");
	    p=strtok(NULL, " "); //����Ƿ�ȥ����ȡ���ڵ�ǰϵͳ��ps�󣬽���ID���Ƿ��ǵ�һ���ֶ� pclose(pstr);	

		if((NULL == p) || ( 0 == strlen(p))) 
		{
			exit(1) ;
		}

		pid_now = atoi(p);

		if(pid_now == pid_n)
		{
			printf("process exist : %d\n", pid_now);	
			exit(1);
		}
	}while(0);

    pclose(pstr);
	return fpid;

}


void init_dae::write_pid_log(FILE *fp)
{
	int pid, fd;
	rewind(fp);
	fd = fileno(fp);
	ftruncate(fd, 0);	
	pid = getpid();
	char pid_buf[24];
	snprintf(pid_buf, sizeof(pid_buf), "%d", pid);
	fwrite(pid_buf, strlen(pid_buf), 1 , fp);
	fclose(fp);

}

void init_dae::init_daemon(char *pname)
{
#define FORK_FAIL "fork failed !(%s:,%d,  %s) \n"
#define CHECK_FAIL "check failed !(%s:,%d,  %s) \n"
	
	


	int pid;
	if( (pid = fork()) > 0 ) //parent
	{
		exit(0);
	}
		
	else if ( pid < 0 )
	{
		printf(FORK_FAIL , __FILE__, __LINE__, __FUNCTION__);
		exit(1);
	}
	//child
	setsid();
	//������̨���̵�ʱ�򣬸����̲��˳�������waitpid����̨�ӽ����Ƿ�ҵ���ɱ��
	//waitpid���غ���ѭ������������
	
	while(true)
	{
		if( (pid = fork()) == 0 ) //child ֱ�ӷ��ؿ�ʼ��ʽ������
		{
			return;
		}
			
		else if( pid < 0 )
		{
			printf(FORK_FAIL , __FILE__, __LINE__, __FUNCTION__);
			exit(1);
		}
		else //parent ����exit�����������ӽ���������waitpid���أ��ٴ�forkһ���ӽ���
		{
		
			//openlog("db_update", LOG_CONS|LOG_PID, 0);
			//syslog(LOG_INFO, "db_update pid:%u start", pid);
			//closelog();
			printf("%s start..\n", pname);
			waitpid(pid, &status_, 0);
			sleep(10);
		}
			
	}
	
}


void init_dae::parse_cmd_line(int argc, char *argv[])
{


	FILE *ps = check_process();
	
	
	if(NULL == ps) exit(1);

	
	
	int opt;
	char *optstring = (char*)"N";
	bool daemon = true;

  static struct option long_options[] = {  

       {"nodae",  no_argument,       NULL, 'N'},  
       {0,0,0,0}
    
  	};

	while((opt = getopt_long(argc, argv, optstring, long_options, NULL)) != -1)
	{
		switch (opt)
		{
			case 'N':
			{
				daemon = false;
			}
			break;
			
			default:
			break;
		}
	}

	if (daemon)
	{
		init_daemon(argv[0]);
		setbacktrace(SIGSEGV);
		//������־ģ�飬�첽д��־
		g_async_log.reset(new AsyncLogging);
		g_async_log->start();
		Logger::setOutput(asyncOutput);
		//logger.start();
	}	

	write_pid_log(ps);
}



