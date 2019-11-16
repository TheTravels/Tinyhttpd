//#include "parse.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/param.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>
#include <dirent.h>


//void init_daemon(void)
//{
//	//int i;
//	pid_t pid;
//	struct sigaction sa;
//	umask(0);
//	pid=fork();
//	if(pid>0)
//		exit(0);
//	else if(pid<0)
//		exit(1);
//	setsid();

//	sa.sa_handler=SIG_IGN;
//	sigemptyset(&sa.sa_mask);
//	sa.sa_flags=0;
//	sigaction(SIGHUP,&sa,NULL);

//#if 1
//	pid=fork();
//	if(pid>0)
//		exit(0);
//	else if(pid<0)
//		exit(1);
//#if 0
//	for(int i=0;i<NOFILE;++i)
//		close(i);
//#endif
//#endif
//	//chdir("/");
//	//chdir("/home/public/server");
//	//chdir(_daemon_path);
//}
