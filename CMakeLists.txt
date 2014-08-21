project(rtrlib-benchmark)
cmake_minimum_required(VERSION 2.6)

include_directories(/usr/local/include/)
find_package(Threads REQUIRED)
find_library(RT_LIB rtr PATHS /usr/local/lib64)

ADD_SUBDIRECTORY(pfx-lpfst-benchmark)
