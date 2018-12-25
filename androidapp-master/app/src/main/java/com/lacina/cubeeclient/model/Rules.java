package com.lacina.cubeeclient.model;

import java.util.List;


@SuppressWarnings("ALL")
public class Rules {
    @SuppressWarnings("CanBeFinal")
    private String _id;

    private String idOwner;

    @SuppressWarnings("CanBeFinal")
    private String name;

    private List<RuleTask> ruleTasks;

    @SuppressWarnings("SameParameterValue")
    public Rules(String _id, String name, String idOwner, List<RuleTask> ruleTaskList) {
        this._id = _id;
        this.name = name;
        this.idOwner = idOwner;

        this.ruleTasks = ruleTaskList;
    }

    public String get_id() {
        return _id;
    }

    public String getName() {
        return name;
    }


    @SuppressWarnings("unused")
    public String getIdOwner() {
        return idOwner;
    }

    @SuppressWarnings("unused")
    public void setIdOwner(String idOwner) {
        this.idOwner = idOwner;
    }

    public List<RuleTask> getRuleTasks() {
        return ruleTasks;
    }

    @SuppressWarnings("unused")
    public void setRuleTasks(List<RuleTask> ruleTasks) {
        this.ruleTasks = ruleTasks;
    }
}
