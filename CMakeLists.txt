cmake_minimum_required(VERSION 3.10)
project(OSG_LOD_Demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows下需要定义这个宏以使用std::filesystem
if(WIN32)
    add_definitions(-D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING)
endif()

# 查找OpenSceneGraph库
find_package(OpenSceneGraph REQUIRED COMPONENTS
    osg
    osgDB
    osgGA
    osgViewer
    osgUtil
)

# 添加可执行文件
add_executable(${PROJECT_NAME} main.cpp)

# 链接OSG库
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSCENEGRAPH_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${OPENSCENEGRAPH_LIBRARIES})

# 如果使用MSYS2/MinGW，可能需要添加OpenGL库
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} OpenGL::GL)

