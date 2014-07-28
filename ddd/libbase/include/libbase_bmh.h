/*!
 * \file libbase_bmh.c
 * \brief 单模式匹配算法
 *
 * \author Adam Xiao (iefcu), iefcuxy@gmail.com
 * \date 10/08/2012 10:50:17 AM
 */

#ifndef BMH_H_2010_5_22
#define BMH_H_2010_5_22

#include <stdint.h>
#include <sys/cdefs.h>

typedef intptr_t bmh_handle;

__BEGIN_DECLS
/* __bmh_chr
 * 在buf开始的nlen个字节中搜索bmh指明的关键字
 * 返回值指向buf中的关键字开始位置，如果未匹配返回NULL
 */
const unsigned char * bmh_chr(bmh_handle bmh,
        const unsigned char * buf, int len);
const unsigned char * bmh_chrlower(bmh_handle bmh,
        const unsigned char * buf, int len);
bmh_handle bmh_init(const unsigned char * patt, int len);
void bmh_uninit(bmh_handle bmh);

__END_DECLS

#endif
