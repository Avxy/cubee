package com.lacina.cubeeclient.serverConnection.requests;

import android.util.Log;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.RetryPolicy;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.StringRequest;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetStringRequest;


public class GetStringRequest {

    private final OnGetStringRequest callback;

    private final String url;

    public GetStringRequest(OnGetStringRequest callback, String url) {
        this.callback = callback;
        this.url = url;
    }


    public Request getRequest() {
        //Create request, set URL, Params, and callbacks
        StringRequest request = new StringRequest(Request.Method.GET, url, new Response.Listener<String>() {
            @Override
            public void onResponse(String response) {
                callback.onGetStringRequestSuccess(response);
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                Log.d("VOLEYERRO", (error.getMessage() == null) ? "Error message empty" : error.getMessage());
                error.printStackTrace();
                callback.onGetStringRequestError();
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
