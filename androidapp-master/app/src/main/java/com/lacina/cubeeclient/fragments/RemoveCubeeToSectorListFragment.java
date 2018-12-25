package com.lacina.cubeeclient.fragments;


import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.ListView;

import com.android.volley.VolleyError;
import com.facebook.login.LoginManager;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.LoginAndBLETabsActivity;
import com.lacina.cubeeclient.activities.MainActivity;
import com.lacina.cubeeclient.adapters.SelectCubeeListAdapter;
import com.lacina.cubeeclient.controllers.CubeeController;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.User;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetCubeesListBySectorCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetUserInfoCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.serverConnection.requests.old.GetCubeesListBySectorRequest;
import com.lacina.cubeeclient.serverConnection.requests.old.GetUserInfoRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;
import com.lacina.cubeeclient.utils.ConnectionUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Fragment for fill the central of the main activity at the beggining.
 * Shows all Cubees in a list view and control the interaction this them.
 */
@SuppressWarnings({"ALL", "JavaDoc"})
public class RemoveCubeeToSectorListFragment extends Fragment {
    /**
     * Tag used to auxiliate the application log
     */
    private static final String TAG = "RemoveCubeeToSecFrag";

    /**
     * List view to show all the user CUBEEs
     */
    @SuppressWarnings("unused")
    @BindView(R.id.lv_cubees)
    public ListView lvCubeesList;

    /**
     * Field to represents this activity
     */
    private Activity activity;

    private String sectorIdToSet;

    private String sectorId;

    /**
     * Callback for get cubee request callback
     * If sucess add all cubees to the list
     */
    @SuppressWarnings("unused")
    private boolean firstTimeRequest = true;

    /**
     * Token who represents a firebaseUser used to validation
     */
    private String userToken;

    @SuppressWarnings("unused")
    private FragmentManager fragmentManager;

    /**
     * Field to represents this applicationContext
     */
    private Context context;

    /**
     * Cubee Controller for cubee related operations:
     */
    private CubeeController cubeeController;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    private SelectCubeeListAdapter selectCubeeListAdapter;

    @SuppressWarnings("CanBeFinal")
    private List<Cubee> listCubeesToRemoveFromASector = new ArrayList<Cubee>();

    @SuppressWarnings("unused")
    public RemoveCubeeToSectorListFragment() {
        // Required empty public constructor

    }

    @SuppressWarnings("SameParameterValue")
    public static RemoveCubeeToSectorListFragment newInstance(String sectorIdToSet, String sectorId) {
        Bundle args = new Bundle();
        args.putString("sectorIdToSet", sectorIdToSet);
        args.putString("sectorId", sectorId);
        RemoveCubeeToSectorListFragment fragment = new RemoveCubeeToSectorListFragment();
        fragment.setArguments(args);

        Log.d(TAG, "Sector id = " + sectorIdToSet);
        return fragment;
    }

    /**
     * Set the fields
     *
     * @param savedInstanceState
     */
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        activity = getActivity();
        this.fragmentManager = getFragmentManager();
        this.context = activity.getApplicationContext();
        this.cubeeController = CubeeController.getInstance();
        setCubeeListView();

    }

    /**
     * Bind the view
     *
     * @param inflater
     * @param container
     * @param savedInstanceState
     * @return
     */
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View fragmentView = inflater.inflate(R.layout.fragment_remove_cubee_to_sector, container, false);
        ButterKnife.bind(this, fragmentView);
        sectorIdToSet = getArguments().getString("sectorIdToSet");
        sectorId = getArguments().getString("sectorId");
        initButtons(fragmentView);

        return fragmentView;
    }

    private void initButtons(View view) {

        Button unregister = (Button) view.findViewById(R.id.unregister_cubee_in_sector);
        unregister.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                unregisterCubeeInASector();
            }
        });


        Button cancel = (Button) view.findViewById(R.id.cancel_unregister_cubee_in_sector);
        cancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                goToMainFragment();
            }
        });


    }

    private void goToMainFragment() {
        FragmentActivity fa = (FragmentActivity) activity;
        CubeeGridFragment cubeeGridFragment = CubeeGridFragment.newInstance(sectorId);
        FragmentTransaction transaction = fa.getSupportFragmentManager().beginTransaction();
        transaction.replace(R.id.fl_fragment_center, cubeeGridFragment);

        try {
            transaction.commit();
        } catch (IllegalStateException ignored) {
            // TODO
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        setCubeeListView();
        CubeeController.getInstance().setCubeeAtual(null);
        ActivityUtils.showProgressDialog(activity, "carregando.");
    }

    private void unregisterCubeeInASector() {
        ActivityUtils.showProgressDialog(activity, "Removendo CUBEE (s)");
        @SuppressWarnings("Convert2Diamond") List<String> listIdCubeeToRemovefromASector = new ArrayList<String>();
        for (Cubee cubee : listCubeesToRemoveFromASector) {
            listIdCubeeToRemovefromASector.add(cubee.get_id());
        }

        final String jsonArrayCubeesToRegisterInASector = new Gson().toJson(listIdCubeeToRemovefromASector);
        //Log.d(TAG, jsonArrayCubeesToRegisterInASector);

        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                userToken = task.getResult().getToken();
                                checkUser(userToken);
                                Map<String, String> params = new HashMap<>();
                                params.put("idToken", userToken);
                                params.put("idSector", sectorIdToSet);
                                params.put("listIdCubees", jsonArrayCubeesToRegisterInASector);

                                //request
                                PostJsonObjectRequest requestRegisterCubeesInASector = new PostJsonObjectRequest(registerOrUnregisterCubeesInASectorCallback(), AppConfig.getInstance().getRegisterCubeesInASector());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(requestRegisterCubeesInASector.getRequest(params));

                                //GetCubeesListBySectorRequest getUserInfo = new GetCubeesListBySectorRequest(getCubeesListCallback(), AppConfig.getInstance().getCubeesBySector());
                                // RequestQueueSingleton.getInstance(activity).addToRequestQueue(getUserInfo.getRequest(headers));

                            } else {
                                logout();
                                //Log if error
                                //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                                Log.e(TAG, "error on Get Token:" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                            }
                        }
                    });

        }
    }

    private OnPostJsonObjectCallback registerOrUnregisterCubeesInASectorCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.showToast(context, "CUBEE (s) removido (s) com sucesso.");
                goToMainFragment();
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                String myMessage = "Erro ao registrar o CUBEE no setor. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }


        };
    }

    /**
     * Click listener for each item at Cubee List
     */
    private AdapterView.OnItemClickListener onItemClickListener() {


        return new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Cubee cubeeSelected = (Cubee) parent.getItemAtPosition(position);
                CubeeController.getInstance().setCubeeAtual(cubeeSelected);
                ImageView imgCubee = (ImageView) view.findViewById(R.id.cubee_img);

                if (!listCubeesToRemoveFromASector.contains(cubeeSelected)) {
                    imgCubee.setBackgroundResource(R.mipmap.img_cubee_signal_off);
                    cubeeSelected.setIdSector(sectorIdToSet);
                    listCubeesToRemoveFromASector.add(cubeeSelected);

                } else {
                    imgCubee.setTag("Unclicked");
                    imgCubee.setBackgroundResource(R.mipmap.cubee_simple);
                    cubeeSelected.setIdSector(null);
                    listCubeesToRemoveFromASector.remove(cubeeSelected);
                }

                selectCubeeListAdapter.setItemSelected(listCubeesToRemoveFromASector);

                Log.d(TAG, "" + cubeeSelected);


            }
        };
    }

    /**
     * Setup list with all cubees from the service.
     */
    private void setCubeeListView() {
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                userToken = task.getResult().getToken();
                                checkUser(userToken);
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", userToken);
                                headers.put("idSector", sectorId);
                                GetCubeesListBySectorRequest getUserInfo = new GetCubeesListBySectorRequest(getCubeesListCallback(), AppConfig.getInstance().getCubeesBySector());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getUserInfo.getRequest(headers));

                            } else {
                                logout();
                                //Log if error
                                //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                                Log.e(TAG, "error on Get Token:" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                            }
                        }
                    });

        }


    }

    private void checkUser(String token) {
        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", token);
        GetUserInfoRequest getUserInfo = new GetUserInfoRequest(onGetUserInfoCallback(), AppConfig.getInstance().getUser());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(getUserInfo.getRequest(headers));

    }

    /**
     * Logout form the aplication and go back to {@link MainActivity}
     */
    private void logout() {
        LoginManager loginManager = LoginManager.getInstance();

        if (loginManager != null) {
            LoginManager.getInstance().logOut();
        }

        FirebaseAuth.getInstance().signOut();

        Intent intent = new Intent(activity, LoginAndBLETabsActivity.class);
        activity.startActivity(intent);
        activity.finish();
    }

    private OnGetCubeesListBySectorCallback getCubeesListCallback() {
        return new OnGetCubeesListBySectorCallback() {
            @Override
            public void onGetCubeesListCallbackSucess(ArrayList<Cubee> cubeeListResponse) {
                ActivityUtils.cancelProgressDialog();
                if (cubeeListResponse.isEmpty()) {
                    ActivityUtils.showToast(activity, "Voce não tem cubees neste Grupo ou Setor");
                    firstTimeRequest = false;


                } else {
                    CubeeController.getInstance().setCubeeList(cubeeListResponse);
                    cubeeController.setCubeeList(cubeeListResponse);
                    selectCubeeListAdapter = new SelectCubeeListAdapter(activity, CubeeController.getInstance().getCubeeList());
                    lvCubeesList.setAdapter(selectCubeeListAdapter);
                    lvCubeesList.setOnItemClickListener(onItemClickListener());
                }


            }

            @Override
            public void onGetCubeesListCallbackError(String error) {
                ActivityUtils.cancelProgressDialog();
            }
        };
    }

    private OnGetUserInfoCallback onGetUserInfoCallback() {
        return new OnGetUserInfoCallback() {
            @Override
            public void onGetUserInfoCallbackSuccess(User user) {

            }

            @Override
            public void onGetUserInfoCallbackError(String message) {
                ActivityUtils.showToast(activity, "Erro de autenticação");
                ConnectionUtils.logout(activity);
            }
        };
    }
}