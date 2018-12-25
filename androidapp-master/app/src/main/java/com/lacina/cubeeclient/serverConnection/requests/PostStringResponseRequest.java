package com.lacina.cubeeclient.serverConnection.requests;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.RetryPolicy;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonObjectRequest;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostStringResponseCallback;

import org.json.JSONObject;

import java.util.Map;

/**
 * Post a cubee registration request
 */
@SuppressWarnings("ALL")
public class PostStringResponseRequest {

    private final OnPostStringResponseCallback callback;

    private final String url;

    @SuppressWarnings("unused")
    public PostStringResponseRequest(OnPostStringResponseCallback callback, String url) {
        this.url = url;
        this.callback = callback;
    }

    public Request getRequest(Map<String, String> params) {
        //Create request, set URL, Params, and callbacks
        Request request = new JsonObjectRequest(url, new JSONObject(params), new Response.Listener<JSONObject>() {
            @Override
            public void onResponse(JSONObject response) {
                callback.onPostStringCallbackSucess(response.toString());
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                error.printStackTrace();
                callback.onPostStringCallbackError(error);
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

