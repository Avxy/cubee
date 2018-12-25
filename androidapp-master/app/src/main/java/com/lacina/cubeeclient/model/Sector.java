package com.lacina.cubeeclient.model;


@SuppressWarnings("ALL")
public class Sector {

    private String name;

    private String _id;


    public Sector(String name, String _id) {
        this.name = name;
        this._id = _id;
    }

    public String get_id() {
        return _id;
    }

    @SuppressWarnings("unused")
    public void set_id(String _id) {
        this._id = _id;
    }

    public String getName() {
        return name;
    }

    @SuppressWarnings("unused")
    public void setName(String name) {
        this.name = name;
    }


}
