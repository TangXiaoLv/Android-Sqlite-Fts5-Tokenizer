#include <string.h>
#include "jni.h"
#include "test.h"
#include "icucompat.h"
#include "wcicu_tokenizer.h"
#include "wcicu_utils.h"

#define LOG_TAG "Test"

#include "ALog-priv.h"

static int init_fts5_tokenizer(const char *filename);

static sqlite3 *db = NULL;
static sqlite3_stmt *stmt_insert = NULL;

JNIEXPORT jstring JNICALL
Java_org_sqlite_database_extra_Enhance_initFtsTokenizer(JNIEnv *env, jobject jobj, jstring jpath) {
    sqlite3_soft_heap_limit(8 * 1024 * 1024);
    // Initialize SQLite.
    sqlite3_initialize();

    const char *path = env->GetStringUTFChars(jpath, NULL);
    ALOGE("%s", path);
    // Load and initialize ICU library.
    if (init_icucompat() != 0) {
        ALOGE("failed to load ICU library.");
        return NULL;
    }

    int code = init_fts5_tokenizer(path);

    const char *sql = "CREATE VIRTUAL TABLE IF NOT EXISTS email USING fts5(title, keywords, body,tokenize='wcicu zh_CN');";
    code = sqlite3_exec(db, sql, NULL, NULL, NULL);
    if (SQLITE_OK != code) {
        ALOGE("Can't create virtual table email, %s", sqlite3_errmsg(db));

        sqlite3_close(db);
        return env->NewStringUTF("" + code);
    }

    // Register utility functions.
    if (sqlite3_register_mm_utils(db) != SQLITE_OK) {
        ALOGE("failed to register mm utils.");
        return NULL;
    }

    env->ReleaseStringUTFChars(jpath, path);
    return jpath;
}

JNIEXPORT jstring JNICALL
Java_org_sqlite_database_extra_Enhance_insert(JNIEnv *env, jobject jobj, jstring jtext) {
    if (NULL == stmt_insert) {
        const char *zSql = "INSERT INTO email(title,keywords,body) VALUES(?,?,?);";
        int rc = sqlite3_prepare(db, zSql, strlen(zSql), &stmt_insert, 0);
        if (SQLITE_OK != rc) {
            ALOGE("Can't prepare stmt_insert, %s", sqlite3_errmsg(db));
            stmt_insert = NULL;
            return NULL;
        }
    }

    const char *text = env->GetStringUTFChars(jtext, NULL);
    sqlite3_bind_text(stmt_insert, 1, text, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 2, "很", -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 3, "a b c def", -1, SQLITE_STATIC);

    sqlite3_step(stmt_insert);
    sqlite3_reset(stmt_insert);

    env->ReleaseStringUTFChars(jtext, text);
    return NULL;

}

JNIEXPORT jstring JNICALL
Java_org_sqlite_database_extra_Enhance_query(JNIEnv *env, jobject jobj, jstring jtext) {
    sqlite3_stmt *stmt_query = NULL;
    const char *zSql = "SELECT * FROM email WHERE email MATCH ?;";
    int rc = sqlite3_prepare(db, zSql, strlen(zSql), &stmt_query, 0);
    if (SQLITE_OK != rc) {
        ALOGE("Can't prepare stmt_insert, %s", sqlite3_errmsg(db));
        stmt_insert = NULL;
        return NULL;
    }

    const char *text = env->GetStringUTFChars(jtext, NULL);
    sqlite3_bind_text(stmt_query, 1, text, -1, SQLITE_STATIC);
    int ncols = sqlite3_column_count(stmt_query);

    if (ncols != 3) {
        ALOGW("FTS queryFTSEntities: ncols != 3");
        return NULL;
    }
    rc = sqlite3_step(stmt_query);
    if (rc == SQLITE_DONE) {
        ALOGI("queryFTSEntities none");
        return NULL;
    }
    else if (rc != SQLITE_ROW) {
        ALOGE("Can't queryFTSEntities, %s", sqlite3_errmsg(db));
        return NULL;
    }

    while (rc == SQLITE_ROW) {
        //title,keywords,body
        const unsigned char *title = sqlite3_column_text(stmt_query, 0);
        const unsigned char *keywords = sqlite3_column_text(stmt_query, 1);
        const unsigned char *body = sqlite3_column_text(stmt_query, 2);
        ALOGI("%s:%s:%s", title, keywords, body);
        rc = sqlite3_step(stmt_query);
    }

    if (stmt_query != NULL) {
        sqlite3_reset(stmt_query);
        sqlite3_finalize(stmt_query);
        stmt_query = NULL;
    }

    env->ReleaseStringUTFChars(jtext, text);
    return NULL;
}

fts5_api *fts5_api_from_db(sqlite3 *db);

static int init_fts5_tokenizer(const char *filename) {
    //打开db
    int code = sqlite3_open(filename, &db);
    if (SQLITE_OK != code) {
        //打开db错误
        ALOGE("Can't open database %s, %s", filename, sqlite3_errmsg(db));
        sqlite3_close(db);
        return code;
    }

    //拿到FTS5的api指针
    fts5_api *pFts5Api = fts5_api_from_db(db);

    //构造分词器
    fts5_tokenizer tokenizer = {fts5WcicuCreate, fts5WcicuDelete, fts5WcicuTokenize};
    //注册分词器
    pFts5Api->xCreateTokenizer(pFts5Api, "wcicu", (void *) pFts5Api, &tokenizer, 0);

    return SQLITE_OK;
}

/*
** Return a pointer to the fts5_api pointer for database connection db.
** If an error occurs, return NULL and leave an error in the database
** handle (accessible using sqlite3_errcode()/errmsg()).
*/
fts5_api *fts5_api_from_db(sqlite3 *db) {
    fts5_api *pRet = 0;
    sqlite3_stmt *pStmt = 0;

    if (SQLITE_OK == sqlite3_prepare(db, "SELECT fts5()", -1, &pStmt, 0)
        && SQLITE_ROW == sqlite3_step(pStmt)
        && sizeof(pRet) == sqlite3_column_bytes(pStmt, 0)
            ) {
        memcpy(&pRet, sqlite3_column_blob(pStmt, 0), sizeof(pRet));
    }
    sqlite3_finalize(pStmt);
    return pRet;
}