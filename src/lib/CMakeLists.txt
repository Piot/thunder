cmake_minimum_required(VERSION 3.17.3)
project(thunder C)

set(CMAKE_C_STANDARD 11)
add_compile_definitions(_POSIX_C_SOURCE=200112L CONFIGURATION_DEBUG TYRAN_CONFIGURATION_DEBUG)

if (DEFINED PLATFORM)
    message("platform ${PLATFORM}")
    if (PLATFORM STREQUAL "OS64" )
        add_compile_definitions(TORNADO_OS_IOS)
        set(OS_IOS 1)
        set(OS_NAME "ios")
    endif()
elseif (APPLE)
    add_compile_definitions(TORNADO_OS_MACOS)
    set(OS_MACOS 1)
    set(OS_NAME "macos")
elseif (UNIX)
    add_compile_definitions(TORNADO_OS_LINUX)
    set(OS_LINUX 1)
    set(OS_NAME "linux")

elseif (WIN32)
    add_compile_definitions(TORNADO_OS_WINDOWS)
    set(OS_WINDOWS 1)
    set(OS_NAME "windows")
endif()

file(GLOB_RECURSE lib_src FOLLOW_SYMLINKS
        "*.c"
        )

if (OS_MACOS)
file(GLOB_RECURSE ios_src FOLLOW_SYMLINKS
        "platform/ios/*.c"
        )
     list (APPEND lib_src ${ios_src})
elseif (OS_LINUX)
file(GLOB_RECURSE linux_src FOLLOW_SYMLINKS
        "platform/pulseaudio/*.c"
        )
     list (APPEND lib_src ${linux_src})
endif()


add_library(thunder
        ${lib_src}
        )

set(DEPS ../../deps/)

target_include_directories(thunder PUBLIC ../include)

target_include_directories(thunder PUBLIC ${DEPS}piot/basal-c/src/include)
target_include_directories(thunder PUBLIC ${DEPS}piot/clog/src/include)
target_include_directories(thunder PUBLIC ${DEPS}piot/tiny-libc/src/include)
target_include_directories(thunder PUBLIC ${DEPS}piot/imprint/src/include)
        


