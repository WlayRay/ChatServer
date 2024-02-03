#include "groupmodel.hpp"
#include "db.h"

// 创建群组
bool GroupModel::createGroup(Group &group)
{
    string sql = "insert into AllGroup (groupname, groupdesc) value ('";
    sql += group.getName();
    sql += "', '";
    sql += group.getDesc();
    sql += "')";

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

// 加入群组
void GroupModel::addGroup(int groupid, int userid, string role)
{
    string sql = "insert into GroupUser (groupid, userid, grouprole) value (";
    sql += to_string(groupid);
    sql += ", ";
    sql += to_string(userid);
    sql += ", '";
    sql += role;
    sql += "')";

    MySQL mysql;
    if (mysql.connect())
    {
        mysql.update(sql);
    }
}

// 查询用户所在群组信息
vector<Group> GroupModel::queryGroups(int userid)
{
    string sql = "SELECT AllGroup.id, AllGroup.groupname, AllGroup.groupdesc FROM AllGroup JOIN GroupUser ON AllGroup.id = GroupUser.groupid WHERE GroupUser.userid = ";
    sql += to_string(userid);

    vector<Group> groupVec;

    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
        }
        mysql_free_result(res);

        // 查询群组用户信息
        for (Group &group : groupVec)
        {
            string sql = "select id, name, state, grouprole from GroupUser left join User on userid = id where groupid = ";
            sql += to_string(group.getId());

            MYSQL_RES *res = mysql.query(sql);
            if (res != nullptr)
            {
                MYSQL_ROW row;
                while ((row = mysql_fetch_row(res)) != nullptr)
                {
                    GroupUser user;
                    user.setId(atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

// 根据指定的groupid查询群组用户id列表，除userid自己，用户群聊中群主给其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    string sql = "select userid from GroupUser where userid != ";
    sql += to_string(userid);
    sql += " and groupid = ";
    sql += to_string(groupid);

    vector<int> idVec;
    MySQL mysql;
    if (mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if (res != nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res)) != nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}