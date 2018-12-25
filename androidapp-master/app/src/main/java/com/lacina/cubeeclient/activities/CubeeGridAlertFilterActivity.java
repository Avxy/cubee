package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.GridView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.volley.VolleyError;
import com.facebook.login.LoginManager;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.CubeeGridFilterAdapter;
import com.lacina.cubeeclient.controllers.CubeeController;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.User;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetUserInfoCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.old.GetUserInfoRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.ConnectionUtils;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

public class CubeeGridAlertFilterActivity extends AppCompatActivity {

    /**
     * Apply Filter button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btm_save_filter)
    public Button btnFilter;

    /**
     * Cancel Register Filter button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btm_cancel_filter)
    public Button cancelFilterCubee;

    /**
     * Alert text view
     * Show a alert if there is not any cubee in this sector or group
     */
    @SuppressWarnings("unused")
    @BindView(R.id.tv_alert_text)
    public TextView tv_alert_text;

    /**
     * Buttons to apply filter or cancel
     */
    @SuppressWarnings("unused")
    @BindView(R.id.ln_buttons_filter)
    public LinearLayout lnButtonsFilter;

    /**
     * List fragmentView to show all the user CUBEEs
     */
    @SuppressWarnings("unused")
    @BindView(R.id.gridviewcubees)
    public GridView gvCubeesList;

    /**
     * Field to represents this activity
     */
    private Activity activity;

    /**
     * List of selected cubees
     */
    private ArrayList<Cubee> listCubeeSelected;

    /**
     * Token who represents a firebaseUser used to validation
     */
    private String userToken;

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

    /**
     * Adapter of the cubee grid
     */
    private CubeeGridFilterAdapter adapterCubeeGrid;

    /**
     * ALL_CUBEES Tag, used to identify if user selected to filter by all cubees.
     */
    private static final String ALL_CUBEES = "Todos os CUBEEs";

    /**
     * ARRAYLIST Tag, used in bundle to recover informations.
     */
    private static final String ARRAY_LIST = "ARRAYLIST";

    /**
     * Model of CUBEE to represents all cubees in GridView.
     */
    private Cubee allCubeesGridModel;

    /**
     * Current position of ALL_CUBEES option in GridView
     */
    private static final int ALL_CUBEES_FILTER = 0;

    /**
     * Set the fields
     *
     * @param savedInstanceState
     */
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_cubee_grid_alert_filter);

        ButterKnife.bind(this);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        activity = this;
        this.cubeeController = CubeeController.getInstance();
        listCubeeSelected = new ArrayList<>();

        initButtons();
        setCubeeListView();
    }

    @Override
    public void onResume() {
        super.onResume();
        if (listCubeeSelected != null) {
            listCubeeSelected.clear();
        }
        CubeeController.getInstance().setCubeeAtual(null);
    }


    @Override
    public void onPause() {
        super.onPause();
        ActivityUtils.cancelProgressDialog();
    }

    // Buttons ----------------------------------------------------------------------

    private void initButtons() {

        btnFilter.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (!listCubeeSelected.isEmpty()) {
                    List<Cubee> selectedCubees = adapterCubeeGrid.getSelectedCubees();
                    Intent returnIntent = new Intent();

                    Bundle args = new Bundle();
                    args.putSerializable(ARRAY_LIST,(Serializable)selectedCubees);
                    returnIntent.putExtra(ARRAY_LIST, args);
                    setResult(Activity.RESULT_OK,returnIntent);
                    finish();
                } else {
                    ActivityUtils.showToast(activity, "Por favor, selecione pelo menos um CUBEE.");
                }
            }
        });


        cancelFilterCubee.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                finish();
            }
        });

    }

    //Set GriView ------------------------------------------------------------------------

    /**
     * Setup list with all cubees from the service.
     * CALLBACK: {@link #getCubeesListCallback()}
     */
    private void setCubeeListView() {
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                userToken = task.getResult().getToken();
                                checkUser(userToken);
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", userToken);

                                GetJsonArrayRequest cubeeList = new GetJsonArrayRequest(getCubeesListCallback(), AppConfig.getInstance().getCubees());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(cubeeList.getRequest(headers));
                            } else {
                                logout();
                                //Log if error
                                //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                                Log.e("GridFilter", "error on Get Token:" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                            }
                        }
                    });

        }


    }

    private OnGetJsonArrayCallback getCubeesListCallback() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                Gson gson = new Gson();
                ArrayList cubeeListResponse = new ArrayList();
                for (int i = 0; i < jsonArray.size(); i++) {
                    //noinspection unchecked
                    cubeeListResponse.add(gson.fromJson(jsonArray.get(i), Cubee.class));
                }

                ActivityUtils.cancelProgressDialog();

                checkNoCubeesTextView(cubeeListResponse);

                //noinspection unchecked
                CubeeController.getInstance().setCubeeList(cubeeListResponse);
                //noinspection unchecked
                cubeeController.setCubeeList(cubeeListResponse);
                ArrayList<Cubee> allCubees = CubeeController.getInstance().getCubeeList();
                addDefaultGridOption(allCubees);

                adapterCubeeGrid = new CubeeGridFilterAdapter(activity, allCubees);
                gvCubeesList.setAdapter(adapterCubeeGrid);
                gvCubeesList.setOnItemClickListener(onItemClickListener());
                setCurrentFilters();
            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
            }

        };
    }

    /**
     * Check visibility of no cubees text view
     * @param cubeeListResponse
     */
    private void checkNoCubeesTextView(ArrayList cubeeListResponse){
        if (cubeeListResponse.isEmpty()) {
            tv_alert_text.setVisibility(View.VISIBLE);
        } else {
            tv_alert_text.setVisibility(View.GONE);
        }

    }

    //Grid View click listeners --------------------------------------------------------------

    /**
     * Click listener for each item at Cubee List
     */
    private AdapterView.OnItemClickListener onItemClickListener() {
        return new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Cubee cubeeSelected = (Cubee) parent.getItemAtPosition(position);

                Cubee allCubeesItem = (Cubee) parent.getItemAtPosition(ALL_CUBEES_FILTER);

                if ((!listCubeeSelected.contains(cubeeSelected) &&
                        cubeeSelected.get_id().equals(ALL_CUBEES)) ||
                        !listCubeeSelected.contains(cubeeSelected)
                        && listCubeeSelected.contains(allCubeesItem)) {
                    listCubeeSelected = new ArrayList<>();
                    listCubeeSelected.add(cubeeSelected);
                }else if (listCubeeSelected.contains(cubeeSelected)) {
                    listCubeeSelected.remove(cubeeSelected);
                } else if (!listCubeeSelected.contains(allCubeesGridModel)){
                    listCubeeSelected.add(cubeeSelected);
                }

                adapterCubeeGrid.setListSelectedCubees(listCubeeSelected);
                adapterCubeeGrid.notifyDataSetChanged();
            }
        };
    }

    // Check User Login Methods -----------------------------------------------------
    private void checkUser(String token) {
        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", token);
        GetUserInfoRequest getUserInfo = new GetUserInfoRequest(onGetUserInfoCallback(), AppConfig.getInstance().getUser());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(getUserInfo.getRequest(headers));

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

    /**
     * Logout form the application and go back to {@link MainActivity}
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

    /**
     * Update selected cubees according to current filters.
     */
    private void setCurrentFilters(){
        Intent intent = getIntent();
        Bundle args = intent.getBundleExtra(ARRAY_LIST);

        listCubeeSelected = (ArrayList<Cubee>) args.get(ARRAY_LIST);

        adapterCubeeGrid.setListSelectedCubees(listCubeeSelected);
        adapterCubeeGrid.notifyDataSetChanged();
    }

    /**
     * Create a model to represents ALL_CUBEES in GridView.
     * @param cubees
     */
    private void addDefaultGridOption(ArrayList<Cubee> cubees) {
        allCubeesGridModel = new Cubee(ALL_CUBEES, ALL_CUBEES, ALL_CUBEES, "true", "true", "true"
                , ALL_CUBEES);
        cubees.add(0, allCubeesGridModel);
    }
}
