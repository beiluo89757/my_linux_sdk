# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (test)

# 查找目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
aux_source_directory(. DIR_SRCS)

# 头文件包含路径
INCLUDE_DIRECTORIES(../../include)


# 指定生成目标
add_executable(test ${DIR_SRCS})