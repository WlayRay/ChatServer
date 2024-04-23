#include "redis.hpp"
#include <iostream>
#include <string.h>
using namespace std;

Redis::Redis()
    : _publish_context(nullptr), _subscribe_context(nullptr)
{
}

Redis::~Redis()
{
    if (_publish_context != nullptr)
    {
        redisFree(_publish_context);
    }

    if (_subscribe_context != nullptr)
    {
        redisFree(_subscribe_context);
    }

    if (_string_context != nullptr)
    {
        redisFree(_string_context);
    }
}

bool Redis::connect()
{
    // 负责publish发布消息的上下文连接
    _publish_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _publish_context)
    {
        cerr << "连接redis失败！" << endl;
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    _subscribe_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _subscribe_context)
    {
        cerr << "连接redis失败！" << endl;
        return false;
    }

    // token同步上下文对象，负责将token存入redis中
    _string_context = redisConnect("127.0.0.1", 6379);
    if (nullptr == _string_context)
    {
        cerr << "连接redis失败！" << endl;
        return false;
    }

    // 在单独的线程中监听通道上的事件，有消息给业务层进行上报
    thread t([&]()
             { observer_channel_message(); });
    t.detach();

    cout << "连接redis-server成功！" << endl;
    return true;
}

bool Redis::publish(int channel, string message)
{
    redisReply *reply = (redisReply *)redisCommand(_publish_context, "PUBLISH %d %s", channel, message.c_str());
    if (nullptr == reply)
    {
        cerr << "publish命令执行失败！" << endl;
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subscribe_context, "SUBSCRIBE %d", channel))
    {
        cerr << "subscribe命令执行失败！" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲数据发送完毕（done被置1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        {
            cerr << "subscribe命令执行失败！" << endl;
            return false;
        }
    }
    return true;
}

bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(this->_subscribe_context, "UNSUBSCRIBE %d", channel))
    {
        cerr << "unsubscribe命令执行失败！" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲数据发送完毕（done被置1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(this->_subscribe_context, &done))
        {
            cerr << "unsubscribe命令执行失败！" << endl;
            return false;
        }
    }
    return true;
}

void Redis::observer_channel_message()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(this->_subscribe_context, (void **)&reply))
    {
        // 订阅消息收到一个有三个元素的数组
        if (reply != nullptr && reply->element[2] != nullptr && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息
            _notify_message_handler(atoi(reply->element[1]->str), reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
    cerr << ">>>>>>>>>>>>>>> observer_channel_message 退出 <<<<<<<<<<<<<<<<";
}

void Redis::init_notify_handler(function<void(int, string)> fn)
{
    this->_notify_message_handler = fn;
}

// 将token存储到redis中
bool Redis::setEx(const int key, const char *value, const int expirationSeconds)
{
    redisReply *reply = (redisReply *)redisCommand(_string_context, "SETEX %d %d %s", key, expirationSeconds, value);
    if (reply == NULL)
    {
        return false;
    }
    bool success = (reply->type == REDIS_REPLY_STATUS && strcmp(reply->str, "OK") == 0);
    freeReplyObject(reply);
    return success;
}

char *Redis::get(const int key)
{
    redisReply *reply = (redisReply *)redisCommand(_string_context, "GET %d", key);
    if (!reply)
    {
        cerr << "get命令执行失败！" << endl;
        return nullptr;
    }
    char *token = strdup(reply->str);
    freeReplyObject(reply);
    return token;
}
