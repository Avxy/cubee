package com.lacina.cubeeclient.serverConnection.requests;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.RetryPolicy;
import com.android.volley.VolleyError;
import com.google.gson.JsonArray;
import com.google.gson.reflect.TypeToken;
import com.lacina.cubeeclient.serverConnection.GsonReflectionRequest;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;

import java.lang.reflect.Type;
import java.util.Map;

/**
 * Request to get a json with all cubees
 */
public class GetJsonArrayRequest {

    private final OnGetJsonArrayCallback callback;

    private final String url;

    public GetJsonArrayRequest(OnGetJsonArrayCallback callback, String url) {

        this.callback = callback;
        this.url = url;
    }

    /**
     * @param headers map with idToken, the id form the user
     */
    public Request getRequest(Map<String, String> headers) {
        //Create request, set URL, Params, and callbacks
        Type type = new TypeToken<JsonArray>() {
        }.getType();
        GsonReflectionRequest<JsonArray> request = new GsonReflectionRequest<>(url, type, headers, new Response.Listener<JsonArray>() {
            @Override
            public void onResponse(JsonArray response) {
                JsonArray jsonArray = response.getAsJsonArray();
                callback.onGetJsonArrayCallbackSuccess(jsonArray);
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                error.printStackTrace();
                callback.onGetJsonArrayCallbackError(error);
            }
        });


        //SetRequestPolicy
        request.setRetryPolicy(new RetryPolicy() {
            @Override
            public int getCurrentTimeout() {
                return 3000;
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