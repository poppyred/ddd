/*
 * ini文件读取
 * author: hyb
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "libbase_iniparser.h"
#include "libbase_memory.h"
#include "libbase_strutils.h"

/**
 * ini文件描述结构
 */
struct h_ini_file_st {
    h_hash_st *sessions;
};

#ifdef _MSC_VER
#define strdup _strdup
#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')
#endif

/* file read line buf size */
#define MAX_LINE_BUF        1024

/* hash data destroy operate, session data is another hash */
static void ini_sess_data_destroy(void *data)
{
    h_hash_st *h = (h_hash_st *)data;
    if (h)
        h_hash_destroy(h);
}

/* hash data destroy operate, value data is a string(by strdup) */
static void ini_keyval_data_destroy(void *data)
{
    h_free(data);
}

/* load the file into an wns_ini_file_st */
h_ini_file_st *h_ini_file_load(const char *filename)
{
    FILE *fp = NULL;
    char buf[MAX_LINE_BUF];
    h_hash_st *cursess = NULL;
    h_ini_file_st *ret;

    fp = fopen(filename, "r");
    if (!fp)
        return NULL;

    if ((ret = h_malloc(sizeof(h_ini_file_st))) == NULL) {
        fclose(fp);
        return NULL;
    }

    ret->sessions = h_hash_create(ini_sess_data_destroy, NULL, 0);
    if (ret->sessions == NULL) {
        fclose(fp);
        h_free(ret);
        return NULL;
    }

    while (fgets(buf, MAX_LINE_BUF - 1, fp)) {
        char *key, *val;
        char *s;

        buf[MAX_LINE_BUF - 1] = 0;

        if (buf[0] == '\'' || buf[0] == ';' || buf[0] == '#')       /* ignore comment */
            continue;

        if (buf[0] == '[') {        /* session */
            key = &buf[1];
            s = strchr(key, ']');
            if (!s)
                continue;

            *s = 0;

            cursess = h_hash_create(ini_keyval_data_destroy, NULL, 0);
            h_hash_insert(ret->sessions, key, (int)strlen(key) + 1, cursess);
            continue;
        }

        /* key = value */
        s = strchr(buf, '=');
        if (!s)
            continue;

        *s = 0;
        key = buf;
        val = s + 1;

        /* trim key */
        while (key < s && isspace((int)*key)) {
            key++;
        }
        --s;
        while (s >= key && isspace((int)*s)) {
            *s = 0;
            --s;
        }

        /* trim val */
        s = val + strlen(val);
        while (val < s && isspace((int)*val)) {
            val++;
        }
        --s;
        while (s >= val && isspace((int)*s)) {
            *s = 0;
            --s;
        }

        if (cursess && strlen(key) > 0 && strlen(val) > 0) {
            h_hash_insert(cursess, key, (int)strlen(key) + 1, strdup_s(val));
        }
    }

    fclose(fp);
    return ret;
}

/* query ini session and key, return value(NULL if not found) */
const char *h_ini_file_get(h_ini_file_st *ini, const char *session_name,
        const char *key)
{
    void *h;
    void *val;

    if (!ini || !ini->sessions || !session_name || !key)
        return NULL;

    if (h_hash_search(ini->sessions, session_name,
                (int)strlen(session_name) + 1, &h))
        return NULL;

    if (h_hash_search((h_hash_st *)h, key, (int)strlen(key) + 1, &val))
        return NULL;

    return (const char *)val;
}

struct ini_walk_mid {
    const char *section_name;
    void *true_userdata;
    ini_section_traval_func_t callback;
};

static int ini_section_walk_fn(const void *key, int klen, void *value,
        void *userdata)
{
    struct ini_walk_mid *mid = (struct ini_walk_mid *)userdata;
    int r;

    if ((r = mid->callback(mid->section_name, (const char *)key,
                    (const char *)value, mid->true_userdata)))
    {
        return r;
    }
    return 0;
}

static int ini_all_section_walk_fn(const void *key, int klen, void *value,
        void *userdata)
{
    struct ini_walk_mid *mid = (struct ini_walk_mid *)userdata;
    int r;

    if ((r = mid->callback(key, NULL, NULL, mid->true_userdata)))
    {
        return r;
    }
    return 0;
}

/* traval a section of ini file */
int h_ini_file_section_foreach(h_ini_file_st *ini,
        const char *section_name, void *userdata,
        ini_section_traval_func_t callback)
{
    h_hash_st *h;
    struct ini_walk_mid mid;
    void *tmp;

    if (!ini || !ini->sessions || !section_name || !callback)
        return 0;

    if (h_hash_search(ini->sessions, section_name,
                (int)strlen(section_name) + 1, &tmp))
        return 0;
    h = (h_hash_st *)tmp;

    mid.section_name = section_name;
    mid.true_userdata = userdata;
    mid.callback = callback;

    return h_hash_walk(h, &mid, ini_section_walk_fn);
}

/* traval all section of ini file */
int h_ini_file_foreach(h_ini_file_st *ini,
        void *userdata,
        ini_section_traval_func_t callback)
{
    struct ini_walk_mid mid;

    if (!ini || !ini->sessions || !callback)
        return 0;

    mid.section_name = NULL;
    mid.true_userdata = userdata;
    mid.callback = callback;

    return h_hash_walk(ini->sessions, &mid, ini_all_section_walk_fn);
}


/* destroy everything */
void h_ini_file_free(h_ini_file_st *ini)
{
    if (!ini)
        return;

    h_hash_destroy(ini->sessions);
    h_free(ini);
}

#ifdef __INI_TEST__
int main(int argc, char **argv)
{
    h_ini_file_st *ini;
    int i;

    if (argc < 2)
        return 0;

    ini = h_ini_file_load(argv[1]);
    if (!ini)
        return 0;

    for (i = 2; i < argc; i += 2) {
        printf("%s.%s = %s\n",
                argv[i], argv[i+1], h_ini_file_get(ini, argv[i], argv[i+1]));
    }

    h_ini_file_free(ini);
    return 0;
}

#endif
