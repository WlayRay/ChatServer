#!/bin/bash

set -e
set -x

rm -rf "$(pwd)/build/"*
cd "$(pwd)/build" &&
	cmake .. &&
	make || exit 1

echo "项目构建成功"
