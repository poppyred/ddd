/*
 * 国际化例程
 * author: hyb
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libbase_hash.h"
#include "libbase_iniparser.h"
#include "libbase_error.h"
#include "libbase_memory.h"
#include "libbase_strutils.h"

#include "libbase_i18n.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif

static h_hash_st *s_i18n_strings;

/* 从源串翻译为国际化串 */
const char *wns_i18n_gettext(const char *ori_str)
{
    void *str;

    if (!s_i18n_strings)
        return ori_str;

    if (h_hash_search(s_i18n_strings, ori_str, (int)(strlen(ori_str)), &str))
        return ori_str;

    return (char *)str;
}

/* 遍历回调 */
static int i18n_trvl_fn(const char *section_name, const char *key,
        const char *value, void *data)
{
    char *val = strdup_s(value);

    if (!val)
        wns_die("out of memory\n");

    h_hash_insert((h_hash_st *)data, key, (int)(strlen(key)), val);

    return 0;
}

/* 加载资源文件并读入所有字串 */
int wns_i18n_load_resource(const char *lang_id, const char *file)
{
    h_ini_file_st *rini;
    int r;

    rini = h_ini_file_load(file);
    if (!rini)
        return -1;

    if (!s_i18n_strings) {
        s_i18n_strings = h_hash_create((hash_data_free_func_t)h_free,
                NULL, 0);
        if (!s_i18n_strings)
            wns_die("can not create i18n hash\n");
    }

    r = h_ini_file_section_foreach(rini, lang_id,
            s_i18n_strings, i18n_trvl_fn);
    h_ini_file_free(rini);

    return r;
}
