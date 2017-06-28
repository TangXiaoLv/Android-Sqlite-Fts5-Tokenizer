package org.sqlite.database.extra;

public class Enhance {

    public static native String initFtsTokenizer(String dbPath);

    public static native void insert(String text);

    public static native void query(String text);
}
