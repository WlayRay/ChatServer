# 定义变量SRC_LIST，包含当前目录下的所有源文件
aux_source_directory(. SRC_LIST)
aux_source_directory(./db DB_LIST)
aux_source_directory(./model MODEL_LIST)
aux_source_directory(./redis REDIS_LIST)

#查找链接所需要的库
find_library(SSL_LIB ssl)  
find_library(CRYPTO_LIB crypto) 

# 设置目标链接库   
set(LIBS ${SSL_LIB} ${CRYPTO_LIB}) 

# 指定可生成文件
add_executable(ChatServer ${SRC_LIST} ${DB_LIST} ${MODEL_LIST} ${REDIS_LIST})

# 指定可执行文件链接时需要依赖的库文件
target_link_libraries(ChatServer muduo_net muduo_base mysqlclient hiredis pthread ${LIBS})
