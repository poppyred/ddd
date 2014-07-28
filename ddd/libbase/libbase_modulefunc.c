#ifndef __LIBWNS_MODULEFUNC_H__
#define __LIBWNS_MODULEFUNC_H__
#include<unistd.h>
//#include <execinfo.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/file.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>
#include <linux/types.h>
#include <stdlib.h>
#include <stdio.h>
#include "libbase_daemon.h"
#include "libbase_comm.h"
#include "libbase_type.h"
#include "libbase_strutils.h"
#include <inttypes.h>

#define MAX_LENGTH_PID	16
#define LOCK_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
static int _lockfile(int fd)
{
	struct flock fl;
	fl.l_type = F_WRLCK;
	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return fcntl(fd, F_SETLK, &fl);
}
/* 初始化linux守护进程 */
int32_t _wns_init_daemon(char *path)
{
	int pid;
	//int i;
	pid = fork();
	if(pid > 0)
		exit(0);
	else if(pid < 0)
		return -1;
	//	exit(1);

	setsid();
	pid = fork();
	if(pid > 0)
		exit(0);
	else if(pid < 0)
		return -1;
	//		exit(1);

	/*  for(i = 0; i < NOFILE; ++i)
		close(i);
     */
    close(STDIN_FILENO); /*  parasoft-suppress BD-RES-INVFREE-1 */
    close(STDOUT_FILENO); /*  parasoft-suppress BD-RES-INVFREE-1 */
    close(STDERR_FILENO); /*  parasoft-suppress BD-RES-INVFREE-1 */
	open("/dev/null",O_RDWR);
	dup2(0,1);
	dup2(0,2);
    	/* 改变工作目录到/tmp */
	if (chdir(path ? path : "/tmp") != 0)
        	return -2;
	//	chdir("/");
	umask(0);
	return 0;
}

int32_t _wns_already_running(char *path)
{
	int32_t ret = 0;
	int32_t fd;
	char buf[16];
	memset(buf,0,sizeof(buf));
	if ((fd = open((const char*)path, O_RDWR|O_CREAT, LOCK_MODE)) < 0)
	{
		printf("can't open %s:%s", path, strerror(errno));
		exit(1);
	}
	int32_t file_flag = fcntl(fd, F_GETFD);
	file_flag |= FD_CLOEXEC;
	if (fcntl(fd, F_SETFD, file_flag) < 0)
	{
		printf( "already_running, fcntl failed:%s", strerror(errno));
	}

	if (_lockfile(fd) < 0)
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			close(fd);
			return -1;
		}
		printf("can't lock %s:%s", path, strerror(errno));
		exit(1);
	}
	ret = ftruncate(fd, 0);
	if (ret < 0)
	{
		return -1;
	}
	int64_t pid_len=(int64_t)getpid();
	sprintf_n(buf, sizeof(pid_len),"%"PRIu64"", pid_len);
//	sprintf_n(buf, sizeof(buf),"%lu", pid_len);
	ret = write(fd, buf, strlen(buf)+1);
	if (ret < 0)
	{
		return -1;
	}
	
	return 0;
}
int32_t _wns_parse_arg(int argc, char* argv[])
{
	for(int i=1; i < argc; i++)
	{
		if(strcmp("app", argv[i]) == 0)
			return 1;
	}
	return 0;
}


#endif

