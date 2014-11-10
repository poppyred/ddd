//
// $Id: fio_log2.h 12633 2014-04-08 03:31:29Z likunxiang $
//

#ifndef __FIO_LOG2_H__
#define __FIO_LOG2_H__

#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <stdarg.h>
#include "fio_list.h"
#include "fio_nm_util.h"

#define FLOG2_TYPE_ERROR         0
#define FLOG2_TYPE_LOG           1
#define FLOG2_TYPE_TRACE         2

#define MAX_FLOG2_TYPE           3       //日志的种类数量
#define MAX_FLOG2_LEVEL          5		//日志的分级
#define MAX_FLOG2_LINE_NUM           1000000	//循环日志的行数
#define MAX_FLOG2_FILE_NUM           10		//循环日志的文件数
#define MAX_FLOG2_BUFFER         10000
#define MAX_FLOG2_INTERVAL_TIME      5        //顺序日志拆分时间间隔(单位：分)
#define MAX_FLOG2_BUFF_LEN        8192

typedef struct FLOG2BufferNode{
	int type;
	int len;
	char buff[MAX_FLOG2_BUFF_LEN];
    struct fio_list_head link_;
} FLOG2BufferNode;

typedef struct FLOG2
{
    int             active_;
    pthread_t       tid_;
    time_t          cur_ts_;
    char            time_str_[2][256]; 
    int             time_len_[2];
	int				olog_fn_;				//循环日志的文件号
	int				olog_ln_;				//循环日志的行号

	int				daemon_mode_;
	char			log_path_[MAX_FIO_PATH];	//循环log，记录一定时期的log，备案

	FILE*			olog_;                  //循环日志句柄

	FILE*			log_[MAX_FLOG2_TYPE];   //日志文件句柄
	
	pthread_mutex_t mutex_;
    FLOG2BufferNode *buf_mem_;
    int             buf_max_;
    int             buf_new_;
	struct fio_list_head write_list_;       //need clean
    struct fio_list_head free_list_;        

    /*! 日志的个数 */
    pthread_mutex_t buf_count_mtx_;
    unsigned long  buf_count_;

}FLOG2;

FLOG2 * FLOG2_create(int buf_max);
void FLOG2_destroy(FLOG2 *_log);
int FLOG2_startup( FLOG2 *_log, const char* _log_path, const char* _err_path, int _daemon_mode);
void FLOG2_shutdown(FLOG2 *_log);
void FLOG2_write_file(const char* _filename, const char* format, ... );
void FLOG2_write_log(FLOG2 *_log, int type, const char* format, ... );
void FLOG2_process_timer(FLOG2 *_log, time_t tm_now);

/*
* 所有的log在terminal方式下运行都是输出到标准IO，在daemon方式下运行输出到文件
* 分级如下：
* LOG(1)		循环写的log
*/

#define		FLOG2_FLOG	FLOG2_write_file

#define     FLOG2_ERR(_log,format,...) if (sysconfig.prdebug) FLOG2_write_log(_log,FLOG2_TYPE_ERROR,"%s [%d] " format,__FUNCTION__, __LINE__,##__VA_ARGS__)
#define     FLOG2_LOG(_log,format,...) if (sysconfig.prdebug) FLOG2_write_log(_log,FLOG2_TYPE_LOG,"%s [%d] " format,__FUNCTION__, __LINE__,##__VA_ARGS__)
#define     FLOG2_TRACE(_log,format,...) if (sysconfig.prdebug) FLOG2_write_log(_log,FLOG2_TYPE_TRACE,"%s [%d] " format,__FUNCTION__, __LINE__,##__VA_ARGS__)

#define FLOG2_PROCESS_TIMER(_log,t)  FLOG2_process_timer(_log, t)

extern FLOG2 *g_log2;

#endif
