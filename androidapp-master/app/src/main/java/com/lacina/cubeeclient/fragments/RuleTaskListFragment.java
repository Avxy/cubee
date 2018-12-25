package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;

import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.RuleTaskShowListAdapter;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.Rules;

/**
 * Created by Igor on 15/08/17.
 */

@SuppressWarnings({"ALL", "DefaultFileTemplate"})
public class RuleTaskListFragment extends BackableFragment {
    private static Rules actualRule;

    private ListView lvRuleTasks;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    @SuppressWarnings("unused")
    private String idRule;

    /**
     * idToken used in request to autenticate with backend
     */
    @SuppressWarnings("unused")
    private String idToken;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    @SuppressWarnings("unused")
    private FirebaseUser firebaseUser;

    @SuppressWarnings("unused")
    public RuleTaskListFragment() {
        // Required empty public constructor
    }


    public static RuleTaskListFragment newInstance(Rules rule) {
        Bundle args = new Bundle();
        actualRule = rule;
        RuleTaskListFragment fragment = new RuleTaskListFragment();
        fragment.setArguments(args);

        return fragment;
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View fragmentView = inflater.inflate(R.layout.fragment_task_list, container, false);
        lvRuleTasks = (ListView) fragmentView.findViewById(R.id.lv_event_and_rules);
        activity = getActivity();
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();

        // init();


        lvRuleTasks.setAdapter(new RuleTaskShowListAdapter(activity, actualRule.getRuleTasks()));


        return fragmentView;
    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragmentGridFragmentToSelected();
        }
    }

//    private void init() {
//        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
//        if (firebaseUser != null) {
//            firebaseUser.getIdToken(true)
//                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
//                        public void onComplete(@NonNull com.google.android.gms.tasks.Task<GetTokenResult> task) {
//                            if (task.isSuccessful()) {
//                                idToken = task.getResult().getToken();
//                                Map<String, String> params = new HashMap<>();
//                                params.put("idToken", idToken);
//                                params.put("idRule", idRule);
//                                GetJsonArrayRequest getTaskByEventRequest = new GetJsonArrayRequest(getTaskByEventCallback(), AppConfig.getInstance().getTaskByEvent());
//                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getTaskByEventRequest.getRequest(params));
//                            } else {
//                                Log.e(TAG, "Error Get token" + (task.getException()==null ? "null exception" : task.getException().getMessage()));
//                                FirebaseAuth.getInstance().signOut();
//                                Intent intent = new Intent(activity, LoginTabFragment.class);
//                                startActivity(intent);
//                                activity.finish();
//                            }
//                        }
//                    });
//        }
//
//
//
//
//
//
//    }

//    public OnGetTaskByEventRequestCallback getTaskByEventCallback() {
//        return new OnGetTaskByEventRequestCallback() {
//            @Override
//            public void onGetTaskByEventRequestSucess(ArrayList taskList) {
//                lvRuleTasks.setAdapter(new TaskShowListAdapter(activity, taskList));
//            }
//
//            @Override
//            public void onGetTaskByEventRequestError(String message) {
//
//            }
//        };
//    }
//}
}
