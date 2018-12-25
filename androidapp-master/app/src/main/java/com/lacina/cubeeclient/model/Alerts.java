package com.lacina.cubeeclient.model;

import java.util.Arrays;
import java.util.List;

@SuppressWarnings("ALL")
public enum Alerts {

    ACTIVATION("Ativando"),
    DEACTIVATION("Desativando"),
    RESTART("Reiniciando"),
    UPPER_THRESHOLD("Medição acima do limiar superior."),
    LOWER_THRESHOLD("Medição abaixo do limiar inferior."),
    RFID("RFID");


    private final String name;

    @SuppressWarnings("unused")
    Alerts(String name) {
        this.name = name;
    }

    public static String[] getListAlerts() {
        List<Alerts> list = Arrays.asList(Alerts.values());
        String[] alertsName = new String[list.size()];
        for (int i = 0; i < list.size(); i++) {
            alertsName[i] = list.get(i).getName();
        }
        return alertsName;
    }

    public String getName() {
        return name;
    }


}

