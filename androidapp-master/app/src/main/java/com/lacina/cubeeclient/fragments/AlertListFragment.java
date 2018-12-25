package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.widget.SwipeRefreshLayout;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.FrameLayout;
import android.widget.ListView;
import android.widget.TextView;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.CubeeGridAlertFilterActivity;
import com.lacina.cubeeclient.adapters.AlarmFilterAdapter;
import com.lacina.cubeeclient.adapters.AlarmListAdapter;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.Alarm;
import com.lacina.cubeeclient.model.AlertsFilter;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

import static com.facebook.GraphRequest.TAG;

@SuppressWarnings("ALL")
public class AlertListFragment extends BackableFragment {

    /**
     * List of events view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.lv_event_and_rules)
    public ListView lvAlarm;

    /**
     * Swipe view, used to refresh alarms
     */
    @SuppressWarnings("unused")
    @BindView(R.id.refresh)
    public SwipeRefreshLayout refreshLayout;

    /**
     * TextView when there is no alerts.
     */
    @SuppressWarnings("unused")
    @BindView(R.id.no_alerts_tv)
    public TextView noAlertsTextView;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    /**
     * idToken used in request to autenticate with backend
     */
    private String idToken;

    /**
     * List with all alerts.
     */
    ArrayList<Alarm> allAlertsList;

    /**
     * List with current alerts which are in adapter.
     */
    ArrayList<Alarm> currentAlertList;

    /**
     * Alert list adapter.
     */
    AlarmListAdapter alertListAdapter;

    /**
     * List view of alert's filters
     */
    private ListView lvFilters;

    /**
     * AlertsFilter's adapter
     */
    private AlarmFilterAdapter filterAdapter;

    private static final int LOW_CRITICALITY = 3;
    private static final int MEDIUM_CRITICALITY = 2;
    private static final int HIGH_CRITICALITY = 1;
    private static final int CUBEE_FILTER = 0;

    private static final String HIGH = "HIGH";
    private static final String MEDIUM = "MEDIUM";
    private static final String LOW = "LOW";

    private List<Cubee> currentCubeesFilter;
    private ArrayList<String> currentIdCubeesFilter;

    private static final String ALL_CUBEES = "Todos os CUBEEs";

    private static final String ARRAY_LIST = "ARRAYLIST";

    @SuppressWarnings("unused")
    public AlertListFragment() {
        // Required empty public constructor
    }

    public static AlertListFragment newInstance() {
        return new AlertListFragment();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View fragmentView = inflater.inflate(R.layout.fragment_alert_list, container, false);
        ButterKnife.bind(this, fragmentView);

        currentCubeesFilter = new ArrayList<>();
        currentIdCubeesFilter = new ArrayList<>();

        activity = getActivity();
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        allAlertsList = new ArrayList<>();
        currentAlertList = new ArrayList<>();
        lvFilters = (ListView) activity.findViewById(R.id.lv_alarms_filter);
        lvFilters.setOnItemClickListener(getFilterClickListener());
        filterAdapter = (AlarmFilterAdapter)lvFilters.getAdapter();

        init();
        return fragmentView;
    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragmentGridFragmentToSelected();
        }
    }

    /**
     * If user logged request to alarms
     */
    private void init() {
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull com.google.android.gms.tasks.Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                idToken = task.getResult().getToken();
                                refreshAlarms();
                            } else {
                                //Log Erro
                                //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                                Log.e(TAG, "Error Get token" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                                FirebaseAuth.getInstance().signOut();
                                Intent intent = new Intent(activity, LoginTabFragment.class);
                                startActivity(intent);
                                activity.finish();
                            }
                        }
                    });
        }


        refreshLayout.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                alertListAdapter.notifyDataSetChanged();
                refreshLayout.setRefreshing(false);
            }
        });

    }

    /**
     * Request to get alarms from server
     * CALLBACK: {@link #getAlarmCallback()}
     */
    private void refreshAlarms() {
        if (idToken != null) {
            Map<String, String> headers = new HashMap<>();
            headers.put("idToken", idToken);
            GetJsonArrayRequest getAlarmsRequest = new GetJsonArrayRequest(getAlarmCallback(), AppConfig.getInstance().getAlarms());
            ActivityUtils.showProgressDialog(activity, "Carregando alarmes.");
            RequestQueueSingleton.getInstance(activity).addToRequestQueue(getAlarmsRequest.getRequest(headers));
        }
    }

    private OnGetJsonArrayCallback getAlarmCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                alarmJsonToArray(jsonArray);
                ActivityUtils.cancelProgressDialog();
                if(allAlertsList.isEmpty()){
                    noAlertsTextView.setVisibility(View.VISIBLE);
                }else{
                    noAlertsTextView.setVisibility(View.GONE);
                }
                createAdapter();
                addAlarms();

                lvAlarm.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
                    @Override
                    public boolean onItemLongClick(AdapterView<?> parent, View view, final int position, long id) {
                        deleteAlarmsDialog(position);
                        return true;
                    }
                });

                lvAlarm.setOnScrollListener(new AbsListView.OnScrollListener() {
                    @Override
                    public void onScrollStateChanged(AbsListView view, int scrollState) {
                        if (lvAlarm.getLastVisiblePosition() == lvAlarm.getAdapter().getCount() -1 &&
                                lvAlarm.getChildAt(lvAlarm.getChildCount() - 1).getBottom() <= lvAlarm.getHeight())
                        {// Reached the end of the page
                            addAlarms();
                        }
                    }

                    @Override
                    public void onScroll(AbsListView view, int firstVisibleItem, int visibleItemCount, int totalItemCount) {

                    }
                });
            }


            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {

            }
        };
    }

    /**
     * Create an ArrayList from JsonArray.
     * @param jsonArray
     */
    private void alarmJsonToArray(JsonArray jsonArray){
        Gson gson = new Gson();
        Alarm currentAlarm;
        for (int i = 0; i < jsonArray.size(); i++) {
            currentAlarm = gson.fromJson(jsonArray.get(i), Alarm.class);

            allAlertsList.add(currentAlarm);
        }
    }

    /**
     * Instantiate AlarmListAdapter.
     */
    private void createAdapter(){
        alertListAdapter = new AlarmListAdapter(getActivity(), currentAlertList);
        lvAlarm.setAdapter(alertListAdapter);
    }

    /**
     * This method copy elements from alertList(contains all alarms) to currentAlertList.
     * currentAlertList is the list used in adapter.
     * Adapter will load first 10 alarms, then when reach the end of the page will load more ten.
     */
    private void addAlarms() {
        if(currentAlertList != null && allAlertsList != null){
            int count = 0;
            int index = alertListAdapter.getCount();
            for(int i = index; i < index+10; i++){
                if(allAlertsList.size() > i){
                    currentAlertList.add(allAlertsList.get(i));
                    count++;
                }
            }
            alertListAdapter.notifyDataSetChanged();
        }
    }

    /**
     * Request to delete an alarm
     * CALLBACK: {@link #onDeleteAlarmCallback()}
     * @param alarm alarm to delete
     */
    private void deleteAlarmRequest(Alarm alarm) {
        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", idToken);
        headers.put("idAlarm", alarm.get_id());
        PostJsonObjectRequest deleteAlarmRequest = new PostJsonObjectRequest(onDeleteAlarmCallback(alarm), AppConfig.getInstance().deleteAlarm());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(deleteAlarmRequest.getRequest(headers));
    }

    private OnPostJsonObjectCallback onDeleteAlarmCallback(final Alarm alarm) {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                allAlertsList.remove(alarm);
                currentAlertList.remove(alarm);
                alertListAdapter.notifyDataSetChanged();

                ActivityUtils.showToast(activity, "Notificação deletada");
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao deletar Notificação. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }
        };
    }

    /**
     * Request to delete all alarms
     * CALLBACK: {@link #onDeleteAlarmCallback()}
     * @param alarm alarm to delete
     */
    private void deleteAllAlarmsRequest() {
        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", idToken);
        PostJsonObjectRequest deleteAlarmRequest = new PostJsonObjectRequest(onDeleteAllAlarmsCallback(), AppConfig.getInstance().deleteAllAlarms());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(deleteAlarmRequest.getRequest(headers));
    }

    private OnPostJsonObjectCallback onDeleteAllAlarmsCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                allAlertsList = new ArrayList<>();
                currentAlertList = new ArrayList<>();
                refreshAlarms();

                ActivityUtils.showToast(activity, "Todas as notificações foram deletadas");
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {

            }
        };
    }

    /**
     * Instantiate a dialog when deleting an alert.
     * Checkbox to give user possibility to delete all alerts.
     * @param position current selected alarm
     */
    private void deleteAlarmsDialog(final int position){
        View checkBoxView = View.inflate(activity, R.layout.checkbox, null);
        final CheckBox checkBox = (CheckBox) checkBoxView.findViewById(R.id.checkbox);
        checkBox.setText("Deletar todos os alarmes");
        FrameLayout.LayoutParams checkBoxLayoutParams = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT, FrameLayout.LayoutParams.WRAP_CONTENT);

        checkBoxLayoutParams.leftMargin = 35;
        checkBoxLayoutParams.topMargin = 15;
        checkBox.setLayoutParams(checkBoxLayoutParams);

        AlertDialog.Builder alertDialogBuilder = new AlertDialog.Builder(activity);
        alertDialogBuilder.setView(checkBoxView);
        alertDialogBuilder.setTitle("Deletar alarme.");
        alertDialogBuilder.setMessage("Você deseja realmente deletar esse alarme?");
        alertDialogBuilder.setPositiveButton("Ok", new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface arg0, int arg1) {
                if(checkBox.isChecked()){
                    deleteAllAlarmsRequest();
                }else{
                    deleteAlarmRequest(allAlertsList.get(position));
                }

            }
        });
        alertDialogBuilder.show();
    }

    /**
     * Left side bar onItemClickListener
     * @return
     */
    private AdapterView.OnItemClickListener getFilterClickListener() {
        return new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                filterAdapter.setSelectedFilters(position);
                filterAdapter.notifyDataSetChanged();

                switch (position) {
                    case CUBEE_FILTER: {
                        Intent intent = new Intent(activity, CubeeGridAlertFilterActivity.class);

                        Bundle args = new Bundle();
                        args.putSerializable(ARRAY_LIST,(Serializable)currentCubeesFilter);
                        intent.putExtra(ARRAY_LIST, args);
                        startActivityForResult(intent, 1);
                        break;
                    }
                    case LOW_CRITICALITY: {
                        applyFilter();
                        break;
                    }
                    case MEDIUM_CRITICALITY: {
                        applyFilter();
                        break;
                    }
                    case HIGH_CRITICALITY: {
                        applyFilter();
                        break;
                    }

                    default: {
                        break;
                    }
                }
            }
        };
    }

    /**
     * Get current filters and apply in AlertList.
     */
    private void applyFilter(){
        ArrayList filtersTags = createFiltersTags();
        refreshAlarmsByFilter(filtersTags, currentIdCubeesFilter);
    }

    /**
     * According to options selecteds in left side bar, create an ArrayList with tags.
     * @return ArrayList with filter's tags.
     */
    private ArrayList createFiltersTags(){
        ArrayList currentFilters = filterAdapter.getFilters();
        ArrayList filtersTags = new ArrayList();

        if(currentFilters.contains(LOW_CRITICALITY)){
            filtersTags.add(LOW);
        }
        if(currentFilters.contains(MEDIUM_CRITICALITY)){
            filtersTags.add(MEDIUM);
        }
        if(currentFilters.contains(HIGH_CRITICALITY)){
            filtersTags.add(HIGH);
        }

        return filtersTags;
    }

    /**
     * Refresh alarms by filter with informations from server.
     * @param filterTags contains criticality tags.
     * @param cubeeIds contains cubees ids selecteds to filter.
     */
    private void refreshAlarmsByFilter(ArrayList filterTags, ArrayList cubeeIds){
        AlertsFilter filters = new AlertsFilter(filterTags, cubeeIds);

        Gson gson = new Gson();
        String jsonEvent = gson.toJson(filters);

        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", idToken);
        headers.put("filter", jsonEvent);

        GetJsonArrayRequest getAlarmsRequest = new GetJsonArrayRequest(getAlarmByFilterCallback(), AppConfig.getInstance().getAlarmsByFilter());
        ActivityUtils.showProgressDialog(activity, "Carregando alarmes.");
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(getAlarmsRequest.getRequest(headers));
    }

    private OnGetJsonArrayCallback getAlarmByFilterCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                allAlertsList = new ArrayList<>();
                currentAlertList = new ArrayList<>();
                alarmJsonToArray(jsonArray);

                ActivityUtils.cancelProgressDialog();

                checkNoAlertsTv();
                createAdapter();
                addAlarms();
            }
            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {

            }
        };
    }

    private void checkNoAlertsTv() {
        if (allAlertsList.isEmpty()) {
            noAlertsTextView.setVisibility(View.VISIBLE);
        } else {
            noAlertsTextView.setVisibility(View.GONE);
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (requestCode == 1) {
            if(resultCode == Activity.RESULT_OK){
                Bundle args = data.getBundleExtra(ARRAY_LIST);

                currentCubeesFilter = (ArrayList<Cubee>) args.get(ARRAY_LIST);
                currentIdCubeesFilter = getIdCubeesArray();
                applyFilter();
            }
            if (resultCode == Activity.RESULT_CANCELED) {
                //Write your code if there's no result
            }
        }
    }

    /**
     * Make an array with all cubees' ids that are selected to filter.
     * ALL_CUBEES means that user selected to view all cubees(no filter by cubees).
     *
     * @return ArrayList<String> with cubees' ids.
     */
    private ArrayList<String> getIdCubeesArray() {
        ArrayList<String> cubeeIds = new ArrayList<>();
        for(int i = 0; i < currentCubeesFilter.size(); i++){
            cubeeIds.add(currentCubeesFilter.get(i).get_id());
        }
        if(cubeeIds.contains(ALL_CUBEES)){
            return new ArrayList<>();
        }else{
            return cubeeIds;
        }
    }
}