cmake_minimum_required(VERSION 3.4.1)
project(icuuc C CXX)

aux_source_directory(${ICU_COMMON_DIR} ICU_COMMON_SOURCE)

INCLUDE_DIRECTORIES(${ICU_COMMON_DIR})
INCLUDE_DIRECTORIES(${ICU_COMMON_DIR}/unicode)
INCLUDE_DIRECTORIES(${ICU_I18N_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_REENTRANT")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DU_COMMON_IMPLEMENTATION")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -fvisibility=hidden")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
-Wno-unused-parameter \
-Wno-missing-field-initializers \
-Wno-sign-compare \
-Wno-deprecated-declarations")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
-DPIC \
-fPIC")

#add_library(icuuc SHARED ${ICU_COMMON_SOURCE})
add_library(icuuc STATIC ${ICU_COMMON_SOURCE})

find_library(libdl dl)

target_link_libraries(icuuc ${libdl})