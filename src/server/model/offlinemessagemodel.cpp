#include "offlinemessagemodel.hpp"
#include "db.h"

// 存储用户的离线消息
void OfflineMsgModel::insert(int userid, string msg)
{
    std::string sql = "insert into OfflineMessage (userid, message) values (";
    sql += std::to_string(userid);
    sql += ",'";
    sql += msg;
    sql += "')";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    std::string sql = "delete from OfflineMessage where userid = ";
    sql += std::to_string(userid);

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}
// 查询用户的离线消息
vector<string> OfflineMsgModel::query(int userid)
{
    std::string sql = "select message from OfflineMessage where userid = ";
    sql += std::to_string(userid);
    vector<string> vec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                vec.push_back(row[0]);
            };
            mysql_free_result(res);
        }
    }
    return vec;
}