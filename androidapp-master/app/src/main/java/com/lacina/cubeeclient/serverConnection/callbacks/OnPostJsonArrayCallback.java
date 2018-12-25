package com.lacina.cubeeclient.serverConnection.callbacks;


import com.android.volley.VolleyError;
import com.google.gson.JsonArray;


@SuppressWarnings("ALL")
public interface OnPostJsonArrayCallback {
    void onPostJsonArrayCallbackSucess(JsonArray jsonArray);
    void onPostJsonArrayCallbackError(VolleyError message);

}
