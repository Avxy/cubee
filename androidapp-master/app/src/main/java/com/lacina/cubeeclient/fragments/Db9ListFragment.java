package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.design.widget.FloatingActionButton;
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
import com.lacina.cubeeclient.activities.SelectAreasActivity;
import com.lacina.cubeeclient.adapters.Db9ListAdapter;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.Db9Rule;
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

/**
 * Fragment to list DB9 Rules to consultation.
 * Let add Rules.
 */
@SuppressWarnings("ALL")
public class Db9ListFragment extends BackableFragment {

    private final String TAG = "RulesList";

    /**
     * List of all DB9 Rules.
     */
    @SuppressWarnings({"WeakerAccess", "unused"})
    @BindView(R.id.lv_db9_list)
    public ListView lvRules;

    /**
     * Message if there is no DB9 Rules.
     */
    @SuppressWarnings({"WeakerAccess", "unused"})
    @BindView(R.id.no_db9_tv)
    public TextView tvMsgNoRules;

    /**
     * Floating button to add a new DB9 Rule.
     */
    @SuppressWarnings({"WeakerAccess", "unused"})
    @BindView(R.id.float_btn_db9_list)
    public FloatingActionButton btnAddRule;

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
     * idToken of firebase
     */
    private String idToken;

    /**
     * Adapter to DB9 Rules list.
     */
    private Db9ListAdapter ruleListAdapter;

    private View fragmentView;

    @SuppressWarnings("unused")
    public Db9ListFragment() {
        // Required empty public constructor
    }


    public static Db9ListFragment newInstance() {
        return new Db9ListFragment();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        fragmentView = inflater.inflate(R.layout.fragment_db9_list, container, false);
        ButterKnife.bind(this, fragmentView);
        activity = getActivity();
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        return fragmentView;
    }

    @Override
    public void onResume() {
        init();
        super.onResume();
    }

    /**
     * Do firebase authentication and get all user's DB9 rules.
     * <p>
     * Callback: {@link #getRulesCallback()} ()}
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
                                GetJsonArrayRequest getRules = new GetJsonArrayRequest(getRulesCallback(), AppConfig.getInstance().getDB9ByUser());
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

    /**
     * Callback from getting all user's DB9 rules.
     * Here, the adapter and it's clicks listeners are also setted.
     *
     * @return OnGetJsonArrayCallback
     */
    private OnGetJsonArrayCallback getRulesCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                Gson gson = new Gson();
                Type type = new TypeToken<List<Db9Rule>>() {
                }.getType();
                ArrayList<Db9Rule> ruleList = gson.fromJson(jsonArray, type);
                ruleListAdapter = new Db9ListAdapter(activity, ruleList);
                if (ruleList.size() == 0) {
                    tvMsgNoRules.setVisibility(View.VISIBLE);
                } else {
                    tvMsgNoRules.setVisibility(View.GONE);
                }

                lvRules.setAdapter(ruleListAdapter);

                lvRules.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                        try {
                            ChangeCenterFragmentInterface changeCenterFragmentActivity = ((ChangeCenterFragmentInterface) activity);
                            Fragment ruleTaskListFragment = Db9ListCubeesFragment.newInstance(ruleListAdapter.getItemEvent(position));
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
                                headers.put("ruleId", ruleListAdapter.getItemEvent(position).get_id());
                                PostJsonObjectRequest deleteRequest = new PostJsonObjectRequest(onDeleteDB9Rule(), AppConfig.getInstance().deleteDB9());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(deleteRequest.getRequest(headers));
                            }
                        });
                        return true;
                    }
                });

                onClickAddRule();
            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao recuperar os deletar a regra. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }
        };

    }

    /**
     * Callback after deleting a DB9 Rule
     *
     * @return OnPostJsonObjectCallBack
     */
    private OnPostJsonObjectCallback onDeleteDB9Rule() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.showToast(activity, "Regra deletada com sucesso.");
                ActivityUtils.cancelProgressDialog();
                init();
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao deletar a regra.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }
        };
    }

    /**
     * On click of Float Button
     */
    private void onClickAddRule() {
        btnAddRule.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(activity, SelectAreasActivity.class);
                startActivity(intent);
            }
        });
    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragmentGridFragmentToSelected();
        }
    }
}
