/**
* @filename   	dns_tool.h
* @brief  		一些公用的方法
* @author hezuoxiang      @date 2013/07/23
**/



/**
* @brief   安全内存拷贝
* @param   dst           	目标
* @param   dst_len       	目标缓冲长度
* @param   src				源
* @param   src_len          要拷贝的长度
*
* @return  0 成功，-1 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/23
**/
int memcpy_safe(void*dst,int dst_len,void * src,int src_len);

/**
* @brief   安全内存拷贝
* @param   filename           	文件名
* @param   content       		要写入的内容
* @param   maxlen				内容的最大长度
*
* @return  0 成功，-1 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/23
**/
int write_file(char*filename,char*content,int maxlen);

