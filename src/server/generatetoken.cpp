#include "generatetoken.hpp"
#include "string.h"
#include <sstream>
#include <iomanip>

// 生成秘钥
void GenerateToken::generateKey(unsigned char *key, int key_length)
{
    RAND_bytes(key, key_length);
}

char *GenerateToken::base64Encode(const unsigned char *input, int length)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);

    BIO_write(bio, input, length);
    BIO_flush(bio);

    BIO_get_mem_ptr(bio, &bufferPtr);

    char *output = (char *)malloc(bufferPtr->length + 1);

    memcpy(output, bufferPtr->data, bufferPtr->length);
    output[bufferPtr->length] = '\0';

    BIO_free_all(bio);

    return output;
}

string GenerateToken::sha224(const char *data)
{
    unsigned char hash[SHA224_DIGEST_LENGTH];
    SHA224((const unsigned char *)data, sizeof(data), hash);

    stringstream ss;
    for (int i = 0; i < SHA224_DIGEST_LENGTH; i++)
    {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// 生成随机数并拼接用户名
unsigned char *GenerateToken::generateRandomData(int userid)
{
    time_t currentTime = time(nullptr);
    string combinedString = to_string(userid) + to_string(currentTime);
    unsigned char *result = new unsigned char[combinedString.length() + 1];
    strcpy((char *)result, combinedString.c_str());
    return result;
}

string GenerateToken::generateToken(int userid)
{
    std::string data = std::to_string(userid);

    unsigned char key[32]; // 32字节的密钥

    // 生成密钥
    generateKey(key, sizeof(key));
    // 分配足够的空间来存储HMAC签名结果
    unsigned char digest[EVP_MAX_MD_SIZE];
    unsigned int digest_length;

    // 使用HMAC函数进行签名
    HMAC(EVP_sha224(), key, sizeof(key), reinterpret_cast<const unsigned char *>(data.c_str()), data.length(), digest, &digest_length);

    // 将签名结果转换为Base64编码
    std::string base64_token = base64Encode(digest, digest_length);
    return base64_token;
}