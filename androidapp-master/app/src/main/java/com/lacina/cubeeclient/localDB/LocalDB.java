package com.lacina.cubeeclient.localDB;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;


public class LocalDB {

    public static void savePasswordWifi(Activity activity, String SSID, String password){
        SharedPreferences sharedPreferences = activity.getPreferences(Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(SSID, password);
        editor.apply();
    }

    public static String getPasswordWifiIfSaved(Activity activity, String SSID){
        SharedPreferences sharedPreferences = activity.getPreferences(Context.MODE_PRIVATE);
        return sharedPreferences.getString(SSID, "");
    }
//
//    public static void saveNewAlert(Activity activity, String SSID, String password){
//        SharedPreferences sharedPreferences = activity.getPreferences(Context.MODE_PRIVATE);
//        SharedPreferences.Editor editor = sharedPreferences.edit();
//        editor.putString(SSID, password);
//        editor.commit();
//    }

}
