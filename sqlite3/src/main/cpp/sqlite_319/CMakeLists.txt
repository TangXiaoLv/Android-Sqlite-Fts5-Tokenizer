cmake_minimum_required(VERSION 3.4.1)
project(sqliteX C CXX)

# If using SEE, uncomment the following:
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSQLITE_HAS_CODEC")

#Define HAVE_USLEEP, otherwise ALL sleep() calls take at least 1000ms
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_USLEEP=1")

# Enable SQLite extensions.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
-DSQLITE_ENABLE_FTS3 \
-DSQLITE_ENABLE_FTS4 \
-DSQLITE_ENABLE_FTS5 \
-DSQLITE_ENABLE_RTREE \
-DSQLITE_ENABLE_JSON1")
#-DSQLITE_ENABLE_ICU

# This is important - it causes SQLite to use memory for temp files. Since
# Android has no globally writable temp directory, if this is not defined the
# application throws an exception when it tries to create a temp file.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSQLITE_TEMP_STORE=3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_CONFIG_H -DKHTML_NO_EXCEPTIONS -DGKWQ_NO_JAVA")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNO_SUPPORT_JS_BINDING -DQT_NO_WHEELEVENT -DKHTML_NO_XBL")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U__APPLE__")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_STRCHRNUL=0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSQLITE_USE_URI=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter -Wno-int-to-pointer-cast")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-uninitialized -Wno-parentheses")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null")

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${DEVICE_SQLITE_FLAGS}")

if ("${CMAKE_ANDROID_ARCH_ABI}" STREQUAL arm)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPACKED='__attribute__ ((packed))'")
else ()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPACKED")
endif ()

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(./extra)
INCLUDE_DIRECTORIES(nativehelper)
#INCLUDE_DIRECTORIES(${ICU_COMMON_DIR})
#INCLUDE_DIRECTORIES(${ICU_COMMON_DIR}/unicode)
#INCLUDE_DIRECTORIES(${ICU_I18N_DIR})
#INCLUDE_DIRECTORIES(${ICU_I18N_DIR}/unicode)
INCLUDE_DIRECTORIES(${ICU_COMPAT_DIR})
INCLUDE_DIRECTORIES(${ICU_COMPAT_DIR}/unicode)

aux_source_directory(. SQLITE_SOURCE)
aux_source_directory(./extra EXTRA)

add_library(sqliteX SHARED ${SQLITE_SOURCE} ${EXTRA})

find_library(liblog log)
find_library(libdl dl)
#find_library(libsupc supc++)

target_link_libraries(sqliteX icucompat ${liblog} ${libdl})
#icuuc icui18n
#icucompat
#${libsupc}
#${ICU_LIB}/fts.a ${ICU_LIB}/icucompat.a