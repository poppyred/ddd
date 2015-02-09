#include "fio_log.h"
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <stdarg.h>

static void fio_log_backup(struct fio_log *fl)
{
    if(fl->m_fp)
    {
        fclose(fl->m_fp);
        char bak_name[MAX_FIO_PATH] = "";
        time_t now = time(0);   
        struct tm t1;
        struct tm *ts = localtime_r(&now, &t1);  
        char szBakPath[MAX_FIO_PATH]={0};
        char *nPos=strrchr(fl->m_fname, '/');
        snprintf(szBakPath, sizeof(szBakPath), "%s/%04d%02d%02d",
                fl->m_logpath, ts->tm_year+1900, ts->tm_mon+1, ts->tm_mday);
        szBakPath[MAX_FIO_PATH-1] = 0;
        if(access(szBakPath, 0) )
            mkdir( szBakPath, 0777 );
        if( ts && nPos != NULL)
        {
            snprintf( bak_name, sizeof(bak_name), "%s/%s.%04d%02d%02d%02d%02d%02d",             
                    szBakPath, nPos+1,
                    ts->tm_year+1900, ts->tm_mon+1, ts->tm_mday, 
                    ts->tm_hour, ts->tm_min, ts->tm_sec);
            bak_name[MAX_FIO_PATH-1] = 0;
            fl->m_lines = 0;
            unlink(bak_name);
            rename(fl->m_fname, bak_name);
        }
        fl->m_fp = fopen(fl->m_fname, "wt");
    }
}

static int fio_log_open(struct fio_log *fl)
{
    FIO_LOG_PRLOCK(fl->lock);
    if (fl->m_fp)
    {
        fclose(fl->m_fp);
        fl->m_fp = NULL;
    }

    if (strlen(fl->m_fname) < 1)
    {
        FIO_LOG_PRUNLOCK(fl->lock);
        return -1;
    }

    if(access(fl->m_logpath, 0) )
        mkdir(fl->m_logpath, 0777);

    if (access(fl->m_fname, 0)==0)//exists then backup
    {   
        char fn_bak[MAX_FIO_PATH]="";

        time_t now = time(0);   
        struct tm t1;
        struct tm *ts = localtime_r(&now, &t1);  
        char szBakPath[MAX_FIO_PATH]={0};
        char *nPos=strrchr(fl->m_fname, '/');
        snprintf(szBakPath, sizeof(szBakPath), "%s/%04d%02d%02d",
                fl->m_logpath, ts->tm_year+1900, ts->tm_mon+1, ts->tm_mday);
        szBakPath[MAX_FIO_PATH-1] = 0;
        if(access(szBakPath, 0) )
            mkdir( szBakPath, 0777 );
        if( ts && nPos != NULL)
        {
            snprintf( fn_bak, sizeof(fn_bak), "%s/%s.%04d%02d%02d%02d%02d%02d",             
                    szBakPath, nPos+1,
                    ts->tm_year+1900, ts->tm_mon+1, ts->tm_mday, 
                    ts->tm_hour, ts->tm_min, ts->tm_sec);
            fn_bak[MAX_FIO_PATH-1] = 0;
        }

        unlink( fn_bak ) ;
        rename( fl->m_fname, fn_bak);
    }
    fl->m_fp = fopen(fl->m_fname, "wt");

    FIO_LOG_PRUNLOCK(fl->lock);
    return 0;
}

static void fio_log_close(struct fio_log *fl)
{
    FIO_LOG_PRLOCK(fl->lock);

    if(fl->m_fp)
    {
        time_t curr_time = time(NULL);
        struct tm *pt = NULL;
        struct tm t1;
        pt = localtime_r(&curr_time, &t1);
        fprintf(fl->m_fp, "%02d%02d %02d:%02d:%02d NOTE: LOG FILE CLOSE!!!\n",
                pt->tm_mon+1, pt->tm_mday, pt->tm_hour, pt->tm_min, pt->tm_sec);
        fclose(fl->m_fp);
        fl->m_fp = NULL;
    }

    FIO_LOG_PRUNLOCK(fl->lock);
}

static void fio_log_print(struct fio_log *fl, const char *format, ...)
{
    FIO_LOG_PRLOCK(fl->lock);
    if (fl->m_lines > fl->m_maxlines)
        fl->vtbl.backup(fl);
    if(!fl->m_fp)
    {
        FIO_LOG_PRUNLOCK(fl->lock);
        return;
    }

    time_t curr_time = time(NULL);
    struct tm *pt = NULL;
    struct tm t1;
    pt = localtime_r(&curr_time, &t1);
    if( pt )
    {
        va_list ap;
        va_start(ap, format);
        fprintf(fl->m_fp, "%02d%02d %02d:%02d:%02d ",
                pt->tm_mon+1,pt->tm_mday,
                pt->tm_hour,pt->tm_min,pt->tm_sec);
        char buffer[1024] = {0};
        vsnprintf(buffer, 1023, format, ap);
        buffer[1023] = 0;
        fprintf(fl->m_fp, "%s", buffer);
        va_end(ap);
        fflush(fl->m_fp);
        fl->m_lines++;
    }
    FIO_LOG_PRUNLOCK(fl->lock);
}

int fio_log_init(struct fio_log *fl, char *logpath, char *fname)
{
    fl->vtbl.open = fio_log_open;
    fl->vtbl.print = fio_log_print;
    fl->vtbl.close = fio_log_close;
    fl->vtbl.backup = fio_log_backup;
    fl->m_fp = NULL;
    fl->lock = 0;
    fl->m_lines = 0;
    fl->m_maxlines = LOG_MAX_LINE;
    strncpy(fl->m_logpath, logpath, sizeof(fl->m_logpath));
    snprintf(fl->m_fname, sizeof(fl->m_fname), "%s/%s", fl->m_logpath, fname);
    return fl->vtbl.open(fl);
}

