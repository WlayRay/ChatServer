FROM ubuntu:22.04  

# 安装基本的依赖  
RUN apt-get update && apt-get install -y --no-install-recommends \  
    build-essential \  
    git \  
    cmake \  
    libboost-all-dev \  
    libssl-dev \  
    libhiredis-dev \  
    redis-server \  
    nginx \  
    mysql-server \  
    libmysqlclient-dev \  
    --reinstall ca-certificates \
    && rm -rf /var/lib/apt/lists/*  

# 设置MySQL root密码预设值  
# 使用环境变量来设置密码  
ARG MYSQL_ROOT_PASSWORD=123456  
ENV MYSQL_ROOT_PASSWORD $MYSQL_ROOT_PASSWORD  
RUN { \  
    echo mysql-server mysql-server/root_password password ${MYSQL_ROOT_PASSWORD}; \  
    echo mysql-server mysql-server/root_password_again password ${MYSQL_ROOT_PASSWORD}; \  
    } | debconf-set-selections  

# 添加chat.sql到容器中
COPY chat.sql /docker-entrypoint-initdb.d/

# 等待 MySQL 服务启动并设置新的密码  
RUN service mysql start && \  
    mysql -uroot -p${MYSQL_ROOT_PASSWORD} -e "ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY '${MYSQL_ROOT_PASSWORD}'; FLUSH PRIVILEGES;" && \  
    service mysql stop  

# 复制 Nginx 配置文件  
COPY conf/nginx.conf /etc/nginx/sites-available/default  

# 创建软链接到 sites-enabled 目录  
RUN ln -sf /etc/nginx/sites-available/default /etc/nginx/sites-enabled/default  

# 检查Nginx配置文件的语法是否正确  
# RUN nginx -t  

# 下载并编译muduo库  
WORKDIR /usr/src  
RUN git clone https://github.com/chenshuo/muduo.git && \  
    cd muduo && \  
    mkdir build && \  
    cd build && \  
    cmake .. && \  
    make && \  
    make install && \  
    cd .. && \  
    rm -rf muduo  

# 复制项目源代码到镜像中  
COPY . /usr/src/ChatServer  

# 暴露端口  
EXPOSE 8001 6000 6001 6002  

# 设置启动脚本  
COPY entrypoint.sh /usr/local/bin/entrypoint.sh  
RUN chmod +x /usr/local/bin/entrypoint.sh  


# 设置MySQL在容器启动时执行chat.sql文件
CMD ["mysqld", "--init-file=/docker-entrypoint-initdb.d/chat.sql"]

# 设置入口点脚本  
ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]  
