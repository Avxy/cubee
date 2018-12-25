package com.lacina.cubeeclient.controllers;

import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.CubeeMeasurement;

import java.util.ArrayList;


public class CubeeController {

    private static CubeeController INSTANCE;

    private Cubee cubeeAtual;

    private ArrayList<Cubee> cubeeList;

    private boolean isCubeeRegistered = false;

    private CubeeMeasurement cubeeMeasurement;

    private CubeeController() {
    }

    public static CubeeController getInstance() {
        if (INSTANCE == null) {
            INSTANCE = new CubeeController();
        }

        return INSTANCE;
    }

    public Cubee getCubeeAtual() {
        return cubeeAtual;
    }

    public void setCubeeAtual(Cubee cubeeAtual) {
        this.cubeeAtual = cubeeAtual;
    }

    public ArrayList<Cubee> getCubeeList() {


        return cubeeList;
    }

    public void setCubeeList(ArrayList<Cubee> cubeeList) {
        this.cubeeList = cubeeList;
    }

    public boolean isCubeeRegistered() {
        return isCubeeRegistered;
    }

    public void setCubeeRegistered(boolean cubeeRegistered) {
        isCubeeRegistered = cubeeRegistered;
    }

    public CubeeMeasurement getCubeeMeasurement() {
        return cubeeMeasurement;
    }

    public void setCubeeMeasurement(CubeeMeasurement cubeeMeasurement) {
        this.cubeeMeasurement = cubeeMeasurement;
    }
}
