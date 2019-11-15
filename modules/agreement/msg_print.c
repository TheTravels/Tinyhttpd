#include "msg_print.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <stdarg.h>

#include <time.h>
#include "DateTime.h"
#include <stdio.h>

static struct msg_print_obj* msg_print_constructed(struct msg_print_obj* const _print_fops, void* const _obj_fops, const char _prefix[], char* const _stream, const size_t _n)
{
    struct msg_print_obj _fops = { _print_fops->fops, "PathPrefix", _stream, _n};
    char* const prefix = _fops.PathPrefix;
    struct msg_print_obj* _obj = (struct msg_print_obj*)_obj_fops;
    memset(prefix, 0, sizeof(_fops.PathPrefix));
    memcpy(prefix, _prefix, strlen(_prefix));
    memcpy(_obj, &_fops, sizeof(_fops));
    return _obj;
}
static int init(struct msg_print_obj* const _fops)
{
    memset(_fops->__stream, 0, _fops->__n);
    return 0;
}

#if 0
int msg_print(char *__stream, const size_t __n, const char *__format, ...)
{
    char* text=NULL;
    size_t _size=0;
    va_list ap;
    _size = strlen(__stream);
    if(_size>=__n) return -1;
    text = &__stream[_size];
    va_start(ap, __format);
    //vprintf(__format, ap);
    //snprintf(text, sizeof (text), __format, ap);
    vsprintf(text, __format, ap);
    va_end(ap);
    return 0;
}
#endif
static int msg_print(struct msg_print_obj* const _fops, const char *__format, ...)
{
    char* text=NULL;
    char *const __stream = _fops->__stream;
    const size_t __n = _fops->__n;
    size_t _size=0;
    va_list ap;
    _size = strlen(__stream);
    if(_size>=__n) return -1;
    text = &__stream[_size];
    va_start(ap, __format);
    //vprintf(__format, ap);
    //snprintf(text, sizeof (text), __format, ap);
    vsprintf(text, __format, ap);
    va_end(ap);
    return 0;
}

static int msg_fflush(struct msg_print_obj* const _fops)
{
    FILE* fd = NULL;
    char filename[128];

    memset(filename, 0, sizeof(filename));
    _fops->fops->date2filename(_fops, time(NULL), filename, sizeof(filename), "print-");
    fd = fopen(filename, "a+");  // w+ : 可读可写, 可以不存在, 必会擦掉原有内容从头写
    if(NULL==fd)
    {
        //printf("fopen fail!\n"); fflush(stdout);
        //fflush(stdout);
        return -3;
    }
    fwrite(_fops->__stream, strlen(_fops->__stream), 1, fd);
    fwrite("\n", 1, 1, fd);
    fflush(fd);
    fclose(fd);
    //fflush(stdout);
    return 0;
}

static void utc2filename(struct msg_print_obj* const _fops, const uint32_t times, void* const buf, const size_t _size)
{
    DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
    DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};
    if(times > INT32_MAX)
    {
        utctime = GregorianCalendarDateAddSecond(utctime, INT32_MAX);
        utctime = GregorianCalendarDateAddSecond(utctime, (int)(times - INT32_MAX));
    }
    else
    {
        utctime = GregorianCalendarDateAddSecond(utctime, (int)times);
    }

    GregorianCalendarDateToModifiedJulianDate(utctime);
    localtime = GregorianCalendarDateAddHour(utctime, 8);
    snprintf((char *)buf, (size_t)_size, "./log/jsons-%d-%.2d-%.2d-%02d%02d%02d.txt", localtime.year, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
}
static void date2filename(struct msg_print_obj* const _fops, const uint32_t times, void* const buf, const size_t _size, const char* prefix)
{
    DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
    DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};
    if(times > INT32_MAX)
    {
        utctime = GregorianCalendarDateAddSecond(utctime, INT32_MAX);
        utctime = GregorianCalendarDateAddSecond(utctime, (int)(times - INT32_MAX));
    }
    else
    {
        utctime = GregorianCalendarDateAddSecond(utctime, (int)times);
    }

    GregorianCalendarDateToModifiedJulianDate(utctime);
    localtime = GregorianCalendarDateAddHour(utctime, 8);
    snprintf((char *)buf, (size_t)_size, "./log/%s-%d-%.2d-%.2d.txt", prefix, localtime.year, localtime.month, localtime.day);
}
static void utc_format(struct msg_print_obj* const _fops, const uint32_t times, uint8_t buf[], const size_t _size)
{
    (void)_size;
    DateTime      utctime   = {.year = 1970, .month = 1, .day = 1, .hour = 0, .minute = 0, .second = 0};
    DateTime      localtime = {.year = 1970, .month = 1, .day = 1, .hour = 8, .minute = 0, .second = 0};
    if(times > INT32_MAX)
    {
      utctime = GregorianCalendarDateAddSecond(utctime, INT32_MAX);
      utctime = GregorianCalendarDateAddSecond(utctime, (int)(times - INT32_MAX));
    }
    else
    {
      utctime = GregorianCalendarDateAddSecond(utctime, (int)times);
    }

    GregorianCalendarDateToModifiedJulianDate(utctime);
    localtime = GregorianCalendarDateAddHour(utctime, 8);
    //gpstime   = GregorianCalendarDateAddSecond(utctime, 18);
    //gpstimews = GregorianCalendarDateToGpsWeekSecond(gpstime);

#if debug_log
    printf("Local | %d-%.2d-%.2d %.2d:%.2d:%.2d | timezone UTC+8\n",
           localtime.year, localtime.month, localtime.day,
           localtime.hour, localtime.minute, localtime.second);

    printf("UTC   | %d-%.2d-%.2d %.2d:%.2d:%.2d \n",
           utctime.year, utctime.month, utctime.day,
           utctime.hour, utctime.minute, utctime.second);
    fflush(stdout);
#endif
    //snprintf((char *)buf, (size_t)_size, "%02d%02d%02d", localtime.hour, localtime.minute, localtime.second);
    //snprintf((char *)buf, (size_t)_size, "%1d%1d%1d%1d%1d%1d", localtime.year%100, localtime.month, localtime.day, localtime.hour, localtime.minute, localtime.second);
    buf[0] = localtime.year%100;
    buf[1] = localtime.month%12;
    buf[2] = localtime.day%31;
    buf[3] = localtime.hour%24;
    buf[4] = localtime.minute%59;
    buf[5] = localtime.second%59;
}

const struct msg_print_fops _msg_print_fops = {
    .constructed = msg_print_constructed,
    .init = init,
    .print = msg_print,
    .fflush = msg_fflush,
    .utc2filename = utc2filename,
    .date2filename = date2filename,
    .utc_format = utc_format,
};
static char __stream[1024*30] = "\0";
struct msg_print_obj _msg_obj = {
    .fops = &_msg_print_fops,
    .__stream = __stream,
    .__n = sizeof(__stream),
};

