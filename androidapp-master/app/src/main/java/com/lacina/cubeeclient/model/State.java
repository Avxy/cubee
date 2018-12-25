package com.lacina.cubeeclient.model;

@SuppressWarnings("ALL")
public class State {

    private int time;
    private int value;

    public State(int time, int output){
        this.time = time;
        this.value = output;
    }

    @SuppressWarnings("unused")
    public int getOutput() {
        return value;
    }

    @SuppressWarnings("unused")
    public void setOutput(int output) {
        this.value = output;
    }

    @SuppressWarnings("unused")
    public int getTime() {
        return time;
    }

    @SuppressWarnings("unused")
    public void setTime(int time) {
        this.time = time;
    }
}
