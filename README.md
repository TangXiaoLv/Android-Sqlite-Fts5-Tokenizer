# Android-Sqlite-Fts5

Sqlite3全文检索是本地搜索引擎，用来搜索大数据量(几十万条聊天数据)，文本数据再适合不过。

源码配置
---
Sqlite3源码：[3.19.3](http://sqlite.org/android/info/a7c884060e3c562e)

fts5分词器：移植于微信的[fts4分词器](https://github.com/Tencent/wcdb/tree/master/fts)并对搜索精度做了调优。

使用
---
[Fts5官方使用教程](https://sqlite.org/fts5.html)
```
//加载库
System.loadLibrary("sqliteX");

//创建虚拟表
"CREATE VIRTUAL TABLE IF NOT EXISTS message USING fts5(msg,tokenize='wcicu zh_CN');"
```

编译
---
使用CMake脚本编译，方便调试及跨平台。

编译选项[Compile-time Options](http://www.sqlite.org/compile.html)

