//
// $Id: log_log.h 12633 2014-04-08 03:31:29Z likunxiang $
//

#ifndef __LOG_LOG_H__
#define __LOG_LOG_H__

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include "log_list.h"
#include "log_util.h"

#define LOG_TYPE_ERROR         0
#define LOG_TYPE_LOG           1
#define LOG_TYPE_TRACE         2

#define MAX_LOG_TYPE           3       //日志的种类数量
#define MAX_LOG_LEVEL          5		//日志的分级
#define MAX_LINE_NUM           1000000	//循环日志的行数
#define MAX_FILE_NUM           10		//循环日志的文件数
#define MAX_LOG_BUFFER         10000
#define MAX_INTERVAL_TIME      5        //顺序日志拆分时间间隔(单位：分)

typedef struct BufferNode {
	int type;
	int len;
	char buff[MAX_BUFF_LEN];
    struct log_list_head link_;
} BufferNode;

typedef struct NLog
{
    int             dopr;
    int             active_;
    pthread_t       tid_;
    time_t          cur_ts_;
    char            time_str_[2][256]; 
    int             time_len_[2];
	int				olog_fn_;				//循环日志的文件号
	int				olog_ln_;				//循环日志的行号

	int				daemon_mode_;
	char			log_path_[MAX_PATH];	//循环log，记录一定时期的log，备案

	FILE*			olog_;                  //循环日志句柄

	FILE*			log_[MAX_LOG_TYPE];   //日志文件句柄
	
	pthread_mutex_t mutex_;
    BufferNode*     buf_mem_;
    int             buf_max_;
    int             buf_new_;
	struct log_list_head write_list_;       //need clean
    struct log_list_head free_list_;        

    /*! 日志的个数 */
    pthread_mutex_t buf_count_mtx_;
    unsigned long  buf_count_;

}NLog;

NLog * NLog_create(int buf_max);
void NLog_destroy(NLog *_log);
int NLog_startup( NLog *_log, const char* _log_path, const char* _err_path, int _daemon_mode);
void NLog_shutdown(NLog *_log);
void NLog_write_file(const char* _filename, const char* format, ... );
void NLog_write_log(NLog *_log, int type, const char* format, ... );
void NLog_process_timer(NLog *_log, time_t tm_now);
void NLog_enable(NLog *_log);

/*
* 所有的log在terminal方式下运行都是输出到标准IO，在daemon方式下运行输出到文件
* 分级如下：
* LOG(1)		循环写的log
*/

#define		FLOG	NLog_write_file

#define     ERR(_log,format,...) do{if(_log->dopr)NLog_write_log(_log,LOG_TYPE_ERROR,"%s [%d] " format,__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#define     LOG(_log,format,...) do{if(_log->dopr)NLog_write_log(_log,LOG_TYPE_LOG,"%s [%d] " format,__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)
#define     TRACE(_log,format,...) do{if(_log->dopr)NLog_write_log(_log,LOG_TYPE_TRACE,"%s [%d] " format,__FUNCTION__,__LINE__,##__VA_ARGS__);}while(0)

#define LOG_PROCESS_TIMER(_log,t)  NLog_process_timer(_log, t)

#endif
