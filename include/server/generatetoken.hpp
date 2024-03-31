#ifndef GENERATETOKEN_H
#define GENERATETOKEN_H

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <string>
using namespace std;

class GenerateToken
{

public:
    // 生成AES秘钥
    static void generateKey(unsigned char *key, int key_length);

    // AES CBC加密
    static void encryptData(const unsigned char *plaintext, int plaintext_length,
                     const unsigned char *key, unsigned char *iv, unsigned char *ciphertext);

    // base64编码
    static char *base64Encode(const unsigned char *input, int length);

    // sha224哈希算法
    static string sha224(const char *data);

    // 生成随机数并拼接用户名
    static unsigned char *generateRandomData(int userid);

    // 生成Token
    static string generateToken(int userid);
};

#endif