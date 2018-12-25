package com.lacina.cubeeclient.serverConnection;

/**
 * Singleton class with methods to build all URLs.
 **/
public class AppConfig {

    private static final String cubee = "/cubee";

    private static final String host = "http://150.165.85.21:8080";

//    private static final String host = "http://192.168.0.200:8080";//Servidor Adson

//    private static final String host = "http://192.168.0.110:8080";//Servidor Matheus

//    private static final String host = "http://192.168.0.109:8080";//Servidor Francisco

    private static final String user = "/user";

    private static final String register = "/register";

    private static final String facebook = "/fbLogin";

    private static final String edit = "/edit";

    private static final String delete = "/delete";

    private static final String setCommand = "/command";

    private static final String newId = "/newId";

    private static final String sector = "/sector";

    private static final String registerCubeesInASector = "/registercubees";

    private static final String unregisterCubeesInASector = "/unregistercubees";

    private static final String event = "/event";

    private static final String measurement = "/measurement";

    private static final String get = "/get";

    private static final String alarm = "/alarm";

    private static final String threshold = "/threshold";

    private static final String registrationToken = "/registrationToken";

    private static final String rule = "/rule";

    private static final String task = "/task";

    private static final String byUser = "/byUser";

    private static final String bySector = "/bySector";

    private static final String byCubee = "/byCubee";


    private static final String byEvent = "/byEvent";

    private static final String byDB9 = "/byDB9";

    private static final String db9 = "/db9";

    private static final String set = "/set";

    private static final String output = "/output";

    private static final String all = "/all";

    private static final String filter = "/filter";

    private static AppConfig instance;

    public static AppConfig getInstance() {
        if (instance == null) {
            instance = new AppConfig();
        }
        return instance;
    }

    public String registerUser() {
        return host +
                user +
                register;
    }

    public String registerFbUser() {
        return host +
                user +
                facebook;
    }

    public String editUser() {
        return host +
                user +
                edit;
    }

    public String host() {
        return host;
    }

    public String getUser() {
        return host +
                user;
    }

    public String registerCubee() {
        return host +
                cubee +
                register;
    }

    public String getCubees() {
        return host +
                cubee +
                byUser;
    }

    public String editCubee() {
        return host +
                cubee +
                edit;
    }

    public String deleteCubee() {
        return host +
                cubee +
                delete;
    }

    public String setCommandCubee() {
        return host +
                cubee +
                setCommand;
    }

    public String getCubeeById() {
        return host +
                cubee;
    }

    public String getIdNewCubee() {
        return host +
                cubee +
                newId;
    }

    public String getSector() {
        return host +
                sector;
    }

    public String getCubeesBySector() {
        return host +
                cubee +
                bySector;
    }

    public String getRegisterCubeesInASector() {
        return host +
                sector +
                registerCubeesInASector;
    }

    public String getUnregisterCubeesInASector() {
        return host +
                sector +
                unregisterCubeesInASector;
    }

    public String postSector() {
        return host +
                sector +
                register;
    }

    public String deleteSector() {
        return host +
                sector +
                delete;
    }

    public String postEvent() {
        return host +
                event +
                register;

    }

    public String getCubeeMeasurement() {
        return host +
                cubee +
                get +
                measurement;
    }


    public String getEvents() {
        return host +
                event;
    }

    public String getTaskByEvent() {
        return host +
                 task +
                byEvent;
    }

    public String deleteEvent() {
        return host +
                event +
                delete;
    }

    public String deleteRule() {
        return host +
                rule +
                delete;
    }

    public String getTasksByCubee() {
        return host +
                task +
                byCubee;
    }

    public String getIdEventsByCubee() {
        return host +
                event +
                byCubee;
    }

    public String getAlarms() {
        return host +
                alarm;
    }

    public String postThreshold() {
        return host +
                cubee +
                threshold;
    }

    public String deleteAlarm() {
        return host +
                alarm +
                delete;
    }

    public String deleteAllAlarms() {
        return host +
                alarm +
                delete + all;
    }

    public String postToken() {
        return host +
                user+
                registrationToken;
    }

    public String getRules() {
        return host +
                rule;
    }

    public String registerRule() {
        return host +
                rule +
                register;
    }

    public String registerDb9Rule(){
        return host +
                db9 +
                register;
    }

    public String getCubeesByDB9(){
        return host +
                cubee
                + byDB9;
    }

    public String deleteDB9(){
        return host +
                db9 +
                delete;
    }

    public String setDB9ToCubee(){
        return host +
                db9 +
                set;
    }

    public String getDB9ByUser(){
        return host +
                db9 +
                byUser;
    }

    public String setCubeeOutput(){
        return host +
                cubee +
                output;
    }

    public String getAlarmsByFilter() {
        return host +
                alarm +
                filter;
    }

}