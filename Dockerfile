FROM ubuntu:22.04  

# 安装基本的依赖  
RUN apt-get update && apt-get install -y \  
    build-essential \  
    git \  
    cmake \  
    libboost-all-dev \  
    libssl-dev \  
    libmysqlclient-dev \  
    libhiredis-dev \  
    nginx \  
    redis-server \  
    && rm -rf /var/lib/apt/lists/*  

# 下载并编译muduo库  
WORKDIR /usr/src/lib  
RUN git clone https://github.com/chenshuo/muduo.git  
WORKDIR /usr/src/lib/muduo/build  
RUN cmake .. && make && make install  

# 复制你的项目源代码到镜像中，并确保脚本可执行  
COPY --chmod=+x . /usr/src/ChatServer  

# 复制Nginx配置文件  
COPY conf/nginx.conf /etc/nginx/nginx.conf  

# 暴露端口  
EXPOSE 8001 6000  

# 构建你的项目  
WORKDIR /usr/src/ChatServer  
RUN ./autobuild.sh  

# 设置入口点命令，确保start.sh不会立即退出  
ENTRYPOINT ["/usr/src/ChatServer/start.sh"]