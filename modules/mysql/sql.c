/**
 * 数据库接口
 */

#include "sql.h"
#include <stdio.h>
#include <string.h>
#include <memory.h>

#pragma comment(lib,"libmysql.lib")
#include "mysql.h"
#include "data_base.h"

int sql_main(void)
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
		exit(1);
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
#if 0
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
    fflush(stdout);
	return 0;
}

/*  保存硬件ID和序列号到数据库  */
int sql_insert_sn(const char hard_id[], const char sn[])
{
    //MYSQL conn;
    static const struct sql_storage_item items_format[] = {
        // "vin, prot, mil, status, ready, svin, cin, iupr, fault_total, fault,",
        {"hard_id"   , "%s", ""},
        {"SN"   , "%s", ""},
    };
    static const unsigned int items_format_size = sizeof (items_format)/sizeof (items_format[0]);
    struct sql_storage_item sql_items[32];
    struct data_base_obj _db = {
        .fops = &_data_base_fops,
        ._format = items_format,
        ._format_size = items_format_size,
        ._items = sql_items,
        ._items_size = items_format_size,
        .tbl_name = "tbl_sn",
        .update_flag = 0,
        .sql_query = "",
    };
    /*mysql_init(&conn);  //如果传入的指针为空，会分配一个MYSQL的结构体，并由mysql_close()释放掉
    //if( mysql_real_connect(&conn, "localhost", "root", "123456y", "hniois", 0, NULL, 0) == NULL )
    if (mysql_real_connect(&conn, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, NULL, 0) == NULL)
    {
        printf("Error %u: %s\n", mysql_errno(&conn), mysql_error(&conn));
        return -1;// exit(1);
    }*/
    _db.fops->init(&_db);
    _db.fops->insert_item_string(&_db, "hard_id", hard_id);
    _db.fops->insert_item_string(&_db, "SN", sn);
    _db.fops->insert_sql(&_db/*, &conn*/);
    //mysql_close(&conn);
    _db.fops->close(&_db);
    return 0;
}


/*  保存硬件ID和序列号到数据库  */
int sql_insert_fw_update(const char vin[], const char version[], const char Des[], const char hard_id[], const char sn[], const int fw_crc)
{
    static const struct sql_storage_item items_format[] = {
        {"vin"       , "%s", ""},
        {"version"   , "%s", ""},
        {"Des"       , "%s", ""},
        {"hard_id"   , "%s", ""},
        {"SN"        , "%s", ""},
        {"fw_crc"    , "%d", ""},
    };
    struct sql_storage_item sql_items[32];
    static const unsigned int items_format_size = sizeof (items_format)/sizeof (items_format[0]);
    struct data_base_obj _db = {
        .fops = &_data_base_fops,
        ._format = items_format,
        ._format_size = items_format_size,
        ._items = sql_items,
        ._items_size = items_format_size,
        .tbl_name = "tbl_obd_4g_fw_update",
        .update_flag = 0,
        .sql_query = "",
    };
    /*MYSQL* conn_prt;
    conn_prt = mysql_init(NULL);  //如果传入的指针为空，会分配一个MYSQL的结构体，并由mysql_close()释放掉
    if(!conn_prt)
    {
        fprintf(stderr,"mysql init failed\n");
        return -1;
    }
    //if( mysql_real_connect(&conn, "localhost", "root", "123456y", "hniois", 0, NULL, 0) == NULL )
    if (mysql_real_connect(conn_prt, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, NULL, 0) == NULL)
    {
        printf("Error %u: %s\n", mysql_errno(conn_prt), mysql_error(conn_prt));
        return -1;// exit(1);
    }*/
    _db.fops->init(&_db);
    _db.fops->insert_item_string(&_db, "vin", vin);
    _db.fops->insert_item_string(&_db, "version", version);
    _db.fops->insert_item_string(&_db, "Des", Des);
    _db.fops->insert_item_string(&_db, "hard_id", hard_id);
    _db.fops->insert_item_string(&_db, "SN", sn);
    _db.fops->insert_item_format(&_db, "fw_crc", fw_crc);
    _db.fops->insert_sql(&_db/*, conn_prt*/);
    //mysql_close(conn_prt);
    _db.fops->close(&_db);
    //memset(sql_report_items, 0, sizeof (sql_report_items));
    //db_format_init(sql_report_items_format, sql_report_items, sql_report_items_size);
    return 0;
}
static char sql_query[1024 * 10];   // SQL语句
int sql_select(const char* const tbl_name, const char* const field, const char* const value, char* const sql_field[], char* const sql_value[])
{
        MYSQL *conn_ptr;
        memset(sql_query, 0, sizeof (sql_query));
        conn_ptr = mysql_init(NULL);  //如果传入的指针为空，会分配一个MYSQL的结构体，并由mysql_close()释放掉
        //	conn_ptr = mysql_init(NULL);
        if(!conn_ptr)
        {
            fprintf(stderr,"mysql init failed\n");
            return -1;
        }
        /*else
        {
            conn_ptr =mysql_real_connect(conn_ptr,"localhost","xcwy","xcwy_data","xcwy",3306,NULL,0);

            if(conn_ptr)
            {
                printf("connection success \n");
            }
        }*/
        //printf("@%s-%d\n", __func__, __LINE__);
        //if( mysql_real_connect(&conn, "localhost", "root", "123456y", "hniois", 0, NULL, 0) == NULL )
        if(mysql_real_connect(conn_ptr, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, NULL, 0)==NULL)
        {
                printf("Error %u: %s\n", mysql_errno(conn_ptr), mysql_error(conn_ptr));
                return -1;
        }
        //printf("@%s-%d\n", __func__, __LINE__);
        /* int mysql_query(MYSQL *mysql, const char *stmt_str) */
        /* Zero for success. Nonzero if an error occurred. */
        //if( mysql_query(&conn, "select * from tbl_obd_4g") )
        snprintf(sql_query, sizeof (sql_query)-1, "SELECT * FROM `%s` WHERE `%s` LIKE '%s'", tbl_name, field, value);
        //if( mysql_query(&conn, "SELECT * FROM `tbl_obd_4g_fw_update` WHERE `SN` LIKE '102906420208' LIMIT 0 , 30") )
        //printf("@%s-%d\n", __func__, __LINE__);
        if(mysql_query(conn_ptr, sql_query))
        {
                printf("Error %u: %s\n", mysql_errno(conn_ptr), mysql_error(conn_ptr));
                return -1;
        }

        //printf("@%s-%d\n", __func__, __LINE__);
        MYSQL_RES* result;
        MYSQL_ROW row;
        //unsigned long *lengths;
        MYSQL_FIELD *fields;

        //printf("@%s-%d\n", __func__, __LINE__);
        result=mysql_use_result(conn_ptr);  //内部会分配MYSQL_RES结构体，由mysql_free_result()释放掉

        int num_fields=mysql_num_fields(result);       //结果集有多少列
        //printf("mysql_use_result列：%d\n",num_fields);

        //printf("@%s-%d\n", __func__, __LINE__);
        /* fields是MYSQL_FIELD结构体数组的首地址，MYSQL_FIELD结构体的name项是列名 */
        fields = mysql_fetch_fields(result);      //估计fields指向的是静态局部变量
        int i=0;
        for(i = 0; i < num_fields; i++)
        {
                //printf("Field %u is %s\n", i, fields[i].name);
                memcpy(sql_field[i], fields[i].name, strlen(fields[i].name));

        }
#if 1
        /* 实际检索来自服务器的行，检索结果集的下一行;直至为返回NULL，检索完毕 */
        //printf("@%s-%d\n", __func__, __LINE__);
        while( NULL!=(row=mysql_fetch_row(result)) )
        {
                my_ulonglong num_rows=mysql_num_rows(result);  //返回结果集的行号
                //printf("mysql_use_result行号: %lu\n",(unsigned long)num_rows);

                /* 显示该行号中的数据 */
                for(i = 0; i < num_fields; i++)
                {
                        //printf("values:%s   \n",row[i]);
                        memset(sql_value[i], 0, strlen(row[i])+1);
                        memcpy(sql_value[i], row[i], strlen(row[i]));
                }
#if 0
                /* 显示该行中的每一列数据长度 */
                lengths = mysql_fetch_lengths(result);      //生成一维int数组(静态局部变量)，把首地址给lengths；储存每一列的长度
                for(i = 0; i < num_fields; i++)
                {
                        printf("Column %u is %lu bytes in length.\n",i, lengths[i]);
                }
#endif
        }
#endif
        //printf("@%s-%d\n", __func__, __LINE__);
        mysql_free_result(result);
        //printf("@%s-%d\n", __func__, __LINE__);
        mysql_close(conn_ptr);
        //printf("@%s-%d\n", __func__, __LINE__);
        return 0;
}
#if 0
static const struct sql_storage_item sql_report_items_format[] = {
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
static const int sql_report_items_size = sizeof (sql_report_items_format)/sizeof (sql_report_items_format[0]);
#endif
#if 0
static MYSQL* _conn_ofp;
static struct sql_storage_item sql_report_items[128];
void MySqlInit(void)
{
#if 0
//	MYSQL * conn_ptr;
    //struct sql_storage_item *item=NULL;
    //int index = 0;
    _conn_ofp = mysql_init(NULL);
    if(!_conn_ofp)
    {
        fprintf(stderr,"mysql init failed\n");
        return -1;
    }
    if(NULL==mysql_real_connect(_conn_ofp, _mysql_host, _mysql_user, _mysql_passwd, _mysql_db, _mysql_port, NULL, 0))
    {
        printf("Error %u: %s\n", mysql_errno(_conn_ofp), mysql_error(_conn_ofp));
        return ;// exit(1);
    }
    memset(sql_report_items, 0, sizeof (sql_report_items));
    //memcpy(sql_report_items, sql_report_items_format, sizeof (sql_report_items_format));
    //db_format_init(sql_report_items_format, sql_report_items, sql_report_items_size);
#endif
    db_obj_report.fops->init(&db_obj_report);
}
// 存单个数据
int insert_item(const char* const field, const double value)
{
    //return db_insert_item(sql_report_items, sql_report_items_size, field, value);
    return db_obj_report.fops->insert_item(&db_obj_report, field, value);
}
int insert_item_string(const char* const field, const char* const string_value)
{
    //return db_insert_item_string(sql_report_items, sql_report_items_size, field, string_value);
    return db_obj_report.fops->insert_item_string(&db_obj_report, field, string_value);
}
int insert_item_int(const char* const field, const int value)
{
    //return db_insert_item_int(sql_report_items, sql_report_items_size, field, value);
    return db_obj_report.fops->insert_item_int(&db_obj_report, field, value);
}
// 存入数据库
int insert_sql(void)
{
    int ret = 0;
    //ret = db_insert_sql(sql_report_items, sql_report_items_size, &conn_ofp, "tbl_obd_4g", db_update_flag());
    ret = db_obj_report.fops->insert_sql(&db_obj_report/*, _conn_ofp*/);
    memset(sql_report_items, 0, sizeof (sql_report_items));
    //db_format_init(sql_report_items_format, sql_report_items, sql_report_items_size);
    //db_obj_report.fops->init(&db_obj_report);
    db_obj_report.fops->clear(&db_obj_report);
    return ret;
}
void MySqlClose(void)
{
    //mysql_close(_conn_ofp);
    db_obj_report.fops->close(&db_obj_report);
}
#endif


