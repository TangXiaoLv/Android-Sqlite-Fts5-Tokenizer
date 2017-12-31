/**************************************************************************
** Start of ascii tokenizer implementation.
*/

/*
** For tokenizers with no "unicode" modifier, the set of token characters
** is the same as the set of ASCII range alphanumeric characters. 
*/
#ifndef ASCII_TOKENIZER_H
#define ASCII_TOKENIZER_H

#include "../sqlite3.h"

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

EXTERN_C_BEGIN

/*
** Delete a "ascii" tokenizer.
*/
void fts5AsciiDelete(Fts5Tokenizer *p);

/*
** Create an "ascii" tokenizer.
*/
int fts5AsciiCreate(
        void *pUnused,
        const char **azArg, int nArg,
        Fts5Tokenizer **ppOut
);


/*
** Tokenize some text using the ascii tokenizer.
*/
int fts5AsciiTokenize(
        Fts5Tokenizer *pTokenizer,
        void *pCtx,
        int iUnused,
        const char *pText, int nText,
        int (*xToken)(void *pCtx, int tflags, const char *pToken, int nToken, int iStart, int iEnd)
);

/**************************************************************************/

EXTERN_C_END
#endif //ASCII_TOKENIZER_H