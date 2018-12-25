package com.lacina.cubeeclient.serverConnection.callbacks.old;

import com.lacina.cubeeclient.model.User;


public interface OnGetUserInfoCallback {
    void onGetUserInfoCallbackSuccess(User user);
    void onGetUserInfoCallbackError(String message);
}
