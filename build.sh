#!/bin/bash

# 配置本地变量
export GCC_ENV_DIR=$(which gcc)
export GPP_ENV_DIR=$(which g++)

# 执行构建
./clean.sh  # 先清除一下之前生成的文件
mkdir -p build
cd build && cmake .. && make
