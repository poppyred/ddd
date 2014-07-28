/*
 ** $Id: mwm.h,v 1.1 2003/10/20 15:03:42 chrisgreen Exp $
 **
 **  mwm.h
 **
 ** Copyright (C) 2002 Sourcefire,Inc
 ** Marc Norton
 **
 ** Modifed Wu-Manber style Multi-Pattern Matcher
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 **
 **
 */

#ifndef __MWM_H__
#define __MWM_H__

#include <stdint.h>
#include <sys/cdefs.h>

typedef intptr_t mwm_handle;
#define SHORT_PREFIX_MATCH 0
#define LONGEST_PREFIX_MATCH 1

typedef int (*action_proc)(intptr_t udata, unsigned char * Tx, void * in,
        void * out);

__BEGIN_DECLS

/*
 ** PROTOTYPES
 */
mwm_handle apm_mwmNew(void);
void apm_mwmFree(mwm_handle pv);

void apm_mwmSetPrefixMatch(mwm_handle pv, unsigned char method);

int apm_mwmAddPatternEx(mwm_handle pv, const unsigned char * P, int len, int off,
        int deep, intptr_t ud);
int apm_mwmPrepPatterns(mwm_handle pv);

/*!
 * \brief 在指定字符串中搜索多个模式
 * @return 返回值比较奇怪, 还没有搞清楚
 * @return n:匹配到第n个模式后, 回调函数返回值非0
 * @return 1:匹配的回调函数返回非0,
 * @return 0:没有找到, 或者匹配的回调函数返回值都是0
 */
int apm_mwmSearch(mwm_handle pv,
        unsigned char * T, int n,
        void * in, void * out,
        action_proc match);

#ifdef DEBUG_MWM
// 仅供测试使用的接口
/* Not so useful, but not ready to be dumped  */
int apm_mwmGetNumPatterns(mwm_handle pv);
void apm_mwmFeatures(void);
void apm_mwmShowStats(mwm_handle pv);
void apm_mwmGroupDetails(mwm_handle pv);
uint32_t apm_mwmMemCourst();
#endif

__END_DECLS

#endif

