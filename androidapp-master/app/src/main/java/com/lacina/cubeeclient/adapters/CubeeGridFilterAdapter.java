package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Cubee;

import java.util.ArrayList;

public class CubeeGridFilterAdapter extends CubeeGridAdapter {

    private static final String ALL_CUBEES = "Todos os CUBEEs";

    public CubeeGridFilterAdapter(Context context, ArrayList<Cubee> items) {
        super(context, items);
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        //SET ITEM VIEW
        String nomeCubee = getItems().get(position).getName();
        View view = LayoutInflater.from(getContext()).inflate(R.layout.adapter_cubee, parent, false);
        TextView tvNomeCubee = (TextView) view.findViewById(R.id.nome_cubee);
        TextView tvEventOrder = (TextView) view.findViewById(R.id.tv_event_order);
        ImageView v = (ImageView) view.findViewById(R.id.img_cubee);

        //SET VIEW WITH CORRESPONDING ITEM INFORMATION
        tvNomeCubee.setText(nomeCubee);
        Cubee cubee = getItems().get(position);
        //CHANGE VIEW WHEN A ITEM IS ADDED TO SELECTED LIST
        try {
            if (cubee.getCubeeBtn()) {
                if(getListSelectedCubees().contains(cubee) && cubee.get_id().equals(ALL_CUBEES)){
                    v.setBackgroundResource(R.mipmap.all_cubees_clicked);
                }else if(getListSelectedCubees().contains(cubee)) {
                    v.setBackgroundResource(R.mipmap.img_cubee_event_signal_selected);
                }else if(cubee.get_id().equals(ALL_CUBEES)){
                    v.setBackgroundResource(R.mipmap.all_cubees_unclicked);
                } else {
                    v.setBackgroundResource(R.mipmap.img_cubee_signal_on);
                }

            } else {
                if(getListSelectedCubees().contains(cubee) && cubee.get_id().equals(ALL_CUBEES)){
                    v.setBackgroundResource(R.mipmap.all_cubees_clicked);
                }if (getListSelectedCubees().contains(cubee)) {
                    v.setBackgroundResource(R.mipmap.img_cubee_event_selected);
                }else if(cubee.get_id().equals(ALL_CUBEES)){
                    v.setBackgroundResource(R.mipmap.all_cubees_unclicked);
                } else {
                    tvEventOrder.setVisibility(View.GONE);
                }
            }
        } catch (Exception e) {
            Log.d("Home", e.getMessage());
        }
        return view;
    }

    @Override
    public void setListSelectedCubees(ArrayList<Cubee> listSelectedCubees) {
        super.setListSelectedCubees(listSelectedCubees);
    }

    @Override
    public ArrayList<Cubee> getListSelectedCubees() {
        return super.getListSelectedCubees();
    }
}
