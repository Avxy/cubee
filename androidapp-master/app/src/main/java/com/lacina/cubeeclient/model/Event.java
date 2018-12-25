package com.lacina.cubeeclient.model;

import java.util.List;


@SuppressWarnings("ALL")
public class Event {
    @SuppressWarnings("CanBeFinal")
    private String _id;

    @SuppressWarnings("CanBeFinal")
    private String name;

    @SuppressWarnings("CanBeFinal")
    private Boolean available;

    private String idOwner;

    private List<Task> taskList;

    @SuppressWarnings("SameParameterValue")
    public Event(String _id, String name, String idOwner, Boolean avaliable, List<Task> taskList) {
        this._id = _id;
        this.name = name;
        this.idOwner = idOwner;
        this.available = avaliable;
        this.taskList = taskList;
    }

    public String get_id() {
        return _id;
    }

    public String getName() {
        return name;
    }

    @SuppressWarnings("unused")
    public Boolean getAvailable() {
        return available;
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
    public List<Task> getTaskList() {
        return taskList;
    }

    @SuppressWarnings("unused")
    public void setTaskList(List<Task> taskList) {
        this.taskList = taskList;
    }
}
