package com.lacina.cubeeclient.model;


import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.Serializable;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;



@SuppressWarnings("ALL")
public class EventList implements Serializable {

    //action
    private List<Cubee> listOfCubees;

    private Map<Cubee, Date> mapCubeeHour;

    @SuppressWarnings("unused")
    public EventList() {
    }

    public List<Cubee> getListOfCubees() {
        return listOfCubees;
    }

    public void setListOfCubees(List<Cubee> listOfCubees) {
        this.listOfCubees = listOfCubees;
        //noinspection Convert2Diamond
        mapCubeeHour = new HashMap<Cubee, Date>();
        for (Cubee cubee : listOfCubees) {
            Calendar defaultCalendar = Calendar.getInstance();
            defaultCalendar.set(Calendar.HOUR, 12);
            defaultCalendar.set(Calendar.MINUTE, 0);
            Date date = defaultCalendar.getTime();
            mapCubeeHour.put(cubee, date);
        }
    }


    public void putDate(Cubee cubee, Date date, int position) {
        Date dateToEdit = this.getDateCubee(cubee);
        dateToEdit.setYear(date.getYear());
        dateToEdit.setMonth(date.getMonth());
        dateToEdit.setDate(date.getDate());


        this.mapCubeeHour.put(cubee, date);
        for (int i = position + 1; i < listOfCubees.size(); i++) {
            Date afterDate = getDateCubee(listOfCubees.get(i));
            if (afterDate.before(dateToEdit)) {
                afterDate.setYear(date.getYear());
                afterDate.setMonth(date.getMonth());
                afterDate.setDate(date.getDate());
                this.mapCubeeHour.put(listOfCubees.get(i), afterDate);

            }
        }
    }

    public void putHour(Cubee cubee, Date date, int position) {
        Date dateToEdit = this.getDateCubee(cubee);
        dateToEdit.setHours(date.getHours());
        dateToEdit.setMinutes(date.getMinutes());

        this.mapCubeeHour.put(cubee, date);
        for (int i = position; i < listOfCubees.size(); i++) {
            Date afterDate = getDateCubee(listOfCubees.get(i));
            if (afterDate.before(dateToEdit)) {
                afterDate.setHours(date.getHours());
                afterDate.setMinutes(date.getMinutes());
                this.mapCubeeHour.put(listOfCubees.get(i), afterDate);
            }
        }
    }

    private Date getDateCubee(Cubee cubee) {
        return mapCubeeHour.get(cubee);
    }

    public String getDateString(Cubee cubee) {
        SimpleDateFormat dateDateFormat = new SimpleDateFormat("dd/MM/yyyy", Locale.ENGLISH);
        Date date = mapCubeeHour.get(cubee);
        return dateDateFormat.format(date);
    }

    public String getHourString(Cubee cubee) {
        SimpleDateFormat hourDateFormat = new SimpleDateFormat("HH:mm", Locale.ENGLISH);
        Date date = mapCubeeHour.get(cubee);
        return hourDateFormat.format(date);
    }

    public JSONArray toJson() {
        JSONArray jsonArray = new JSONArray();
        for (Cubee cubee : listOfCubees) {
            JSONObject jsonObject = new JSONObject();
            try {
                jsonObject.put(cubee.get_id(), mapCubeeHour.get(cubee));
            } catch (JSONException e) {
                e.printStackTrace();
            }
            jsonArray.put(jsonObject);
        }
        return jsonArray;
    }

    public Date getDateIndex(int position) {
        return mapCubeeHour.get(listOfCubees.get(position));
    }
}
