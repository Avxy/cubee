package com.lacina.cubeeclient.utils;

/**
 * Created by matheus on 02/08/17.
 */

@SuppressWarnings({"ALL", "DefaultFileTemplate"})
public class NotificationEvent {
    private String body;

    public NotificationEvent(String body) {
        this.body = body;
    }

    @SuppressWarnings("unused")
    public String getBody() {
        return body;
    }

    @SuppressWarnings("unused")
    public void setBody(String body) {
        this.body = body;
    }
}