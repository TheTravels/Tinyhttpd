#ifndef _ENCRYPT_H_
#define _ENCRYPT_H_

#include <stdint.h>

#ifdef __cplusplus
 extern "C" {
#endif

 extern void rsa_int(void);
 extern int rsa_encrypt(void *output, const int outputLen, void *input, const int inputLen);
 extern int rsa_decrypt(void *output, const int outputLen, const void *const input, const int inputLen);
 extern void rsa_main(void);
 //extern uint8_t rsa_buffer[1024];

 struct encrypt_obj;
 struct encrypt_fops{
     void (*const init)(struct encrypt_obj* const _fops);
     int (*const encrypt)(struct encrypt_obj* const _fops, void *input, const int inputLen);
     int (*const decrypt)(struct encrypt_obj* const _fops, const void *const input, const int inputLen);
 };
 struct encrypt_obj{
     struct encrypt_fops* const fops;
     char data[2048];
     int len;
 };

 extern struct encrypt_fops rsa_fops;
 extern struct encrypt_obj rsa_obj;

#ifdef __cplusplus
}
#endif


#endif // _ENCRYPT_H_
