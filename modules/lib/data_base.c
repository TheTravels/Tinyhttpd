
/**
 * 数据库接口
 */

#include "../lib/data_base.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <memory.h>

#pragma comment(lib,"libmysql.lib")
#include "mysql.h"
#ifndef BUILD_SERVER_YN
#define BUILD_SERVER_YN   0
#endif

static int sql_flag = 0;

/*static const struct sql_storage_item sql_report_items_format[] = {
	// "vin, prot, mil, status, ready, svin, cin, iupr, fault_total, fault,",
	{"hard_id"   , "%s", ""},
	{"SN"   , "%s", ""},
};
static const int sql_report_items_size = sizeof(sql_report_items_format) / sizeof(sql_report_items_format[0]);*/
char _mysql_host[64] = "localhost";
char _mysql_user[64] = "root";
char _mysql_passwd[64] = "123456";
char _mysql_db[64] = "obd";
unsigned int _mysql_port = 3306;

int data_base_mysqlcfg(const char _host[], const char _user[], const char _passwd[], const char _db[], unsigned int _port)
{
    memset(_mysql_host, 0, sizeof(_mysql_host));
    memset(_mysql_user, 0, sizeof(_mysql_user));
    memset(_mysql_passwd, 0, sizeof(_mysql_passwd));
    memset(_mysql_db, 0, sizeof(_mysql_db));
    memcpy(_mysql_host, _host, strlen(_host));
    memcpy(_mysql_user, _user, strlen(_user));
    memcpy(_mysql_passwd, _passwd, strlen(_passwd));
    memcpy(_mysql_db, _db, strlen(_db));
    _mysql_port = _port;
    return 0;
}

/**************************************** struct data_base_fops *****************************************/
static int data_base_fops_clear(struct data_base_obj *const _db)
{
    memset(_db->sql_query, 0, sizeof(_db->sql_query));
    memset(_db->_items, 0, _db->_items_size*sizeof(const struct sql_storage_item));
    memcpy(_db->_items, _db->_format, _db->_items_size*sizeof(const struct sql_storage_item));
    _db->update_flag = 0;
    return 0;
}
static int data_base_fops_init(struct data_base_obj *const _db)
{
    MYSQL* _mysql = mysql_init(NULL);
    data_base_fops_clear(_db);
    memset(_db->sql_pool, 0, sizeof(_db->sql_pool));
    _db->sql_pool_size = 0;
    if(NULL==_mysql)
    {
        fprintf(stderr,"mysql init failed\n");
        _db->mysql = NULL;
        return -1;
    }
    if(NULL==mysql_real_connect(_mysql, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, NULL, 0))
    {
        printf("Error %u: %s\n", mysql_errno(_mysql), mysql_error(_mysql));
        return -1;// exit(1);
    }
    _db->mysql = _mysql;
    return 0;
}
static int data_base_fops_close(struct data_base_obj *const _db)
{
    mysql_close(_db->mysql);
    return 0;
}
static int data_base_fops_create_sql(struct data_base_obj *const _db)
{
    char sql_field[1024 * 5];   // 字段
    char sql_value[1024 * 5];   // 值
    //char sql_query[1024 * 10];   // SQL语句
    unsigned int index = 0;
    char* _field = NULL;
    char* _value = NULL;
    struct sql_storage_item* item = NULL;
    if (0 == _db->update_flag) return 0;
    memset(sql_field, 0, sizeof (sql_field));
    memset(sql_value, 0, sizeof (sql_value));
    memset(_db->sql_query, 0, sizeof(_db->sql_query));
    //memset(sql_query, 0, sizeof (sql_query));
    memset(_db->sql_query, 0, sizeof(_db->sql_query));
    for (index = 0; index < _db->_items_size - 1; index++)
    {
        item = &_db->_items[index];
        _field = &sql_field[strlen(sql_field)];
        _value = &sql_value[strlen(sql_value)];
        sprintf(_field, "%s, ", item->field);
        sprintf(_value, "%s, ", item->value);
    }
    item = &_db->_items[index];
    _field = &sql_field[strlen(sql_field)];
    _value = &sql_value[strlen(sql_value)];
    sprintf(_field, "%s", item->field);
    sprintf(_value, "%s", item->value);
    // "INSERT INTO tbl_obd_4g(vin,lat,lng) VALUES(%s,%f,%f)"
    // 合成 SQL 语句
    //sprintf(sql_query, "INSERT INTO tbl_obd_4g(%s) VALUES(%s)", sql_field, sql_value);
    sprintf(_db->sql_query, "INSERT INTO %s(%s) VALUES(%s)", _db->tbl_name, sql_field, sql_value);
    //printf("sql_query:%s \n", sql_query); fflush(stdout);
#if 0
    memcpy(_db->sql_pool[_db->sql_pool_size].sql, _db->sql_query, strlen(_db->sql_query));
    _db->sql_pool_size++;
#endif
    return 0;
}
#if 1
static int data_base_fops_insert_sql(struct data_base_obj *const _db)
{
    MYSQL* const conn = _db->mysql;//(MYSQL*)_conn;
    int ret = 0;
    if(NULL==conn) return -1;
    data_base_fops_create_sql(_db);
    ret = mysql_query(conn, _db->sql_query);
    if (!ret)
    {
        //printf("Inserted %lu rows\n",(unsigned long)mysql_affected_rows(conn_ptr));
        return 0;
    }
    else
    {
        printf("Connect Erro:%d %s\n", mysql_errno(conn), mysql_error(conn));
    }
    return -1;
}
#else
static int data_base_fops_insert_sql(struct data_base_obj *const _db)
{
    MYSQL* const conn = _db->mysql;
    int ret = 0;
    int i;
    if(NULL==conn) return -1;
    for(i=0; i<_db->sql_pool_size; i++)
    {
        ret = mysql_query(conn, _db->sql_pool[i].sql);
        if (0!=ret)
        {
            printf("Connect Erro:%d %s\n", mysql_errno(conn), mysql_error(conn));
            return -1;
        }
        //printf("Inserted %lu rows\n",(unsigned long)mysql_affected_rows(conn_ptr));
    }
    return 0;
}
#endif
static int data_base_fops_insert_item_format(struct data_base_obj *const _db, const char* const field, ...)
{
    int index = 0;
    struct sql_storage_item* item = NULL;
    for (index = 0; index < _db->_items_size; index++)
    {
        item = &_db->_items[index];
        if (0 == strcmp(field, item->field))
        {
            //sprintf(item->value, item->format, value);
            _db->update_flag = 1;
            //return 0;
            break;
        }
    }
    if (index < _db->_items_size)
    {
        va_list ap;
        va_start(ap, field);
        //vprintf(__format, ap);
        //memset(text, 0, sizeof(text));
        //snprintf(text, sizeof (text), __format, ap);
        vsprintf(item->value, item->format, ap);
        va_end(ap);
        //printf("item->value:%s\n", item->value);
        return 0;
    }
    return -1;
}
static int data_base_fops_insert_item(struct data_base_obj *const _db, const char* const field, const double _value)
{
    int index = 0;
    struct sql_storage_item* item = NULL;
    for (index = 0; index < _db->_items_size; index++)
    {
        item = &_db->_items[index];
        if (0 == strcmp(field, item->field))
        {
            sprintf(item->value, item->format, _value);
            _db->update_flag = 1;
            return 0;
        }
    }
    return -1;
}
static int data_base_fops_insert_item_string(struct data_base_obj *const _db, const char* const field, const char* const _value)
{
    int index = 0;
    struct sql_storage_item* item = NULL;
    for (index = 0; index < _db->_items_size; index++)
    {
        item = &_db->_items[index];
        if (0 == strcmp(field, item->field))
        {
#if 0
            sprintf(item->value, "\"%s\"", string_value);
#else
            int i=0;
            item->value[0] = '"';
            for(i=0; i<strlen(_value); i++)
            {
                if('"'==_value[i]) item->value[i+1] = '-';  // 替换 '\"'
                else item->value[i+1] = _value[i];
            }
            item->value[i+1] = '"';
#endif
            _db->update_flag = 1;
            return 0;
        }
    }
    return -1;
}
static int data_base_fops_insert_item_int(struct data_base_obj *const _db, const char* const field, const int _value)
{
    int index = 0;
    struct sql_storage_item* item = NULL;
    for (index = 0; index < _db->_items_size; index++)
    {
        item = &_db->_items[index];
        if (0 == strcmp(field, item->field))
        {
            sprintf(item->value, item->format, _value);
            _db->update_flag = 1;
            return 0;
        }
    }
    return -1;
}

// 构造函数
static struct data_base_obj* data_base_obj_constructed(struct data_base_obj* const _this, void* const _obj_fops)
{
    struct data_base_obj* const _obj = (struct data_base_obj* const)_obj_fops;
    if(NULL==_obj_fops) return NULL;
    memcpy(_obj_fops, _this, sizeof(struct data_base_obj));
    return _obj;
}

/**************************************** struct data_base_fops data define *****************************************/
const struct data_base_fops _data_base_fops = {
    .constructed = data_base_obj_constructed,
    .init = data_base_fops_init,
    .clear = data_base_fops_clear,
    .close = data_base_fops_close,
    .create_sql = data_base_fops_create_sql,
    .insert_sql = data_base_fops_insert_sql,
    .insert_item_format = data_base_fops_insert_item_format,
    .insert_item = data_base_fops_insert_item,
    .insert_item_string = data_base_fops_insert_item_string,
    .insert_item_int = data_base_fops_insert_item_int,
};

static struct sql_storage_item _sql_items_fw[32];      // 数据项
static const struct sql_storage_item sql_items_format_fw[] = {
    {"vin"       , "%s", ""},
    {"version"   , "%s", ""},
    {"Des"       , "%s", ""},
    {"hard_id"   , "%s", ""},
    {"SN"        , "%s", ""},
    {"fw_crc"    , "%d", ""},
};
struct data_base_obj db_obj_fw = {
    .fops = &_data_base_fops,
    ._format = sql_items_format_fw,
    ._format_size = sizeof(sql_items_format_fw)/sizeof (sql_items_format_fw[0]),
    ._items = _sql_items_fw,
    ._items_size = sizeof(sql_items_format_fw)/sizeof (sql_items_format_fw[0]),
    .tbl_name = "tbl_obd_4g_fw_update",
    .update_flag = 0,
    .sql_query = "",
};
static struct sql_storage_item _sql_items_sn[32];      // 数据项
static const struct sql_storage_item sql_items_format_sn[] = {
    {"hard_id"  , "%s",     ""},
    {"SN"       , "%s",     ""},
};
struct data_base_obj db_obj_sn = {
    .fops = &_data_base_fops,
    ._format = sql_items_format_sn,
    ._format_size = sizeof(sql_items_format_sn)/sizeof (sql_items_format_sn[0]),
    ._items = _sql_items_sn,
    ._items_size = sizeof(sql_items_format_sn)/sizeof (sql_items_format_sn[0]),
    .tbl_name = "tbl_obd_4g",
    .update_flag = 0,
    .sql_query = "",
};

static struct sql_storage_item _sql_items_report[128];      // 数据项
// 数据格式
const struct sql_storage_item sql_items_format_report[] = {
    // "vin, prot, mil, status, ready, svin, cin, iupr, fault_total, fault,",
    {"UTC"   , "%s", ""},
    {"vin"   , "%s", ""},
    {"prot"  , "%d", ""},
    {"mil"   , "%d", ""},
    {"status", "%d", ""},
    {"ready" , "%d", ""},
    {"svin"  , "%s", ""},
    {"cin"   , "%s", ""},
    {"iupr"  , "%s", ""},
    {"fault_total", "%d", ""},
    {"fault", "%s", ""},
    // "speed, kpa, nm, nmf, rpm, Lh, ppm_up, ppm_down, urea, kgh, SCR_in, SCR_out, DPF, coolant_temp, tank, gps_status, lat, lng, mil_total, ",
    {"speed"   , "%f", ""},
    {"kpa"  , "%f", ""},
    {"nm"   , "%f", ""},
    {"nmf", "%f", ""},
    {"rpm" , "%f", ""},
    {"Lh"  , "%f", ""},
    {"ppm_up"   , "%f", ""},
    {"ppm_down"  , "%f", ""},
    {"urea", "%f", ""},
    {"kgh", "%f", ""},
    {"SCR_in"   , "%f", ""},
    {"SCR_out"  , "%f", ""},
    {"DPF"   , "%f", ""},
    {"coolant_temp", "%f", ""},
    {"tank" , "%f", ""},
    {"gps_status"  , "%d", ""},
    {"lat"   , "%f", ""},
    {"lon"  , "%f", ""},
    {"mil_total", "%f", ""},
    // "Nm_mode, accelerator, oil_consume, urea_temp, urea_actual, urea_total, gas_temp, version",
    {"Nm_mode"   , "%d", ""},
    {"accelerator"  , "%f", ""},
    {"oil_consume"   , "%f", ""},
    {"urea_temp", "%f", ""},
    {"urea_actual" , "%f", ""},
    {"urea_total"  , "%f", ""},
    {"gas_temp"   , "%f", ""},
    {"version"  , "%d", ""},
};
// 数据格式大小
const unsigned int _sql_items_format_report_size= sizeof(sql_items_format_report)/sizeof (sql_items_format_report[0]);                // 数据格式大小
struct data_base_obj db_obj_report = {
    .fops = &_data_base_fops,
    ._format = sql_items_format_report,
    ._format_size = sizeof(sql_items_format_report)/sizeof (sql_items_format_report[0]),
    ._items = _sql_items_report,
    ._items_size = sizeof(sql_items_format_report)/sizeof (sql_items_format_report[0]),
    .tbl_name = "tbl_obd_4g",
    .update_flag = 0,
    .sql_query = "",
};


/**************************************** 测试代码 *****************************************/

int db_main_cpp(void)
{
    MYSQL conn;
    mysql_init(&conn);  //如果传入的指针为空，会分配一个MYSQL的结构体，并由mysql_close()释放掉
    //if( mysql_real_connect(&conn, "localhost", "root", "123456y", "hniois", 0, NULL, 0) == NULL )
    if (mysql_real_connect(&conn, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, NULL, 0) == NULL)
    {
        printf("Error %u: %s\n", mysql_errno(&conn), mysql_error(&conn)); fflush(stdout);
        return 0;// exit(1);
    }
    /* int mysql_query(MYSQL *mysql, const char *stmt_str) */
    /* Zero for success. Nonzero if an error occurred. */
    if (mysql_query(&conn, "select * from tbl_obd_4g"))
    {
        printf("Error %u: %s\n", mysql_errno(&conn), mysql_error(&conn));
		return -1; // exit(1);
    }

    MYSQL_RES* result;
    MYSQL_ROW row;
    unsigned long* lengths;
    MYSQL_FIELD* fields;

    result = mysql_use_result(&conn);  //内部会分配MYSQL_RES结构体，由mysql_free_result()释放掉

    int num_fields = mysql_num_fields(result);       //结果集有多少列
    printf("mysql_use_result列：%d\n", num_fields);

    /* fields是MYSQL_FIELD结构体数组的首地址，MYSQL_FIELD结构体的name项是列名 */
    fields = mysql_fetch_fields(result);      //估计fields指向的是静态局部变量
    int i = 0;
    for (i = 0; i < num_fields; i++)
    {
        printf("Field %u is %s\n", i, fields[i].name);
    }
#if 1
    /* 实际检索来自服务器的行，检索结果集的下一行;直至为返回NULL，检索完毕 */
    while (NULL != (row = mysql_fetch_row(result)))
    {
        my_ulonglong num_rows = mysql_num_rows(result);  //返回结果集的行号
        printf("mysql_use_result行号: %lu\n", (unsigned long)num_rows);

        /* 显示该行号中的数据 */
        for (i = 0; i < num_fields; i++)
        {
            printf("values:%s   \n", row[i]);
        }

        /* 显示该行中的每一列数据长度 */
        lengths = mysql_fetch_lengths(result);      //生成一维int数组(静态局部变量)，把首地址给lengths；储存每一列的长度
        for (i = 0; i < num_fields; i++)
        {
            printf("Column %u is %lu bytes in length.\n", i, lengths[i]);
        }
    }
#endif
    mysql_free_result(result);
    mysql_close(&conn);
    return 0;
}

/*  保存硬件ID和序列号到数据库  */
int db_insert_test_cpp(const char hard_id[], const char sn[])
{
#if 0
    static struct sql_storage_item sql_report_items[128];
    static const struct sql_storage_item sql_report_items_format[] = {
        // "vin, prot, mil, status, ready, svin, cin, iupr, fault_total, fault,",
        {"hard_id"   , "%s", ""},
        {"SN"   , "%s", ""},
    };
#endif
    /*MYSQL conn;
    mysql_init(&conn);  //如果传入的指针为空，会分配一个MYSQL的结构体，并由mysql_close()释放掉
    //if( mysql_real_connect(&conn, "localhost", "root", "123456y", "hniois", 0, NULL, 0) == NULL )
    if (mysql_real_connect(&conn, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, NULL, 0) == NULL)
    {
        printf("Error %u: %s\n", mysql_errno(&conn), mysql_error(&conn));
        return -1;// exit(1);
    }*/
#if 0
    db_format_init(sql_report_items_format, sql_report_items, 2);
    db_insert_item_string(sql_report_items, 2, "hard_id", hard_id);
    db_insert_item_string(sql_report_items, 2, "SN", sn);
    db_insert_sql(sql_report_items, 2, &conn, "tbl_obd_4g", sql_flag);
#else
    db_obj_sn.fops->init(&db_obj_sn);
    db_obj_sn.fops->insert_item_string(&db_obj_sn, "hard_id", hard_id);
    db_obj_sn.fops->insert_item_string(&db_obj_sn, "SN", sn);
    db_obj_sn.fops->insert_sql(&db_obj_sn/*, &conn*/);
    db_obj_sn.fops->close(&db_obj_sn);
#endif
    //mysql_close(&conn);
    return 0;
}

