#ifndef STORAGE_PACK_H
#define STORAGE_PACK_H

#include <stdint.h>
#include "agreement.h"
#include "obd_agree_shanghai.h"
#include "../obd/thread_list.h"
#include "../cJSON/cJSON.h"
#include "obd_agree_fops.h"

#ifdef __cplusplus
 extern "C" {
#endif

 extern void json_table_update(const char* filename, const char* protocol);
 extern int json_ShangHai_update(const char* filename, const struct general_pack_shanghai* const _pack);
 extern int json_device(char* buffer, const uint16_t _bsize, const struct device_list* const device, const uint8_t pack[], const uint16_t _psize);
 extern int json_device_parse(const char *json_buf, char hex[], const uint16_t _hsize, struct device_list* const device, int *const _psize);
 extern int json_device_save(const char* filename, struct device_list* const device, const uint8_t pack[], const uint16_t _psize);

 extern int json_obd_save(const char* filename, struct obd_agree_obj* const _obd_fops, const uint8_t pack[], const uint16_t _psize);

#ifdef __cplusplus
}
#endif

#endif // STORAGE_PACK_H
