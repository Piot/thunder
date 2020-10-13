cmake_minimum_required(VERSION 3.17.3)
project(thunder_example C)

set(CMAKE_C_STANDARD 11)
add_compile_definitions(_POSIX_C_SOURCE=200112L TORNADO_OS_LINUX CONFIGURATION_DEBUG TYRAN_CONFIGURATION_DEBUG)

include_directories(include)

include_directories(../deps/basal-c/src/include)
include_directories(../deps/clog/src/include)
include_directories(../deps/tyran/src/include)
include_directories(../deps/tiny-libc/src/include)
include_directories(../deps/imprint/src/include)
include_directories(../deps/sense/src/include)
include_directories(../deps/breathe/src/include)
include_directories(../deps/burst/src/include)
include_directories(../deps/latch/src/include)
include_directories(../deps/scenery/src/include)
include_directories(../deps/lightning/src/include)
include_directories(../deps/swamp-runtime/src/include)
include_directories(../deps/swamp-script-scope/src/include)
include_directories(../deps/swamp-unpack-c/src/include)
include_directories(../deps/turmoil-resource/src/include)

file(GLOB_RECURSE deps_src FOLLOW_SYMLINKS
        "../deps/*/src/*.c"
        )

# Remove iOS and SDL (including Webassembly)
list(FILTER deps_src EXCLUDE REGEX ".*sdl_main.c" )
list(FILTER deps_src EXCLUDE REGEX ".*webassembly/main.c" )
list(FILTER deps_src EXCLUDE REGEX ".*src/examples/main.c" )
list(FILTER deps_src EXCLUDE REGEX ".*src/example/main.c" )
list(FILTER deps_src EXCLUDE REGEX ".*src/platform/ios/.*" )
list(FILTER deps_src EXCLUDE REGEX ".*src/platform/posix/.*" )
list(FILTER deps_src EXCLUDE REGEX ".*src/platform/webassembly/.*" )
list(FILTER deps_src EXCLUDE REGEX ".*src/platform/sdl/.*" )


file(GLOB_RECURSE lib_src FOLLOW_SYMLINKS
        "lib/*.c"
        )


add_executable(thunder_example
        ${deps_src}
        ${lib_src}
        ../deps/burst/src/platform/posix/burst_file_loader_posix.c
        example/app.c
#       platform/pulseaudio/pulseaudio_sound_driver.c
#       platform/pulseaudio/sound_driver.c
        platform/sdl/sound_driver.c
        platform/sdl_common/sdl_sound_driver.c
        )

find_package (Vulkan)
if (Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    target_link_libraries (thunder_example ${Vulkan_LIBRARIES})
endif (Vulkan_FOUND)
find_package(glfw3 3.3 REQUIRED)

# SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
target_link_libraries(thunder_example ${SDL2_LIBRARIES})

# PulseAudio
#find_package (PulseAudio REQUIRED)
#include_directories(${PulseAudio_INCLUDE_DIRS})
#target_link_libraries (thunder_example ${PulseAudio_LIBRARIES})

target_link_libraries(thunder_example m)
target_link_libraries(thunder_example glfw)
target_link_libraries(thunder_example pulse-simple)
target_link_libraries(thunder_example pulse)
target_link_libraries(thunder_example -fsanitize=address)

