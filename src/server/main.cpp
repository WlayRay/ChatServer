#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>
using namespace std;

void resetHandler(int signum) {  
    ChatService::instance()->reset();  
    exit(0);  
}  

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "命令无效! 样例：./ChatServer 127.0.0.1 6000" << endl;
    }

    string ip = argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT, resetHandler);

    EventLoop loop;
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}