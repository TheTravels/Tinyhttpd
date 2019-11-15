#include "encrypt.h"

#if 0
/***  RSA  ***/
#include "global.h"
#include "rsa_incl.h"
#include "rsaref.h"
#include "rsa.h"
#include "md5.h"
#include "des.h"
#include "nn.h"
#include "ied.h"
#define Encrypt_RSA  1
#else
#include "bignum.h"
#include "rsa.h"
#define Encrypt_RSA  0
#endif
#include "rsa_keys.h"

#define  debug_log     0
#ifndef debug_log
#define  debug_log     1
#endif
//#undef   debug_log
#include <stdio.h>
#include <string.h>

//#ifdef debug_log
#if debug_log
#define pr_debug(fmt, ...) printf(fmt, ##__VA_ARGS__); fflush(stdout);
#else
#define pr_debug(fmt, ...) ;
#endif

//uint8_t rsa_buffer[1024];

#if 0
typedef struct {
    unsigned short int bits;                     /* length in bits of modulus */
    unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
    unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* public exponent */
} R_RSA_PUBLIC_KEY;
typedef struct {
    unsigned short int bits;                     /* length in bits of modulus */
    unsigned char modulus[MAX_RSA_MODULUS_LEN];  /* modulus */
    unsigned char publicExponent[MAX_RSA_MODULUS_LEN];     /* public exponent */
    unsigned char exponent[MAX_RSA_MODULUS_LEN]; /* private exponent */
    unsigned char prime[2][MAX_RSA_PRIME_LEN];   /* prime factors */
    unsigned char primeExponent[2][MAX_RSA_PRIME_LEN];     /* exponents for CRT */
    unsigned char coefficient[MAX_RSA_PRIME_LEN];          /* CRT coefficient */
} R_RSA_PRIVATE_KEY;
#endif

void print_bn_arr(char *TAG, uint8_t *array, int len)
{
    int i = 0;

    printf("%s", TAG);
    while(array[i] == 0) {
        i++;
    }
    for(; i<len; i++) {
        printf("0x%02X,", array[i]);
    }
    printf("\n");
}

#if  Encrypt_RSA
/* RSA public key */
static R_RSA_PUBLIC_KEY rsa_public_key;
/* RSA private key */
static R_RSA_PRIVATE_KEY rsa_private_key;
void print_pk(R_RSA_PUBLIC_KEY *pk)
{
    printf("PK[%d]:\n", pk->bits);
    print_bn_arr("  modulus: ", pk->modulus, MAX_RSA_MODULUS_LEN);
    print_bn_arr("  exponent: ", pk->exponent, MAX_RSA_MODULUS_LEN);
}

void print_sk(R_RSA_PRIVATE_KEY *sk)
{
    printf("SK[%d]:\n", sk->bits);
    print_bn_arr("  modulus: ", sk->modulus, MAX_RSA_MODULUS_LEN);
    print_bn_arr("  public_exponet: ", sk->publicExponent, MAX_RSA_MODULUS_LEN);
    print_bn_arr("  exponent: ", sk->exponent, MAX_RSA_MODULUS_LEN);
    print_bn_arr("  prime1: ", sk->prime[0], MAX_RSA_PRIME_LEN);
    print_bn_arr("  prime2: ", sk->prime[1], MAX_RSA_PRIME_LEN);
    print_bn_arr("  primeExponent1: ", sk->primeExponent[0], MAX_RSA_PRIME_LEN);
    print_bn_arr("  primeExponent2: ", sk->primeExponent[1], MAX_RSA_PRIME_LEN);
    print_bn_arr("  coefficient: ", sk->coefficient, MAX_RSA_PRIME_LEN);
}
void rsa_int(void)
{
    memset(&rsa_public_key, 0, sizeof (rsa_public_key));
    memset(&rsa_private_key, 0, sizeof (rsa_private_key));
    rsa_public_key.bits = KEY_M_BITS;
    memcpy(&rsa_public_key.modulus[MAX_RSA_MODULUS_LEN-sizeof(public_modulus)], public_modulus, sizeof(public_modulus));
    memcpy(&rsa_public_key.exponent[MAX_RSA_MODULUS_LEN-sizeof(public_exponent)], public_exponent, sizeof(public_exponent));

    rsa_private_key.bits = KEY_M_BITS;
    memcpy(&rsa_private_key.modulus[MAX_RSA_MODULUS_LEN-sizeof(private_modulus)], private_modulus, sizeof(private_modulus));
    memcpy(&rsa_private_key.publicExponent[MAX_RSA_MODULUS_LEN-sizeof(private_public_exponet)], private_public_exponet, sizeof(private_public_exponet));
    memcpy(&rsa_private_key.exponent[MAX_RSA_MODULUS_LEN-sizeof(private_exponent)], private_exponent, sizeof(private_exponent));
    memcpy(&rsa_private_key.prime[0][MAX_RSA_PRIME_LEN-sizeof(key_p1)], key_p1, sizeof(key_p1));
    memcpy(&rsa_private_key.prime[1][MAX_RSA_PRIME_LEN-sizeof(key_p2)], key_p2, sizeof(key_p2));
    memcpy(&rsa_private_key.primeExponent[0][MAX_RSA_PRIME_LEN-sizeof(key_e1)], key_e1, sizeof(key_e1));
    memcpy(&rsa_private_key.primeExponent[1][MAX_RSA_PRIME_LEN-sizeof(key_e2)], key_e2, sizeof(key_e2));
    memcpy(&rsa_private_key.coefficient[MAX_RSA_PRIME_LEN-sizeof(key_c)], key_c, sizeof(key_c));
#if 0
    //rsa_generate_keys(&rsa_public_key, &rsa_private_key, 1024);
    print_pk(&rsa_public_key);
    printf("\n");
    print_sk(&rsa_private_key);
#endif
}

int rsa_encrypt(void *output, const int outputLen, void *input, const int inputLen)
{
#if 0 // 短报文加密
    int status = 0;
    unsigned int dlen=(unsigned int)outputLen;
    pr_debug("rsa_encrypt RSA data_len: %d inputLen:%d\n", dlen, inputLen);
    status = RSAPrivateEncrypt(output, &dlen, input, (unsigned int)inputLen, &rsa_private_key);
    //status = RSAPrivateEncrypt(dat,&dlen,md5,sizeof(md5), &rsa_private_key);
    pr_debug("rsa_encrypt RSA data_len: %d status:0x%02X \n", dlen, status);
    if(ID_OK!=status) return -1;
    return (int)dlen;
#else // 长报文分包加密
    int status = 0;
    int ilen=0;
    int olen=0;
    unsigned int _size=0;
    unsigned int dlen=(unsigned int)outputLen;
    pr_debug("rsa_encrypt RSA data_len: %d inputLen:%d\n", dlen, inputLen);
    olen=0;
    for(ilen=0; ilen<inputLen; ilen += 117) // 拆分成 117字节包进行加密
    {
        if((ilen+117)<inputLen) _size = 117;
        else _size = inputLen-ilen;
        status = RSAPrivateEncrypt(((uint8_t*)output+olen), &dlen, ((uint8_t*)input+ilen), _size, &rsa_private_key);
        pr_debug("rsa_encrypt[%d] RSA data_len: %d status:0x%02X \n", ilen, dlen, status);
        if(ID_OK!=status) return -1;
        olen += dlen;
    }
    return olen;
#endif
}

int rsa_decrypt(void *output, const int outputLen, void *input, const int inputLen)
{
#if 0 // 短报文解密
    int status = 0;
    unsigned int dlen=(unsigned int)outputLen;
    pr_debug("rsa_decrypt RSA data_len: %d inputLen:%d\n", dlen, inputLen);
    status = RSAPublicDecrypt(output, &dlen, input, (unsigned int)inputLen, &rsa_public_key);
    //status = RSAPublicDecrypt(dat,&dlen,dat,dlen,&rsa_public_key);
    pr_debug("rsa_decrypt RSA data_len: %d status:0x%02X \n", dlen, status);
    if(ID_OK!=status) return -1;
    return (int)dlen;
#else // 长报文分包解密
    int status = 0;
    int ilen=0;
    int olen=0;
    unsigned int _size=0;
    unsigned int dlen=(unsigned int)outputLen;
    pr_debug("rsa_decrypt RSA data_len: %d inputLen:%d\n", dlen, inputLen);
    olen=0;
    for(ilen=0; ilen<inputLen; ilen += 128) // 拆分成 128 字节包进行解密
    {
        if((ilen+128)<inputLen) _size = 128;
        else _size = inputLen-ilen;
        status = RSAPublicDecrypt(((uint8_t*)output+olen), &dlen, ((uint8_t*)input+ilen), _size, &rsa_public_key);
        pr_debug("rsa_decrypt[%d] RSA data_len: %d status:0x%02X \n", ilen, dlen, status);
        if(ID_OK!=status) return -1;
        olen += dlen;
    }
    return olen;
#endif
}
#else
/* RSA public key */
rsa_pk_t rsa_public_key;
/* RSA private key */
rsa_sk_t rsa_private_key;

void print_pk(rsa_pk_t *pk)
{
    printf("PK[%d]:\n", pk->bits);
    print_bn_arr("  modulus: ", pk->modulus, RSA_MAX_MODULUS_LEN);
    print_bn_arr("  exponent: ", pk->exponent, RSA_MAX_MODULUS_LEN);
}

void print_sk(rsa_sk_t *sk)
{
    printf("SK[%d]:\n", sk->bits);
    print_bn_arr("  modulus: ", sk->modulus, RSA_MAX_MODULUS_LEN);
    print_bn_arr("  public_exponet: ", sk->public_exponet, RSA_MAX_MODULUS_LEN);
    print_bn_arr("  exponent: ", sk->exponent, RSA_MAX_MODULUS_LEN);
    print_bn_arr("  prime1: ", sk->prime1, RSA_MAX_PRIME_LEN);
    print_bn_arr("  prime2: ", sk->prime2, RSA_MAX_PRIME_LEN);
    print_bn_arr("  primeExponent1: ", sk->prime_exponent1, RSA_MAX_PRIME_LEN);
    print_bn_arr("  primeExponent2: ", sk->prime_exponent2, RSA_MAX_PRIME_LEN);
    print_bn_arr("  coefficient: ", sk->coefficient, RSA_MAX_PRIME_LEN);
}
#if 0
void rsa_int(void)
{
    memset(&rsa_public_key, 0, sizeof (rsa_public_key));
    memset(&rsa_private_key, 0, sizeof (rsa_private_key));
    rsa_public_key.bits = KEY_M_BITS;
    memcpy(&rsa_public_key.modulus[RSA_MAX_MODULUS_LEN-sizeof(public_modulus)], public_modulus, sizeof(public_modulus));
    memcpy(&rsa_public_key.exponent[RSA_MAX_MODULUS_LEN-sizeof(public_exponent)], public_exponent, sizeof(public_exponent));

    rsa_private_key.bits = KEY_M_BITS;
    memcpy(&rsa_private_key.modulus[RSA_MAX_MODULUS_LEN-sizeof(private_modulus)], private_modulus, sizeof(private_modulus));
    memcpy(&rsa_private_key.public_exponet[RSA_MAX_MODULUS_LEN-sizeof(private_public_exponet)], private_public_exponet, sizeof(private_public_exponet));
    memcpy(&rsa_private_key.exponent[RSA_MAX_MODULUS_LEN-sizeof(private_exponent)], private_exponent, sizeof(private_exponent));
    memcpy(&rsa_private_key.prime1[RSA_MAX_PRIME_LEN-sizeof(key_p1)], key_p1, sizeof(key_p1));
    memcpy(&rsa_private_key.prime2[RSA_MAX_PRIME_LEN-sizeof(key_p2)], key_p2, sizeof(key_p2));
    memcpy(&rsa_private_key.prime_exponent1[RSA_MAX_PRIME_LEN-sizeof(key_e1)], key_e1, sizeof(key_e1));
    memcpy(&rsa_private_key.prime_exponent2[RSA_MAX_PRIME_LEN-sizeof(key_e2)], key_e2, sizeof(key_e2));
    memcpy(&rsa_private_key.coefficient[RSA_MAX_PRIME_LEN-sizeof(key_c)], key_c, sizeof(key_c));
#if 0
    //rsa_generate_keys(&rsa_public_key, &rsa_private_key, 1024);
    print_pk(&rsa_public_key);
    printf("\n");
    print_sk(&rsa_private_key);
#endif
}
#else
/** 密钥
obd@obd-vm:~/key$ openssl rsa -pubin -inform PEM -text < rsa_public_key.pem
RSA Public-Key: (1024 bit)
Modulus:
    00:c8:b1:d9:f5:58:4b:0d:d5:6d:7f:df:30:7f:57:
    e6:3e:3f:87:ef:14:93:ba:95:24:ce:2c:ec:ca:15:
    b1:25:be:e7:97:92:79:6f:48:0e:fd:82:ba:89:23:
    3a:5f:eb:a3:f7:ab:28:4d:34:be:eb:b8:e6:db:27:
    7b:53:c4:01:8f:81:54:ce:5b:8a:1a:11:df:49:fb:
    ea:94:0a:42:85:ce:0b:a2:d6:07:c6:18:32:f2:ce:
    fe:7d:f8:9d:27:69:c9:61:f7:bf:f5:65:a6:7e:e5:
    b9:7c:75:71:75:9a:ff:e1:53:55:64:7e:38:de:d7:
    d8:49:a3:0f:69:59:16:29:ab
Exponent: 65537 (0x10001)
writing RSA key
-----BEGIN PUBLIC KEY-----
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDIsdn1WEsN1W1/3zB/V+Y+P4fv
FJO6lSTOLOzKFbElvueXknlvSA79grqJIzpf66P3qyhNNL7ruObbJ3tTxAGPgVTO
W4oaEd9J++qUCkKFzgui1gfGGDLyzv59+J0naclh97/1ZaZ+5bl8dXF1mv/hU1Vk
fjje19hJow9pWRYpqwIDAQAB
-----END PUBLIC KEY-----
obd@obd-vm:~/key$ openssl rsa -inform PEM -text < rsa_private_key.pem
RSA Private-Key: (1024 bit, 2 primes)
modulus:
    00:c8:b1:d9:f5:58:4b:0d:d5:6d:7f:df:30:7f:57:
    e6:3e:3f:87:ef:14:93:ba:95:24:ce:2c:ec:ca:15:
    b1:25:be:e7:97:92:79:6f:48:0e:fd:82:ba:89:23:
    3a:5f:eb:a3:f7:ab:28:4d:34:be:eb:b8:e6:db:27:
    7b:53:c4:01:8f:81:54:ce:5b:8a:1a:11:df:49:fb:
    ea:94:0a:42:85:ce:0b:a2:d6:07:c6:18:32:f2:ce:
    fe:7d:f8:9d:27:69:c9:61:f7:bf:f5:65:a6:7e:e5:
    b9:7c:75:71:75:9a:ff:e1:53:55:64:7e:38:de:d7:
    d8:49:a3:0f:69:59:16:29:ab
publicExponent: 65537 (0x10001)
privateExponent:
    00:91:82:41:60:dc:ef:2d:cc:7c:63:a5:d2:67:c1:
    b7:31:92:a6:5f:f5:2f:56:ac:23:cf:48:4b:36:09:
    9c:32:9d:c3:13:e6:23:1d:47:c5:76:90:7d:e6:48:
    20:5e:c3:5e:52:87:49:e5:10:45:0c:6b:37:15:d5:
    fe:58:b1:57:df:06:6f:c2:52:46:2f:bd:da:5f:de:
    d6:e4:33:11:ad:42:33:ce:26:87:de:44:d4:cf:04:
    63:54:33:5f:bd:55:e1:67:2e:18:c9:36:51:83:d0:
    de:d9:ae:25:32:73:81:67:2c:ab:76:40:a8:9c:e5:
    4a:e5:8d:64:aa:e5:9a:72:c1
prime1:
    00:f4:43:ea:82:fa:b7:d8:11:1b:3e:59:c4:73:bb:
    b9:43:42:24:56:4e:7f:f1:39:d4:65:ca:1b:cb:28:
    11:1b:1c:f8:3a:34:66:47:22:2b:3e:ee:c7:ee:db:
    0d:7b:19:e2:7b:70:03:11:77:f4:d1:5b:c2:a9:51:
    0d:7d:b0:3b:b3
prime2:
    00:d2:56:15:c4:8e:f1:1b:36:b1:a1:ec:41:62:e7:
    15:e1:2d:87:c0:cb:b7:91:c8:75:51:76:f8:c7:c8:
    23:b3:fd:65:06:05:26:c6:9e:64:3b:90:23:6b:81:
    22:0f:48:9e:49:0b:50:c3:f9:a0:87:03:fa:ad:ea:
    9f:4c:5b:fe:29
exponent1:
    00:b1:58:36:05:ce:be:77:d3:43:b2:6b:3e:64:c0:
    a0:eb:a2:33:fb:ad:96:da:af:1f:f4:9f:5e:ba:8b:
    66:90:06:e2:7a:6f:dc:ea:3c:76:a5:84:7c:08:81:
    66:32:40:42:dd:58:20:02:28:d3:c8:c5:14:7a:15:
    ec:f3:9a:66:5d
exponent2:
    5d:29:cc:12:77:cd:cb:63:fa:61:e2:27:44:0b:5e:
    e1:92:2d:22:f4:18:a6:f5:c7:21:02:35:47:28:1a:
    c8:4a:60:48:be:57:62:7a:1d:a3:54:40:9c:09:62:
    70:aa:95:91:7c:f5:95:18:28:46:25:39:0f:77:70:
    7f:7d:ef:a1
coefficient:
    37:50:c0:cd:38:ed:ad:ef:c4:06:0b:63:17:05:59:
    a3:9e:3c:6b:91:2d:05:76:14:13:8d:5f:32:de:ab:
    58:2e:12:20:28:b1:d9:82:be:bb:f3:18:f1:c7:d0:
    e0:e1:cd:9a:29:16:fd:fe:39:96:3f:36:d4:c7:0e:
    dd:b5:20:a0
writing RSA key
-----BEGIN RSA PRIVATE KEY-----
MIICXQIBAAKBgQDIsdn1WEsN1W1/3zB/V+Y+P4fvFJO6lSTOLOzKFbElvueXknlv
SA79grqJIzpf66P3qyhNNL7ruObbJ3tTxAGPgVTOW4oaEd9J++qUCkKFzgui1gfG
GDLyzv59+J0naclh97/1ZaZ+5bl8dXF1mv/hU1Vkfjje19hJow9pWRYpqwIDAQAB
AoGBAJGCQWDc7y3MfGOl0mfBtzGSpl/1L1asI89ISzYJnDKdwxPmIx1HxXaQfeZI
IF7DXlKHSeUQRQxrNxXV/lixV98Gb8JSRi+92l/e1uQzEa1CM84mh95E1M8EY1Qz
X71V4WcuGMk2UYPQ3tmuJTJzgWcsq3ZAqJzlSuWNZKrlmnLBAkEA9EPqgvq32BEb
PlnEc7u5Q0IkVk5/8TnUZcobyygRGxz4OjRmRyIrPu7H7tsNexnie3ADEXf00VvC
qVENfbA7swJBANJWFcSO8Rs2saHsQWLnFeEth8DLt5HIdVF2+MfII7P9ZQYFJsae
ZDuQI2uBIg9InkkLUMP5oIcD+q3qn0xb/ikCQQCxWDYFzr5300Oyaz5kwKDrojP7
rZbarx/0n166i2aQBuJ6b9zqPHalhHwIgWYyQELdWCACKNPIxRR6FezzmmZdAkBd
KcwSd83LY/ph4idEC17hki0i9Bim9cchAjVHKBrISmBIvldieh2jVECcCWJwqpWR
fPWVGChGJTkPd3B/fe+hAkA3UMDNOO2t78QGC2MXBVmjnjxrkS0FdhQTjV8y3qtY
LhIgKLHZgr678xjxx9Dg4c2aKRb9/jmWPzbUxw7dtSCg
-----END RSA PRIVATE KEY-----
*/
const char Public_Key_Modulus[] = "\
        00:c8:b1:d9:f5:58:4b:0d:d5:6d:7f:df:30:7f:57:\
        e6:3e:3f:87:ef:14:93:ba:95:24:ce:2c:ec:ca:15:\
        b1:25:be:e7:97:92:79:6f:48:0e:fd:82:ba:89:23:\
        3a:5f:eb:a3:f7:ab:28:4d:34:be:eb:b8:e6:db:27:\
        7b:53:c4:01:8f:81:54:ce:5b:8a:1a:11:df:49:fb:\
        ea:94:0a:42:85:ce:0b:a2:d6:07:c6:18:32:f2:ce:\
        fe:7d:f8:9d:27:69:c9:61:f7:bf:f5:65:a6:7e:e5:\
        b9:7c:75:71:75:9a:ff:e1:53:55:64:7e:38:de:d7:\
        d8:49:a3:0f:69:59:16:29:ab";
const char Public_Key_Exponent[] = {
            0x01,0x00,0x01,
};
const char Private_Key_modulus[] = "\
        00:c8:b1:d9:f5:58:4b:0d:d5:6d:7f:df:30:7f:57: \
        e6:3e:3f:87:ef:14:93:ba:95:24:ce:2c:ec:ca:15: \
        b1:25:be:e7:97:92:79:6f:48:0e:fd:82:ba:89:23: \
        3a:5f:eb:a3:f7:ab:28:4d:34:be:eb:b8:e6:db:27: \
        7b:53:c4:01:8f:81:54:ce:5b:8a:1a:11:df:49:fb: \
        ea:94:0a:42:85:ce:0b:a2:d6:07:c6:18:32:f2:ce: \
        fe:7d:f8:9d:27:69:c9:61:f7:bf:f5:65:a6:7e:e5: \
        b9:7c:75:71:75:9a:ff:e1:53:55:64:7e:38:de:d7: \
        d8:49:a3:0f:69:59:16:29:ab";
const char Private_Key_Exponent[] = {
            0x01,0x00,0x01,
};
const char Private_Key_privateExponent[] = "\
        00:91:82:41:60:dc:ef:2d:cc:7c:63:a5:d2:67:c1:\
        b7:31:92:a6:5f:f5:2f:56:ac:23:cf:48:4b:36:09:\
        9c:32:9d:c3:13:e6:23:1d:47:c5:76:90:7d:e6:48:\
        20:5e:c3:5e:52:87:49:e5:10:45:0c:6b:37:15:d5:\
        fe:58:b1:57:df:06:6f:c2:52:46:2f:bd:da:5f:de:\
        d6:e4:33:11:ad:42:33:ce:26:87:de:44:d4:cf:04:\
        63:54:33:5f:bd:55:e1:67:2e:18:c9:36:51:83:d0:\
        de:d9:ae:25:32:73:81:67:2c:ab:76:40:a8:9c:e5:\
        4a:e5:8d:64:aa:e5:9a:72:c1";
const char Private_Key_prime1[] = "\
        00:f4:43:ea:82:fa:b7:d8:11:1b:3e:59:c4:73:bb:\
        b9:43:42:24:56:4e:7f:f1:39:d4:65:ca:1b:cb:28:\
        11:1b:1c:f8:3a:34:66:47:22:2b:3e:ee:c7:ee:db:\
        0d:7b:19:e2:7b:70:03:11:77:f4:d1:5b:c2:a9:51:\
        0d:7d:b0:3b:b3";
const char Private_Key_prime2[] = "\
        00:d2:56:15:c4:8e:f1:1b:36:b1:a1:ec:41:62:e7:\
        15:e1:2d:87:c0:cb:b7:91:c8:75:51:76:f8:c7:c8:\
        23:b3:fd:65:06:05:26:c6:9e:64:3b:90:23:6b:81:\
        22:0f:48:9e:49:0b:50:c3:f9:a0:87:03:fa:ad:ea:\
        9f:4c:5b:fe:29";
const char Private_Key_exponent1[] = "\
        00:b1:58:36:05:ce:be:77:d3:43:b2:6b:3e:64:c0:\
        a0:eb:a2:33:fb:ad:96:da:af:1f:f4:9f:5e:ba:8b:\
        66:90:06:e2:7a:6f:dc:ea:3c:76:a5:84:7c:08:81:\
        66:32:40:42:dd:58:20:02:28:d3:c8:c5:14:7a:15:\
        ec:f3:9a:66:5d";
const char Private_Key_exponent2[] = "\
        5d:29:cc:12:77:cd:cb:63:fa:61:e2:27:44:0b:5e:\
        e1:92:2d:22:f4:18:a6:f5:c7:21:02:35:47:28:1a:\
        c8:4a:60:48:be:57:62:7a:1d:a3:54:40:9c:09:62:\
        70:aa:95:91:7c:f5:95:18:28:46:25:39:0f:77:70:\
        7f:7d:ef:a1";
const char Private_Key_coefficient[] = "\
        37:50:c0:cd:38:ed:ad:ef:c4:06:0b:63:17:05:59:\
        a3:9e:3c:6b:91:2d:05:76:14:13:8d:5f:32:de:ab:\
        58:2e:12:20:28:b1:d9:82:be:bb:f3:18:f1:c7:d0:\
        e0:e1:cd:9a:29:16:fd:fe:39:96:3f:36:d4:c7:0e:\
        dd:b5:20:a0";
extern uint8_t hex2int(const uint8_t hex);
int key_convert(uint8_t _key[], const uint16_t _ksize, const char _key_hex[], uint8_t _key_buf[])
{
        uint16_t _kindex, index;
        char ch;
        _kindex=0;
        for(index=0; '\0'!=_key_hex[index]; index++)
        {
            ch = _key_hex[index];
            while('\0'!=ch)
            {
                if((ch>='0') && (ch<='9')) break;
                if((ch>='a') && (ch<='z')) break;
                if((ch>='A') && (ch<='Z')) break;
                index++;
                ch = _key_hex[index];
            }
            if('\0'==ch) break;
            // ASCII to HEX
            _key_buf[_kindex] = 0;
            _key_buf[_kindex++] = ((hex2int(_key_hex[index])&0x0F)<<4) | (hex2int(_key_hex[index+1])&0x0F);
            ch = _key_hex[index];
            while('\0'!=ch)
            {
                if(':'==ch) break;
                index++;
                ch = _key_hex[index];
            }
            if('\0'==ch) break;
        }
        if(0==_key_buf[0])
        {
            _kindex--;
            if(_kindex<=_ksize)
            {
                memcpy(&_key[_ksize-_kindex], &_key_buf[1], _kindex);
            }
        }
        else
        {
            if(_kindex<=_ksize)
            {
                memcpy(&_key[_ksize-_kindex], _key_buf, _kindex);
            }
        }
        return 0;
}
void rsa_int(void)
{
    uint8_t rsa_buffer[2048];
    memset(&rsa_public_key, 0, sizeof (rsa_public_key));
    memset(&rsa_private_key, 0, sizeof (rsa_private_key));
    memset(rsa_buffer, 0, sizeof (rsa_buffer));
    rsa_public_key.bits = KEY_M_BITS;
    //memcpy(&rsa_public_key.modulus[RSA_MAX_MODULUS_LEN-sizeof(public_modulus)], public_modulus, sizeof(public_modulus));
    key_convert(rsa_public_key.modulus, RSA_MAX_MODULUS_LEN, Public_Key_Modulus, rsa_buffer);
    memcpy(&rsa_public_key.exponent[RSA_MAX_MODULUS_LEN-sizeof(Public_Key_Exponent)], Public_Key_Exponent, sizeof(Public_Key_Exponent));

    rsa_private_key.bits = KEY_M_BITS;
    //memcpy(&rsa_private_key.modulus[RSA_MAX_MODULUS_LEN-sizeof(private_modulus)], private_modulus, sizeof(private_modulus));
    //memcpy(&rsa_private_key.public_exponet[RSA_MAX_MODULUS_LEN-sizeof(Private_Key_Exponent)], Private_Key_Exponent, sizeof(Private_Key_Exponent));
    //memcpy(&rsa_private_key.exponent[RSA_MAX_MODULUS_LEN-sizeof(private_exponent)], private_exponent, sizeof(private_exponent));
    //memcpy(&rsa_private_key.prime1[RSA_MAX_PRIME_LEN-sizeof(key_p1)], key_p1, sizeof(key_p1));
    //memcpy(&rsa_private_key.prime2[RSA_MAX_PRIME_LEN-sizeof(key_p2)], key_p2, sizeof(key_p2));
    //memcpy(&rsa_private_key.prime_exponent1[RSA_MAX_PRIME_LEN-sizeof(key_e1)], key_e1, sizeof(key_e1));
    //memcpy(&rsa_private_key.prime_exponent2[RSA_MAX_PRIME_LEN-sizeof(key_e2)], key_e2, sizeof(key_e2));
    //memcpy(&rsa_private_key.coefficient[RSA_MAX_PRIME_LEN-sizeof(key_c)], key_c, sizeof(key_c));
    key_convert(rsa_private_key.modulus, RSA_MAX_MODULUS_LEN, Private_Key_modulus, rsa_buffer);
    key_convert(rsa_private_key.public_exponet, RSA_MAX_MODULUS_LEN, Private_Key_privateExponent, rsa_buffer);
    //key_convert(rsa_private_key.exponent, RSA_MAX_MODULUS_LEN, Private_Key_Exponent, rsa_buffer);
    memcpy(&rsa_private_key.exponent[RSA_MAX_MODULUS_LEN-sizeof(Private_Key_Exponent)], Private_Key_Exponent, sizeof(Private_Key_Exponent));
    key_convert(rsa_private_key.prime1, RSA_MAX_PRIME_LEN, Private_Key_prime1, rsa_buffer);
    key_convert(rsa_private_key.prime2, RSA_MAX_PRIME_LEN, Private_Key_prime2, rsa_buffer);
    key_convert(rsa_private_key.prime_exponent1, RSA_MAX_PRIME_LEN, Private_Key_exponent1, rsa_buffer);
    key_convert(rsa_private_key.prime_exponent2, RSA_MAX_PRIME_LEN, Private_Key_exponent2, rsa_buffer);
    key_convert(rsa_private_key.coefficient, RSA_MAX_PRIME_LEN, Private_Key_coefficient, rsa_buffer);
#if 0
    //rsa_generate_keys(&rsa_public_key, &rsa_private_key, 1024);
    print_pk(&rsa_public_key);
    printf("\n");
    print_sk(&rsa_private_key);
    fflush(stdout);
#endif
}
#endif


int rsa_encrypt(void *output, const int outputLen, void *input, const int inputLen)
{
    // 长报文分包加密
    int status = 0;
    int ilen=0;
    int olen=0;
    unsigned int _size=0;
    uint32_t dlen=(uint32_t)outputLen;
    //rsa_generate_keys(&rsa_public_key, &rsa_private_key, 1024);
    pr_debug("rsa_encrypt RSA data_len: %d inputLen:%d\n", dlen, inputLen);
    olen=0;
    for(ilen=0; ilen<inputLen; ilen += 117) // 拆分成 117字节包进行加密
    {
        if((ilen+117)<inputLen) _size = 117;
        else _size = inputLen-ilen;
        status = rsa_private_encrypt(((uint8_t*)output+olen), &dlen, ((uint8_t*)input+ilen), _size, &rsa_private_key);
        pr_debug("rsa_encrypt[%d] RSA data_len: %d status:0x%02X \n", ilen, dlen, status);
        if(0!=status) return -1;
        olen += dlen;
    }
    return olen;
}

int rsa_decrypt(void *output, const int outputLen, const void *const input, const int inputLen)
{
    // 长报文分包解密
    int status = 0;
    int ilen=0;
    int olen=0;
    unsigned int _size=0;
    uint32_t dlen=(uint32_t)outputLen;
    pr_debug("rsa_decrypt RSA data_len: %d inputLen:%d\n", dlen, inputLen);
    olen=0;
    for(ilen=0; ilen<inputLen; ilen += 128) // 拆分成 128 字节包进行解密
    {
        if((ilen+128)<inputLen) _size = 128;
        else _size = inputLen-ilen;
        status = rsa_public_decrypt(((uint8_t*)output+olen), &dlen, ((uint8_t*)input+ilen), _size, &rsa_public_key);
        pr_debug("rsa_decrypt[%d] RSA data_len: %d status:0x%02X \n", ilen, dlen, status);
        if(0!=status) return -1;
        olen += dlen;
    }
    return olen;
}
#endif

static void rsa_fops_init(struct encrypt_obj* const _fops)
{
    uint8_t rsa_buffer[2048];
    memset(&rsa_public_key, 0, sizeof (rsa_public_key));
    memset(&rsa_private_key, 0, sizeof (rsa_private_key));
    memset(rsa_buffer, 0, sizeof (rsa_buffer));
    rsa_public_key.bits = KEY_M_BITS;
    //memcpy(&rsa_public_key.modulus[RSA_MAX_MODULUS_LEN-sizeof(public_modulus)], public_modulus, sizeof(public_modulus));
    key_convert(rsa_public_key.modulus, RSA_MAX_MODULUS_LEN, Public_Key_Modulus, rsa_buffer);
    memcpy(&rsa_public_key.exponent[RSA_MAX_MODULUS_LEN-sizeof(Public_Key_Exponent)], Public_Key_Exponent, sizeof(Public_Key_Exponent));

    rsa_private_key.bits = KEY_M_BITS;
    //memcpy(&rsa_private_key.modulus[RSA_MAX_MODULUS_LEN-sizeof(private_modulus)], private_modulus, sizeof(private_modulus));
    //memcpy(&rsa_private_key.public_exponet[RSA_MAX_MODULUS_LEN-sizeof(Private_Key_Exponent)], Private_Key_Exponent, sizeof(Private_Key_Exponent));
    //memcpy(&rsa_private_key.exponent[RSA_MAX_MODULUS_LEN-sizeof(private_exponent)], private_exponent, sizeof(private_exponent));
    //memcpy(&rsa_private_key.prime1[RSA_MAX_PRIME_LEN-sizeof(key_p1)], key_p1, sizeof(key_p1));
    //memcpy(&rsa_private_key.prime2[RSA_MAX_PRIME_LEN-sizeof(key_p2)], key_p2, sizeof(key_p2));
    //memcpy(&rsa_private_key.prime_exponent1[RSA_MAX_PRIME_LEN-sizeof(key_e1)], key_e1, sizeof(key_e1));
    //memcpy(&rsa_private_key.prime_exponent2[RSA_MAX_PRIME_LEN-sizeof(key_e2)], key_e2, sizeof(key_e2));
    //memcpy(&rsa_private_key.coefficient[RSA_MAX_PRIME_LEN-sizeof(key_c)], key_c, sizeof(key_c));
    key_convert(rsa_private_key.modulus, RSA_MAX_MODULUS_LEN, Private_Key_modulus, rsa_buffer);
    key_convert(rsa_private_key.public_exponet, RSA_MAX_MODULUS_LEN, Private_Key_privateExponent, rsa_buffer);
    //key_convert(rsa_private_key.exponent, RSA_MAX_MODULUS_LEN, Private_Key_Exponent, rsa_buffer);
    memcpy(&rsa_private_key.exponent[RSA_MAX_MODULUS_LEN-sizeof(Private_Key_Exponent)], Private_Key_Exponent, sizeof(Private_Key_Exponent));
    key_convert(rsa_private_key.prime1, RSA_MAX_PRIME_LEN, Private_Key_prime1, rsa_buffer);
    key_convert(rsa_private_key.prime2, RSA_MAX_PRIME_LEN, Private_Key_prime2, rsa_buffer);
    key_convert(rsa_private_key.prime_exponent1, RSA_MAX_PRIME_LEN, Private_Key_exponent1, rsa_buffer);
    key_convert(rsa_private_key.prime_exponent2, RSA_MAX_PRIME_LEN, Private_Key_exponent2, rsa_buffer);
    key_convert(rsa_private_key.coefficient, RSA_MAX_PRIME_LEN, Private_Key_coefficient, rsa_buffer);
#if 0
    //rsa_generate_keys(&rsa_public_key, &rsa_private_key, 1024);
    print_pk(&rsa_public_key);
    printf("\n");
    print_sk(&rsa_private_key);
    fflush(stdout);
#endif
    memset(_fops->data, 0, sizeof(_fops->data));
    _fops->len = 0;
}

static int rsa_fops_encrypt(struct encrypt_obj* const _fops, void *input, const int inputLen)
{
    // 长报文分包加密
    int status = 0;
    int ilen=0;
    int olen=0;
    unsigned int _size=0;
    void *output = _fops->data;
    uint32_t dlen=(uint32_t)sizeof(_fops->data);
    //rsa_generate_keys(&rsa_public_key, &rsa_private_key, 1024);
    pr_debug("rsa_encrypt RSA data_len: %d inputLen:%d\n", dlen, inputLen);
    memset(_fops->data, 0, sizeof(_fops->data));
    _fops->len = 0;
    olen=0;
    for(ilen=0; ilen<inputLen; ilen += 117) // 拆分成 117字节包进行加密
    {
        if((ilen+117)<inputLen) _size = 117;
        else _size = inputLen-ilen;
        status = rsa_private_encrypt(((uint8_t*)output+olen), &dlen, ((uint8_t*)input+ilen), _size, &rsa_private_key);
        pr_debug("rsa_encrypt[%d] RSA data_len: %d status:0x%02X \n", ilen, dlen, status);
        if(0!=status) return -1;
        olen += dlen;
    }
    _fops->len = olen;
    return olen;
}

static int rsa_fops_decrypt(struct encrypt_obj* const _fops, const void *const input, const int inputLen)
{
    // 长报文分包解密
    int status = 0;
    int ilen=0;
    int olen=0;
    unsigned int _size=0;
    void *output = _fops->data;
    uint32_t dlen=(uint32_t)sizeof(_fops->data);
    pr_debug("rsa_decrypt RSA data_len: %d inputLen:%d\n", dlen, inputLen);
    memset(_fops->data, 0, sizeof(_fops->data));
    _fops->len = 0;
    olen=0;
    for(ilen=0; ilen<inputLen; ilen += 128) // 拆分成 128 字节包进行解密
    {
        if((ilen+128)<inputLen) _size = 128;
        else _size = inputLen-ilen;
        status = rsa_public_decrypt(((uint8_t*)output+olen), &dlen, ((uint8_t*)input+ilen), _size, &rsa_public_key);
        pr_debug("rsa_decrypt[%d] RSA data_len: %d status:0x%02X \n", ilen, dlen, status);
        if(0!=status) return -1;
        olen += dlen;
    }
    _fops->len = olen;
    return olen;
}

struct encrypt_fops rsa_fops = {
    .init = rsa_fops_init,
    .encrypt = rsa_fops_encrypt,
    .encrypt = rsa_fops_encrypt,
};

struct encrypt_obj rsa_obj = {
    .fops = &rsa_fops,
    .data = '\0',
    .len  = 0,
};




