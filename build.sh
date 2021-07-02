#!/bin/bash

# 配置本地变量
GCC_ENV_DIR=$(which gcc)
GPP_ENV_DIR=$(which g++)

# 执行构建
cd build && cmake ..
