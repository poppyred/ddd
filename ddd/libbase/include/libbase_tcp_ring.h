/**
 * \file wns_tcp_ring.h
 * 块数据环形缓冲区实现，改造于wns_chunk_ring
 * \defgroup wns_tcp_ring 块数据环形缓冲区
 * \{
 
 * \section 使用例子--伪代码
 
#define MAX_RCV_LEN 64KB
char m_pTmpRcvBuff[MAX_RCV_LEN];

wns_tcp_ring_st *pRcvBuffer;

void OnRead(int nSessionIndex)
{
    int nLoopCount = 0;
    int nHaveRcvLen = 0;
    int nNeedRcvLen = 0;
    int nFreeLen = wns_tcp_ring_freecount(pRcvBuffer);
    if (nFreeLen <= 0)
    {
        TRACE("[TcpNode::OnRead] find the rcvBuff's  freecount <= 0\r\n");
        ManageRcvData(nSessionIndex);
        return;
    }

    do
    {
        nNeedRcvLen = min(MAX_RCV_LEN,nFreeLen);

        nHaveRcvLen = 0;
        nHaveRcvLen = recv(nsocket,m_pTmpRcvBuff,nNeedRcvLen,0);
        TRACE("----------->TcpNode onRead len = %d ------------->\r\n",nHaveRcvLen);
        if (nHaveRcvLen < 0)
        {
            if (errno != EWOULDBLOCK && errno != EINTR)
            {
                .....
            }

            return;
        }

        if (nHaveRcvLen == 0)
        {
            OnClose(nSessionIndex);
            return;
        }
        //最大限度地接收数据放到循环缓冲
        wns_tcp_ring_enqueue(pRcvBuffer, m_pTmpRcvBuff,nHaveRcvLen);
        //处理循环缓冲的数据
        ManageRcvData(nSessionIndex);

        nFreeLen = wns_tcp_ring_freecount(pRcvBuffer);

        nLoopCount++;
    }
    while ((nFreeLen>0) && (nHaveRcvLen==nNeedRcvLen) && nLoopCount<=100);

}

void ManageRcvData()
{
    int nDataLen =  wns_tcp_ring_datacount(pRcvBuffer);

    //先取一个固定长度的头
    while (nDataLen >= SINFOR_PACK_HEAD_LEN)
    {
        // 1.Get a sinfor packet header
        wns_tcp_ring_peek(pRcvBuffer, m_pTmpRcvBuff,SINFOR_PACK_HEAD_LEN);
        
        stSinforPacketHead *pSinforPacket = (stSinforPacketHead*)m_pTmpRcvBuff;

        // 2.Check if recv all sinfor packet data
        int nPacketLen = SINFOR_PACK_HEAD_LEN+pSinforPacket->wDataLen;
        if (nDataLen < nPacketLen)
        {
            return;
        }

        //收到一个完整的数据包
        wns_tcp_ring_dequeue(pRcvBuffer, m_pTmpRcvBuff,nPacketLen);

        处理m_pTmpRcvBuff
        ......
        
        nDataLen =  wns_tcp_ring_datacount(pRcvBuffer);
    }
}
*/

#ifndef __LIBWNS_TCP_RING_H__
#define __LIBWNS_TCP_RING_H__

#include "libbase_type.h"
#include "libbase_config.h"

typedef struct h_tcp_ring_st h_tcp_ring_st;

/**
 * 在指定内存上创建环形缓冲区
 * \param name 缓冲区名称
 * \param size 缓冲区长度，必须为2的N次方（每一个元素需要一个管理结构，因此缓冲区的大小不是简单的元素大小乘以元素个数，而是略大）
 * \return 正常情况返回mem，否则返回NULL（size不是2的N次方）
 *
 * chunk_ring 是多生产者多消费者线程安全的
 */
h_tcp_ring_st *h_tcp_ring_create(const char *name, size_t size);

/**
 * 销毁一个环状队列
 *
 * 必须保证在没有出队入队的操作时进行
 *
 * @param ring  环状队列指针
 *
 */
void h_tcp_ring_destroy(h_tcp_ring_st *r);

/**
 * 入队一个chunk
 * \param r 环形缓冲区
 * \param buffer 块数据指针
 * \param n 块数据长度
 * \return
 *   - 0: 成功
 *   - -ENOBUFS: 空间不足
 */
int h_tcp_ring_enqueue(h_tcp_ring_st *r, const int8_t *buffer, size_t n);

/**
 * 获取一个可以出队的chunk, 但是并不真正出队
 * 注意出队缓冲要能够完整放入一个chunk
 * \param r 环形缓冲区
 * \param buffer 块数据缓冲区指针
 * \param n 输入传入chunk缓冲区的大小
 * \return
 *   - 0: 成功
 *   - -ENOENT:  没有足够的空间存数据
 */
int h_tcp_ring_peek(h_tcp_ring_st *r, int8_t *buffer, size_t n);

/**
 * 出队一个chunk，注意出队缓冲要能够完整放入一个chunk
 * \param r 环形缓冲区
 * \param buffer 块数据缓冲区指针
 * \param n 输入传入chunk缓冲区的大小
 * \return
 *   - 0: 成功
 *   - -ENOENT:  没有足够的空间存数据
 */
int h_tcp_ring_dequeue(h_tcp_ring_st *r, int8_t *buffer, size_t n);



/**
* @brief 清除指定大小的数据
* @param   r   环形缓冲区
* @param   n   数据大小
*
* @return 
* @remark null
* @see     
* @author lwj      @date 2012/11/12
**/
void h_tcp_ring_erase(h_tcp_ring_st *r, size_t n);

/**
* @brief 返回环形缓冲大小
* @param   r   环形缓冲区
*
* @return 环形缓冲大小
* @remark null
* @see     
* @author lwj      @date 2012/11/12
**/
size_t h_tcp_ring_count(const h_tcp_ring_st *r);

/**
* @brief 返回环形缓冲空闲缓存大小
* @param   r   环形缓冲区
*
* @return 环形缓冲空闲大小
* @remark null
* @see     
* @author lwj      @date 2012/11/12
**/
size_t h_tcp_ring_freecount(const h_tcp_ring_st *r);

/**
* @brief 返回环形缓冲已使用空间大小
* @param   r   环形缓冲区
*
* @return 环形缓冲已使用空间大小
* @remark null
* @see     
* @author lwj      @date 2012/11/12
**/
size_t h_tcp_ring_datacount(const h_tcp_ring_st *r);

/** \} */

#endif /* __LIBWNS_CHUNK_RING_H__ */


/* copyright see rte_atomic.h */
