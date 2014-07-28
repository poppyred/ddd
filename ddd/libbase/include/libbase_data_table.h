/**
 * 数据表数据结构
 * 用来保存数据记录的容器
 * 可以对记录的多个域建立索引，通过任意索引域快速搜索数据
 * 支持将数据表备份到文件，以及从文件恢复数据表（一定要在同一体系结构，
 * 听一配置的数据表才能做备份恢复，由于结构体大小，大小端等原因）
 *
 * usage:
 *     第一步： 创建数据表
 *     第二步： 增加索引域
 *     第三步： 增删改查各种操作
 *     第四步： 用完摧毁
 *     so easy;
 *
 * 作者：hyb
 * 2012/10/30
 *
 **/
#ifndef __LIBWNS_DATA_TABLE_H__
#define __LIBWNS_DATA_TABLE_H__

/* 支持最大的搜索域 */
#define DT_MAX_INDEX_FIELD  (8)

/* int错误码，返回int错误码的函数返回 *! 负 的错误码 */
typedef enum {
    DT_ERR_MALLOC = 1,
    DT_ERR_INPUT,       /* 错误的输入参数，比如NULL */
    DT_ERR_LOGIC,       /* 错误的调用逻辑 */
    DT_ERR_FULL,        /* 表满了 */
    DT_ERR_EXITE,       /* 搜索域已经存在，域冲突 */
    DT_ERR_NO_EXITE,    /* 操作的记录不存在 */
    DT_ERR_FILE,        /* 文件操作失败 */
} dt_error_type_t;

/**
 * @brief 数据表  隐式假定说明：
 *   一、必须保证域ID = 0上建立索引域，之后在进行增删改查，不然行为不确定
 *   二、索引域的数据内容唯一性不由库保证，当搜索删除时，相同key只随机删除一个
 *   三、对数据表的索引域进行修改，要先remove改好再insert
 *
 **/

typedef struct h_dt_memory_allocer_st{
    void *(*malloc)(size_t size);
    void (*free)(void *);
    void *(*calloc)(size_t nmemb, size_t size);
} h_dt_memory_allocer_st;

typedef struct h_data_table_st *h_data_table_t;

/**
 * @breif 记录释放回调
 *        用来删除纪录，和destory数据表的时候，释放插入的纪录内存用
 **/
typedef
void (*h_data_table_free_cb_t)(void *data);

/**
 * @breif   创建数据表
 * @param free        释放记录的回调函数（比如你的记录的数据结构是用malloc的
 *                    平面内存结构，这里直接填标准库的free就可以了）
 * @param record_size 每个记录的大小
 * @param max_record  表能存放的最大的记录数（必须是二的倍数）
 * @param allocer     定义内部内存分配器，为NULL则用malloc，free，calloc
 *
 * @return
 *     数据表的句柄
 *     NULL失败
 *
 **/
h_data_table_t h_data_table_create(h_data_table_free_cb_t free,
        unsigned int record_size, unsigned int max_record,
        h_dt_memory_allocer_st *allocer);


/**
 * @breif   从记录中提取key的回调函数
 *          由使用者提供从记录中，找到指定域的能力
 *          假设你的结构体是
 *          struct temp{
 *              int a;
 *              int b;
 *          };
 *          取a域的回调可以定义为
 *          return (void *)&((struct temp *)record)->a;
 * @param [IN]  record;
 * @return
 *     返回 域的指针
 **/
typedef
void* (*h_data_table_get_field_cb_t)(void *record);

/* 创建回调函数的辅助宏，帮助快速定义一个提取key的回调函数 */
#define GEN_GET_FIELD_FUN(fun_name, str_type, member) \
static void * fun_name(void *record) { \
    return (void*)&((str_type *)record)->member; }


/**
 * @breif   添加一个索引域
 *          使得search可以用这个域来进行搜索
 * @param tab        数据表的句柄
 * @param field_id   记录中域的唯一的标识
 * @param field_size 域类型占的内存大小（如sizeof(temp->a)）
 * @param 提取索引域的回调
 *
 * @note    数据表用户必须保证 有add过field_id = 0的索引域，如果
 *          0 号域ID不建立索引，则数据表的行为不确定         XXX
 * @return
 *     0成功
 *     非0失败
 *
 **/

int h_data_table_add_index(h_data_table_t tab, unsigned int field_id,
        unsigned int field_size, h_data_table_get_field_cb_t get_field);

/**
 * @breif   向数据表插入一个记录
 * @param tab     数据表的句柄
 * @param record  要插入的记录的指针
 *
 * @return
 *     0成功
 *     非0失败
 *
 **/
int h_data_table_insert(h_data_table_t tab, void *record);

/**
 * @breif   通过指定域来搜索数据
 * @param field_id 开始add_index的时候，填的域的唯一标识
 * @param key   目标数据
 *
 * @return
 *     记录的指针
 *     NULL失败，没找到
 *
 **/
void*
h_data_table_search(h_data_table_t tab, unsigned int field_id, void *key);

/**
 * @breif 删除记录
 *
 * @param tab   数据表的句柄
 * @param record   目标数据
 *
 * @return
 *     0成功
 *     非0失败
 *
 **/
int
h_data_table_delete(h_data_table_t tab, void* record);

/**
 * @breif 提取记录
 *        这个函数不会调用free_cb去释放记录，
 *        wns_data_table_delete会调用free_cb去释放记录
 *
 * @param tab   数据表的句柄
 * @param record   目标数据
 *
 * @return
 *     0成功
 *     非0失败
 *
 **/
int
h_data_table_remove(h_data_table_t tab, void* record);

/**
 * @breif 删除指定域的记录
 *
 * @param tab   数据表的句柄
 * @param field_id 开始add_index的时候，填的域的唯一标识
 * @param key   目标数据
 *
 * @return
 *     0成功
 *     非0失败
 *
 **/
int
h_data_table_del_by_field(h_data_table_t tab, unsigned int field_id,
        void *key);

/**
 * @breif 按指定域的提取记录
 *        这个函数不会调用free_cb去释放记录，
 *        wns_data_table_del_by_field 会调用free_cb去释放记录
 *
 * @param tab   数据表的句柄
 * @param field_id 开始add_index的时候，填的域的唯一标识
 * @param key   目标数据
 *
 * @return
 *     0成功
 *     非0失败
 *
 **/
int
h_data_table_rmv_by_field(h_data_table_t tab, unsigned int field_id,
        void *key);

/**
 * @breif 销毁数据表
 *
 * @param tab   数据表的句柄
 *
 * @return
 *     无
 *
 **/
void h_data_table_destory(h_data_table_t tab);

/**
 * @breif 将数据表表备份到文件 （同步API）
 *
 * @param tab   数据表的句柄
 * @param path  文件的路径
 *
 * @return
 *        成功返回0
 *        非0失败
 **/
int h_data_table_to_file(h_data_table_t tab, const char *path);

/**
 * @breif 从文件恢复数据表，只恢复数据，不恢复其他的配置（如搜索域，最大
 *        记录数等等）。调用时需要保证，数据表的其他配置已经初始化完毕
 *        并和保存的时候完全一致。 （同步API）
 *
 * @param tab   数据表的句柄
 * @param path  文件的路径
 *
 * @return
 *        成功返回0
 *        非0失败
 **/
int h_data_table_from_file(h_data_table_t tab, const char *path);

/**
 * @breif 遍历回调
 * @param record 添加的记录
 * @param data   walk函数的data
 *
 * @return
 *        0继续遍历
 *        非零停止遍历
 **/
typedef int (*h_data_table_walk_cb_t)(void *record, void *data);

/**
 * @breif 遍历整张数据表
 * @param tab      数据表的句柄
 * @param walk_cb  遍历的回调
 * @param data     传给回调的指针
 *
 * @return
 *       0成功
 *       返回回调返回的值
 *
 **/

int h_data_table_walk(h_data_table_t tab,
        h_data_table_walk_cb_t walk_cb, void *data);

/**
 * @breif 遍历数据表的一个遍历单元
 * @param tab      数据表的句柄
 * @param unit_id  遍历单元id
 * @param walk_cb  遍历的回调
 * @param data     传给回调的指针
 *
 * @return
 *       0成功
 *       返回回调返回的值
 *
 **/
int h_data_table_unit_walk(h_data_table_t tab, unsigned int unit_id,
        h_data_table_walk_cb_t walk_cb, void *data);

/* @breif 返回数据表的遍历单元数 */
unsigned int h_data_table_get_foreach_unit_num(h_data_table_t table);

/* @breif 返回数据表现存的记录数 */
unsigned int h_data_table_get_record_num(h_data_table_t tab);



#ifdef HH_DEBUG
void h_data_table_state_show(h_data_table_t table);
#endif

#endif /* __DATA_TABLE_H__ */
