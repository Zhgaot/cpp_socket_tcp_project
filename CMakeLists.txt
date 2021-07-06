# 指定最小cmake版本
CMAKE_MINIMUM_REQUIRED(VERSION 3.0)

# 在cmake中使用build.sh中设定好的环境变量来指定编译器位置
SET(CMAKE_C_COMPILER $ENV{GCC_ENV_DIR})
SET(CMAKE_CXX_COMPILER $ENV{GPP_ENV_DIR})

# 设置项目名称
PROJECT(socket_project)

# 设置g++编译参数
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")

# 设置为debug版本以方便debug
SET(CMAKE_BUILD_TYPE Debug)

# 指定
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

# 指定需要链接的库
LINK_LIBRARIES(pthread)  # 由于pthread库的路径在默认搜索路径中，因此无需指定库路径

# 生成可执行文件
ADD_EXECUTABLE(mySocket_exe server.cpp src/XTcp.cpp)

# target_link_libraries(mySocket_exe pthread)  # 库已在前面指定了