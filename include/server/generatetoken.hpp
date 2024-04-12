#ifndef GENERATETOKEN_H
#define GENERATETOKEN_H

#include <openssl/aes.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/buffer.h>
#include <openssl/sha.h>
#include <string>
using namespace std;

class GenerateToken
{
private:
    GenerateToken() {}
    GenerateToken(const GenerateToken &) = delete;
    GenerateToken &operator=(const GenerateToken &) = delete;

public:
    static GenerateToken *getInstance()
    {
        static GenerateToken instance;
        return &instance;
    }

    // 生成秘钥
    void generateKey(unsigned char *key, int key_length);

    // base64编码
    char *base64Encode(const unsigned char *input, int length);

    // sha224哈希算法
    string sha224(const char *data);

    // 生成随机数并拼接用户名
    unsigned char *generateRandomData(int userid);

    // 生成Token
    string generateToken(int userid);
};

#endif