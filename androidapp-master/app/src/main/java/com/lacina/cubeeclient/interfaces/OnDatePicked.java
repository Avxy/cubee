package com.lacina.cubeeclient.interfaces;

import com.lacina.cubeeclient.model.Cubee;

import java.util.Date;



public interface OnDatePicked {
    void onDatePicked(Cubee cubee, Date date, int position);
    void onHourPicked(Cubee cubee, Date date, int position);
}
