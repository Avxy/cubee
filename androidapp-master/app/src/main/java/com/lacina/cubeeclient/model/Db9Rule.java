package com.lacina.cubeeclient.model;

import java.util.ArrayList;

@SuppressWarnings("ALL")
public class Db9Rule {

    private String _id;
    private String name;
    private ArrayList<State> states;
    @SuppressWarnings("unused")
    private int nStates;

    public Db9Rule(String ruleName, ArrayList<State> states){
        this.states = states;
        if(this.states != null)
            nStates = this.states.size();
        this.name = ruleName;
    }

    @SuppressWarnings("unused")
    public ArrayList<State> getStates() {
        return states;
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

    @SuppressWarnings("unused")
    public void setStates(ArrayList<State> states) {
        this.states = states;
    }
}
