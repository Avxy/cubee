package com.lacina.cubeeclient.serverConnection.requests;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.RetryPolicy;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonObjectRequest;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;

import org.json.JSONObject;

import java.util.Map;

/**
 * Post a cubee registration request
 */
public class PostJsonObjectRequest {

    private final OnPostJsonObjectCallback callback;

    private final String url;

    public PostJsonObjectRequest(OnPostJsonObjectCallback callback, String url) {
        this.url = url;
        this.callback = callback;
    }

    public Request getRequest(Map<String, String> params) {
        //Create request, set URL, Params, and callbacks
        Request request = new JsonObjectRequest(Request.Method.POST, url, new JSONObject(params), new Response.Listener<JSONObject>() {
            @Override
            public void onResponse(JSONObject response) {
                Gson gson = new Gson();
                JsonObject responseInJsonObject = gson.fromJson(response.toString(), JsonObject.class);
                callback.onPostJsonObjectCallbackSuccess(responseInJsonObject);
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                error.printStackTrace();
                callback.onPostJsonObjectCallbackError(error);
            }

        });

        //SetRequestPolicy
        request.setRetryPolicy(new RetryPolicy() {
            @Override
            public int getCurrentTimeout() {
                return 0;
            }

            @Override
            public int getCurrentRetryCount() {
                return 0;
            }

            @Override
            public void retry(VolleyError error) throws VolleyError {

            }
        });

        return request;
    }


}

