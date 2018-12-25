package com.lacina.cubeeclient.adapters;

import android.content.Context;
import android.support.annotation.IdRes;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.RadioGroup;
import android.widget.Spinner;
import android.widget.TextView;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.interfaces.OnOffChange;
import com.lacina.cubeeclient.interfaces.OnTriggerChange;
import com.lacina.cubeeclient.model.Alerts;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.RuleTask;

import java.util.Arrays;
import java.util.List;

/**
 * ADAPTER TO THE NEW RULE ACTIVITY
 * YOU CAN SET THE TRIGGER AND COMMAND IN EACH ITEM
 */
public class RuleTaskEditListAdapter extends BaseAdapter {

    @SuppressWarnings("CanBeFinal")
    private List<Cubee> cubeeList;


    /**
     * Reference to caller context
     */
    private final Context context;

    @SuppressWarnings("CanBeFinal")
    private List<RuleTask> ruleTaskList;

    @SuppressWarnings("CanBeFinal")
    private OnOffChange onOffChange;

    @SuppressWarnings("CanBeFinal")
    private OnTriggerChange onTriggerChange;

    @SuppressWarnings("CanBeFinal")
    private String[] typeAlarm = Alerts.getListAlerts();

    /**
     *
     * @param context context of the caller class
     * @param cubeeList lis of cubees items to put in a list
     * @param ruleTaskList list of rules to set, each rule correspond to a cubee in cubeeList
     * @param onOffChange interface so the adapter send the set command information to the caller class
     * @param onTriggerChange interface so the adapter send the set trigger information to the caller class
     */
    public RuleTaskEditListAdapter(Context context, List<Cubee> cubeeList, List<RuleTask> ruleTaskList, OnOffChange onOffChange, OnTriggerChange onTriggerChange) {
        super();
        this.context = context;
        this.cubeeList = cubeeList;
        this.ruleTaskList = ruleTaskList;
        this.onOffChange = onOffChange;
        this.onTriggerChange = onTriggerChange;
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
     * Set each list item view.
     **/
    @Override
    public View getView(final int position, View convertView, ViewGroup parent) {
        //CUBEE CORRESPONDING WITH THIS POSITION
        final Cubee cubee = cubeeList.get(position);

        //SET VIEW
        LayoutInflater vi = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        //IF FIRST YOU CANT DEFINE TRIGGER OF COMMAND
        if (position == 0) {
            //SETUP VIEW
            convertView = vi.inflate(R.layout.adapter_rule_task_show, parent, false);
            TextView nomeCubee = (TextView) convertView.findViewById(R.id.nome_cubee);
            TextView tvEventOrder = (TextView) convertView.findViewById(R.id.tv_event_order);

            //SET INFORMATION
            String positionSelected = 1 + cubeeList.indexOf(cubee) + "º";
            tvEventOrder.setText(positionSelected);
            nomeCubee.setText(cubeeList.get(position).getName());
        } else {
            //IF NOT FIRST
            //SET VIEW
            convertView = vi.inflate(R.layout.adapter_rule_task_date, parent, false);
            TextView nomeCubee = (TextView) convertView.findViewById(R.id.nome_cubee);
            TextView tvEventOrder = (TextView) convertView.findViewById(R.id.tv_event_order);
            TextView tvPreviousCubee = (TextView) convertView.findViewById(R.id.tv_previous_cubee_name);
            RadioGroup rgOnOff = (RadioGroup) convertView.findViewById(R.id.rg_on_off);
            Spinner spinner = (Spinner) convertView.findViewById(R.id.sp_alert);
            convertView.setTag(nomeCubee);

            //SET VIEW- TRIGGER SPINNER
            @SuppressWarnings("Convert2Diamond") ArrayAdapter<String> adapter = new ArrayAdapter<String>(context, R.layout.spinner_item, typeAlarm);
            spinner.setAdapter(adapter);


            //SET INFORMATION
            RuleTask ruleTask = ruleTaskList.get(position);
            String positionSelected = 1 + cubeeList.indexOf(cubee) + "º";
            tvEventOrder.setText(positionSelected);
            nomeCubee.setText(cubeeList.get(position).getName());
            tvPreviousCubee.setText(cubeeList.indexOf(cubee) + "º CUBEE");
            Integer initialChecked = ruleTask.getTaskCommand();
            spinner.setSelection(0);
            if (initialChecked != null) {
                if (initialChecked == 1) {
                    rgOnOff.check(R.id.rbtn_on);
                } else if (initialChecked == 2) {
                    rgOnOff.check(R.id.rbtn_off);
                }
            }


            //ON CHANGE COMMAND
            rgOnOff.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(RadioGroup group, @IdRes int checkedId) {
                    int command = 1;
                    if (checkedId == R.id.rbtn_on) {
                        command = 1;
                    } else if (checkedId == R.id.rbtn_off) {
                        command = 2;
                    }
                    onOffChange.changeOnOffCommand(position, command);

                }
            });

            //ON CHANGE TRIGGER
            spinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
                @Override
                public void onItemSelected(AdapterView<?> parent, View view, int positionSpinner, long id) {
                    onTriggerChange.onTriggerChanged(position, Arrays.asList(Alerts.values()).get(positionSpinner).toString());
                }

                @Override
                public void onNothingSelected(AdapterView<?> parent) {

                }
            });

        }
        return convertView;
    }

}

