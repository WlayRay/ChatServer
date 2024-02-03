#include "usermodel.hpp"
#include "db.h"

bool UserModel::insert(User &user)
{
    std::string sql = "insert into User (name, password, state) values ('";
    sql += user.getName();
    sql += "', '";
    sql += user.getPwd();
    sql += "', '";
    sql += user.getState();
    sql += "')";

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    std::string sql = "SELECT * FROM `User` WHERE `id` = ";
    sql += std::to_string(id); // 使用to_string来转换id，防止SQL注入

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if (row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::updateState(User user)
{
    std::string sql = "UPDATE User SET state = '";
    sql += user.getState();
    sql += "' WHERE id = '";
    sql += std::to_string(user.getId());
    sql += "'";
    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    std::string sql = "UPDATE User SET state = 'offline' WHERE state = 'online'";
    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}