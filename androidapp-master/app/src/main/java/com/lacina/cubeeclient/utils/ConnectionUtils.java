package com.lacina.cubeeclient.utils;

import android.app.Activity;
import android.content.Intent;
import android.util.Log;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.facebook.login.LoginManager;
import com.google.firebase.auth.FirebaseAuth;
import com.lacina.cubeeclient.activities.LoginAndBLETabsActivity;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetCheckConnectionServerCallback;

/**
 * Class with utility connection methods
 **/
public abstract class ConnectionUtils {

    /**
     * Check id the connection with server is on
     * @param activity                Activity where the request happens
     * @param onCheckConnectionServer  Callback used to send a response
     */
    public static void checkServerConnection(Activity activity, final OnGetCheckConnectionServerCallback onCheckConnectionServer) {

        String url = AppConfig.getInstance().host();

        StringRequest checkServer = new StringRequest(Request.Method.GET, url, new Response.Listener<String>() {
            @Override
            public void onResponse(String response) {
                onCheckConnectionServer.onGetCheckConnectionServerResponse(true);


            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                Log.d("VOLEYERRO", (error.getMessage()== null) ? "Error message empty": error.getMessage());
                error.printStackTrace();
                onCheckConnectionServer.onGetCheckConnectionServerResponse(false);
            }
        });
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(checkServer);

    }

    /**
     * Logout the user
     */
    public static void logout(Activity activity) {
        LoginManager loginManager = LoginManager.getInstance();

        if (loginManager != null) {
            LoginManager.getInstance().logOut();
        }

        FirebaseAuth.getInstance().signOut();
        Intent intent = new Intent(activity, LoginAndBLETabsActivity.class);
        ActivityUtils.showToast(activity, "Usuário deslogado com sucesso.");
        activity.startActivity(intent);
        activity.finish();
    }
}
