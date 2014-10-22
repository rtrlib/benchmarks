project(rtrlib-benchmark)
cmake_minimum_required(VERSION 2.6)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -Wall -g -Wextra -Wformat-security -Winit-self -Waggregate-return -Wmissing-format-attribute -Wundef -Wbad-function-cast -Wwrite-strings -Wformat=2")

include_directories(.)
include_directories(/home/nils/git/rtrlib)

find_library(RTR_LIB rtr PATHS /home/nils/git/rtrlib/build)
find_package(Threads REQUIRED)

ADD_SUBDIRECTORY(pfx-benchmark)
ADD_SUBDIRECTORY(spki-benchmark)
