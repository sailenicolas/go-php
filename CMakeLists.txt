cmake_minimum_required(VERSION 3.21)
project(gophp C)

set(CMAKE_C_STANDARD 17)

include_directories(includes/php8)
include_directories(/usr/include/php/main)
include_directories(/usr/include/php/Zend)
include_directories(/usr/include/php/TSRM)
include_directories(/usr/include/php)
add_executable(gophp includes/context.c includes/engine.c includes/receiver.c includes/value.c)
