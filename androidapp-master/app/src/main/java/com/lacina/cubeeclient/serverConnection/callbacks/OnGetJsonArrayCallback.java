package com.lacina.cubeeclient.serverConnection.callbacks;

import com.android.volley.VolleyError;
import com.google.gson.JsonArray;


public interface OnGetJsonArrayCallback {
    void onGetJsonArrayCallbackSuccess(JsonArray jsonArray);
    void onGetJsonArrayCallbackError(VolleyError volleyError);

}
