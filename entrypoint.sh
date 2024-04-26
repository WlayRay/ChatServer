#!/bin/bash  
  
# 启动MySQL  
service mysql start  
  
# 启动Redis  
service redis-server start  
  
# 启动Nginx  
service nginx start  
  
# 等待服务启动，或者添加逻辑确保它们已准备好  
  
# 构建和运行你的项目  
cd /usr/src/ChatServer  
./autobuild.sh  
cd bin/
./ChatServer 127.0.0.1 6002