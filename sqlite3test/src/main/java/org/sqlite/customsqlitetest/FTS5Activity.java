package org.sqlite.customsqlitetest;

import android.app.Activity;
import android.content.ContentValues;
import android.database.Cursor;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.sqlite.database.sqlite.SQLiteDatabase;

import java.io.File;

public class FTS5Activity extends Activity {

    private EditText search;
    private EditText insert;
    private TextView showing;
    private SQLiteDatabase db;
    private File dbFile;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_fts5);
        this.showing = (TextView) findViewById(R.id.result);
        this.search = (EditText) findViewById(R.id.content);
        this.insert = (EditText) findViewById(R.id.insert);

        load();
        dbFile = getApplicationContext().getDatabasePath("test2.db");
        dbFile.getParentFile().mkdirs();
    }

    public void search(View view) {
        if (db == null) return;

        showing.setText("");
        String text = search.getText().toString();
        if (!TextUtils.isEmpty(text)) {
            Cursor query = db.rawQuery("SELECT * FROM message WHERE message MATCH ?", new String[]{text});
            StringBuilder b = new StringBuilder("Count:");
            b.append(query.getCount()).append("\n");
            while (query.moveToNext()) {
                String s1 = query.getString(0);
                b.append("content:").append(s1).append("\n");
            }
            showing.setText(b.toString());
            query.close();
        }
    }

    public void create(View view) {
        if (db == null) {
            db = SQLiteDatabase.openOrCreateDatabase(dbFile, null);
            try {
                String a = "CREATE VIRTUAL TABLE IF NOT EXISTS message USING fts5(msg,tokenize='wcicu zh_CN');";
                db.execSQL(a);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private void load() {
        try {
            /*System.loadLibrary("icuuc");
            System.loadLibrary("icui18n");*/
            System.loadLibrary("sqliteX");
        } catch (Exception e) {
            //igoned
        }
    }

    public void insert(View view) {
        if (db == null) return;

        String text = insert.getText().toString();
        ContentValues values = new ContentValues();
        values.put("msg", text);
        db.insert("message", null, values);
    }
}
