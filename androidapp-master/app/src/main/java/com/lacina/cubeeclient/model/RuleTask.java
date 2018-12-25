package com.lacina.cubeeclient.model;


@SuppressWarnings("ALL")
public class RuleTask {


    private String _id;

    private String idCubee;

    private String cubeeName;

    private String previousIdCubee;

    private String typeAlert;

    private Integer taskCommand;

    @SuppressWarnings("SameParameterValue")
    public RuleTask(String _id, String idCubee, String cubeeName, String previousIdCubee, String typeAlert, Integer taskCommand) {
        this._id = _id;
        this.idCubee = idCubee;
        this.cubeeName = cubeeName;
        this.previousIdCubee = previousIdCubee;
        this.typeAlert = typeAlert;
        this.taskCommand = taskCommand;
    }

    public String getCubeeName() {
        return cubeeName;
    }

    @SuppressWarnings("unused")
    public void setCubeeName(String cubeeName) {
        this.cubeeName = cubeeName;
    }


    @SuppressWarnings("unused")
    public String get_id() {
        return _id;
    }

    @SuppressWarnings("unused")
    public void set_id(String _id) {
        this._id = _id;
    }

    @SuppressWarnings("unused")
    public String getIdCubee() {
        return idCubee;
    }

    @SuppressWarnings("unused")
    public void setIdCubee(String idCubee) {
        this.idCubee = idCubee;
    }

    @SuppressWarnings("unused")
    public String getPreviousIdCubee() {
        return previousIdCubee;
    }

    @SuppressWarnings("unused")
    public void setPreviousIdCubee(String previousIdCubee) {
        this.previousIdCubee = previousIdCubee;
    }

    public String getTypeAlert() {
        return typeAlert;
    }

    public void setTypeAlert(String typeAlert) {
        this.typeAlert = typeAlert;
    }

    public Integer getTaskCommand() {
        return taskCommand;
    }

    public void setTaskCommand(int taskCommand) {
        this.taskCommand = taskCommand;
    }

}