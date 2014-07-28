#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdarg.h> 
#define END_P -400

#include <syslog.h>
#include <stdarg.h>
#include "Utils.h"
//#include "Storage.h"
#include "Dnslist.h"
//#include <semaphore.h>

#define MAX_DATA_SIZE  (1536)
#define MAX_LABEL_LEN (63)

#define DNS_HOST  0x01
#define DNS_CNAME 0x05

#define MAX_THREAD_NUM 10

#define LISTEN_THREAD_SIZE 4
#define ANSWER_THREAD_SIZE 4

#define SYSTEM_THREAD_NUM 8

#define DNS_CONFIG_PATH "EflyDns.conf"


//#define USE_LOG	1
#define HEAD_DISPLAY_FORMAT	"0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x "
#define HEAD_DISPLAY_VALUE(buf)	buf[0],buf[1],buf[2],buf[3],buf[4],buf[5],buf[6],buf[7],buf[8],buf[9],buf[10],buf[11]


struct temp_buf
{
	int status;  	//o/1
	char*buf;
};

/* Read_config.c*/
int read_config(const char *fname);
int action_for_day();


//***************log
void write_client_domain_log(int thread_num,char*domain,char*ip_src_address);
void read_client_domain_log(void *arg);
void get_client_domain_filename(char * filename);

#endif /* _EFLYDNSPROXY_H_ */


