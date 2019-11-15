/*
 * 数据包存储
 * json_table: {
 *     index: 1
 *     item0:{
 *          Filename: VIN1234567890-091236.json
 *          protocol: ShangHai
 *     }
 *     item1:{
 *          filename: VIN1234567890-091336.json
 *          protocol: ShangHai
 *     }
 * }
 * json_log: {
 *     index: 1
 *     login:{
 *     }
 *     UTC:{
 *     }
 *     logout:{
 *     }
 *     item0:{
 *          hex: pack ==> hex
 *          text...
 *     }
 *     item1:{
 *          hex: pack ==> hex
 *          text..
 *     }
 * }
 */

#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "storage_pack.h"
#include "../cJSON/cJSON.h"
#include "../cJSON/mem_malloc.h"
#include "agreement.h"
#include "obd_agree_shanghai.h"

#ifdef offsetof
#undef offsetof
#endif
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:    the pointer to the member.
 * @type:   the type of the container struct this is embedded in.
 * @member: the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({          \
    const typeof(((type *)0)->member)*__mptr = (ptr);    \
             (type *)((char *)__mptr - offsetof(type, member)); })

//#define   json_table    "json_table.json"
//#define   json_table_name    "json_table"
static const char* const json_table = "json_table.json";
static const char* const json_table_name = "json_table";
#define   Index_Max     5000
static char json_table_buf[1024*32]={0}; // 32K
//static long json_table_size=0;
#if 0
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void json_table_update(const char* filename, const char* protocol)
{
    cJSON *_root = NULL;
    cJSON *item_json = NULL;
//    cJSON *info = NULL;
//    cJSON *data = NULL;
    long _size=0;
    int Index = 0;
    char item[128];
    char *out = NULL;
    int fd = open(json_table, O_RDONLY);  // r : 只能读, 必须存在, 可在任意位置读取
    Index = 1;
    memset(item, 0, sizeof (item));
    // create file
    if(fd < 0)
    {
        printf("file %s not exist! \n", json_table);  fflush(stdout);
empty:       // empty
       _root = cJSON_CreateObject();
       cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
       cJSON_AddNumberToObject(_root, "Index", Index);
       snprintf(item, sizeof (item)-1, "Item%d", Index);
       cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
       cJSON_AddStringToObject(item_json, "Filename", filename);
       cJSON_AddStringToObject(item_json, "Protocol", protocol);
       out = cJSON_Print(_root);
       cJSON_Delete(_root);
    }
    else
    {
        printf("file %s already exist! \n", json_table);  fflush(stdout);
        lseek(fd, 0, SEEK_END);
        _size = tell(fd);
        lseek(fd, 0, SEEK_SET);
        printf("_size %s :%d \n", json_table, _size);  fflush(stdout);
        if(_size<=0)
        {
            close(fd);
            goto empty;
        }
        // read
        memset(json_table_buf, 0, sizeof (json_table_buf));
        _size = read(fd, json_table_buf, _size);
        printf("read: %d\n", _size); fflush(stdout);
        printf("json: %s\n", json_table_buf); fflush(stdout);
        close(fd);
        // change Index
        cJSON *index_json = NULL;


        printf("cJSON_Parse\n"); fflush(stdout);
        _root = cJSON_Parse(json_table_buf);
        if(NULL == _root)  // 数据损坏
        {
            printf("data bad\n"); fflush(stdout);
            goto empty;
        }

        printf("cJSON_GetObjectItem\n"); fflush(stdout);
        index_json = cJSON_GetObjectItem(_root, "Index");
        if(NULL == index_json)  // 数据损坏
        {
            printf("index_json bad\n"); fflush(stdout);
            cJSON_Delete(_root);
            return;
        }
        Index = index_json->valueint+1;
        printf("Index : %d  \n", Index);  fflush(stdout);
        index_json = cJSON_CreateNumber(Index);
        cJSON_ReplaceItemInObject(_root, "Index", index_json);
        snprintf(item, sizeof (item)-1, "Item%d", Index);
        cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
        cJSON_AddStringToObject(item_json, "Filename", filename);
        cJSON_AddStringToObject(item_json, "Protocol", protocol);
        out = cJSON_Print(_root);
        cJSON_Delete(_root);
    }
    printf("data write to %s  \n", json_table);  fflush(stdout);
   // out = cJSON_Print(_root);
    fd = open(json_table, O_RDWR);  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    printf("fd:%d\n", fd); fflush(stdout);
    printf("data write to %s  \n", json_table);  fflush(stdout);
    write(fd, out, strlen(out));
    //close(fd);
    mem_free(out);
    close(fd);
    fflush(stdout);
}
#else
void json_table_update(const char* filename, const char* protocol)
{
    cJSON *_root = NULL;
    cJSON *item_json = NULL;
//    cJSON *info = NULL;
//    cJSON *data = NULL;
    long _size=0;
    int Index = 0;
    int json_id = 0;
    char item[128];
    char *out = NULL;
#if 0
    FILE* fd = fopen(json_table, "r");  // r : 只能读, 必须存在, 可在任意位置读取
    Index = 1;
    memset(item, 0, sizeof (item));
    // create file
    if(NULL == fd)
    {
        printf("file %s not exist! \n", json_table);  fflush(stdout);
empty:       // empty
       _root = cJSON_CreateObject();
       cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
       cJSON_AddNumberToObject(_root, "Index", Index);
       snprintf(item, sizeof (item)-1, "Item%d", Index);
       cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
       cJSON_AddStringToObject(item_json, "Filename", filename);
       cJSON_AddStringToObject(item_json, "Protocol", protocol);
       out = cJSON_Print(_root);
       cJSON_Delete(_root);
    }
    else
    {
        printf("file %s already exist! \n", json_table);  fflush(stdout);
        fseek(fd, 0, SEEK_END);
        _size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        printf("_size %s :%d \n", json_table, _size);  fflush(stdout);
        if(_size<=0)
        {
            fclose(fd);
            goto empty;
        }
        // read
        memset(json_table_buf, 0, sizeof (json_table_buf));
        _size = fread(json_table_buf, _size, 1, fd);
        printf("read: %d | %d\n", _size, ftell(fd)); fflush(stdout);
        //printf("json: %s\n", json_table_buf); fflush(stdout);
        fclose(fd);
        //if(_size<=0) return;
        // change Index
        cJSON *index_json = NULL;


        //printf("cJSON_Parse\n"); fflush(stdout);
        _root = cJSON_Parse(json_table_buf);
        if(NULL == _root)  // 数据损坏
        {
            printf("data bad\n"); fflush(stdout);
            goto empty;
        }

        //printf("cJSON_GetObjectItem\n"); fflush(stdout);
        index_json = cJSON_GetObjectItem(_root, "Index");
        if(NULL == index_json)  // 数据损坏
        {
            printf("index_json bad\n"); fflush(stdout);
            cJSON_Delete(_root);
            return;
        }
        Index = index_json->valueint+1;
        //printf("Index : %d  \n", Index);  fflush(stdout);
        index_json = cJSON_CreateNumber(Index);
        cJSON_ReplaceItemInObject(_root, "Index", index_json);
        snprintf(item, sizeof (item)-1, "Item%d", Index);
        cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
        cJSON_AddStringToObject(item_json, "Filename", filename);
        cJSON_AddStringToObject(item_json, "Protocol", protocol);
        out = cJSON_Print(_root);
        cJSON_Delete(_root);
    }
    //printf("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    printf("%s\n", out); fflush(stdout);
    fd = fopen(json_table, "w+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        printf("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        return;
    }
    fwrite(out, strlen(out), 1, fd);
    //fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    mem_free(out);
    fflush(stdout);
#else
    FILE* fd = fopen(json_table, "r");  //
    Index = 1;
    memset(item, 0, sizeof (item));
    // create file
    if(NULL == fd)
    {
        printf("file %s not exist!..4 \n", json_table);  fflush(stdout);
        //fclose(fd);
    }
    else
    {
        printf("file %s already exist! \n", json_table);  fflush(stdout);
        fseek(fd, 0, SEEK_END);
        _size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        printf("_size %s :%ld \n", json_table, _size);  fflush(stdout);
        if(_size<=0)
        {
            fclose(fd);
            goto empty;
        }
        // read end line
        memset(json_table_buf, 0, sizeof (json_table_buf));
        if(_size>300) fseek(fd, _size-300, SEEK_SET);
        while(1)
        {
            long _tail=0;
            char* line = fgets(json_table_buf, sizeof (json_table_buf), fd);
            _tail = ftell(fd);
            if((NULL == line) && (_tail<_size))
            {
                fclose(fd);
                 goto empty;
            }
            if(_tail>=_size) break;
        }
        printf("_tail: %ld\n", ftell(fd)); fflush(stdout);
        //printf("json: %s\n", json_table_buf); fflush(stdout);
        fclose(fd);
        //if(_size<=0) return;
        // change Index
        cJSON *index_json = NULL;

        //printf("cJSON_Parse\n"); fflush(stdout);
        _root = cJSON_Parse(json_table_buf);
        if(NULL == _root)  // 数据损坏
        {
            printf("data bad\n"); fflush(stdout);
            goto empty;
        }

        //printf("cJSON_GetObjectItem\n"); fflush(stdout);
        index_json = cJSON_GetObjectItem(_root, "Index");
        if(NULL == index_json)  // 数据损坏
        {
            printf("index_json bad\n"); fflush(stdout);
            cJSON_Delete(_root);
            return;
        }
        Index = index_json->valueint+1;
        index_json = cJSON_GetObjectItem(_root, "JSON");
        if(NULL == index_json)  // 数据损坏
        {
            printf("json_id bad\n"); fflush(stdout);
            cJSON_Delete(_root);
            return;
        }
        json_id = index_json->valueint;
        cJSON_Delete(_root);
        if(Index_Max<=Index)
        {
            // move file
            char new_name[128];
            memset(new_name, 0, sizeof (new_name));
            snprintf(new_name, sizeof (new_name), "%s-%d.json", json_table_name, json_id);
            rename(json_table, new_name);
            Index=0;
            json_id++;
        }
    }
empty:
    _root = cJSON_CreateObject();
    //cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddNumberToObject(_root, "JSON", json_id);
    cJSON_AddNumberToObject(_root, "Index", Index);
    snprintf(item, sizeof (item)-1, "Item%d", Index);
    cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
    cJSON_AddStringToObject(item_json, "Filename", filename);
    cJSON_AddStringToObject(item_json, "Protocol", protocol);
    //out = cJSON_Print(_root);
    out = cJSON_PrintUnformatted(_root);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return;
    }
    //printf("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    printf("%s\n", out); fflush(stdout);
    fd = fopen(json_table, "a+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        printf("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        return;
    }
    fwrite(out, strlen(out), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    mem_free(out);
    printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
#endif
}
#endif

/*
 * 数据包存储
 * json_log: {
 *     index: 1
 *     login:{
 *     }
 *     UTC:{
 *     }
 *     logout:{
 *     }
 *     item0:{
 *          hex: pack ==> hex
 *          text...
 *     }
 *     item1:{
 *          hex: pack ==> hex
 *          text..
 *     }
 * }
 */
static int handle_report_real(cJSON *const _root, const struct general_pack_shanghai *const _pack)
{
    const struct shanghai_report_real *const msg = (const struct shanghai_report_real *const)(_pack->data);
    uint16_t index=0;
//    uint16_t data_len=0;
    struct report_head* nmsg=NULL;  // next msg
    const struct report_head* fault=NULL;
    const struct shanghai_data_obd *obd=NULL;
    const struct shanghai_data_stream *stream=NULL;
    const struct shanghai_data_att *att=NULL;

    //pr_debug("Report [%d]: %s \n", msg->count, msg->UTC);
    cJSON_AddNumberToObject(_root, "count", msg->count);
    cJSON_AddStringToObject(_root, "UTC", (const char * const)msg->UTC);
    fflush(stdout);
    // 消息，可包含多条
    nmsg = msg->msg;
    //pr_debug("msg.msg : %d \n", (NULL!=msg->msg)); fflush(stdout);
    while(NULL!=nmsg)
    {
        //pr_debug("type_msg : %d \n", nmsg->type_msg); fflush(stdout);
        switch (nmsg->type_msg)
        {
            // 1） OBD 信息数据格式和定义见表 A.6 所示。
            case MSG_OBD:       // 0x01  OBD 信息
                obd = (const struct shanghai_data_obd *) container_of(nmsg, struct shanghai_data_obd, head);
                cJSON_AddNumberToObject(_root, "protocol", obd->protocol);
                cJSON_AddNumberToObject(_root, "MIL", obd->MIL);
                cJSON_AddNumberToObject(_root, "status", obd->status);
                cJSON_AddNumberToObject(_root, "ready", obd->ready);
                cJSON_AddStringToObject(_root, "VIN", (const char * const)obd->VIN);
                cJSON_AddStringToObject(_root, "SVIN", (const char * const)obd->SVIN);
                cJSON_AddStringToObject(_root, "CVN", (const char * const)obd->CVN);
                //pr_debug("MSG_OBD IUPR: %d \n", obd->IUPR);
                cJSON_AddNumberToObject(_root, "fault_total", obd->fault_total);
                fault = &(obd->fault_list);
                //while(NULL!=fault)
                for(index=0; index<obd->fault_total; index++)
                {
                    cJSON_AddNumberToObject(_root, "fault", fault->data);  // 故障码, BYTE（4）
                    fault = fault->next;  // 下一个数据
                    if(NULL==fault) break;
                }
                fflush(stdout);
                break;
            // 2）数据流信息数据格式和定义见表 A.7 所示，补充数据流信息数据格式和定义见表 A.8 所示。
            case MSG_STREAM:     // 0x02  数据流信息
                stream = (const struct shanghai_data_stream *)container_of(nmsg, struct shanghai_data_stream, head);
                cJSON_AddNumberToObject(_root, "speed", stream->speed);
                cJSON_AddNumberToObject(_root, "kPa", stream->kPa);
                cJSON_AddNumberToObject(_root, "Nm", stream->Nm);
                cJSON_AddNumberToObject(_root, "Nmf", stream->Nmf);
                cJSON_AddNumberToObject(_root, "rpm", stream->rpm);
                cJSON_AddNumberToObject(_root, "Lh", stream->Lh);
                cJSON_AddNumberToObject(_root, "ppm_down", stream->ppm_down);
                cJSON_AddNumberToObject(_root, "urea_level", stream->urea_level);
                cJSON_AddNumberToObject(_root, "kgh", stream->kgh);
                cJSON_AddNumberToObject(_root, "SCR_in", stream->SCR_in);
                cJSON_AddNumberToObject(_root, "SCR_out", stream->SCR_out);
                cJSON_AddNumberToObject(_root, "DPF", stream->DPF);
                cJSON_AddNumberToObject(_root, "coolant_temp", stream->coolant_temp);
                cJSON_AddNumberToObject(_root, "tank_level", stream->tank_level);
                cJSON_AddNumberToObject(_root, "gps_status", stream->gps_status);
                cJSON_AddNumberToObject(_root, "longitude", stream->longitude);
                cJSON_AddNumberToObject(_root, "latitude", stream->latitude);
                cJSON_AddNumberToObject(_root, "mileages_total", stream->mileages_total);
                break;
            case MSG_STREAM_ATT: // 0x80  补充数据流
                att = (const struct shanghai_data_att *)container_of(nmsg, struct shanghai_data_att, head);
                cJSON_AddNumberToObject(_root, "Nm_mode", att->Nm_mode);
                cJSON_AddNumberToObject(_root, "accelerator", att->accelerator);
                cJSON_AddNumberToObject(_root, "oil_consume", att->oil_consume);
                cJSON_AddNumberToObject(_root, "urea_tank_temp", att->urea_tank_temp);
                cJSON_AddNumberToObject(_root, "mlh_urea_actual", att->mlh_urea_actual);
                cJSON_AddNumberToObject(_root, "mlh_urea_total", att->mlh_urea_total);
                cJSON_AddNumberToObject(_root, "exit_gas_temp", att->exit_gas_temp);
                fflush(stdout);
                break;
            // 0x03-0x7F  预留
            // 0x81~0xFE  用户自定义
            default:
                break;
        }
        nmsg = nmsg->next;   // 下一个数据
    }
    fflush(stdout);
    return 0;
}
int json_ShangHai_update(const char* filename, const struct general_pack_shanghai* const _pack)
{
    cJSON *_root = NULL;
    cJSON *item_json = NULL;
//    cJSON *info = NULL;
    cJSON *data = NULL;
    long _size=0;
    int Index = 0;
    int json_id = 0;
    char item[128];
//    char hex[1024];
//    int count=0;
    char *out = NULL;
    const struct shanghai_login *request;
    FILE* fd = fopen(filename, "r");  //
    Index = 1;
    memset(item, 0, sizeof (item));
    // create file
    if(NULL == fd)
    {
        printf("file %s not exist!..5 \n", filename);  fflush(stdout);
        fclose(fd);
    }
    else
    {
        printf("file %s already exist! \n", filename);  fflush(stdout);
        fseek(fd, 0, SEEK_END);
        _size = ftell(fd);
        fseek(fd, 0, SEEK_SET);
        printf("_size %s :%ld \n", filename, _size);  fflush(stdout);
        if(_size<=0)
        {
            fclose(fd);
            goto empty;
        }
        // read end line
        memset(json_table_buf, 0, sizeof (json_table_buf));
        if(_size>300) fseek(fd, _size-300, SEEK_SET);
        while(1)
        {
            long _tail=0;
            char* line = fgets(json_table_buf, sizeof (json_table_buf), fd);
            _tail = ftell(fd);
            if((NULL == line) && (_tail<_size))
            {
                fclose(fd);
                 goto empty;
            }
            if(_tail>=_size) break;
        }
        printf("_tail: %ld\n", ftell(fd)); fflush(stdout);
        //printf("json: %s\n", json_table_buf); fflush(stdout);
        fclose(fd);
        //if(_size<=0) return;
        // change Index
        cJSON *index_json = NULL;

        //printf("cJSON_Parse\n"); fflush(stdout);
        _root = cJSON_Parse(json_table_buf);
        if(NULL == _root)  // 数据损坏
        {
            printf("data bad\n"); fflush(stdout);
            goto empty;
        }

        //printf("cJSON_GetObjectItem\n"); fflush(stdout);
        index_json = cJSON_GetObjectItem(_root, "Index");
        if(NULL == index_json)  // 数据损坏
        {
            printf("index_json bad\n"); fflush(stdout);
            cJSON_Delete(_root);
            goto ret;
        }
        Index = index_json->valueint+1;
        index_json = cJSON_GetObjectItem(_root, "JSON");
        if(NULL == index_json)  // 数据损坏
        {
            printf("json_id bad\n"); fflush(stdout);
            cJSON_Delete(_root);
            goto ret;
        }
        json_id = index_json->valueint;
        cJSON_Delete(_root);
//        if(Index_Max<=Index)
//        {
//            // move file
//            char new_name[128];
//            memset(new_name, 0, sizeof (new_name));
//            snprintf(new_name, sizeof (new_name), "%s-%d.json", filename_name, json_id);
//            rename(filename, new_name);
//            Index=0;
//            json_id++;
//        }
        //fclose(fd);
    }
empty:
    _root = cJSON_CreateObject();
    //cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    cJSON_AddNumberToObject(_root, "JSON", json_id);
    cJSON_AddNumberToObject(_root, "Index", Index);
    cJSON_AddNumberToObject(_root, "Cmd", _pack->cmd);
    cJSON_AddStringToObject(_root, "VIN", (const char * const)_pack->VIN);
    cJSON_AddNumberToObject(_root, "softV", _pack->soft_version);
    cJSON_AddNumberToObject(_root, "ssl", _pack->ssl);
    cJSON_AddNumberToObject(_root, "data_len", _pack->data_len);
    cJSON_AddNumberToObject(_root, "BCC", _pack->BCC);
    snprintf(item, sizeof (item)-1, "Item%d", Index);
    //cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
    //cJSON_AddStringToObject(item_json, "Filename", filename);
    item_json = _root;
    switch(_pack->cmd)
    {
        case CMD_LOGIN:        // 车辆登入
            request = (const struct shanghai_login *)(_pack->data);
            cJSON_AddItemToObject(_root, "Login", data = cJSON_CreateObject());
            cJSON_AddStringToObject(data, "UTC", (const char * const)request->UTC);
            cJSON_AddNumberToObject(data, "count", request->count);
            cJSON_AddStringToObject(data, "UTC", (const char * const)request->ICCID);
            break;
        case CMD_REPORT_REAL:  // 实时信息上报
            if(NULL==item_json) cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
            cJSON_AddItemToObject(item_json, "Report", data = cJSON_CreateObject());
            handle_report_real(item_json, _pack);
            break;
        case CMD_REPORT_LATER: // 补发信息上报
            if(NULL==item_json) cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
            cJSON_AddItemToObject(item_json, "Report_Later", data = cJSON_CreateObject());
            handle_report_real(item_json, _pack);
            break;
        case CMD_LOGOUT:       // 车辆登出
            if(NULL==item_json) cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
            //pack_len = handle_request_logout((const struct shanghai_logout *const)msg_buf, pack);
            cJSON_AddItemToObject(item_json, "Logout", data = cJSON_CreateObject());
            cJSON_AddStringToObject(data, "UTC", (const char * const)((const struct shanghai_logout *)(_pack->data))->UTC);
            cJSON_AddNumberToObject(data, "count", ((const struct shanghai_logout *)(_pack->data))->count);
            break;
        case CMD_UTC:          // 终端校时
            if(NULL==item_json) cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
            cJSON_AddItemToObject(item_json, "UTC", data = cJSON_CreateObject());
            cJSON_AddStringToObject(data, "终端校时", "OK");
            break;
        case CMD_USERDEF:      // 用户自定义
            if(NULL==item_json) cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
            cJSON_AddItemToObject(item_json, "user def", data = cJSON_CreateObject());
            cJSON_AddStringToObject(data, "自定义", "不支持");
            break;
        default:
            if(NULL==item_json) cJSON_AddItemToObject(_root, item, item_json = cJSON_CreateObject());
            cJSON_AddItemToObject(item_json, "default", data = cJSON_CreateObject());
            cJSON_AddStringToObject(data, "default", "不支持");
            break;
    }
//    memset(hex, 0, sizeof (hex));
//    for(count=0; count<sizeof (hex)-1; count++)
//    {
//        hex[]
//    }
//    cJSON_AddStringToObject(item_json, "HEX", filename);
    //out = cJSON_Print(_root);
    out = cJSON_PrintUnformatted(_root);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        goto ret;
    }
    printf("data write to %s  \n", filename);  fflush(stdout);
    printf("%s\n", out); fflush(stdout);
    fd = fopen(filename, "a+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        printf("fopen fail!\n"); fflush(stdout);
        mem_free(out);
        fflush(stdout);
        goto ret;
    }
    fwrite(out, strlen(out), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    mem_free(out);
    printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
ret:
    return Index;
}

/*
 struct device_list{   // 设备列表
    struct list_head list;
    int socket;       // TCP / IP
    int save_log;
    int type;
    char sn[32];      // 序列号
    char VIN[32];     // 车辆识别码
    struct list_head msg_list;  // 消息链表
};
 */
int json_device(char* buffer, const uint16_t _bsize, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize)
{
    cJSON *_root = NULL;
    char time[32];
    char hex[1024];
    char *out = NULL;
    int i=0, j=0;

    if(_psize>=(sizeof (hex)/4)) return -1;
    memset(hex, 0, sizeof (hex));
    _root = cJSON_CreateObject();
    //cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    memset(time, 0, sizeof (time));
    snprintf(time, sizeof(time)-1, "%04d.%02d.%02d %02d:%02d:%02d", device->UTC[0]+2000, device->UTC[1], device->UTC[2]\
            , device->UTC[3], device->UTC[4], device->UTC[5]);
    cJSON_AddStringToObject(_root, "UTC", time);
    cJSON_AddNumberToObject(_root, "socket", device->socket);
    cJSON_AddNumberToObject(_root, "save_log", device->save_log);
    cJSON_AddNumberToObject(_root, "type", device->type);
    cJSON_AddStringToObject(_root, "SN", device->sn);
    cJSON_AddStringToObject(_root, "VIN", device->VIN);
    cJSON_AddNumberToObject(_root, "psize", _psize);
    //printf("_psize:%d\n", _psize); fflush(stdout);
    for(i=0, j=0; i<_psize; i++)
    {
        sprintf(&hex[j], " %02X", pack[i]);
        j += 3;
    }
    cJSON_AddStringToObject(_root, "msg_hex", hex);
    //out = cJSON_Print(_root);
    out = cJSON_PrintUnformatted(_root);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        //printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return -2;
    }
    //printf("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    //printf("%s\n", out); fflush(stdout);
    memset(buffer, 0, _bsize);
    memcpy(buffer, out, strlen(out));
    mem_free(out);
    //printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}
int json_obd(char* buffer, const uint16_t _bsize, const struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize)
{
    cJSON *_root = NULL;
    char time[32];
    char hex[1024];
    char *out = NULL;
    int i=0, j=0;

    if(_psize>=(sizeof (hex)/4)) return -1;
    memset(hex, 0, sizeof (hex));
    _root = cJSON_CreateObject();
    //cJSON_AddStringToObject(_root, "cJSON Version", cJSON_Version());
    memset(time, 0, sizeof (time));
    snprintf(time, sizeof(time)-1, "%04d.%02d.%02d %02d:%02d:%02d", _obd_fops->UTC[0]+2000, _obd_fops->UTC[1], _obd_fops->UTC[2]\
            , _obd_fops->UTC[3], _obd_fops->UTC[4], _obd_fops->UTC[5]);
    cJSON_AddStringToObject(_root, "UTC", time);
    //cJSON_AddNumberToObject(_root, "socket", device->socket);
    //cJSON_AddNumberToObject(_root, "save_log", device->save_log);
    cJSON_AddNumberToObject(_root, "type", _obd_fops->fops->protocol);
    cJSON_AddStringToObject(_root, "SN", _obd_fops->sn);
    cJSON_AddStringToObject(_root, "VIN", _obd_fops->VIN);
    cJSON_AddNumberToObject(_root, "psize", _psize);
    //printf("_psize:%d\n", _psize); fflush(stdout);
    for(i=0, j=0; i<_psize; i++)
    {
        sprintf(&hex[j], " %02X", pack[i]);
        j += 3;
    }
    cJSON_AddStringToObject(_root, "msg_hex", hex);
    //out = cJSON_Print(_root);
    out = cJSON_PrintUnformatted(_root);
    cJSON_Delete(_root);
    if(NULL==out)
    {
        //printf("no memory, perused: %d  \n", mem_perused());  fflush(stdout);
        return -2;
    }
    //printf("data write to %s  \n", json_table);  fflush(stdout);
    //out = cJSON_Print(_root);
    //printf("%s\n", out); fflush(stdout);
    memset(buffer, 0, _bsize);
    memcpy(buffer, out, strlen(out));
    mem_free(out);
    //printf("memory perused: %d  \n", mem_perused());  fflush(stdout);
    fflush(stdout);
    return 0;
}

int json_device_parse(const char *json_buf, char hex[], const uint16_t _hsize, struct device_list* const device, int *const _psize)
{
    cJSON *_root = NULL;
    //int _psize = 0;
    cJSON *node_json = NULL;
    _root = cJSON_Parse(json_buf);
    if(NULL==_root) return -2; //goto _error;
    node_json = cJSON_GetObjectItem(_root, "socket");
    if(NULL==node_json) goto _error;
    device->socket = node_json->valueint;
    node_json = cJSON_GetObjectItem(_root, "save_log");
    if(NULL==node_json) goto _error;
    device->save_log = node_json->valueint;
    node_json = cJSON_GetObjectItem(_root, "type");
    if(NULL==node_json) goto _error;
    device->type = node_json->valueint;
    node_json = cJSON_GetObjectItem(_root, "SN");
    if(NULL==node_json) goto _error;
    memcpy(device->sn, node_json->valuestring, strlen(node_json->valuestring));
    node_json = cJSON_GetObjectItem(_root, "VIN");
    if(NULL==node_json) goto _error;
    memcpy(device->VIN, node_json->valuestring, strlen(node_json->valuestring));
    node_json = cJSON_GetObjectItem(_root, "psize");
    if(NULL==node_json) goto _error;
    *_psize = node_json->valueint;
    if(*_psize>_hsize) goto _error;
    node_json = cJSON_GetObjectItem(_root, "msg_hex");
    if(NULL==node_json) goto _error;
    memcpy(hex, node_json->valuestring, strlen(node_json->valuestring));

    cJSON_Delete(_root);
    return 0;

_error:
    cJSON_Delete(_root);
    return -1;
}
int json_device_save(const char* filename, struct device_list* const device, const uint8_t pack[], const uint16_t _psize)
{
    FILE* fd = NULL;
    char buffer[2048];
//    struct list_head *head=NULL;
//    uint16_t index=0;

    if(sizeof (device->cache)>_psize)  // save data to ram
    {
        device->len = _psize;
        device->write++;
        memcpy(device->cache, pack,  _psize);
        //printf("json_device_save[0x%08x]:%d | %d\n", device, _psize, device->write);
    }
    if(NULL==filename)
    {
        return  0;
    }
    json_device(buffer, sizeof(buffer), device, pack,  _psize);
    fd = fopen(filename, "a+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        //printf("fopen fail!\n"); fflush(stdout);
        //fflush(stdout);
        return -3;
    }
    fwrite(buffer, strlen(buffer), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    //fflush(stdout);
    return 0;
}
int json_obd_save(const char* filename, struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize)
{
    FILE* fd = NULL;
    char buffer[2048];

    if(NULL==filename)
    {
        return  0;
    }
    json_obd(buffer, sizeof(buffer), _obd_fops, pack,  _psize);
    fd = fopen(filename, "a+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        //printf("fopen fail!\n"); fflush(stdout);
        fflush(stdout);
        return -3;
    }
    fwrite(buffer, strlen(buffer), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    fflush(stdout);
    return 0;
}





