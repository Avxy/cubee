package com.lacina.cubeeclient.model;

public class InstantCommandBluetooth {

    private int currentOutput;

    public InstantCommandBluetooth(int output){
        this.currentOutput = output;
    }

    public int getCurrentOutput() {
        return currentOutput;
    }

    public void setCurrentOutput(int currentOutput) {
        this.currentOutput = currentOutput;
    }
}
