package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Task;

import java.util.List;

/**
 * Adapter for the list of tasks in a Rule
 * This adapter just show ths tasks
 */
public class TaskShowListAdapter extends BaseAdapter {

    /**
     * Reference to caller context
     */
    private final Context context;

    /**
     * Task list
     */
    @SuppressWarnings("CanBeFinal")
    private List<Task> taskList;

    public TaskShowListAdapter(Context context, List<Task> taskList) {
        this.context = context;
        this.taskList = taskList;
    }

    @Override
    public int getCount() {
        return taskList.size();
    }

    @Override
    public Object getItem(int position) {
        return taskList.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        //SET VIEW
        LayoutInflater vi = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        convertView = vi.inflate(R.layout.adapter_task_show_date, parent, false);
        TextView nomeCubee = (TextView) convertView.findViewById(R.id.nome_cubee);
        TextView tvEventOrder = (TextView) convertView.findViewById(R.id.tv_event_order);
        TextView tvDate = (TextView) convertView.findViewById(R.id.tv_date);
        TextView tvHour = (TextView) convertView.findViewById(R.id.tv_hour);
        TextView tvCommand = (TextView) convertView.findViewById(R.id.tv_command);
        convertView.setTag(nomeCubee);

        // SET INFORMATION
        final String dateString = taskList.get(position).getDateTaskDataString();
        final String hourString = taskList.get(position).getDateTaskHourString();
        final String nameCubee = taskList.get(position).getCubeeName();
        String commandString;
        if (taskList.get(position).getAppcommand() == 1) {
            commandString = "Ativar";
        } else {
            commandString = "Desativar";
        }
        nomeCubee.setText(nameCubee);
        String positionSelected = 1 + position + "º";
        tvEventOrder.setText(positionSelected);
        tvDate.setText(dateString);
        tvHour.setText(hourString);
        tvCommand.setText(commandString);


        return convertView;
    }
}