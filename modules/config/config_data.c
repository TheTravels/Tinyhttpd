/************************ (C) COPYLEFT 2018 Merafour *************************

* File Name          : config_data.c
* Author             : Merafour
* Last Modified Date : 11/15/2019
* Description        : 加载配置文件并解析.
********************************************************************************
* https://merafour.blog.163.com
* merafour@163.com
* https://github.com/merafour
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include "config_data.h"
#include "../mysql/data_base.h"

static int load_mysql_cfg(struct config_load_obj* const _load_obj, const char *const section, struct mysql_cfg* const _cfg)
{
    // mysql
    _load_obj->fops.get_field_value(_load_obj, section, "Host", "localhost", _cfg->host);
    _cfg->port = 0xffff & _load_obj->fops.get_int(_load_obj, section, "Port", 0);
    _load_obj->fops.get_field_value(_load_obj, section, "User", "root", _cfg->user);
    _load_obj->fops.get_field_value(_load_obj, section, "Passwd", "123456", _cfg->passwd);
    _load_obj->fops.get_field_value(_load_obj, section, "db", "obd", _cfg->db);
    printf("MySQL Config[%s] Host:%s, Port:%d, User:%s, Passwd:%s, db:%s\n", section, _cfg->host, _cfg->port, _cfg->user, _cfg->passwd, _cfg->db); fflush(stdout);
    if(0!=_cfg->port) return 0;
    return -1;
}
static int load_transfer_cfg(struct config_load_obj* const _load_obj, const char *const section, struct data_transfer_cfg* const _cfg)
{
    // data_transfer_cfg
    _load_obj->fops.get_field_value(_load_obj, section, "Host", "localhost", _cfg->host);
    _cfg->port = 0xffff & _load_obj->fops.get_int(_load_obj, section, "Port", 0);
    _load_obj->fops.get_field_value(_load_obj, section, "Format", "bin", _cfg->format);
    _load_obj->fops.get_field_value(_load_obj, section, "TurnOn", "no", _cfg->turn_on);
    if(0==strcmp(_cfg->format, "bin")) _cfg->_format = 0;
    else if(0==strcmp(_cfg->format, "json")) _cfg->_format = 1;
    else if(0==strcmp(_cfg->format, "all")) _cfg->_format = 2;
    else _cfg->_format = 0;
    if(0==strcmp(_cfg->turn_on, "no")) _cfg->_turn_on = 0;
    else if(0==strcmp(_cfg->turn_on, "yes")) _cfg->_turn_on = 1;
    else _cfg->_turn_on = 0;
    if(0!=_cfg->port) return 0;
    return -1;
}
static int load_vin_cfg(struct config_load_obj* const _load_obj, const char *const section, struct get_vin_cfg* const _cfg)
{
    // get_vin_cfg
    _load_obj->fops.get_field_value(_load_obj, section, "Host", "localhost", _cfg->host);
    _cfg->port = 0xffff & _load_obj->fops.get_int(_load_obj, section, "Port", 0);
    _cfg->connect_counts = 0xffff & _load_obj->fops.get_int(_load_obj, section, "ConnectCounts", 3);
    _load_obj->fops.get_field_value(_load_obj, section, "TurnOn", "no", _cfg->turn_on);
    if(0==strcmp(_cfg->turn_on, "no")) _cfg->_turn_on = 0;
    else if(0==strcmp(_cfg->turn_on, "yes")) _cfg->_turn_on = 1;
    else _cfg->_turn_on = 0;
    if(0!=_cfg->port) return 0;
    return -1;
}
static int load_host_cfg(struct config_load_obj* const _load_obj, const char *const section, struct host_listening* const _cfg)
{
    _load_obj->fops.get_field_value(_load_obj, section, "Host", "0.0.0.0", _cfg->host);
    _cfg->port = 0xffff & _load_obj->fops.get_int(_load_obj, section, "Port", 0);
    return 0;
}
static int data_load_func(struct config_load_obj* const _load_obj)
{
    int i;
    int ret=-1;
    int result = 0;
    char server[32] = "Server1";
    struct local_config_data* const _data = (struct local_config_data*)_load_obj->data;
    const int _list_size = sizeof(_data->local_list)/sizeof(_data->local_list[0]);
    printf("[%s-%d] _cfg_path: %s\n", __func__, __LINE__, _load_obj->_cfg_path);  fflush(stdout);
    ret = _load_obj->fops.load(_load_obj);
    //printf("[%s-%d] cfg:%s\n", __func__, __LINE__, (char*)_load_obj->_stream);  fflush(stdout);
    //printf("[%s-%d] load:%d %d\n", __func__, __LINE__, ret, _load_obj->_len);  fflush(stdout);
    if(0==ret) // load data
    {
        _data->nCfgPthreadCounts_ = 0xffff & _load_obj->fops.get_int(_load_obj, "LocalCFG", "ThreadCounts", 0);
        //printf("@%s-%d\n", __func__, __LINE__);
        //_data->nCpuPhtreadCounts_ = 2;//sysconf(_SC_NPROCESSORS_ONLN);
        _data->nCpuPhtreadCounts_ = sysconf(_SC_NPROCESSORS_ONLN);
        if(0 == _data->nCfgPthreadCounts_) _data->nCfgPthreadCounts_ = _data->nCpuPhtreadCounts_ * 2;
        if(_data->nCfgPthreadCounts_<4) _data->nCfgPthreadCounts_ = 4;
        printf("nCfgPthreadCounts_:%d\n", _data->nCfgPthreadCounts_); fflush(stdout);

        load_host_cfg(_load_obj, "LocalCFG", &_data->local_listen);
        printf("LocalConfig host:%s, port:%d\n", _data->local_listen.host, _data->local_listen.port); fflush(stdout);
        load_host_cfg(_load_obj, "LocalCFG_CXX", &_data->local_listenCXX);
        printf("LocalConfigCXX host:%s, port:%d\n", _data->local_listenCXX.host, _data->local_listenCXX.port); fflush(stdout);
        load_host_cfg(_load_obj, "LocalGeneralDevice", &_data->device_cfg);
        printf("GeneralDevice host:%s, port:%d\n", _data->device_cfg.host, _data->device_cfg.port); fflush(stdout);
        //LOG_INFO << "LocalConfig, listenning on " << local_listen.host << ":" << local_listen.port;
        // 加载服务器列表
        for (i = 0; i < _list_size; i++)
        {
            //server[6] = '1' + (char)i;
            memset(server, 0, sizeof(server));
            snprintf(server, sizeof(server)-1, "Server%d", i+1);
            //GetPrivateProfileStringEx(server, "Host", "0.0.0.0", local_list[i].host);
            //local_list[i].port = 0xffff & GetPrivateProfileIntEx(server, "Port", def_port);
            load_host_cfg(_load_obj, server, &_data->local_list[i]);
            if(_data->local_list[i].port > 0) {printf("server%d host:%s, port:%d\n", i, _data->local_list[i].host, _data->local_list[i].port); fflush(stdout);}
            //LOG_INFO << "server, listenning on " << local_list[i].host << ":" << local_list[i].port;
        }
        // mysql
        result = load_mysql_cfg(_load_obj, "MySQLDefault", &_data->_mysql_cfg);
        if(0!=result)result = load_mysql_cfg(_load_obj, "MySQLConfig", &_data->_mysql_cfg);
        if(0!=result)result = load_mysql_cfg(_load_obj, "MySQL_VM", &_data->_mysql_cfg);
        printf("MySQL Config Host:%s, Port:%d, User:%s, Passwd:%s, db:%s\n", _data->_mysql_cfg.host, _data->_mysql_cfg.port, _data->_mysql_cfg.user, _data->_mysql_cfg.passwd, _data->_mysql_cfg.db); fflush(stdout);
        data_base_mysqlcfg(_data->_mysql_cfg.host, _data->_mysql_cfg.user, _data->_mysql_cfg.passwd, _data->_mysql_cfg.db, _data->_mysql_cfg.port);

        load_transfer_cfg(_load_obj, "DataTransfer", &_data->_data_transfer_cfg);
        printf("DataTransfer Config Host:%s, Port:%d, Format[%d]:%s, TurnOn[%d]:%s\n", _data->_data_transfer_cfg.host, _data->_data_transfer_cfg.port, _data->_data_transfer_cfg._format, _data->_data_transfer_cfg.format, _data->_data_transfer_cfg._turn_on, _data->_data_transfer_cfg.turn_on); fflush(stdout);
        load_vin_cfg(_load_obj, "GetVIN", &_data->_vin_cfg);
        printf("GetVIN Config Host:%s, Port:%d, TurnOn[%d]:%s ConnectCounts:%d\n", _data->_vin_cfg.host, _data->_vin_cfg.port, _data->_vin_cfg._turn_on, _data->_vin_cfg.turn_on, _data->_vin_cfg.connect_counts); fflush(stdout);

        _load_obj->fops.get_field_value(_load_obj, "LocalCFG", "vinList", "./upload/vin.list", _data->vinList);
        printf("vinList file:%s\n", _data->vinList); fflush(stdout);

        //printf("[%s-%d] \n", __func__, __LINE__);
        //result = load_mysql_cfg(_load_obj, "MySQLDefault", &_data->_mysql_cfg);
        //printf("MySQL Config Host:%s, Port:%d, User:%s, Passwd:%s, db:%s\n", _data->_mysql_cfg.host, _data->_mysql_cfg.port, _data->_mysql_cfg.user, _data->_mysql_cfg.passwd, _data->_mysql_cfg.db); fflush(stdout);
    }
    return ret;
}
static char load_data[1024*1024]; // 1M, 本地配文件最大为 1MB
static char _config_data_buf[sizeof(struct config_load_obj)];
static struct local_config_data __data;
struct config_load_obj* _local_config_data=NULL;
void local_config_data_init(const char _cfg_path[])
{
    //char cfg_path[256];
    //memset(cfg_path, 0, sizeof(cfg_path));
    //snprintf(cfg_path, sizeof(cfg_path)-1, "%s/ServerConfig.cfg", _cfg_dir);
    //printf("[%s-%d] \n", __func__, __LINE__);  fflush(stdout);
    //_local_config_data = config_load_dft.fops.constructed(&config_load_dft, _config_data_buf, data_load_func, "./ServerConfig.cfg", load_data, sizeof(load_data), &__data);
    _local_config_data = config_load_dft.fops.constructed(&config_load_dft, _config_data_buf, data_load_func, _cfg_path, load_data, sizeof(load_data), &__data);
}
