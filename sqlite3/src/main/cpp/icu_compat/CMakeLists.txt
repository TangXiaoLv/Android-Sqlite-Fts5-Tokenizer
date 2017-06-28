cmake_minimum_required(VERSION 3.4.1)
project(icucompat C CXX)

aux_source_directory(${ICU_COMPAT_DIR} ICU_COMPAT_DIR)

INCLUDE_DIRECTORIES(${ICU_COMPAT_DIR})
INCLUDE_DIRECTORIES(unicode)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_REENTRANT \
-Wall -Werror -ffunction-sections -fdata-sections \
-DSQLITE_HAS_CODEC -DSQLITE_CORE -DSQLITE_OS_UNIX")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPIC -fPIC")

add_library(icucompat STATIC ${ICU_COMPAT_DIR})