package com.gotoubun;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Build;
import android.provider.Settings;

public class Launcher {

    static {
        System.loadLibrary("nino");
    }

    private static SharedPreferences m_Prefs;
    public static void Init(Object object) {
        Context m_Context = (Context) object;
        Activity m_Activity = (Activity) object;
        Init(m_Context);
        if (Build.VERSION.SDK_INT >= 23) {
            if (!Settings.canDrawOverlays(m_Context)) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION, Uri.parse("package:" + m_Context.getPackageName()));
                m_Activity.startActivity(intent);
            }
        }
            Intent i = new Intent(m_Context.getApplicationContext(), Floating.class);
            m_Context.startService(i);
        }
    private static native void Init(Context mContext);
}
