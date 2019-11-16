#include "fw.h"
#include "sql.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
static pthread_mutex_t fw_mutex;
static pthread_mutex_t* mutex=NULL;
static char _sql_field[8][16];
static char* const sql_field[8] = {_sql_field[0], _sql_field[1], _sql_field[2], _sql_field[3], \
                           _sql_field[4], _sql_field[5], _sql_field[6], _sql_field[7], };
static char sql_value_id[32];
static char sql_value_create_time[32];
static char sql_value_vin[32];
static char sql_value_version[32];
static char sql_value_Des[512];
static char sql_value_hard_id[32];
static char sql_value_SN[32];
static char sql_value_fw_crc[32];
static char* const sql_value[8] = {sql_value_id, sql_value_create_time, sql_value_vin, sql_value_version, \
                           sql_value_Des, sql_value_hard_id, sql_value_SN, sql_value_fw_crc};

void get_fw(void)
{
    //int i=0;
    if(NULL==mutex)
    {
        //printf("@%s-%d\n", __func__, __LINE__);
        mutex = &fw_mutex;
        pthread_mutex_init(mutex, NULL);
    }
    //printf("@%s-%d\n", __func__, __LINE__);
    pthread_mutex_lock(mutex);
    //printf("@%s-%d\n", __func__, __LINE__);
    memset(_sql_field[0], 0, sizeof (_sql_field));
    memset(sql_value_id, 0, sizeof (sql_value_id));
    memset(sql_value_create_time, 0, sizeof (sql_value_create_time));
    memset(sql_value_vin, 0, sizeof (sql_value_vin));
    memset(sql_value_version, 0, sizeof (sql_value_version));
    memset(sql_value_Des, 0, sizeof (sql_value_Des));
    memset(sql_value_hard_id, 0, sizeof (sql_value_hard_id));
    memset(sql_value_SN, 0, sizeof (sql_value_SN));
    memset(sql_value_fw_crc, 0, sizeof (sql_value_fw_crc));
    //printf("@%s-%d\n", __func__, __LINE__);
    sql_select("tbl_obd_4g_fw_update", "vin", "FW_11111111111111", sql_field, sql_value);
    /*for(i=0; i<8; i++)
    {
        printf("%s : %s\n", sql_field[i], sql_value[i]); fflush(stdout);
    }*/
    //printf("@%s-%d\n", __func__, __LINE__);
    pthread_mutex_unlock(mutex);
}

int match_fw(const char* const cvn)
{
    int i=0;
    int match=-1;
    pthread_mutex_lock(mutex);
    for(i=0; i<8; i++)
    {
        match=-1;
#if 0
        if(0==strcmp("hard_id", sql_field[i]))
        {
            if(0==strcmp(cvn, sql_value[i]))
            {
                match = 0;
            }
            break;
        }
#else
        if(0==strcmp(cvn, sql_value[i]))
        {
            match = 0;
            break;
        }
#endif
    }
    pthread_mutex_unlock(mutex);
    return match;
}

int get_fw_crc(void)
{
    int crc=0;
    pthread_mutex_lock(mutex);
    crc = atoi(sql_value_fw_crc);
    pthread_mutex_unlock(mutex);
    return crc;
}




