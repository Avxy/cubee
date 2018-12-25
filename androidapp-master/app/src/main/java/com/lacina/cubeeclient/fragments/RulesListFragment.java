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
import com.google.gson.reflect.TypeToken;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.RulesListAdapter;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.Rules;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;


@SuppressWarnings("ALL")
public class RulesListFragment extends BackableFragment {

    private final String TAG = "RulesList";

    @SuppressWarnings("unused")
    private static Rules actualRule;

    @SuppressWarnings("unused")
    @BindView(R.id.lv_event_and_rules)
    public ListView lvRules;

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

    private RulesListAdapter ruleListAdapter;

    private View fragmentView;

    @SuppressWarnings("unused")
    public RulesListFragment() {
        // Required empty public constructor
    }


    public static RulesListFragment newInstance() {
        return new RulesListFragment();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        fragmentView = inflater.inflate(R.layout.fragment_rule_list, container, false);
        ButterKnife.bind(this, fragmentView);
        activity = getActivity();
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        init();
        return fragmentView;
    }

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
                                GetJsonArrayRequest getRules = new GetJsonArrayRequest(getRulesCallback(), AppConfig.getInstance().getRules());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getRules.getRequest(headers));

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


    private OnGetJsonArrayCallback getRulesCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                Gson gson = new Gson();
                Type type = new TypeToken<List<Rules>>() {
                }.getType();
                ArrayList<Rules> ruleList = gson.fromJson(jsonArray, type);
                ruleListAdapter = new RulesListAdapter(getActivity(), ruleList);
                TextView tvMsgNoEvents = (TextView) fragmentView.findViewById(R.id.tv_msg_no_events);
                if (ruleList.size() == 0) {
                    tvMsgNoEvents.setVisibility(View.VISIBLE);
                } else {
                    tvMsgNoEvents.setVisibility(View.GONE);
                }


                lvRules.setAdapter(ruleListAdapter);
                lvRules.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                        //ActivityUtils.showToast(activity, ruleListAdapter.getItemEvent(position).getName());


                        try {
                            ChangeCenterFragmentInterface changeCenterFragmentActivity = ((ChangeCenterFragmentInterface) activity);
                            Fragment ruleTaskListFragment = RuleTaskListFragment.newInstance(ruleListAdapter.getItemEvent(position));
                            changeCenterFragmentActivity.changeCenterFragment(ruleTaskListFragment);

                        } catch (ClassCastException cce) {
                            cce.printStackTrace();
                        }
                    }
                });

                lvRules.setOnItemLongClickListener(new AdapterView.OnItemLongClickListener() {
                    @Override
                    public boolean onItemLongClick(AdapterView<?> parent, View view, final int position, long id) {
                        ActivityUtils.alertDialogSimpleCallbackBtnMessages(activity, "Alerta", "Você deseja realmente deletar esta Regra?", "Cancel", "OK", new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", idToken);
                                headers.put("idRule", ruleListAdapter.getItemEvent(position).get_id());
                                PostJsonObjectRequest deleteRequest = new PostJsonObjectRequest(onDeleteCallback(), AppConfig.getInstance().deleteRule());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(deleteRequest.getRequest(headers));
                            }
                        });
                        return true;
                    }
                });


            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                Log.d(TAG, "Generic" +VolleyErrorUtils.getGenericMessage(volleyError, activity));
                Log.d(TAG, "Server" + VolleyErrorUtils.getMessageFromServer(volleyError));
                ActivityUtils.showToast(activity, "Um Erro desconhecido ocorreu. Por favor, tente novamente.");
            }
        };


    }


    private OnPostJsonObjectCallback onDeleteCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.showToast(activity, "Regra deletada com sucesso.");
                init();

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Não foi possível deletar essa Regra.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }
        };
    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragmentGridFragmentToSelected();
        }
    }
}
