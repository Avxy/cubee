package com.lacina.cubeeclient.controllers;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.support.annotation.NonNull;
import android.util.Log;

import com.android.volley.VolleyError;
import com.facebook.login.LoginFragment;
import com.facebook.login.LoginManager;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;

import java.util.HashMap;
import java.util.Map;

/**
 * User controller singleton for control User related operations.
 **/
@SuppressWarnings("ALL")
public class UserController {


    private static UserController instance;

    private final String TAG = "UserControll";
    /**
     * Field to reference this activity
     */
    private Activity activity;

    @SuppressWarnings("unused")
    private Context context;

    private UserController() {

    }


    public static synchronized UserController getInstance() {
        if (instance == null) {
            instance = new UserController();
        }
        return instance;
    }

    @SuppressWarnings("unused")
    public void sendRegistration(String idToken, Activity activity) {
        this.activity = activity;
        Map<String, String> params = new HashMap<>();
        params.put("idToken", idToken);
        PostJsonObjectRequest registerFbUser = new PostJsonObjectRequest(newOnPostUserCallback(), AppConfig.getInstance().registerFbUser());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(registerFbUser.getRequest(params));
    }

    @NonNull
    private OnPostJsonObjectCallback newOnPostUserCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                LoginManager loginManager = LoginManager.getInstance();

                if (loginManager != null) {
                    LoginManager.getInstance().logOut();
                }

                FirebaseAuth.getInstance().signOut();
                Intent intent = new Intent(activity.getApplicationContext(), LoginFragment.class);
                intent.putExtra("KEY_ERROR", "Autentication error");
                activity.startActivity(intent);
                activity.finish();
            }
        };
    }

    @SuppressWarnings("unused")
    public boolean checkToken() {
        final FirebaseUser mUser = FirebaseAuth.getInstance().getCurrentUser();
        final boolean[] result = {false};
        if (mUser != null) {
            mUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                result[0] = true;
                            }
                        }
                    });
        }
        return result[0];

    }


    @SuppressWarnings("unused")
    public boolean checkFacebookProvider() {
        final FirebaseUser firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        boolean result = false;
        if (firebaseUser != null) {
            result = ((firebaseUser.getProviders() != null && firebaseUser.getProviders().contains("facebook.com")));
            Log.i(TAG, "Providers: " + String.valueOf(firebaseUser.getProviders()));
        }
        return result;

    }
}
