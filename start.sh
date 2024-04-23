#!/bin/bash

# 定义服务的名称和启动命令
SERVICES=("nginx" "redis")
SERVICE_COMMANDS=("nginx" "redis-server")

# 定义ChatServer的启动目录和初始端口
CHATSERVER_DIR="./bin"
CHATSERVER_PORT=6000
CHATSERVER_COMMAND="./ChatServer 127.0.0.1"

# 函数：检查端口是否被占用
function check_port_usage() {
    local port=$1
    netstat -tuln | grep ":$port "
}

# 函数：查找未被占用的端口
function find_unused_port() {
    local start_port=$1
    while check_port_usage $start_port; do
        ((start_port++))
    done
    echo $start_port
}

# 进入ChatServer的启动目录
cd "$CHATSERVER_DIR" || exit 1

# 检查并启动服务
for service in "${SERVICES[@]}"; do
    if ! systemctl is-active --quiet "$service"; then
        echo "Starting $service..."
        sudo systemctl start "$service"
        if ! systemctl is-active --quiet "$service"; then
            echo "Failed to start $service."
            exit 1
        fi
        echo "$service started."
    fi
done

# 查找并设置ChatServer的端口
CHATSERVER_PORT=$(find_unused_port $CHATSERVER_PORT)

# 启动ChatServer作为守护进程
echo "Starting ChatServer on port $CHATSERVER_PORT as a daemon..."
nohup $CHATSERVER_COMMAND $CHATSERVER_PORT >/dev/null 2>&1 &

# 检查ChatServer是否启动
if pgrep -f "$CHATSERVER_COMMAND $CHATSERVER_PORT" >/dev/null; then
    echo "ChatServer started as a daemon."
else
    echo "Failed to start ChatServer as a daemon."
    exit 1
fi