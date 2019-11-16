/**
 * SQL数据库操作
 */

#ifndef   DB_H_
#define   DB_H_

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif

    /*struct sql_storage_format {
        char field[32];   // 字段
        char format[32];  // 格式
    };*/
	struct sql_storage_item {
        char field[32];      // 字段
        char format[32];     // 格式
        char value[512];     // 值
    };
    struct db_sql_pool{
        char sql[1024 * 10];                      // SQL语句
    };
    struct data_base_obj;
    struct data_base_fops{ // 操作函数集
        // 构造函数
        struct data_base_obj* (*const constructed)(struct data_base_obj* const _this, void* const _obj_fops, const struct sql_storage_item* const _format, const unsigned int _format_size, struct sql_storage_item* const _items, const unsigned int _items_size, const char* const _tbl_name);
        int (* const init)(struct data_base_obj *const _db);
        int (* const clear)(struct data_base_obj *const _db);
        int (* const close)(struct data_base_obj *const _db);
        // 创建生成 SQL 语句
        int (* const create_sql)(struct data_base_obj *const _db);
        // 执行 SQL 语句
        //int (* const exec_sql)(struct data_base_obj *const _db);
        int (* const insert_sql)(struct data_base_obj *const _db);
        int (* const insert_item_format)(struct data_base_obj *const _db, const char* const field, ...);
        int (* const insert_item)(struct data_base_obj *const _db, const char* const field, const double _value);
        int (* const insert_item_string)(struct data_base_obj *const _db, const char* const field, const char* const _value);
        int (* const insert_item_int)(struct data_base_obj *const _db, const char* const field, const int _value);
    };
    struct data_base_obj{ // 对象定义
        const struct data_base_fops* const fops;
        void *mysql;//MYSQL *mysql;
        //struct data_base_data data;
        //struct data_base_obj* const father; //  father ptr
        const struct sql_storage_item* const _format;   // 数据格式
        const unsigned int _format_size;                // 数据格式大小
        struct sql_storage_item* const _items;          // 数据项
        const unsigned int _items_size;                 // 数据项大小
        const char* const tbl_name;  // 表名
        //const char tbl_name[64];
        int update_flag;    // 更新标志
        char sql_query[1024 * 10];                      // SQL语句
        struct db_sql_pool sql_pool[1];          // SQL语句
        int sql_pool_size;
    };

    extern const struct data_base_fops _data_base_fops;
    extern struct data_base_obj db_obj_fw;
    extern struct data_base_obj db_obj_sn;
    extern struct data_base_obj db_obj_report;

    extern char _mysql_host[];
    extern char _mysql_user[];
    extern char _mysql_passwd[];
    extern char _mysql_db[];
    extern unsigned int _mysql_port;

    extern const struct sql_storage_item sql_items_format_report[];
    extern const unsigned int _sql_items_format_report_size;

    extern int data_base_mysqlcfg(const char _host[], const char _user[], const char _passwd[], const char _db[], unsigned int _port);

#ifdef	__cplusplus
}
#endif

#endif   // DB_H_
