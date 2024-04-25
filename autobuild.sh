#!/bin/bash  
  
set -e  
set -x  
  
# 定义build目录的路径  
build_dir="$(pwd)/build"  
  
# 检查build目录是否存在，如果不存在则创建它  
if [ ! -d "${build_dir}" ]; then  
    mkdir -p "${build_dir}"  
fi  
  
# 清除build目录下的所有内容  
rm -rf "${build_dir}"/*  
  
# 切换到build目录并执行cmake和make  
cd "${build_dir}" &&  
    cmake .. &&  
    make || exit 1  
  
echo "项目构建成功"