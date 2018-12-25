package com.lacina.cubeeclient.utils;

import android.content.Context;
import android.util.Log;

import com.android.volley.AuthFailureError;
import com.android.volley.NetworkError;
import com.android.volley.NetworkResponse;
import com.android.volley.NoConnectionError;
import com.android.volley.ServerError;
import com.android.volley.TimeoutError;
import com.android.volley.VolleyError;
import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.lacina.cubeeclient.R;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;


@SuppressWarnings({"ALL", "JavaDoc"})
public class VolleyErrorUtils {

    /**
     * Returns appropriate message which is to be displayed to the user
     * against the specified error object.
     *
     * @param error
     * @param context
     * @return
     */
    public static String getGenericMessage(Object error, Context context) {
        String errorMessage = context.getResources().getString(R.string.generic_error);
        if (error instanceof TimeoutError) {
            errorMessage = context.getResources().getString(R.string.timeout);
        } else if (isServerProblem(error)) {
            errorMessage = context.getResources().getString(R.string.error_server);
        } else if (isNetworkProblem(error)) {
            errorMessage =  context.getResources().getString(R.string.nointernet);
        }
        return errorMessage;

    }

    private static void logServerError(Object error) {
        VolleyError er = (VolleyError) error;
        NetworkResponse response = er.networkResponse;
        int status = response.statusCode;
        String message = getMessageFromServer(er);
        Log.d("VolleyErro", "Erro em requisição ao servidor. Status:" + status + " Message:" + message);

    }

    @SuppressWarnings("unused")
    private static String handleServerError(Object error, Context context) {

        VolleyError er = (VolleyError) error;
        NetworkResponse response = er.networkResponse;
        if (response != null) {
            switch (response.statusCode) {

                case 404:
                case 422:
                case 401:
                    try {
                        // server might return error like this { "error": "Some error occured" }
                        // Use "Gson" to parse the result
                        HashMap<String, String> result = new Gson().fromJson(new String(response.data),
                                new TypeToken<Map<String, String>>() {
                                }.getType());

                        if (result != null && result.containsKey("error")) {
                            return result.get("error");
                        }

                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    // invalid request
                    return ((VolleyError) error).getMessage();

                default:
                    return context.getResources().getString(R.string.timeout);
            }
        }

        return context.getResources().getString(R.string.generic_error);
    }

    private static boolean isServerProblem(Object error) {
        return (error instanceof ServerError || error instanceof AuthFailureError);
    }

    private static boolean isNetworkProblem(Object error) {
        //noinspection ConstantConditions
        return (error instanceof NetworkError || error instanceof NoConnectionError);
    }

    public static String getMessageFromServer(VolleyError error) {
        //String json = null;
        String message = "";

        NetworkResponse response = error.networkResponse;
        if (response != null && response.data != null) {
            message = new String(response.data);
            /*            json = new String(response.data);
            json = trimMessage(json, "message");
            if (json != null) {
                message = json;
            }*/
        }
        return message;

    }

    public static String decideRightMessage(Object error, String myMessage, Context context, boolean concatWithServerMessage) {
        String completeErrorMessage = myMessage;
        if(isNetworkProblem(error)){
            completeErrorMessage = getGenericMessage(error, context);
        }else if(isServerProblem(error) && concatWithServerMessage){
            completeErrorMessage = completeErrorMessage.concat(getMessageFromServer((VolleyError) error));
            logServerError(error);
        }
        return completeErrorMessage;
    }


    @SuppressWarnings("unused")
    public static String trimMessage(String json, String key){
        String trimmedString = null;

        try{
            JSONObject obj = new JSONObject(json);
            trimmedString = obj.getString(key);
        } catch(JSONException e){
            e.printStackTrace();
            return null;
        }

        return trimmedString;
    }



}
