cmake_minimum_required(VERSION 3.17.3)
project(thunder_example C)

set(CMAKE_C_STANDARD 11)
add_compile_definitions(_POSIX_C_SOURCE=200112L CONFIGURATION_DEBUG TYRAN_CONFIGURATION_DEBUG)

if (APPLE)
        add_compile_definitions(TORNADO_OS_MACOS)
        set(OS_MACOS 1)
elseif (UNIX)
        add_compile_definitions(TORNADO_OS_LINUX)
        set(OS_LINUX 1)
endif()


include_directories(include)

include_directories(../deps/piot/basal-c/src/include)
include_directories(../deps/piot/clog/src/include)
include_directories(../deps/piot/tiny-libc/src/include)
include_directories(../deps/piot/imprint/src/include)
include_directories(../deps/piot/sense/src/include)
include_directories(../deps/piot/breathe/src/include)
include_directories(../deps/piot/burst/src/include)
include_directories(../deps/piot/latch/src/include)
include_directories(../deps/piot/scenery/src/include)
include_directories(../deps/piot/lightning/src/include)
include_directories(../deps/piot/turmoil-resource/src/include)

file(GLOB_RECURSE deps_src FOLLOW_SYMLINKS
        "../deps/*/src/*.c"
        )

# Remove iOS and SDL (including Webassembly)
list(FILTER deps_src EXCLUDE REGEX ".*sdl_main.c" )
list(FILTER deps_src EXCLUDE REGEX ".*webassembly/main.c" )
list(FILTER deps_src EXCLUDE REGEX ".*/*/src/examples/main.c" )
list(FILTER deps_src EXCLUDE REGEX ".*/*/src/example/main.c" )
list(FILTER deps_src EXCLUDE REGEX ".*/*/src/platform/ios/.*" )
list(FILTER deps_src EXCLUDE REGEX ".*/*/src/platform/posix/.*" )
list(FILTER deps_src EXCLUDE REGEX ".*/*/src/platform/webassembly/.*" )
list(FILTER deps_src EXCLUDE REGEX ".*/*/src/platform/sdl/.*" )





file(GLOB_RECURSE lib_src FOLLOW_SYMLINKS
        "lib/*.c"
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


add_executable(thunder_example
        ${deps_src}
        ${lib_src}
        ../deps/piot/burst/src/platform/posix/burst_file_loader_posix.c
        example/app.c
        )

find_package (Vulkan)
if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries (thunder_example ${Vulkan_LIBRARIES})
endif (Vulkan_FOUND)
find_package(glfw3 3.3 REQUIRED)

if (OS_MACOS)
   find_library(COREAUDIO_LIBRARY CoreAudio)
   find_library(COREFOUNDATION_LIBRARY CoreFoundation)
   find_library(COREAUDIO_TOOLBOX_LIBRARY AudioToolbox)
   message("hello ${COREAUDIO_TOOLBOX_LIBRARY}\n ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY}\n")
   target_link_libraries(thunder_example ${COREAUDIO_LIBRARY})
   target_link_libraries(thunder_example ${COREFOUNDATION_LIBRARY})
   target_link_libraries(thunder_example ${COREAUDIO_TOOLBOX_LIBRARY})
endif(OS_MACOS)

if (OS_LINUX)
        # PulseAudio
        message("Add pulse audio!")
        find_package (PulseAudio REQUIRED)
        include_directories(${PulseAudio_INCLUDE_DIRS})
        target_link_libraries (thunder_example ${PulseAudio_LIBRARIES})
        target_link_libraries(thunder_example pulse-simple)
        target_link_libraries(thunder_example pulse)
endif(OS_LINUX)

target_link_libraries(thunder_example m)
target_link_libraries(thunder_example glfw)
target_link_libraries(thunder_example -fsanitize=address)

