cmake_minimum_required(VERSION 3.10)

project(tcpChat)

cmake_policy(SET CMP0074 NEW)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/out/bin/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/out/bin/release)

set(CMAKE_STATIC_LIBRARY_PREFIX "")

if (UNIX)
    set(CMAKE_CXX_FLAGS_DEBUG "-g3 -ggdb -Wall -pedantic -std=c++17 -Wno-unknown-pragmas")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -pedantic -std=c++17 -Wno-unknown-pragmas")
    set(ADDITIONAL_LIBRARIES pthread)
else()
    if(MINGW)
        set(CMAKE_CXX_FLAGS_DEBUG "-g3 -ggdb -Wall -pedantic -std=c++17 -Wno-unknown-pragmas")
        set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -pedantic -std=c++17 -Wno-unknown-pragmas")
        set(Boost_NAMESPACE "libboost")
        set(Boost_ARCHITECTURE "-x64")
        set(Boost_COMPILER "-mgw8")
        set(ADDITIONAL_LIBRARIES ws2_32)
    elseif(WIN32)
        set(CMAKE_CXX_FLAGS_DEBUG "/Od /Wall /std:c++17")
        set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Oi /Ot /GL /W0 /std:c++17")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
        set(Boost_NAMESPACE "libboost")
        set(Boost_ARCHITECTURE "-x64")
        set(Boost_COMPILER "-vc142")
        set(ADDITIONAL_LIBRARIES ws2_32)
    endif()
endif()

if (CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Debug Release)
endif()

set(SOURCE_CHAT src/chat.cpp)

find_package(Boost REQUIRED date_time regex)

add_executable(
    tcpChat
    ${SOURCE_CHAT}
)

target_include_directories(
    tcpChat
    PUBLIC ${Boost_INCLUDE_DIRS}
)

target_link_libraries(tcpChat ${Boost_LIBRARIES} ${ADDITIONAL_LIBRARIES})

add_custom_target(
    debug
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
)

add_custom_target(
    release
    COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target all
)
