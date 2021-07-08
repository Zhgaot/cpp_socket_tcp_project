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

# 指定头文件的包含路径
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

# 发现指定路径下的所有cc文件
AUX_SOURCE_DIRECTORY(./src LIB_SRCS)

# 指定库生成路径
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 生成链接库
ADD_LIBRARY(basic_tcp SHARED ${LIB_SRCS})

# 指定需要链接的库
LINK_LIBRARIES(pthread)  # 由于pthread库的路径在默认搜索路径中，因此无需指定库路径
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib)
LINK_LIBRARIES(basic_tcp)

# 指定可执行文件的生成路径
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# 生成可执行文件
# ADD_EXECUTABLE(client_exe client.cc src/basic_tcp.cc)
ADD_EXECUTABLE(client_exe client.cc)

# target_link_libraries(client_exe pthread)  # 库已在前面指定了