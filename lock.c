#include <stdio.h>
#include <pthread.h>
#include "lock.h"

static pthread_mutex_t mutex;
int pthread_lock_init(void)
{
	return pthread_mutex_init(&mutex, NULL);
}

int pthread_lock(void)
{
	return pthread_mutex_lock(&mutex);
}

int pthread_unlock(void)
{
	return pthread_mutex_unlock(&mutex);
}

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






