cmake_minimum_required(VERSION 3.0)

project(Vulkan-Engine VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")

set(Vulkan_LIBRARY C:/Users/mizer/dev/Development-Kits/VulkanSDK/1.3.290.0/Lib/vulkan-1.lib)
set(Vulkan_INCLUDE_DIR C:/Users/mizer/dev/Development-Kits/VulkanSDK/1.3.290.0/Include)

find_package(Vulkan REQUIRED)

# add_subdirectory(${CMAKE_SOURCE_DIR}/../Debelopment-Kits/assimp-5.4.2)
# I guess cmake can easily find assimp...
find_package(assimp REQUIRED)

# add_subdirectory(${CMAKE_SOURCE_DIR}/../Development-Kits/glfw-3.3.8)
set (glfw3_DIR "C:/Program Files (x86)/GLFW/lib/cmake/glfw3") # I hate this, but I don't want to spend more time on it, it accomplishes what I wanted to do...
find_package(glfw3 3.3 REQUIRED)

option(IS_SHARED_LIB "build as dll" OFF)

# local source
file(GLOB_RECURSE SOURCES 
${PROJECT_SOURCE_DIR}/src/*.cpp 
# I dont think this does what I think it does...
${PROJECT_SOURCE_DIR}/src/*.hpp
${PROJECT_SOURCE_DIR}/src/*.h
${PROJECT_SOURCE_DIR}/libs/tinyXML/*.cpp
)

if(IS_SHARED_LIB)
    add_library(engine SHARED ${SOURCES})
else()
    add_executable(engine ${SOURCES}) 
endif()

target_include_directories(engine PUBLIC
    ${PROJECT_SOURCE_DIR}/src  
    "C:/Program Files/Mono/include/mono-2.0"
)

target_sources(engine PRIVATE
"${PROJECT_SOURCE_DIR}/libs/imgui/imgui.h"
"${PROJECT_SOURCE_DIR}/libs/imgui/imgui.cpp"

"${PROJECT_SOURCE_DIR}/libs/imgui/imgui_demo.cpp"
"${PROJECT_SOURCE_DIR}/libs/imgui/imgui_draw.cpp"
"${PROJECT_SOURCE_DIR}/libs/imgui/imgui_widgets.cpp"
"${PROJECT_SOURCE_DIR}/libs/imgui/imgui_tables.cpp"

"${PROJECT_SOURCE_DIR}/libs/imgui/imgui_impl_vulkan.cpp"
"${PROJECT_SOURCE_DIR}/libs/imgui/imgui_impl_glfw.cpp"
)

target_link_libraries(engine glfw)
target_link_libraries(engine Vulkan::Vulkan)
target_link_libraries(engine "C:/Program Files/mono/msvc/build/sgen/x64/lib/Debug/mono-2.0-sgen.lib")
target_link_libraries(engine assimp::assimp)