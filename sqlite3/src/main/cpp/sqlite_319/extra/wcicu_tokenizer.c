/*
** 中文分词器
*/
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "wcicu_tokenizer.h"
#include "icucompat.h"
#include "wcicu_utils.h"

#define LOG_TAG "wcicu"

#include "ALog-priv.h"

#ifdef _WIN32
#include <malloc.h>
#endif

#define UN_OUTPUT_TOKEN -1
#define ROUND4(n) (((n) + 3) & ~3)

typedef struct mm_tokenizer_t {
    char locale[16];
} mm_tokenizer_t;

typedef struct mm_cursor_t {
    UBreakIterator *iter;   // UBreakIterator for the text.
    UChar *in_buffer;       // whole input text buffer, in UTF-16,
    // allocated altogather with mm_cursor_t.
    int *in_offset;
    int in_length;          // input text length.

    char *out_buffer;       // output token buffer, int UTF-8,
    // allocated in mmtok_next.
    int out_length;         // output token buffer length.
    int token_count;

    int32_t ideo_start; // ideographic unary/binary tokenizing cursor.
    int32_t ideo_end;   // ideographic unary/binary tokenizing end point.
    int ideo_state;     // 0 for unary output, -1 for invalid status.
} mm_cursor_t;
#define MINIMAL_OUT_BUFFER_LENGTH 512

static
int find_splited_ideo_token(mm_cursor_t *cur, int32_t *start, int32_t *end);

static
int output_token(mm_cursor_t *cur,
                 int32_t start,
                 int32_t end,
                 const char **ppToken,
                 int *pnBytes,
                 int *piStartOffset,
                 int *piEndOffset,
                 int *piPosition);

static
void trim(char *str);

int fts5WcicuCreate(void *pCtx, const char **azArg, int nArg, Fts5Tokenizer **ppOut) {
    mm_tokenizer_t *tok = sqlite3_malloc(sizeof(mm_tokenizer_t));
    if (!tok)
        return SQLITE_NOMEM;

    if (nArg > 0) {
        strncpy(tok->locale, azArg[0], 15);
        tok->locale[15] = 0;
    } else
        tok->locale[0] = 0;

    *ppOut = (Fts5Tokenizer *) tok;
    return SQLITE_OK;
}

void fts5WcicuDelete(Fts5Tokenizer *pTok) {
    sqlite3_free(pTok);
}

/*printer used in test*/
void printEachForward(UBreakIterator *, UChar *);

int fts5WcicuTokenize(
        Fts5Tokenizer *pTokenizer,
        void *pCtx,
        int flags,
        const char *pText, int nText,
        int (*xToken)(void *pCtx, int tflags, const char *pToken, int nToken, int iStart, int iEnd)
) {
    //trim string
    char dText[nText + 1];
    strcpy(dText, pText);
    trim(dText);
    nText = strlen(dText);

    //open
    mm_tokenizer_t *tok = (mm_tokenizer_t *) pTokenizer;
    mm_cursor_t *cur = NULL;

    UErrorCode status = U_ZERO_ERROR;
    int32_t dst_len = 0;
    UChar32 c = 0;

    if (nText < 0)
        nText = strlen(dText);

    dst_len = ROUND4(nText + 1);
    cur = (mm_cursor_t *) sqlite3_malloc(
            sizeof(mm_cursor_t)
            + dst_len * sizeof(UChar) // in_buffer
            + (dst_len + 1) * sizeof(int)// in_offset
    );

    if (!cur)
        return SQLITE_NOMEM;

    memset(cur, 0, sizeof(mm_cursor_t));
    cur->in_buffer = (UChar *) &cur[1];
    cur->in_offset = (int *) &cur->in_buffer[dst_len];
    cur->out_buffer = NULL;
    cur->out_length = 0;
    cur->token_count = 0;
    cur->ideo_start = -1;
    cur->ideo_end = -1;
    cur->ideo_state = -1;

    int i_input = 0;
    int i_output = 0;
    cur->in_offset[i_output] = i_input;

    for (; ;) {
        if (i_input >= nText)
            break;

        U8_NEXT(dText, i_input, nText, c);
        if (!c)
            break;
        if (c < 0)
            c = ' ';

        int is_error = 0;
        U16_APPEND(cur->in_buffer, i_output, dst_len, c, is_error);
        if (is_error) {
            sqlite3_free(cur);
            sqlite3_mm_set_last_error(
                    "Writing UTF-16 character failed. Code point: 0x%x", c);
            return SQLITE_ERROR;
        }
        cur->in_offset[i_output] = i_input;
    }

    //break by 'word'
    cur->iter = ubrk_open(UBRK_CHARACTER, tok->locale, cur->in_buffer, i_output, &status);

    if (U_FAILURE(status)) {
        sqlite3_mm_set_last_error(
                "Open UBreakIterator failed. ICU error code: %d",
                status);
        return SQLITE_ERROR;
    }
    cur->in_length = i_output;

    ubrk_first(cur->iter);

    //next
    int32_t start = 0, end = 0;
    int32_t token_type = 0;

    const char *pToken = "";
    int nBytes = 0;
    int iStartOffset = 0;
    int iPosition = 0;
    int iEndOffset = 0;

    int rc = SQLITE_OK;
    while (iEndOffset < nText) {
        int output_token_flag = UN_OUTPUT_TOKEN;
        // process pending ideographic token.
        if (find_splited_ideo_token(cur, &start, &end)) {
            output_token_flag = output_token(cur, start, end, &pToken, &nBytes, &iStartOffset,
                                             &iEndOffset, &iPosition);
        }

        // if not output_token
        if (UN_OUTPUT_TOKEN == output_token_flag) {
            start = ubrk_current(cur->iter);

            // find first non-NONE token.
            for (; ;) {
                end = ubrk_next(cur->iter);
                if (end == UBRK_DONE) {
                    sqlite3_mm_clear_error();
                    rc = SQLITE_DONE;
                    goto end_to_return;
                }

                token_type = ubrk_getRuleStatus(cur->iter);
                if (token_type >= UBRK_WORD_NONE && token_type < UBRK_WORD_NONE_LIMIT) {
                    // look at the first character, if it's a space or ZWSP, ignore this token.
                    // also ignore '*' because sqlite parser uses it as prefix operator.
                    UChar32 c = cur->in_buffer[start];
                    if (c == '*' || c == 0x200b || u_isspace(c)) {
                        start = end;
                        continue;
                    }
                }

                break;
            }

            // for non-IDEO tokens, just return.
            if (token_type < UBRK_WORD_IDEO || token_type >= UBRK_WORD_IDEO_LIMIT)
                output_token_flag = output_token(cur, start, end, &pToken, &nBytes, &iStartOffset,
                                                 &iEndOffset, &iPosition);

        }

        // if not output_token
        if (UN_OUTPUT_TOKEN == output_token_flag) {
            // for IDEO tokens, find all suffix ideo tokens.
            for (; ;) {
                int32_t e = ubrk_next(cur->iter);
                if (e == UBRK_DONE)
                    break;
                token_type = ubrk_getRuleStatus(cur->iter);
                if (token_type < UBRK_WORD_IDEO || token_type >= UBRK_WORD_IDEO_LIMIT)
                    break;
                end = e;
            }
            ubrk_isBoundary(cur->iter, end);

            cur->ideo_start = start;
            cur->ideo_end = end;
            cur->ideo_state = 0;
            if (find_splited_ideo_token(cur, &start, &end))
                output_token(cur, start, end, &pToken, &nBytes, &iStartOffset, &iEndOffset,
                             &iPosition);
        }

        rc = xToken(pCtx, 0, pToken, nBytes, iStartOffset, iEndOffset);
    }

    goto end_to_return;
end_to_return:
    if (rc == SQLITE_DONE) rc = SQLITE_OK;
    //close
    ubrk_close(cur->iter);
    if (cur->out_buffer)
        sqlite3_free(cur->out_buffer);
    sqlite3_free(cur);
    return rc;
}

static int find_splited_ideo_token(mm_cursor_t *cur, int32_t *start, int32_t *end) {
    int32_t s = 0, e = 0;
    UChar32 c = 0;

    if (cur->ideo_state < 0)
        return 0;

    if (cur->ideo_start == cur->ideo_end) {
        cur->ideo_state = -1;
        return 0;
    }

    // check UTF-16 surrogates, output 2 UChars if it's a lead surrogates, otherwise 1.
    s = cur->ideo_start;
    e = s + 1;
    c = cur->in_buffer[s];
    if (U16_IS_LEAD(c) && cur->ideo_end - s >= 2)
        e++;

    *start = s;
    *end = e;
    cur->ideo_start = e;
    return 1;
}

static char *generate_token_printable_code(const UChar *buf, int32_t length) {
    char *out = (char *) malloc(length * 5 + 1);
    char *pc = out;
    if (!out)
        return "";

    while (length > 0) {
        snprintf(pc, 6, "%04hX ", *buf);
        length--;
        buf++;
        pc += 5;
    }

    return out;
}

static int output_token(mm_cursor_t *cur,
                        int32_t start,
                        int32_t end,
                        const char **ppToken,
                        int *pnBytes,
                        int *piStartOffset,
                        int *piEndOffset,
                        int *piPosition) {
    UChar buf1[256];
    UChar buf2[256];
    UErrorCode status = U_ZERO_ERROR;
    int32_t result = 0;
    int32_t length = 0;

    length = end - start;
    if (length > 256)
        length = 256;
    result = unorm_normalize(cur->in_buffer + start, length, UNORM_NFKD, 0,
                             buf1, sizeof(buf1) / sizeof(UChar), &status);
    // currently, only try fixed length buffer, failed if overflowed.
    if (U_FAILURE(status) || result > sizeof(buf1) / sizeof(UChar)) {
        char *seq = generate_token_printable_code(cur->in_buffer + start, length);
        sqlite3_mm_set_last_error(
                "Normalize token failed. ICU status: %d, input: %s",
                status, seq);
        free(seq);
        return SQLITE_ERROR;
    }

    length = result;
    result = u_strFoldCase(buf2, sizeof(buf2) / sizeof(UChar), buf1, length, U_FOLD_CASE_DEFAULT,
                           &status);
    // currently, only try fixed length buffer, failed if overflowed.
    if (U_FAILURE(status) || result > sizeof(buf2) / sizeof(UChar)) {
        char *seq = generate_token_printable_code(buf1, length);
        sqlite3_mm_set_last_error(
                "FoldCase token failed. ICU status: %d, input: %s",
                status, seq);
        free(seq);
        return SQLITE_ERROR;
    }

    if (cur->out_buffer == NULL) {
        cur->out_buffer =
                (char *) sqlite3_malloc(MINIMAL_OUT_BUFFER_LENGTH * sizeof(char));
        if (!cur->out_buffer)
            return SQLITE_NOMEM;
        cur->out_length = MINIMAL_OUT_BUFFER_LENGTH;
    }

    length = result;
    u_strToUTF8(cur->out_buffer, cur->out_length, &result, buf2, length, &status);
    if (result > cur->out_length) {
        char *b = (char *) sqlite3_realloc(cur->out_buffer, result * sizeof(char));
        if (!b)
            return SQLITE_NOMEM;

        cur->out_buffer = b;
        cur->out_length = result;

        status = U_ZERO_ERROR;
        u_strToUTF8(cur->out_buffer, cur->out_length, &result, buf2, length,
                    &status);
    }
    if (U_FAILURE(status) || result > cur->out_length) {
        char *seq = generate_token_printable_code(buf2, length);
        sqlite3_mm_set_last_error(
                "Transform token to UTF-8 failed. ICU status: %d, input: %s",
                status, seq);
        free(seq);
        return SQLITE_ERROR;
    }

    *ppToken = cur->out_buffer;
    *pnBytes = result;
    *piStartOffset = cur->in_offset[start];
    *piEndOffset = cur->in_offset[end];
    *piPosition = ++cur->token_count;

    /*ALOGI("TOKENIZER > %s, %d, %d, %d, %d",
          *ppToken, *pnBytes, *piStartOffset, *piEndOffset, *piPosition);*/
    return SQLITE_OK;
}

void printTextRange(UChar *str, int32_t start, int32_t end) {
    char charBuf[1000];
    UChar savedEndChar;

    savedEndChar = str[end];
    str[end] = 0;
    u_austrncpy(charBuf, str + start, sizeof(charBuf) - 1);
    charBuf[sizeof(charBuf) - 1] = 0;
    ALOGE("string[%2d..%2d] \"%s\"\n", start, end, charBuf);
    str[end] = savedEndChar;
}

void trim(char *str) {
    if (str == NULL || *str == '\0') {
        return;
    }

    char *start;
    char *end;
    size_t len = strlen(str);

    start = str;
    end = str + len - 1;

    while (1) {
        char c = *start;
        if (!u_isspace(c))
            break;

        start++;
        if (start > end) {
            str[0] = '\0';
            return;
        }
    }


    while (1) {
        char c = *end;
        if (!u_isspace(c))
            break;

        end--;
        if (start > end) {
            str[0] = '\0';
            return;
        }
    }

    memmove(str, start, end - start + 1);
    str[end - start + 1] = '\0';
}

/* Print each element in order: */
void printEachForward(UBreakIterator *boundary, UChar *str) {
    int32_t end;
    int32_t start = ubrk_first(boundary);
    for (end = ubrk_next(boundary);
         end != UBRK_DONE;
         start = end, end = ubrk_next(boundary)) {
        printTextRange(str, start, end);
    }
}