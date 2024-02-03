#include "friendmodel.hpp"
#include "db.h"

// 添加好友关系
void FriendModel::insert(int userid, int friendid)
{
    std::string sql = "insert into Friend (userid, friendid) values (";
    sql += std::to_string(userid);
    sql += ",";
    sql += std::to_string(friendid);
    sql += ")";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 返回用户好友列表
vector<User> FriendModel::query(int userid) 
{
    std::string sql = "select friendid, name, state from Friend left join User on friendid = id where userid = ";
    sql += std::to_string(userid);

    vector<User> vec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            };
            mysql_free_result(res);
        }
    }
    return vec;
}