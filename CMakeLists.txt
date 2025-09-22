cmake_minimum_required(VERSION 3.27)

project(Vulkan-Engine VERSION 1.0)

# hash_combine in src/Utils.hpp requires c++17 :)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Generating Compile_Commands")
    set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
endif()

if(WIN32)
find_file(Vulkan_LIBRARY
    NAMES vulkan-1.lib
    HINTS
        $ENV{VULKAN_SDK}/lib
    REQUIRED
)
set(Vulkan_INCLUDE_DIR $ENV{VULKAN_SDK}/Include)
endif()
find_package(Vulkan REQUIRED)

find_package(assimp REQUIRED)

#somehow, this works on linux, for now I won't touch it.
set (glfw3_DIR "C:/Program Files (x86)/GLFW/lib/cmake/glfw3") #TODO: Replace with find_path
find_package(glfw3 3.3 REQUIRED)

# local source
file(GLOB_RECURSE SOURCES 
${PROJECT_SOURCE_DIR}/src/*.cpp 
${PROJECT_SOURCE_DIR}/src/*.hpp
${PROJECT_SOURCE_DIR}/src/*.h
${PROJECT_SOURCE_DIR}/libs/tinyXML/*.cpp
)

option(IS_STATIC_LIB "build as lib" ON)
option(IS_CONSOLE_APP "show terminal" ON)

# NOTE: My libs are built for debug, not release. at the very least, Assimp does not like the difference :(
if(IS_STATIC_LIB)
    message(STATUS "Building as static library")
    add_library(engine STATIC ${SOURCES})
    target_compile_definitions(engine PRIVATE EDITOR)
else()
    if(IS_CONSOLE_APP)
        message("building as Console Application")
        add_executable(engine ${SOURCES})
        set_target_properties(engine PROPERTIES WIN32_EXECUTABLE OFF)
        target_compile_definitions(engine PRIVATE CONSOLE)
    else()
        message("building as Windows Desktop Application")
        add_executable(engine WIN32 ${SOURCES})
    endif() # IS_CONSOLE_APP
endif() # IS_STATIC_LIB

#this is where I am trying to remove all path problems by defining globals ===========

target_compile_definitions(engine PRIVATE SOURCE_PATH="${PROJECT_SOURCE_DIR}")

option(_ASSET_PATH "define path to game assets" OFF)
if(_ASSET_PATH)
    target_compile_definitions(engine PRIVATE ASSET_PATH=${_ASSET_PATH})
endif() #_ASSET_PATH

#mono stuff =============================================
if(WIN32)
    set(Mono_DIR "C:/Program Files/Mono") # for includes
    find_file(Mono_File
	NAMES mono-2.0-sgen.dll
	HINTS "C:/Program Files/Mono/bin"
	DOCS "I might just use Lua..."
	REQUIRED
    )
endif()
if(UNIX)
    set(Mono_DIR "/usr")
    find_file(Mono_File
	NAMES libmonosgen-2.0.so
	HINTS /lib
	DOCS "Mono might suck"
	REQUIRED
    )
endif()
message(STATUS "Mono DLL found at: ${Mono_File}")
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${Mono_File}"
        $<TARGET_FILE_DIR:engine>/
    COMMENT "Copying Mono runtime to application directory"
)
#===============================================================

target_include_directories(engine PUBLIC
    "${PROJECT_SOURCE_DIR}/src"  
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/libs"
    "${Mono_DIR}/include/mono-2.0"
)

target_link_libraries(engine PUBLIC glfw)
target_include_directories(engine PUBLIC ${GLFW_INCLUDE_DIRS})
target_link_libraries(engine PUBLIC Vulkan::Vulkan)
target_link_libraries(engine PUBLIC assimp::assimp)
if(WIN32)
    target_link_libraries(engine PUBLIC "C:/Program Files/Mono/lib/mono-2.0-sgen.lib")
endif()
if(UNIX)
    target_link_libraries(engine PUBLIC "/usr/lib/libmonosgen-2.0.a")
endif()
