#include "mysql.h"
#include "my_global.h"

int sql_main_01(void)
{
	MYSQL conn;
	mysql_init(&conn);  //如果传入的指针为空，会分配一个MYSQL的结构体，并由mysql_close()释放掉
	//if( mysql_real_connect(&conn, "localhost", "root", "123456y", "hniois", 0, NULL, 0) == NULL )
	if(mysql_real_connect(&conn,"localhost","xcwy","xcwy_data","xcwy",3306,NULL,0)==NULL)
	{
		printf("Error %u: %s\n", mysql_errno(&conn), mysql_error(&conn));
		exit(1);
	}
	/* int mysql_query(MYSQL *mysql, const char *stmt_str) */
	/* Zero for success. Nonzero if an error occurred. */
	//if( mysql_query(&conn, "select * from tbl_obd_4g") )
	if( mysql_query(&conn, "SELECT * FROM `tbl_obd_4g_fw_update` WHERE `SN` LIKE '102906420208' LIMIT 0 , 30") )
	{
		printf("Error %u: %s\n", mysql_errno(&conn), mysql_error(&conn));
		exit(1);
	}

	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned long *lengths;
	MYSQL_FIELD *fields;

	result=mysql_use_result(&conn);  //内部会分配MYSQL_RES结构体，由mysql_free_result()释放掉

	int num_fields=mysql_num_fields(result);       //结果集有多少列
	printf("mysql_use_result列：%d\n",num_fields);

	/* fields是MYSQL_FIELD结构体数组的首地址，MYSQL_FIELD结构体的name项是列名 */
	fields = mysql_fetch_fields(result);      //估计fields指向的是静态局部变量
	int i=0;
	for(i = 0; i < num_fields; i++)
	{
		printf("Field %u is %s\n", i, fields[i].name);
	}
#if 1
	/* 实际检索来自服务器的行，检索结果集的下一行;直至为返回NULL，检索完毕 */
	while( NULL!=(row=mysql_fetch_row(result)) )
	{
		my_ulonglong num_rows=mysql_num_rows(result);  //返回结果集的行号
		printf("mysql_use_result行号: %lu\n",(unsigned long)num_rows);

		/* 显示该行号中的数据 */
		for(i = 0; i < num_fields; i++)
		{
			printf("values:%s   \n",row[i]);
		}
#if 1
		/* 显示该行中的每一列数据长度 */
		lengths = mysql_fetch_lengths(result);      //生成一维int数组(静态局部变量)，把首地址给lengths；储存每一列的长度
		for(i = 0; i < num_fields; i++)
		{
			printf("Column %u is %lu bytes in length.\n",i, lengths[i]);
		}
#endif
	}
#endif
	mysql_free_result(result);
	mysql_close(&conn);
	return 0;
}
#if 0
static char sql_query[1024 * 10];   // SQL语句
int sql_select(const char* const tbl_name, const char* const field, const char* const value, char* const sql_field[], char* const sql_value[])
{
        MYSQL conn;
        memset(sql_query, 0, sizeof (sql_query));
        mysql_init(&conn);  //如果传入的指针为空，会分配一个MYSQL的结构体，并由mysql_close()释放掉
        if(mysql_real_connect(&conn,"localhost","xcwy","xcwy_data","xcwy",3306,NULL,0)==NULL)
        {
                printf("Error %u: %s\n", mysql_errno(&conn), mysql_error(&conn));
                exit(1);
        }
        /* int mysql_query(MYSQL *mysql, const char *stmt_str) */
        /* Zero for success. Nonzero if an error occurred. */
        snprintf(sql_query, sizeof (sql_query)-1, "SELECT * FROM `%s` WHERE `%s` LIKE '%s'", tbl_name, field, value);
        if(mysql_query(&conn, sql_query))
        {
                printf("Error %u: %s\n", mysql_errno(&conn), mysql_error(&conn));
                exit(1);
        }

        MYSQL_RES* result;
        MYSQL_ROW row;
        unsigned long *lengths;
        MYSQL_FIELD *fields;

        result=mysql_use_result(&conn);  //内部会分配MYSQL_RES结构体，由mysql_free_result()释放掉

        int num_fields=mysql_num_fields(result);       //结果集有多少列
        printf("mysql_use_result列：%d\n",num_fields);

        /* fields是MYSQL_FIELD结构体数组的首地址，MYSQL_FIELD结构体的name项是列名 */
        fields = mysql_fetch_fields(result);      //估计fields指向的是静态局部变量
        int i=0;
        for(i = 0; i < num_fields; i++)
        {
                printf("Field %u is %s\n", i, fields[i].name);
		memcpy(sql_field[i], fields[i].name, strlen(fields[i].name));
        }
#if 1
        /* 实际检索来自服务器的行，检索结果集的下一行;直至为返回NULL，检索完毕 */
        while( NULL!=(row=mysql_fetch_row(result)) )
        {
                my_ulonglong num_rows=mysql_num_rows(result);  //返回结果集的行号
                printf("mysql_use_result行号: %lu\n",(unsigned long)num_rows);

                /* 显示该行号中的数据 */
                for(i = 0; i < num_fields; i++)
                {
                        printf("values:%s   \n",row[i]);
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
        mysql_free_result(result);
        mysql_close(&conn);
        return 0;
}
#endif




