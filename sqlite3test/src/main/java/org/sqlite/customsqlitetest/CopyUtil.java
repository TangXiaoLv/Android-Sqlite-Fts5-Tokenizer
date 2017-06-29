package org.sqlite.customsqlitetest;

import android.content.Context;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

public class CopyUtil {

    public static void copy(Context context){
        BufferedInputStream in = null;
        BufferedOutputStream out = null;
        try {
            String filesDir = context.getFilesDir().getAbsolutePath();
            String file = filesDir + "/icudt57l.dat";
            if (!new File(file).exists()) {
                InputStream open = context.getAssets().open("icudt57l.dat");
                in = new BufferedInputStream(open);
                out = new BufferedOutputStream(new FileOutputStream(new File(file)));
                byte[] buf = new byte[1024 * 100];
                while (in.read() != -1) {
                    int len = in.read(buf);
                    out.write(buf, 0, len);
                }

                in.close();
                out.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if (in != null) {
                try {
                    in.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }

            if(out != null){
                try {
                    out.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
    }
}
