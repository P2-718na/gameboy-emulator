CMAKE_MINIMUM_REQUIRED(VERSION 3.11)

PROJECT(
        gameboy-emulator
        VERSION 1.0
        DESCRIPTION "Gameboy emulator"
        HOMEPAGE_URL https://github.com/P2-718na/gameboy-emulator
        LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

FIND_PACKAGE(SFML 2.5 COMPONENTS graphics window REQUIRED)

INCLUDE_DIRECTORIES(
        "${PROJECT_SOURCE_DIR}/src"
        "${PROJECT_SOURCE_DIR}/src/Engine"
        "${PROJECT_SOURCE_DIR}/src/Config"
        "${PROJECT_SOURCE_DIR}/src/Gameboy"
        "${PROJECT_SOURCE_DIR}/src/Processor"
        "${PROJECT_SOURCE_DIR}/src/Memory"
        "${PROJECT_SOURCE_DIR}/src/Cartridge"
        "${PROJECT_SOURCE_DIR}/src/Graphics"
        "${PROJECT_SOURCE_DIR}/src/TimerController"
        "${PROJECT_SOURCE_DIR}/dist")

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(tests)

CONFIGURE_FILE(scripts/quick-run.sh "${PROJECT_BINARY_DIR}" COPYONLY)
CONFIGURE_FILE(assets/tetris.gb "${PROJECT_BINARY_DIR}" COPYONLY)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif ()

MESSAGE("-- Building using ${CMAKE_BUILD_TYPE} configuration")

SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Wimplicit-fallthrough=3")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -fsanitize=\"address\"")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
