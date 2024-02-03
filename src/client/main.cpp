#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <ctime>
#include <string.h>
using namespace std;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"
#include "json.hpp"
using json = nlohmann::json;

// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前用户登录的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群组列表信息
vector<Group> g_currentUserGroupList;
// 显示当前登陆成功用户的基本信息
void showCurrentUserData();

// 接收线程
void readTaskHandler(int clientfd);
// 获取系统时间(聊天信息需要添加时间信息)
string getCurrentTime();
// 主聊天页面程序
void mainMenu(int clientfd);
// 控制聊天页面程序
bool isMainMenuRunning = false;

// 聊天客户端程序实现，mian线程用于发送线程，子线程接收
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        cerr << "命令无效! 样例：./ChatClient 127.0.0.1 8003" << endl;
    }

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == clientfd)
    {
        cerr << "socket创建失败！" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    // 填写服务端的IP和Port（端口）
    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    // client和server进行连接
    if (-1 == connect(clientfd, (sockaddr *)&server, sizeof(sockaddr_in)))
    {
        cerr << "连接至服务器失败！" << endl;
        close(clientfd);
        exit(-1);
    }

    // main线程用于接收用户输入，负责发送数据
    for (;;)
    {
        // 显示首页菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. 登录" << endl;
        cout << "2. 注册" << endl;
        cout << "3. 退出" << endl;
        cout << "========================" << endl;
        cout << "请选择：";
        int choice = 0;
        cin >> choice;
        cin.get(); // 读取缓冲区的回车

        switch (choice)
        {
        case 1: // login业务
        {
            int id = 0;
            char pwd[50] = {0};
            cout << "账号：";
            cin >> id;
            cin.get(); // 读取缓冲区的回车
            cout << "密码：";
            cin.getline(pwd, 50);

            json js;
            js["msgid"] = LOGIN_MSG;
            js["id"] = id;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);

            if (len == -1)
            {
                cerr << "发送登录消息失败：" << request << endl;
            }
            else
            {
                char buffer[8192] = {0};
                len = recv(clientfd, buffer, 8192, 0);
                if (-1 == len)
                {
                    cerr << "接收登录信息失败！" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if (0 != responsejs["error"].get<int>())
                    {
                        // 登录失败
                        cerr << responsejs["errormsg"] << endl;
                    }
                    else
                    {
                        // 登录成功，记录当前用户的id和name
                        g_currentUser.setId(responsejs["id"]);
                        g_currentUser.setName(responsejs["name"]);

                        // 初始化好友列表和群组列表
                        g_currentUserFriendList.clear();
                        g_currentUserGroupList.clear();

                        // 记录用户好友列表信息
                        if (responsejs.contains("friends"))
                        {
                            vector<string> vec = responsejs["friends"];
                            for (string &str : vec)
                            {
                                json js = json::parse(str);
                                User user;
                                user.setId(js["id"].get<int>());
                                user.setName(js["name"]);
                                user.setState(js["state"]);
                                g_currentUserFriendList.push_back(user);
                            }
                        }
                        // 记录当前用户的群组列表信息
                        if (responsejs.contains("groups"))
                        {
                            vector<string> vec1 = responsejs["groups"];
                            for (string &groupstr : vec1)
                            {
                                json grpjs = json::parse(groupstr);
                                Group group;
                                group.setId(grpjs["id"].get<int>());
                                group.setName(grpjs["groupname"]);
                                group.setDesc(grpjs["groupdesc"]);

                                vector<string> vec2 = grpjs["users"];
                                for (string &userstr : vec2)
                                {
                                    json js = json::parse(userstr);
                                    GroupUser user;
                                    user.setId(js["id"].get<int>());
                                    user.setName(js["name"]);
                                    user.setState(js["state"]);
                                    user.setRole(js["role"]);
                                    group.getUsers().push_back(user);
                                }
                                g_currentUserGroupList.push_back(group);
                            }
                        }

                        // 显示登录用户的基本信息
                        showCurrentUserData();

                        // 显示用户的离线消息  个人聊天消息或者群组消息
                        if (responsejs.contains("offlinemsg"))
                        {
                            vector<string> vec = responsejs["offlinemsg"];
                            for (string &str : vec)
                            {
                                json js = json::parse(str);
                                if (ONE_CHAT_MSG == js["msgid"].get<int>())
                                {
                                    cout << js["time"] << "[" << js["id"] << "]" << js["name"] << "：" << js["msg"] << endl;
                                }
                                else if (GROUP_CHAT_MSG == js["msgid"].get<int>())
                                {
                                    cout << "群消息[" << js["groupid"] << "]：" << js["time"] << "[" << js["id"] << "]" << js["name"] << "：" << js["msg"] << endl;
                                    continue;
                                }
                            }
                        }
                        // 登陆成功，启动接收线程负责接收数据
                        static int readthreadnumber = 0;
                        if (readthreadnumber == 0)
                        {
                            std::thread readTask(readTaskHandler, clientfd);
                            readTask.detach();
                            readthreadnumber++;
                        }

                        // 进入聊天主菜单页面
                        isMainMenuRunning = true;
                        mainMenu(clientfd);
                    }
                }
            }
        }
        break;
        case 2: // 注册业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            cout << "用户名：";
            cin.getline(name, 50);
            cout << "密码：";
            cin.get(pwd, 50);

            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            string request = js.dump();

            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);

            if (len == -1)
            {
                cerr << "发送注册信息失败：" << request << endl;
            }
            else
            {
                char buffer[4096] = {0};
                len = recv(clientfd, buffer, 4096, 0);
                if (-1 == len)
                {
                    cerr << "接收注册信息失败！" << endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if (0 != responsejs["error"].get<int>())
                    {
                        // 注册失败
                        cerr << name << "已经存在，注册失败！" << endl;
                    }
                    else
                    {
                        // 注册成功
                        cout << name << "注册成功，账号：" << responsejs["id"] << "，请牢记您的账号！" << endl;
                    }
                }
            }
        }
        break;

        case 3: // 退出业务
            exit(0);
            break;

        default:
            cerr << "输入无效！" << endl;
            break;
        }
    }
    return 0;
}

void help(int fd = 0, string str = " ");
void chat(int, string);
void addfriend(int, string);
void creategroup(int, string);
void addgroup(int, string);
void groupchat(int, string);
void loginout(int, string);

// 系统指出的客户端命令列表
unordered_map<string, string> commandMap = {
    {"help", "显示所有支持的命令，格式：help"},
    {"chat", "一对一聊天，格式：chat:friendid:message"},
    {"addfriend", "添加好友，格式：addfriend:friendid"},
    {"creategroup", "创建群组，格式：creategroup:groupname:groupdesc"},
    {"addgroup", "添加群组，格式：addgroup:groupid"},
    {"groupchat", "群聊，格式：groupchat:groupid:message"},
    {"loginout", "注销，格式：loginout"}};

// 注册支持的客户端命令处理
unordered_map<string, function<void(int, string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}};

// 主聊天页面程序
void mainMenu(int clientfd)
{
    help();

    char buffer[1024] = {0};
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command; // 存储命令
        int idx = commandbuf.find(":");
        if (idx == -1)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);
        }
        auto it = commandHandlerMap.find(command);
        if (it == commandHandlerMap.end())
        {
            cerr << "输入指令无效！" << endl;
            continue;
        }
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - idx)); // 代用命令处理方法
    }
}

void help(int, string)
{
    cout << "命令列表：" << endl;
    for (auto &p : commandMap)
    {
        cout << p.first << " : " << p.second << endl;
    }
    cout << endl;
}

void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "指令输入非法！" << endl;
        return;
    }

    int friendid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "聊天信息发送失败:" << buffer << endl;
    }
}

void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "添加好友信息发送失败：" << buffer << endl;
    }
}

void creategroup(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "指令输入非法！" << endl;
        return;
    }

    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = CREAT_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "创建群聊信息发送失败：" << buffer << endl;
    }
}

void addgroup(int clientfd, string str)
{
    int groupid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len)
    {
        cerr << "添加群聊信息发送失败：" << buffer << endl;
    }
}

void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if (idx == -1)
    {
        cerr << "指令输入非法！" << endl;
        return;
    }

    int groupid = atoi(str.substr(0, idx).c_str());
    string message = str.substr(idx + 1, str.size() - idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "群聊信息发送失败:" << buffer << endl;
    }
}

void loginout(int clientfd, string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (len == -1)
    {
        cerr << "注销信息发送失败:" << buffer << endl;
    }
    isMainMenuRunning = false;
}

// 接收线程
void readTaskHandler(int clientfd)
{
    for (;;)
    {
        char buffer[8192] = {0};
        int len = recv(clientfd, buffer, 8192, 0);
        if (0 == len || -1 == len)
        {
            close(clientfd);
            exit(-1);
        }

        // 接收服务端转发的数据，并反序列化生成json数据对象
        json js = json::parse(buffer);
        if (ONE_CHAT_MSG == js["msgid"].get<int>())
        {
            cout << js["time"] << "[" << js["id"] << "]" << js["name"] << "：" << js["msg"] << endl;
            continue;
        }
        else if (GROUP_CHAT_MSG == js["msgid"].get<int>())
        {
            cout << "群消息[" << js["groupid"] << "]：" << js["time"] << "[" << js["id"] << "]" << js["name"] << "：" << js["msg"] << endl;
            continue;
        }
    }
}

// 显示当前登录成功用户的基本信息
void showCurrentUserData()
{
    cout << "=================== 登录用户 ===================" << '\n';
    cout << "当前用户ID:" << g_currentUser.getId() << " 用户名:" << g_currentUser.getName() << endl;
    cout << "-------------------- 好友列表 --------------------" << '\n';
    if (!g_currentUserFriendList.empty())
    {
        for (User &user : g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }
    cout << "-------------------- 群组列表 --------------------" << '\n';
    if (!g_currentUserGroupList.empty())
    {
        for (Group &group : g_currentUserGroupList)
        {
            cout << "[" << group.getId() << "] " << group.getName() << " " << group.getDesc() << endl;
            for (GroupUser &user : group.getUsers())
            {
                cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
            }
        }
    }
    cout << "===============================================" << endl;
}

// 获取系统时间
string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    std::ostringstream oss;
    oss << std::put_time(ptm, "%Y-%m-%d %H:%M:%S"); // 使用put_time更安全和方便
    return oss.str();
}