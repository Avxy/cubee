package com.lacina.cubeeclient.model;

import java.util.ArrayList;

/**
 * Created by adson on 26/10/17.
 */

public class AlertsFilter {

    private ArrayList<String> level;
    private ArrayList<Integer> cubeeIds;

    public AlertsFilter(ArrayList<String> level, ArrayList<Integer> cubeeIds){
        this.level = level;
        this.cubeeIds = cubeeIds;
    }

    public ArrayList<String> getLevels() {
        return level;
    }

    public void setLevels(ArrayList<String> level) {
        this.level = level;
    }

    public ArrayList<Integer> getCubeeIds() {
        return cubeeIds;
    }

    public void setCubeeIds(ArrayList<Integer> cubeeIds) {
        this.cubeeIds = cubeeIds;
    }
}
