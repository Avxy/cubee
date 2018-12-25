package com.lacina.cubeeclient.model;

import java.util.List;


@SuppressWarnings("ALL")
public class CubeeMeasurement {

    private String idCubee;

    private List<String> keyMeasurements;

    private List<String> valuesMeasurements;


    public CubeeMeasurement(String idCubee, List<String> keyMeasurements, List<String> valuesMeasurements) {
        this.idCubee = idCubee;
        this.keyMeasurements = keyMeasurements;
        this.valuesMeasurements = valuesMeasurements;
    }

    @SuppressWarnings("unused")
    public String getIdCubee() {
        return idCubee;
    }

    @SuppressWarnings("unused")
    public void setIdCubee(String idCubee) {
        this.idCubee = idCubee;
    }

    public List<String> getKeyMeasurements() {
        return keyMeasurements;
    }

    @SuppressWarnings("unused")
    public void setKeyMeasurements(List<String> keyMeasurements) {
        this.keyMeasurements = keyMeasurements;
    }

    public List<String> getValuesMeasurements() {
        return valuesMeasurements;
    }

    @SuppressWarnings("unused")
    public void setValuesMeasurements(List<String> valuesMeasurements) {
        this.valuesMeasurements = valuesMeasurements;
    }


}
