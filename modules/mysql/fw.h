#ifndef FW_H
#define FW_H

#ifdef	__cplusplus
extern "C" {
#endif


    extern void get_fw(void);
    extern int sql_insert_sn(const char hard_id[], const char sn[]);
    extern void MySqlInit(void);
    extern int insert_item(const char* const field, const double value);
    extern int insert_item_string(const char* const field, const char* const string_value);
    extern int insert_item_int(const char* const field, const int value);
    extern int insert_sql(void);
    extern int insert_sql_fw_update(void);
    extern int sql_insert_fw_update(const char vin[], const char version[], const char Des[], const char hard_id[], const char sn[], const int fw_crc);
    extern int sql_select(const char* const tbl_name, const char* const field, const char* const value, char* const sql_field[], char* const sql_value[]);
    extern void MySqlClose(void);

#ifdef	__cplusplus
}
#endif

#endif // FW_H
