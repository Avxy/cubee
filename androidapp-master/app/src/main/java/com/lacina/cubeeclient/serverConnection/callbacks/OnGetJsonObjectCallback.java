package com.lacina.cubeeclient.serverConnection.callbacks;

import com.android.volley.VolleyError;
import com.google.gson.JsonObject;


public interface OnGetJsonObjectCallback {
    void onGetJsonObjectCallbackSuccess(JsonObject jsonObject);
    void onGetJsonObjectCallbackError(VolleyError volleyError);
}
