/**
 * \file iniparser.h
 * 提供ini文件读取功能
 * \author hyb
 * \defgroup base 基础函数库
 * \{
 */
#ifndef __LIBWNS_INIPARSER_H__
#define __LIBWNS_INIPARSER_H__

#include "libbase_hash.h"

typedef struct h_ini_file_st h_ini_file_st;

/**
 * 加载ini文件
 * 使用ini_file_free进行释放
 * \param filename 读取的ini文件路径
 * \retval NULL 加载失败
 * \retval !NULL 加载成功，返回ini_file_st结构
 */
h_ini_file_st *h_ini_file_load(const char *filename);

/**
 * 获得ini字段值
 * \param ini ini描述结构
 * \param section_name 段落名称
 * \param key 键值
 * \retval NULL 键值未找到
 * \retval !NULL 键值取值
 */
const char *h_ini_file_get(h_ini_file_st *ini, const char *section_name, const char *key);

/**
 * ini段落遍历回调
 * \param section_name 段落名称(ini_file_section_foreach的section_name参数)
 * \param key 索引
 * \param value 值
 * \param userdata 用户自定义数据(ini_file_section_foreach中的userdata参数)
 * \return 返回非0值将会导致ini_file_section_foreach立即返回该值
 */
typedef int (*ini_section_traval_func_t)(const char *section_name,
        const char *key, const char *value,
        void *userdata);

/**
 * ini段落遍历
 * \param ini 可用的ini_file_st句柄
 * \param section_name 段落名
 * \param userdata 用户数据
 * \param callback 每个键值上的回调函数
 * \retval 0 正常返回
 * \retval !0 其它由callback返回的值
 */
int h_ini_file_section_foreach(h_ini_file_st *ini, const char *section_name, void *userdata,
        ini_section_traval_func_t callback);

/**
 * ini所有段落遍历
 * \param ini 可用的ini_file_st句柄
 * \param userdata 用户数据
 * \param callback 每个键值上的回调函数
 * \retval 0 正常返回
 * \retval !0 其它由callback返回的值
 */
int h_ini_file_foreach(h_ini_file_st *ini,void *userdata,
        ini_section_traval_func_t callback);

/**
 * 释放ini描述结构
 * \param ini ini描述结构
 */
void h_ini_file_free(h_ini_file_st *ini);

/** \} */
#endif /* __LIBWNS_INIPARSER_H__ */
