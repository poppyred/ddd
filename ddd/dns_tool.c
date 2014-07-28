#include "dns_tool.h"
#include <string.h>
#include <stdio.h>



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

int memcpy_safe(void*dst,int dst_len,void * src,int src_len)
{
	if(dst_len<src_len)
	{
		return -1;
	}
	memcpy(dst,src,src_len);
	return 0;
}

/**
* @brief   安全内存拷贝
* @param   filename           	文件名
* @param   content       		要写入的内容
* @param   len					内容长度
*
* @return  0 成功，-1 失败
* @remark null
* @see     
* @author hezuoxiang      @date 2013/07/23
**/
int write_file(char*filename,char*content,int len)
{
	FILE*file = fopen(filename, "w");
	if(file == NULL)
	{
		return -1;
	}
	
	fwrite(content, len, 1, file); 
	
	fclose(file);
	return 0;
}
