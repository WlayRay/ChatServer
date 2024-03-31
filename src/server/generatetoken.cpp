#include "generatetoken.hpp"
#include "string.h"
#include <sstream>
#include <iomanip>

// 生成AES秘钥
 void GenerateToken::generateKey(unsigned char *key, int key_length)
{
    RAND_bytes(key, key_length);
}

// AES CBC加密
 void GenerateToken::encryptData(const unsigned char *plaintext, int plaintext_length,
                                const unsigned char *key, unsigned char *iv, unsigned char *ciphertext)
{
    AES_KEY aes_key;
    AES_set_encrypt_key(key, 128, &aes_key);
    AES_cbc_encrypt(plaintext, ciphertext, plaintext_length, &aes_key, iv, AES_ENCRYPT);
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
    SHA256_CTX sha224;
    SHA224_Init(&sha224);
    SHA224_Update(&sha224, data, strlen(data));
    SHA224_Final(hash, &sha224);

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
    // 生成随机数种子
    srand(static_cast<unsigned int>(time(nullptr)));

    // 生成四个10到99的随机数
    int randomNumbers[4];
    for (int i = 0; i < 4; ++i)
    {
        randomNumbers[i] = rand() % 90 + 10; // 生成10到99的随机数
    }

    // 将int值和四个随机数拼接成字符串
    std::string combinedString = std::to_string(userid);
    for (int i = 0; i < 4; ++i)
    {
        combinedString += std::to_string(randomNumbers[i]);
    }

    // 将字符串转换成unsigned char*
    unsigned char *result = new unsigned char[combinedString.length() + 1];
    strcpy((char *)result, combinedString.c_str());

    return result;
}

int myStrlen(const unsigned char *str)
{
    int len = 0;
    while (*str != '\0')
    {
        len++;
        str++;
    }
    return len;
}

string GenerateToken::generateToken(int userid)
{
    // AES加解密
    unsigned char key[32]; // 秘钥
    generateKey(key, 32);

    const unsigned char *plaintext = generateRandomData(userid);
    int plaintext_length = myStrlen(plaintext);

    unsigned char iv[16];                       // 初始化随机向量
    unsigned char ciphertext[plaintext_length]; // 存储加密后的数据

    encryptData(plaintext, plaintext_length, key, iv, ciphertext);

    char *base64_encoded_data = base64Encode(ciphertext, plaintext_length);

    string hash_result = sha224(base64_encoded_data);
    return hash_result;
}