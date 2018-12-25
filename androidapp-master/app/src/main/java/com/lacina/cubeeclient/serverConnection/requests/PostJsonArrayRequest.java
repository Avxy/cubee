package com.lacina.cubeeclient.serverConnection.requests;

import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.RetryPolicy;
import com.android.volley.VolleyError;
import com.android.volley.toolbox.JsonArrayRequest;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonArrayCallback;

import org.json.JSONArray;
import org.json.JSONException;

import java.util.Map;

/**
 * Post a cubee registration request
 */
@SuppressWarnings("ALL")
public class PostJsonArrayRequest {

    //TODO: Make a test
    private final OnPostJsonArrayCallback callback;

    private final String url;

    @SuppressWarnings("unused")
    public PostJsonArrayRequest(OnPostJsonArrayCallback callback, String url) {
        this.url = url;
        this.callback = callback;
    }

    public Request getRequest(Map<String, String> params) {
        //Create request, set URL, Params, and callbacks
        JSONArray body = new JSONArray();
        try {
            body = new JSONArray(params);
        } catch (JSONException e) {
            e.printStackTrace();
        }

        final JsonArrayRequest request = new JsonArrayRequest(Request.Method.POST, url, body, new Response.Listener<JSONArray>() {
            @Override
            public void onResponse(JSONArray response) {
                Gson gson = new Gson();
                JsonArray responseInJsonArray = gson.fromJson(response.toString(), JsonArray.class);
                callback.onPostJsonArrayCallbackSucess(responseInJsonArray);
            }
        }, new Response.ErrorListener() {
            @Override
            public void onErrorResponse(VolleyError error) {
                error.printStackTrace();
                callback.onPostJsonArrayCallbackError(error);

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

