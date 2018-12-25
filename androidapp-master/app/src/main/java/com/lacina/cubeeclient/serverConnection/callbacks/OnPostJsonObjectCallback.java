package com.lacina.cubeeclient.serverConnection.callbacks;


import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.RetryPolicy;
import com.android.volley.VolleyError;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.reflect.TypeToken;
import com.lacina.cubeeclient.model.Sector;
import com.lacina.cubeeclient.serverConnection.GsonReflectionRequest;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetSectorsRequestCallback;

import java.lang.reflect.Type;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;

@SuppressWarnings("ALL")
public interface OnPostJsonObjectCallback {
    void onPostJsonObjectCallbackSuccess(JsonObject jsonObject);
    void onPostJsonObjectCallbackError(VolleyError volleyError);

    class GetJsonObjectRequest {

    /**
     * Request to get a json with all sectors
     */

        private final OnGetSectorsRequestCallback callback;
        private final String url;

        @SuppressWarnings("unused")
        public GetJsonObjectRequest(OnGetSectorsRequestCallback callback, String url) {
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
                    Gson gson = new Gson();
                    JsonArray jsonArray = response.getAsJsonArray();
                    List<Sector> sectorList = new CopyOnWriteArrayList<>();
                    for (JsonElement sector:jsonArray) {
                        Sector s = gson.fromJson(sector, Sector.class);
                        sectorList.add(s);
                    }
                    callback.onGetSectorsRequestSuccess(sectorList);
                }
            }, new Response.ErrorListener() {
                @Override
                public void onErrorResponse(VolleyError error) {
                    error.printStackTrace();
                    callback.onGetSectorsRequestError();
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
}
