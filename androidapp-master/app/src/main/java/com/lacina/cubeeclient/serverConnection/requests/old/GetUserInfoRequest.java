package com.lacina.cubeeclient.serverConnection.requests.old;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.RetryPolicy;
import com.android.volley.VolleyError;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.google.gson.reflect.TypeToken;
import com.lacina.cubeeclient.model.User;
import com.lacina.cubeeclient.serverConnection.GsonReflectionRequest;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetUserInfoCallback;

import java.lang.reflect.Type;
import java.util.Map;

/**
 * Request to get a json with all profile information
 */
public class GetUserInfoRequest {

    private final OnGetUserInfoCallback callback;
    private final String url;

    public GetUserInfoRequest(OnGetUserInfoCallback callback, String url) {
        this.callback = callback;
        this.url = url;
    }

    public Request getRequest(Map<String, String> headers) {
        //Create request, set URL, Params, and callbacks
        Type type = new TypeToken<JsonObject>() {
        }.getType();
        GsonReflectionRequest<JsonObject> request = new GsonReflectionRequest<>(url, type, headers, new Response.Listener<JsonObject>() {
            @Override
            public void onResponse(JsonObject response) {
                Gson gson = new Gson();
                User user = gson.fromJson(response, User.class);
                callback.onGetUserInfoCallbackSuccess(user);
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                error.printStackTrace();
                callback.onGetUserInfoCallbackError(error.getMessage());
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
