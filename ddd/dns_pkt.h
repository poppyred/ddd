#ifndef __DNS_PKT_H__
#define __DNS_PKT_H__



/**
* @brief   Main函数中，发DNS请求包到core
* @param   domain   
* @param   type   
* @param   view   
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2014/05/20
**/
void request_to_core(char *domain,int type,int view);


/**
* @brief   Main函数中，回复信息到Mgr
* @param   nameclass   
* @param   type   
* @param   view   
* @param   data   
* @param   result   
*
* @return 
* @remark null
* @see     
* @author hyb      @date 2014/05/22
**/
void answer_to_mgr(char *nameclass,int opt,int type,int view,char *data, int result);


#endif