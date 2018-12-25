package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.content.DialogInterface;
import android.os.Build;
import android.support.v7.app.AlertDialog;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.interfaces.OnActivateDeactivateDB9Rule;
import com.lacina.cubeeclient.model.Cubee;

import java.util.List;

/*
  Created by adson on 27/09/17.
 */

/**
 * Adapter to control list of cubees
 */
@SuppressWarnings({"ALL", "JavaDoc"})
public class CubeeDb9ListAdapter extends BaseAdapter {

    @SuppressWarnings("unused")
    private String TAG = "CubeeListDB9Adpt";

    /**
     * Reference to caller context
     */
    private final Context context;

    private final List<Cubee> cubeeList;

    private final OnActivateDeactivateDB9Rule onActivateDeactivateDB9Rule;

    private AlertDialog.Builder builder;

    public CubeeDb9ListAdapter(Context context, List<Cubee> cubeeList, OnActivateDeactivateDB9Rule onActivateDeactivateDB9Rule) {
        this.context = context;
        this.cubeeList = cubeeList;
        this.onActivateDeactivateDB9Rule = onActivateDeactivateDB9Rule;
    }

    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {

        LayoutInflater vi = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = vi.inflate(R.layout.adapter_db9_show_cubees, parent, false);
        TextView cubeeName = (TextView) convertView.findViewById(R.id.nome_cubee);
        TextView tvEventOrder = (TextView) convertView.findViewById(R.id.tv_event_order);
        TextView tvCommand = (TextView) convertView.findViewById(R.id.tv_command);

        final Cubee cubee = cubeeList.get(position);
        String nameCubee = cubee.getName();
        String positionSelected = 1 + position + "º";
        String possibleCommand = "Ativar";
        if (cubee.getCubeeState()) {
            possibleCommand = "Desativar";
        }
        convertView.setTag(nameCubee);

        tvCommand.setText(possibleCommand);
        cubeeName.setText(nameCubee);
        tvEventOrder.setText(positionSelected);


        setupDialogs();
        tvCommand.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                dialogToggleCubeeRule(cubee);
            }
        });

        return convertView;
    }

    @Override
    public int getCount() {
        return cubeeList.size();
    }

    @Override
    public Object getItem(int position) {
        return cubeeList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    /**
     * Dialog to disable rule
     *
     * @param cubee
     */
    private void dialogToggleCubeeRule(final Cubee cubee) {
        if(cubee.getCubeeState()){
            builder.setTitle("Desativar")
                    .setMessage("Tem certeza que deseja desativar essa regra?")
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            toggleCubee(cubee);
                        }
                    })
                    .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // do nothing
                        }
                    })
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .show();

        }else{

            builder.setTitle("Ativar")
                    .setMessage("Tem certeza que deseja ativar essa regra?")
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            toggleCubee(cubee);
                        }
                    })
                    .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // do nothing
                        }
                    })
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .show();

        }
    }

    /**
     * Instantiate AlertDialog Builder.
     */
    private void setupDialogs() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            builder = new AlertDialog.Builder(context, android.R.style.Theme_Material_Dialog_Alert);
        } else {
            builder = new AlertDialog.Builder(context);
        }
    }

    private void toggleCubee(Cubee cubeeAtual) {
        if (cubeeAtual.getCubeeState()) {
            onActivateDeactivateDB9Rule.deactivateDB9Rule(cubeeAtual);
        } else {
            onActivateDeactivateDB9Rule.activateDB9Rule(cubeeAtual);
        }
    }


}
