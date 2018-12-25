package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
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
import com.lacina.cubeeclient.adapters.EventListAdapter;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.Event;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;


@SuppressWarnings("ALL")
public class EventListFragment extends BackableFragment {

    private final String TAG = "EVENT_LIST_FRAG";

    /**
     * Events list view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.lv_event_and_rules)
    public ListView lvEvent;

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
     * Adapter to control event list view
     */
    private EventListAdapter eventListAdapter;

    private View fragmentView;

    @SuppressWarnings("unused")
    public EventListFragment() {
        // Required empty public constructor
    }

    public static EventListFragment newInstance() {
        return new EventListFragment();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        fragmentView = inflater.inflate(R.layout.fragment_event_list, container, false);
        activity = getActivity();
        ButterKnife.bind(this, fragmentView);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
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
     * Init list
     * GET LIST FROWM LERVER
     * CALLBACK: {@link #getEventCallback()} ]}
     */
    private void init() {
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull com.google.android.gms.tasks.Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                idToken = task.getResult().getToken();
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", idToken);
                                GetJsonArrayRequest getEventsRequest = new GetJsonArrayRequest(getEventCallback(), AppConfig.getInstance().getEvents());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getEventsRequest.getRequest(headers));
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

    }

    private OnGetJsonArrayCallback getEventCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                Gson gson = new Gson();
                ArrayList eventList = new ArrayList();
                for (int i = 0; i < jsonArray.size(); i++) {
                    //noinspection unchecked
                    eventList.add(gson.fromJson(jsonArray.get(i), Event.class));
                }

                //noinspection unchecked
                eventListAdapter = new EventListAdapter(getActivity(), eventList);
                TextView tvMsgNoEvents = (TextView) fragmentView.findViewById(R.id.tv_msg_no_events);
                if (eventList.size() == 0) {
                    tvMsgNoEvents.setVisibility(View.VISIBLE);
                } else {
                    tvMsgNoEvents.setVisibility(View.GONE);
                }


                lvEvent.setAdapter(eventListAdapter);
                lvEvent.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                        try {
                            ChangeCenterFragmentInterface changeCenterFragmentActivity = ((ChangeCenterFragmentInterface) activity);
                            Fragment taskListFragment = TaskListFragment.newInstance(eventListAdapter.getItemEvent(position).get_id());
                            changeCenterFragmentActivity.changeCenterFragment(taskListFragment);

                        } catch (ClassCastException cce) {
                            cce.printStackTrace();
                        }
                    }
                });

                lvEvent.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
                    @Override
                    public boolean onItemLongClick(AdapterView<?> parent, View view, final int position, long id) {
                        ActivityUtils.alertDialogSimpleCallbackBtnMessages(activity, "Deletar evento.", "Você deseja realmente deletar esse evento?", "Cancel", "OK", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", idToken);
                                headers.put("idEvent", eventListAdapter.getItemEvent(position).get_id());
                                PostJsonObjectRequest deleteRequest = new PostJsonObjectRequest(onDeleteCallback(), AppConfig.getInstance().deleteEvent());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(deleteRequest.getRequest(headers));
                            }
                        });
                        return true;
                    }
                });
            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao deletar eventos. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }

        };
    }

    private OnPostJsonObjectCallback onDeleteCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.showToast(activity, "Evento deletado com sucesso.");
                init();

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Não foi possível deletar evento. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }
        };
    }
}
