void MySqlInit(void) 初始化数据库以及连接数据库
void MySqlSeve(const uint8_t VIN[],uint32_t longitude,uint32_t latitude) 将VIN码经纬度存入数据库
void MySqlClose(void) 关闭数据库

include 文件夹为数据库操作所需头文件 应放到OBD_Report文件夹

makefile编译需要加上 -lmysqlclient
