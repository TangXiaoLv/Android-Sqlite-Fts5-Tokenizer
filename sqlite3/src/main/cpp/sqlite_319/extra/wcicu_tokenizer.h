/**************************************************************************
** Start of ascii tokenizer implementation.
*/

/*
** For tokenizers with no "unicode" modifier, the set of token characters
** is the same as the set of ASCII range alphanumeric characters. 
*/
#ifndef ZHCN_TOKENIZER_H
#define ZHCN_TOKENIZER_H

#include "sqlite3.h"

#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif

EXTERN_C_BEGIN

/*
** Create an "wcicu" tokenizer.
**
** 注册分词器初始化时回调一次，insert操作不回调，每次query时回调
*/
int fts5WcicuCreate(
        void *pCtx,//外部传的上下文
        const char **azArg,//外部传进来的参数tokenize='txl a 1 b 2',*azArg =[a,1,b,2];
        int nArg,//参数+参数值个数
        Fts5Tokenizer **ppOut //输出给ftsTxlTokenize的对象
);

/*
** Delete a "wcicu" tokenizer.
**
** 删除实例缓存
*/
void fts5WcicuDelete(Fts5Tokenizer *pTok);

/*
** Tokenize some text using the "wcicu" tokenizer.
**
** 当SQL类型是下列四种类型时,触发回调
** FTS5_TOKENIZE_QUERY     0x0001 select
** FTS5_TOKENIZE_PREFIX    0x0002
** FTS5_TOKENIZE_DOCUMENT  0x0004 insert
** FTS5_TOKENIZE_AUX       0x0008
*/
int fts5WcicuTokenize(
        Fts5Tokenizer *pTokenizer,//ftsTxlCreate中传来的对象
        void *pCtx,//外部传入的上下文,任意类型,可能用于数据传递
        int flags,//上述4个属性之一
        const char *pText,//当前插入列的文本
        int nText,//文本大小,每个中文3字节,英文1字节
        int (*xToken)(//插入token->字符串的映射
                void *pCtx,//上下文
                int tflags, //一般为0(准确)或上述值
                const char *pToken,//当前串中所对应的索引token(通常为整个串中的子串)
                int nToken, //pToken的字节大小
                int iStart, //pToken所在串中对应的起始位置
                int iEnd)//pToken所在串的末尾位置
);

EXTERN_C_END
#endif //ZHCN_TOKENIZER_H