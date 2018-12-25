package com.lacina.cubeeclient.model;


import java.io.Serializable;

@SuppressWarnings("ALL")
public class Cubee implements Serializable {

    private String name;

    private String _id;

    private String idOwner;

    private Boolean validated;

    private Boolean cubeeState;

    private Boolean cubeeBtn;

    private String idSector;

    private Integer lowerThreshold;

    private Integer upperThreshold;


    public Cubee(String name, String _id, String idOwner, String validated,
                 String cubeeState, String cubeeBtn, String idSector) {
        this.name = name;
        this._id = _id;
        this.idOwner = idOwner;
        this.validated = Boolean.parseBoolean(validated);
        this.cubeeState = Boolean.parseBoolean(cubeeState);
        this.cubeeBtn = Boolean.parseBoolean(cubeeBtn);
        this.idSector = idSector;
    }

    public String getName() {
        return name;
    }

    @SuppressWarnings("unused")
    public void setName(String name) {
        this.name = name;
    }

    public String get_id() {
        return _id;
    }

    @SuppressWarnings("unused")
    public void set_id(String _id) {
        this._id = _id;
    }

    @SuppressWarnings("unused")
    public String getIdOwner() {
        return idOwner;
    }

    @SuppressWarnings("unused")
    public void setIdOwner(String idOwner) {
        this.idOwner = idOwner;
    }

    @SuppressWarnings("unused")
    public Boolean getValidated() {
        return validated;
    }

    @SuppressWarnings("unused")
    public void setValidated(Boolean validated) {
        this.validated = validated;
    }

    public Boolean getCubeeState() {
        return cubeeState;
    }

    @SuppressWarnings("unused")
    public void setCubeeState(Boolean cubeeState) {
        this.cubeeState = cubeeState;
    }

    public Boolean getCubeeBtn() {
        return cubeeBtn;
    }

    @SuppressWarnings("unused")
    public void setCubeeBtn(Boolean cubeeBtn) {
        this.cubeeBtn = cubeeBtn;
    }

    @SuppressWarnings("unused")
    public String getIdSector() {
        return idSector;
    }

    public void setIdSector(String idSector) {
        this.idSector = idSector;
    }

    public Integer getLowerThreshold() {
        return lowerThreshold;
    }

    @SuppressWarnings("unused")
    public void setLowerThreshold(Integer lowerThreshold) {
        this.lowerThreshold = lowerThreshold;
    }

    public Integer getUpperThreshold() {
        return upperThreshold;
    }

    @SuppressWarnings("unused")
    public void setUpperThreshold(Integer upperThreshold) {
        this.upperThreshold = upperThreshold;
    }

    @Override
    public String toString() {
        return "Cubee{" +
                "name='" + name + '\'' +
                ", _id='" + _id + '\'' +
                ", idOwner='" + idOwner + '\'' +
                ", validated=" + validated +
                ", cubeeState=" + cubeeState +
                ", cubeeBtn=" + cubeeBtn +
                ", idSector='" + idSector + '\'' +
                ", lowerThreshold=" + lowerThreshold +
                ", upperThreshold=" + upperThreshold +
                '}';
    }

    @Override
    public boolean equals(Object obj) {
        return this._id.equals(((Cubee)obj).get_id());
    }
}
