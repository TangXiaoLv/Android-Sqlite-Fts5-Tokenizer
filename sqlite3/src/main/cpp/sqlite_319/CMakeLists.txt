cmake_minimum_required(VERSION 3.4.1)
project(sqliteX C CXX)

# If using SEE, uncomment the following:
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSQLITE_HAS_CODEC")

#Define HAVE_USLEEP, otherwise ALL sleep() calls take at least 1000ms
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_USLEEP=1")

# Enable SQLite extensions.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
-DSQLITE_ENABLE_FTS3 \
-DSQLITE_ENABLE_FTS3_TOKENIZER \
-DSQLITE_ENABLE_FTS3_BACKWARDS \
-DSQLITE_ENABLE_FTS4 \
-DSQLITE_ENABLE_FTS5 \
-DSQLITE_ENABLE_RTREE \
-DSQLITE_ENABLE_JSON1")

# This is important - it causes SQLite to use memory for temp files. Since
# Android has no globally writable temp directory, if this is not defined the
# application throws an exception when it tries to create a temp file.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
-DSQLITE_TEMP_STORE=3 \
-DHAVE_CONFIG_H -DKHTML_NO_EXCEPTIONS -DGKWQ_NO_JAVA \
-DNO_SUPPORT_JS_BINDING -DQT_NO_WHEELEVENT -DKHTML_NO_XBL \
-U__APPLE__ \
-DHAVE_STRCHRNUL=0 \
-DSQLITE_USE_URI=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
-Wno-unused-parameter \
-Wno-int-to-pointer-cast \
-Wno-uninitialized \
-Wno-parentheses")

#WC
set(WC_SQLITE_FLAGS "${CMAKE_C_FLAGS} \
-Wall -Werror -ffunction-sections -fdata-sections -Wno-unused \
-DSQLITE_CORE \
-DSQLITE_OS_UNIX \
-DSQLITE_ENABLE_MEMORY_MANAGEMENT=1 \
-DSQLITE_HAVE_ISNAN \
-DSQLITE_DEFAULT_FILE_FORMAT=4 \
-DSQLITE_THREADSAFE=2 \
-DSQLITE_ENABLE_SESSION \
-DSQLITE_ENABLE_PREUPDATE_HOOK \
-DSQLITE_DEFAULT_WORKER_THREADS=2 \
-DSQLITE_DEFAULT_JOURNAL_SIZE_LIMIT=1048576 \
-DUSE_PREAD64=1 \
-DSQLITE_ENABLE_STAT4 \
-DSQLITE_ENABLE_EXPLAIN_COMMENTS \
-DOMIT_MEMLOCK \
-DOMIT_CONSTTIME_MEM \
-DSQLCIPHER_CRYPTO_OPENSSL \
-DSQLCIPHER_CRYPTO_CUSTOM \
-DSQLCIPHER_CRYPTO_XXTEA \
-DSQLCIPHER_CRYPTO_DEVLOCK")

#Andorid 7.1.1
set(ANDROID_SQLITE_FLAGS "${WC_SQLITE_FLAGS} \
-Dfdatasync=fdatasync \
-DSQLITE_POWERSAFE_OVERWRITE=1 \
-DSQLITE_OMIT_COMPILEOPTION_DIAGS \
-DSQLITE_OMIT_LOAD_EXTENSION \
-DSQLITE_DEFAULT_FILE_PERMISSIONS=0600 \
-DPIC -fPIC")
#-DSQLITE_UNTESTABLE 禁止调试 / 发布时加上
#-DSQLITE_DEFAULT_AUTOVACUUM=1 打开自动剪裁

set(CMAKE_C_FLAGS "${ANDROID_SQLITE_FLAGS}")

#C++
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-conversion-null")

#release
if (${CMAKE_BUILD_TYPE} STREQUAL "Release")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DSQLITE_UNTESTABLE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DSQLITE_UNTESTABLE")
    message(build type : Release)
endif ()

if (${CMAKE_ANDROID_ARCH_ABI} STREQUAL "arm")
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