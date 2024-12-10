cmake_minimum_required(VERSION 3.16)

# Sets project name
project(projects_realtime LANGUAGES CXX C)
set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Sets C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the gRPC installation path
# set(gRPC_DIR C:/Users/karav/OneDrive/Documents/CS1230/cs1230final/grpc)

# add_subdirectory(abseil)
#find_package(absl REQUIRED)

# Set the Protobuf installation path
#set(Protobuf_DIR /opt/homebrew/opt/protobuf/lib/cmake/protobuf)
#add_subdirectory(protobuf)
include_directories(
${CMAKE_BINARY_DIR}/third_party/grpc/include)

# Find Protobuf

# # Find gRPC
#find_package(gRPC CONFIG REQUIRED)
# add_subdirectory(third_party/grpc)

# Specifies required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Gui)
find_package(Qt6 REQUIRED COMPONENTS OpenGL)
find_package(Qt6 REQUIRED COMPONENTS OpenGLWidgets)
find_package(Qt6 REQUIRED COMPONENTS Xml)
# Add these lines near the top of your CMakeLists.txt, after the existing find_package commands
# find_package(Protobuf REQUIRED)
# find_package(gRPC REQUIRED)


# Allows you to include files from within those directories, without prefixing their filepaths
include_directories(src)

# Specifies .cpp and .h files to be passed to the compiler
add_executable(${PROJECT_NAME}
    src/main.cpp

    src/realtime.cpp
    src/mainwindow.cpp
    src/settings.cpp
    src/utils/scenefilereader.cpp
    src/utils/sceneparser.cpp

    src/mainwindow.h
    src/realtime.h
    src/settings.h
    src/utils/scenedata.h
    src/utils/scenefilereader.h
    src/utils/sceneparser.h
    src/utils/shaderloader.h
    src/utils/aspectratiowidget/aspectratiowidget.hpp
    src/shapes/cube.h src/shapes/cube.cpp
    src/shapes/cone.h src/shapes/cone.cpp
    src/shapes/sphere.h src/shapes/sphere.cpp
    src/shapes/cylinder.h src/shapes/cylinder.cpp
    src/camera/camera.h src/camera/camera.cpp
    src/clientarm.h src/clientarm.cpp
    src/clientwin.h src/clientwin.cpp
    src/udpclientwin.h src/udpclientwin.cpp
)

# GLM: this creates its library and allows you to `#include "glm/..."`
add_subdirectory(glm)

add_subdirectory(entt)

# for networks
#add_subdirectory(${CMAKE_SOURCE_DIR}/../grpc/examples/cpp/helloworld grpc_build)
#target_link_libraries(${PROJECT_NAME} PRIVATE grpc_client)
#include_directories(${CMAKE_BINARY_DIR}/../grpc/examples/cpp/helloworld)

# GLEW: this creates its library and allows you to `#include "GL/glew.h"`
add_library(StaticGLEW STATIC glew/src/glew.c


)
include_directories(${PROJECT_NAME} PRIVATE glew/include)
include_directories(${PROJECT_NAME} PRIVATE entt/single_include)

# Specifies libraries to be linked (Qt components, glew, etc)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::OpenGL
    Qt::OpenGLWidgets
    Qt::Xml
    # absl::flags
    # gRPC::grpc++
    #protobuf::libprotobuf
    StaticGLEW
    # absl::strings
    # absl::base
)

# Specifies other files
qt6_add_resources(${PROJECT_NAME} "Resources"
    PREFIX
        "/"
    FILES
        resources/shaders/default.frag
        resources/shaders/default.vert
        resources/shaders/texture.frag
        resources/shaders/texture.vert
)

# GLEW: this provides support for Windows (including 64-bit)
if (WIN32)
  find_package(Threads REQUIRED)
  add_compile_definitions(GLEW_STATIC)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    opengl32
    glu32
    ws2_32
    Threads::Threads
  )
endif()

# Set this flag to silence warnings on Windows
if (MSVC OR MSYS OR MINGW)
  set(CMAKE_CXX_FLAGS "-Wno-volatile")
endif()
# Set this flag to silence warnings on MacOS
if (APPLE)
  set(CMAKE_CXX_FLAGS "-Wno-deprecated-volatile")
endif()

# set(PROTOC_EXECUTABLE /Users/alexandercueva/anaconda3/bin/protoc)
# execute_process(COMMAND protoc --version OUTPUT_VARIABLE PROTOC_VERSION)
# message(STATUS "Using protoc version: ${PROTOC_VERSION}")
