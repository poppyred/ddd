/*=============================================================================
* @brief                                                                      *
* 事件模型平台                                                                *
*                                                                             *
*                                                                             *
*                                         @作者:黄衍博      @date 2013/05/08  *
==============================================================================*/

#ifndef __EFLY__EV__H__2013_5_8__
#define __EFLY__EV__H__2013_5_8__


typedef struct ev_timer ev_timer;

typedef struct efly_ipc_svr efly_ipc_svr;



/**
* @brief   定时器回调函数
* @param   event_loop_callback_fun   
* @param   user_data   用户传入参数
*
* @return  无
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
typedef void (*event_timer_callback_fun)(void *user_data);



/**
* @brief   ipc信息回调函数
* @param   event_ipc_callback_fun   
* @param   buf   消息本体
* @param   len   消息长度
* @param   src   信息源(对方ipc注册客户端时所填名称)
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
typedef void (*event_ipc_callback_fun)(void *buf, int32_t len, char *src, char *answer);



/**
* @brief    初始化事件模型机制
* @return   非0失败
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
int efly_event_init();



/**
* @brief    开启事件轮询
* @return   -1失败，其余成功
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
int32_t efly_event_loop();



/**
* @brief    开启事件轮询(只运行一次)
* @return   -1失败 其余成功
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
int32_t efly_event_loop_once();


/**
* @brief   设置定时器
* @param   f_timeout   第一次回调时间(毫秒为单位，设为0则立刻回调)
* @param   timeout     回调间隔(毫秒为单位，不建议设为0)
* @param   cb          回调函数
* @param   arg         用户参数
*
* @return              失败返回空
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
ev_timer * efly_timer_set(int32_t f_timeout, int32_t timeout, 
        event_timer_callback_fun cb, void *arg);



/**
* @brief   摧毁指定定时器
* @param   ev_timer   定时器对象
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
void efly_timer_unset(ev_timer *ev_timer);


/**
* @brief   清空所有定时器
* @param  
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/

void efly_timer_clear();


/**
* @brief   注册ipc信息客户端
* @param   pname   
*
* @return  失败返回空
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
efly_ipc_svr *efly_ipc_init(char *pname);


/**
* @brief   注销ipc信息客户端
* @param   pname   
*
* @return  
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
void efly_ipc_destroy(efly_ipc_svr *svr);


/**
* @brief   注册ipc信息应答函数
* @param   svr   客户端对象
* @param   id    回调函数id
* @param   cb    回调函数本体
*
* @return  -1失败，0成功
* @remark null
* @see     
* @author hyb      @date 2013/05/08
**/
int32_t efly_ipc_reg_func(efly_ipc_svr *svr, unsigned short id, 
        event_ipc_callback_fun cb);

#endif
