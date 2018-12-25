package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Alerts;
import com.lacina.cubeeclient.model.RuleTask;

import java.util.List;

/**
 * Created by matheus on 19/07/17.
 */

@SuppressWarnings("DefaultFileTemplate")
public class RuleTaskShowListAdapter extends BaseAdapter {

    /**
     * Reference to caller context
     */
    private final Context context;

    @SuppressWarnings("CanBeFinal")
    private List<RuleTask> ruleTaskList;

    public RuleTaskShowListAdapter(Context context, List<RuleTask> ruleTaskList) {
        this.context = context;
        this.ruleTaskList = ruleTaskList;
    }

    @Override
    public int getCount() {
        return ruleTaskList.size();
    }

    @Override
    public Object getItem(int position) {
        return ruleTaskList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        LayoutInflater view = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);




        if (position == 0) {
            convertView = view.inflate(R.layout.adapter_rule_task_show, parent, false);
        } else {
            // IF NOT FIRST SET TRIGGER AND COMMAND
            convertView = view.inflate(R.layout.adapter_rule_task_show_date, parent, false);
            String trigger = ruleTaskList.get(position).getTypeAlert();
            Integer command = ruleTaskList.get(position).getTaskCommand();
            TextView tvCommand = (TextView) convertView.findViewById(R.id.tv_command);
            TextView tvTrigger = (TextView) convertView.findViewById(R.id.tv_trigger_name);
            TextView tvPreviousCubee = (TextView) convertView.findViewById(R.id.tv_previous_cubee_name);

            String stringCommand;
            if (command == 1) {
                stringCommand = "Ativar";
            } else if (command == 2) {
                stringCommand = "Desativar";
            } else {
                stringCommand = "Comando Desconhecido";
            }
            tvPreviousCubee.setText(position + "º CUBEE");
            tvCommand.setText(stringCommand);
            if (trigger != null) {
                Alerts alert = Alerts.valueOf(trigger);
                trigger = alert.getName();
            }
            tvTrigger.setText(trigger);

        }

        //ALWAYS SET ORDER AND NAME
        TextView tvEventOrder = (TextView) convertView.findViewById(R.id.tv_event_order);
        TextView nomeCubee = (TextView) convertView.findViewById(R.id.nome_cubee);
        String positionSelected = 1 + position + "º";
        String nameCubee = ruleTaskList.get(position).getCubeeName();

        tvEventOrder.setText(positionSelected);
        nomeCubee.setText(nameCubee);

        return convertView;
    }

}
