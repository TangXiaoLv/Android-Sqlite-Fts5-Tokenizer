package org.sqlite.customsqlitetest.benchmark;

import android.content.ContentValues;
import android.database.Cursor;
import android.os.Bundle;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.sqlite.customsqlitetest.R;
import org.sqlite.database.sqlite.SQLiteDatabase;

import java.io.File;

public class BenchmarkActivity extends AppCompatActivity {

    private static final String TAG = "BenchmarkActivity";
    private android.widget.EditText insert;
    private android.widget.EditText content;
    private android.widget.TextView result;
    private File dbFile;
    private SQLiteDatabase db;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_benchmark);
        this.result = (TextView) findViewById(R.id.result);
        this.content = (EditText) findViewById(R.id.content);
        this.insert = (EditText) findViewById(R.id.insert);

        dbFile = getApplication().getDatabasePath("test3.db");
        dbFile.getParentFile().mkdir();
    }

    static {
        System.loadLibrary("sqliteX");
    }

    public void initTable(View view) {
        if (db == null) {
            db = SQLiteDatabase.openOrCreateDatabase(dbFile, null);
            try {
                String nofts = "CREATE TABLE IF NOT EXISTS message(msg TEXT);";
                db.execSQL(nofts);
                String fts = "CREATE VIRTUAL TABLE IF NOT EXISTS ftsmessage USING fts5(msg,tokenize='wcicu zh_CN');";
                db.execSQL(fts);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    public void insert(View view) {
        if (db == null) return;
        String s = insert.getText().toString();
        int howMach = 1;
        try {
            howMach = Integer.parseInt(s);
        } catch (Exception e) {
            //igone
            return;
        }

        int r = 10000 * howMach;
        ContentValues values = new ContentValues();
        db.beginTransaction();
        for (int i = 1; i <= r; i++) {
            values.clear();
            String msg = StringUtil.randomString();
            values.put("msg", msg);
            long ftsmessage = db.insert("ftsmessage", null, values);
            long message = db.insert("message", null, values);
            Log.i(TAG, i + "/" + r + ": " + msg);
        }
        db.setTransactionSuccessful();
        db.endTransaction();
    }

    public void search(View view) {
        if (db == null) return;

        result.setText("");
        String text = content.getText().toString();
        if (!TextUtils.isEmpty(text)) {
            Cursor query = db.rawQuery("SELECT * FROM message WHERE msg LIKE '%" + text + "%'", null);
            StringBuilder b = new StringBuilder("Count:");
            long start = System.currentTimeMillis();
            int count = query.getCount();
            long end = System.currentTimeMillis() - start;
            b.append(count).append("\n")
                    .append("time:").append(end).append("ms");
            result.setText(b.toString());
            query.close();
        }
    }

    public void ftsSearch(View view) {
        if (db == null) return;

        result.setText("");
        String text = content.getText().toString();
        if (!TextUtils.isEmpty(text)) {
            Cursor query = db.rawQuery("SELECT * FROM ftsmessage WHERE ftsmessage MATCH ?", new String[]{text});
            StringBuilder b = new StringBuilder("Count:");
            long start = System.currentTimeMillis();
            int count = query.getCount();
            long end = System.currentTimeMillis() - start;
            b.append(count).append("\n").append("time:").append(end).append("ms");
            result.setText(b.toString());
            query.close();
        }
    }
}