#ifndef _CARINFO_H
#define _CARINFO_H

#ifdef __cplusplus
extern "C"
{
#endif

extern const char* CarListPath;

extern int CreateCarInfo(const char* path);
extern int CarInfoSearch(const char* path, const char* VIN, char data[]);


#ifdef __cplusplus
}
#endif

#endif // _CARINFO_H
