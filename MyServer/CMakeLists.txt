#请求CMake的版本
cmake_minimum_required(VERSION 3.8)

#指定项目名称
project(myServer)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#指定C++标准为17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#在MSVC上必须指定 /Zc:__cplusplus 选项
if(MSVC)
	add_compile_options(/Zc:__cplusplus)
endif()

#指定第三方库的CMake驱动
set(CMAKE_PREFIX_PATH "D:/Qt/6.7.1/msvc2019_64/lib/cmake")

#指定头文件包含目录
include_directories("D:/Qt/6.7.1/msvc2019_64/include")

#设置需要的dll文件的环境变量
set(Qt_DIR "D:/Qt/6.7.1/msvc2019_64/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../bin)
set(ENV{PATH} "$ENV{PATH};${Qt_DIR}")

#查找指定的模块
find_package(Qt6 COMPONENTS Widgets WebSockets REQUIRED)

#指定目标生成位置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/../bin)

#添加宏定义 #QT_NO_DEBUG_OUTPUT
add_compile_definitions( QT_DISABLE_DEPRECATED_BEFORE=0x060000)

#让cmake根据ui文件生成对应头文件
qt_wrap_ui(myServer,"Mainwindow.ui")

#根据指定的文件生成可执行程序
add_executable(myServer "main.cpp" "Server.h" "Server.cpp")

#给目标连接库文件
target_link_libraries(myServer Qt6::Widgets Qt6::WebSockets)