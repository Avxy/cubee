package com.lacina.cubeeclient.model;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

@SuppressWarnings("ALL")
public class Task {


    private String _id;

    private String idEvent;

    private String idCubee;

    private String cubeeName;

    private String previousTaskId;

    private int appCommand;

    private Date dateTask;

    @SuppressWarnings("unused")
    private boolean done;

    @SuppressWarnings("SameParameterValue")
    public Task(String _id, String idEvent, String idCubee, String cubeeName, String previousTaskId, int appcommand, Date dateTask) {
        this._id = _id;
        this.idEvent = idEvent;
        this.idCubee = idCubee;
        this.cubeeName = cubeeName;
        this.previousTaskId = previousTaskId;
        this.appCommand = appcommand;
        this.dateTask = dateTask;
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
    public String getIdEvent() {
        return idEvent;
    }

    @SuppressWarnings("unused")
    public void setIdEvent(String idEvent) {
        this.idEvent = idEvent;
    }

    @SuppressWarnings("unused")
    public String getIdCubee() {
        return idCubee;
    }

    @SuppressWarnings("unused")
    public void setIdCubee(String idCubee) {
        this.idCubee = idCubee;
    }

    public String getCubeeName() {
        return cubeeName;
    }

    @SuppressWarnings("unused")
    public void setCubeeName(String cubeeName) {
        this.cubeeName = cubeeName;
    }

    @SuppressWarnings("unused")
    public String getPreviousTaskId() {
        return previousTaskId;
    }

    @SuppressWarnings("unused")
    public void setPreviousTaskId(String previousTaskId) {
        this.previousTaskId = previousTaskId;
    }

    public int getAppcommand() {
        return appCommand;
    }

    public void setAppcommand(int appCommand) {
        this.appCommand = appCommand;
    }

    public Date getDateTask() {
        return dateTask;
    }

    public void setDateTask(Date dateTask) {
        this.dateTask = dateTask;

    }

    public String getDateTaskDataString() {
        SimpleDateFormat dateDateFormat = new SimpleDateFormat("dd/MM/yyyy", Locale.ENGLISH);
        return dateDateFormat.format(this.dateTask);
    }

    public String getDateTaskHourString() {
        SimpleDateFormat dateDateFormat = new SimpleDateFormat("HH:mm", Locale.ENGLISH);
        return dateDateFormat.format(this.dateTask);
    }


}