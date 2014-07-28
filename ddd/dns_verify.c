#include <stdio.h>
#include <string.h>
#include <openssl/rsa.h>
#include <openssl/dsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
//#include <zmq.h>
#include "dns_comdef.h"


int dns_verify(const char* input, int input_len)
{

    FILE *license = fopen("./file/dnsproxy.license", "r");
    FILE *key = fopen("./file/dnsproxy.public", "r");
    RSA *rsa = NULL;
    //int rsa_len;

    char license_buf[1024] = {0};
    char key_buf[1024] = {0};
    unsigned char md5[16];
    
    //公钥的md5码，绑定于此
    unsigned char key_md51[33] = "2ddd205ecde91edf1c2483b569281ccf";
    unsigned char key_md52[33] = {0};
    int len, i, is_valid_signature = 0;

    if(!license || !key)
    {
        hyb_debug("Can't find verify files!\n");
        if (license)
        {
            fclose(license);
        }
        if (key)
        {
            fclose(key);
        }
        return !0;
    }

    //读取公钥内容并计算md5值，来验证公钥是否合法
    len = 0;
    for(;;)
    {
        int r = fread(key_buf + len, 1, 1024, key);
        if(r <= 0) break;
        len += r;
    }
    
    //计算公钥md5值(DNS_KEY_MD5)
    MD5((const unsigned char *)key_buf, len, md5);
    for (i = 0; i < 16; i++)
    {
        sprintf((char *)key_md52 + i * 2, "%2.2x", md5[i]);
    }

    #ifdef DNS_KEY_MD5
        if(strncasecmp(key_md52, VPN_KEY_MD5, 32))
        {
            fprintf(stderr, "key is not right!\n");
            goto end;
        }
    #else
        if(strncasecmp((const char *)key_md52, (char *)key_md51, 32))
        {
            fprintf(stderr, "key is not right!\n");
            goto end;
        }
    #endif 

    //读取许可文件内容
    len = 0;
    for(;;)
    {
        int r = fread(license_buf + len, 1, 1024, license);
        if(r <= 0) break;
        len += r;
    }

    //加载公钥
    fseek(key, 0, SEEK_SET);
    if ((rsa = PEM_read_RSA_PUBKEY(key, NULL, NULL, NULL)) == NULL)
    {
        ERR_print_errors_fp(stdout);
        goto end;
    }
    else
    {
        //unsigned char mac[64] = {0};
       // unsigned char md51[64] = {0};
        //unsigned char *md52 = "123";
        SHA_CTX s;
        //int i, size;
       // char c[512];
        unsigned char hash[20];


        // 对设备信息(mac)摘要再进行sha1摘要
        SHA1_Init(&s);
        SHA1_Update(&s, input, input_len);
        SHA1_Final(hash, &s);
        //使用公钥验证许可文件是否配对设备信息的最后摘要信息
        is_valid_signature = RSA_verify(NID_sha1, hash/*xbuf*/, 20/*strlen((char*)xbuf)*/, (const unsigned char *)license_buf, len, rsa);
    }

end:
    fclose(key);
    fclose(license);
    if (rsa)
    {
        RSA_free(rsa);
    }
    hyb_debug("The result is :%d\n",is_valid_signature);
    return !is_valid_signature;
}
    



