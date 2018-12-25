package com.lacina.cubeeclient.model;


@SuppressWarnings("ALL")
public class User {

    private String id;

    private String name;

    private String email;

    private String telephone;

    @SuppressWarnings("unused")
    public User() {
    }

    @SuppressWarnings("unused")
    public User(String id, String name, String email, String telephone) {
        this.id = id;
        this.name = name;
        this.email = email;
        this.telephone = telephone;
    }

    @SuppressWarnings("unused")
    public String getId() {
        return id;
    }

    @SuppressWarnings("unused")
    public void setId(String id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    @SuppressWarnings("unused")
    public void setName(String name) {
        this.name = name;
    }


    public String getEmail() {
        return email;
    }

    @SuppressWarnings("unused")
    public void setEmail(String email) {
        this.email = email;
    }

    public String getTelephone() {
        return telephone;
    }

    @SuppressWarnings("unused")
    public void setTelephone(String number) {
        this.telephone = number;
    }

}
