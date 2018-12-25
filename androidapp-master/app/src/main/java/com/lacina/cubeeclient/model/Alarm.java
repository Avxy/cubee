package com.lacina.cubeeclient.model;

import java.util.Date;



@SuppressWarnings("ALL")
public class Alarm {
    private String _id;

    private String title;

    private String idOwner;

    private String body;

    private Date date;

    private String level;


    public Alarm(String _id, String title, String idOwner, String body, Date date, String level) {
        this._id = _id;
        this.title = title;
        this.idOwner = idOwner;
        this.body = body;
        this.date = date;
        this.level = level;
    }

    public String get_id() {
        return _id;
    }

    @SuppressWarnings("unused")
    public void set_id(String _id) {
        this._id = _id;
    }

    public String getTitle() {
        return title;
    }

    @SuppressWarnings("unused")
    public void setTitle(String title) {
        this.title = title;
    }

    public String getBody() {
        return body;
    }

    @SuppressWarnings("unused")
    public void setBody(String body) {
        this.body = body;
    }

    @SuppressWarnings("unused")
    public String getIdOwner() {
        return idOwner;
    }

    @SuppressWarnings("unused")
    public void setIdOwner(String idOwner) {
        this.idOwner = idOwner;
    }

    public Date getDate() {
        return date;
    }

    @SuppressWarnings("unused")
    public void setDate(Date date) {
        this.date = date;
    }

    public String getLevel() {
        return level;
    }

    @SuppressWarnings("unused")
    public void setLevel(String level) {
        this.level = level;
    }
}
