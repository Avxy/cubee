package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.os.Build;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.ActionBar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ListView;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.AlarmFilterAdapter;
import com.lacina.cubeeclient.adapters.LeftSideButtonsListAdapter;
import com.lacina.cubeeclient.controllers.CubeeController;
import com.lacina.cubeeclient.controllers.SectorController;
import com.lacina.cubeeclient.controllers.UserController;
import com.lacina.cubeeclient.fragments.AddCubeeToSectorListFragment;
import com.lacina.cubeeclient.fragments.AlertListFragment;
import com.lacina.cubeeclient.fragments.CubeeGridFragment;
import com.lacina.cubeeclient.fragments.CubeeInformationFragment;
import com.lacina.cubeeclient.fragments.Db9ListFragment;
import com.lacina.cubeeclient.fragments.EventListFragment;
import com.lacina.cubeeclient.fragments.ProfileFragment;
import com.lacina.cubeeclient.fragments.RegisterSectorFragment;
import com.lacina.cubeeclient.fragments.RemoveCubeeToSectorListFragment;
import com.lacina.cubeeclient.fragments.RulesListFragment;
import com.lacina.cubeeclient.fragments.ScanCubeesTabFragment;
import com.lacina.cubeeclient.interfaces.BleScanChangeable;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.interfaces.OnSectorAdded;
import com.lacina.cubeeclient.model.Sector;
import com.lacina.cubeeclient.model.TokenEvent;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetCheckConnectionServerCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.ConnectionUtils;
import com.lacina.cubeeclient.utils.NotificationEvent;
import com.lacina.cubeeclient.utils.SensorOrientationChangeNotifier;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.CopyOnWriteArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Main application activity
 * <p>
 * From this activity its possible to go to all the screens in the application.
 *
 **/
public class MainActivity extends AppCompatActivity implements OnSectorAdded, ChangeCenterFragmentInterface, BleScanChangeable,  SensorOrientationChangeNotifier.Listener {

    private final String TAG = "MainActivity";

    /**
     * Field to reference this activity.
     */
    private Activity activity;

    /**
     * Centre frame layout, this frame is constantly replaced
     */
    @BindView(R.id.fl_fragment_center)
    public FrameLayout flFragmentCenter;

    /**
     * Add a cubee button.
     * Replace {@link #flFragmentCenter}with {@link com.lacina.cubeeclient.fragments.RegisterCubeeFragment}
     */
    @BindView(R.id.ibtn_add_cubee)
    public ImageButton ibtnAddCubee;

    /**
     * Home image Button to return {@link #flFragmentCenter} to {@link CubeeGridFragment}
     */
    @BindView(R.id.ibtn_home_icon)
    public ImageButton ibtnHomeIcon;

    /**
     * Top toolbar. Add Cubee, Home button and options buttons are in this toolbar.
     */
    @BindView(R.id.top_toolbar)
    public Toolbar toolbar;

    /**
     * User Controller for common user related methods, like get actual user.
     */
    private UserController userController;

    /**
     * Sector controller for sector related operations: set sector list.
     */
    private SectorController sectorController;

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
     * Field to reference application context.
     */
    private Context context;

    /**
     * List View to show all sectors.
     */
    private ListView lvSector;

    private ListView lvFilters;

    /**
     * Adapter to control buttons list at the side o the screen;
     */
    private LeftSideButtonsListAdapter leftSideButtonsListAdapter;

    private AlarmFilterAdapter filterAdapter;

    /**
     * idToken used in request to authenticate with backend
     */
    private String idToken;

    /**
     * Alert dialog showed qhen a long click is performed
     */
    private AlertDialog sectorOptionsAlertDialog;

    /**
     * Current fragment.
     */
    public static Fragment currentFragment;

    /**
     * Number ID of the current Fragment
     */
    private Integer sectorSelected = 0;

    /**
     * Activity action bar
     */
    private ActionBar actionBar;

    /**
     * Menu of options.
     */
    private Menu menu;

    private android.support.v7.app.AlertDialog.Builder builder;

    private android.support.v7.app.AlertDialog myDialog;

    public MainActivity() {
        // Required empty public constructor
    }

    //Activity Methods -----------------------------------------------------------------------------
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //Setup variables and UI
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        context = getApplicationContext();
        activity = this;
        ButterKnife.bind(this);

        //Setup controllers
        userController = UserController.getInstance();
        cubeeController = CubeeController.getInstance();
        sectorController = SectorController.getInstance();

        //Get Sector list
        sectorListRequest();

        //Set current Fragment
        currentFragment = EventListFragment.newInstance();

        //Setup actionbar
        initMenuSideBarButtons();
        setSupportActionBar(toolbar); //Use designed by us toolbar as actionbar
        actionBar = getSupportActionBar();
        if (actionBar != null) {
            actionBar.setDisplayShowTitleEnabled(false);
        }
        SharedPreferences sharedPreferences = PreferenceManager.getDefaultSharedPreferences(this);
        String registrationToken = sharedPreferences.getString("registration_id", null);
        sendFirebaseNotificationToken(registrationToken);

    }

    /**
     * Check if server is on at on resume. If cubee is just got registered change center fragment to all cubees;
     **/
    @Override
    protected void onResume() {
        super.onResume();
        ConnectionUtils.checkServerConnection(activity, new OnGetCheckConnectionServerCallback() {
            @Override
            public void onGetCheckConnectionServerResponse(boolean response) {
                if (!response) {
                    ConnectionUtils.logout(activity);
                }
            }
        });


        if (cubeeController != null && cubeeController.isCubeeRegistered()) {
            cubeeController.setCubeeRegistered(false);
            changeCenterFragmentGridFragment("all_cubees");
        }

        SensorOrientationChangeNotifier.getInstance(this).addListener(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        SensorOrientationChangeNotifier.getInstance(this).remove(this);
    }

    @Override
    public void onStart() {
        super.onStart();
        EventBus.getDefault().register(this);
    }

    @Override
    public void onStop() {
        super.onStop();
        EventBus.getDefault().unregister(this);
    }

    @Override
    public void onBackPressed() {
        ActivityUtils.alertDialogSimpleCallbackBtnMessages(activity, "Alerta", "Você deseja realmente deslogar da aplicação?", "Não", "Sim", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                ConnectionUtils.logout(activity);
            }
        });
    }

    /**
     * Inflate toolbar with menu items
     **/
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.toolbar_items, menu);
        this.menu = menu;
        return super.onCreateOptionsMenu(menu);
    }

    /**
     * Toolbar items item no_sector_selected listeners
     **/
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            //Logout btn
            case R.id.logout:
                ConnectionUtils.logout(activity);
                return true;

            //See profile
            case R.id.profile:
                ProfileFragment profileFragment = ProfileFragment.newInstance();
                changeCenterFragment(profileFragment);
                return true;

            //Alert button(bell icon)
            case R.id.alertButton:
                AlertListFragment alertListFragment = AlertListFragment.newInstance();
                changeCenterFragment(alertListFragment);
                menu.getItem(0).setIcon(ContextCompat.getDrawable(context, R.mipmap.btn_alert));
//                getResources().getDrawable(R.mipmap.btn_alert)
                return true;
            case R.id.view_rules:
                changeCenterFragment(RulesListFragment.newInstance());
                break;

            case R.id.view_events:
                changeCenterFragment(EventListFragment.newInstance());
                break;
            case R.id.view_db9:
                changeCenterFragment(Db9ListFragment.newInstance());
                break;

            default:
                return super.onOptionsItemSelected(item);

        }
        return true;
    }

    /**
     * Activity needs to implements{@link BleScanChangeable} this to decide which activity to go to
     * when a device is successfully scanned.
     * Scan Fragment {@link ScanCubeesTabFragment}
     * Activity to go: {@link SetUpWifiCubeeActivity}
     * @return
     */
    @Override
    public Class activityToGoTo() {
        return SetUpWifiCubeeActivity.class;
    }

    // Notifications Methods -----------------------------------------------------------------------

    /**
     * Method called when a Notification Event happens
     * @param event Event that happened
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onEvent(final NotificationEvent event) {
        menu.getItem(0).setIcon(ContextCompat.getDrawable(context, R.mipmap.btn_new_alert));
//        getResources().getDrawable(R.mipmap.btn_new_alert)
    }

    /**
     * Method called when a Notification Event happens.
     * @param event Event that happened
     */
    @Subscribe(threadMode = ThreadMode.MAIN)
    public void onMessageEvent(TokenEvent event) {
        sendFirebaseNotificationToken(event.token);
    }

    /**
     * Send token to backend to identify the device
     * so the server send notification to a unique device
     * Callback: {@link #onPostNotificationToken()}
     * @param notificationToken token used to identify this device
     */
    private void sendFirebaseNotificationToken(final String notificationToken) {
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                idToken = task.getResult().getToken();
                                if (idToken != null && notificationToken != null) {
                                    Map<String, String> params = new HashMap<>();
                                    params.put("idToken", idToken);
                                    params.put("registrationToken", notificationToken);
                                    PostJsonObjectRequest postNotificationTokenRequest = new PostJsonObjectRequest(onPostNotificationToken(), AppConfig.getInstance().postToken());
                                    RequestQueueSingleton.getInstance(context).addToRequestQueue(postNotificationTokenRequest.getRequest(params));
                                }
                            }
                        }
                    });
        }
    }

    private OnPostJsonObjectCallback onPostNotificationToken() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String message = VolleyErrorUtils.getGenericMessage(volleyError, activity);
                Log.d(TAG, message);
            }
        };
    }

    // Command Bars Methods ------------------------------------------------------------------------

    /**
     * Set on click listeners for the side bar buttons
     **/
    private void initMenuSideBarButtons() {

        //Menu bar
        ibtnHomeIcon.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //getTokenAndSendRegistration(firebaseUser);
                if (lvSector != null) {
                    lvSector.performItemClick(lvSector.getAdapter().getView(1, null, null), 1, lvSector.getAdapter().getItemId(1));
                }

            }
        });

        ibtnAddCubee.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Fragment scanCubeeFragment = ScanCubeesTabFragment.newInstance(SetUpWifiCubeeActivity.class.getName());
                changeCenterFragment(scanCubeeFragment);

            }
        });


    }

    // Sectors -------------------------------------------------------------------------------------

    /**
     * Request a list of existing sectors
     * Callback: {@link #onGetSectorsRequest()}
     */
    private void sectorListRequest() {
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                idToken = task.getResult().getToken();
                                Map<String, String> params = new HashMap<>();
                                params.put("userToken", idToken);
                                GetJsonArrayRequest getSectorsRequest = new GetJsonArrayRequest(onGetSectorsRequest(), AppConfig.getInstance().getSector());
                                RequestQueueSingleton.getInstance(context).addToRequestQueue(getSectorsRequest.getRequest(params));

                            } else {
                                ConnectionUtils.logout(activity);
                                //Log erro
                                Log.e("Main Activity", "error Get Token:" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                            }
                        }
                    });
        }
    }


    private OnGetJsonArrayCallback onGetSectorsRequest() {
        return new OnGetJsonArrayCallback() {
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                //Reflection
                Gson gson = new Gson();
                List<Sector> sectorList = new CopyOnWriteArrayList<>();
                for (JsonElement sector:jsonArray) {
                    Sector s = gson.fromJson(sector, Sector.class);
                    sectorList.add(s);
                }

                //Set list at controller
                sectorController.setSectorList(sectorList);

                //Init View
                initSectorAndFilterListView(sectorController.getSectorList());
            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                //If error, show message to user
                String myMessage = "Erro ao recuperar os setores e cubees";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }
        };
    }

    /**
     * Alert Dialog to show options:
     * Delete the selected sector
     * Add a cubee to the selected sector
     * Remove cubee from sector
     * @param selectedSector Sector selected to view options
     */
    private void alertDialogSectorManagement(final Sector selectedSector) {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle("Configurações do Setor");
        LayoutInflater li = getLayoutInflater();
        View view = li.inflate(R.layout.alert_sector_management, null);

        //DELETE THE SECTOR
        view.findViewById(R.id.btn_delete_sector).setOnClickListener(new View.OnClickListener() {
            public void onClick(View arg0) {
                alertDeleteSector(selectedSector);
                sectorOptionsAlertDialog.dismiss();
            }
        });

        //ADD A CUBEE TO SECTOR
        view.findViewById(R.id.btn_add_cubee).setOnClickListener(new View.OnClickListener() {
            public void onClick(View arg0) {
                AddCubeeToSectorListFragment cubeeListFragment = AddCubeeToSectorListFragment.newInstance(selectedSector.get_id());
                changeCenterFragment(cubeeListFragment);
                sectorOptionsAlertDialog.dismiss();
            }
        });

        //REMOVE CUBEE FROM SECTOR
        view.findViewById(R.id.btn_unregister_cubee).setOnClickListener(new View.OnClickListener() {
            public void onClick(View arg0) {
                RemoveCubeeToSectorListFragment cubeeListFragment = RemoveCubeeToSectorListFragment.newInstance("no_sector", selectedSector.get_id());
                changeCenterFragment(cubeeListFragment);
                sectorOptionsAlertDialog.dismiss();
            }
        });

        builder.setView(view);
        sectorOptionsAlertDialog = builder.create();
        sectorOptionsAlertDialog.show();
    }

    /**
     * Delete a sector request
     * Callback: {@link #onDeleteSectorCallback()}
     * @param selectedSector current sector
     */
    private void alertDeleteSector(final Sector selectedSector) {
        ActivityUtils.alertDialogSimpleDualButton(activity, "Alerta",
                "Você tem certeza que deseja excuir o Setor " + selectedSector.getName() + "?",
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        PostJsonObjectRequest deleteSectorRequest = new PostJsonObjectRequest(onDeleteSectorCallback(), AppConfig.getInstance().deleteSector());
                        Map<String, String> params = new HashMap<String, String>();
                        params.put("idSector", selectedSector.get_id());
                        params.put("idToken", idToken);
                        RequestQueueSingleton.getInstance(context).addToRequestQueue(deleteSectorRequest.getRequest(params));
                        ActivityUtils.showProgressDialog(activity, "Excluindo Setor");
                    }
                });
    }

    private OnPostJsonObjectCallback onDeleteSectorCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                //If sucess request for sector again to update view
                sectorListRequest();

                //Cancel progress and show message
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.showToast(context, "Setor excluído com sucesso");
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                //Cancel progress and show error message
                ActivityUtils.cancelProgressDialog();
                String myMessage = "Erro ao excluir setor";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }


        };
    }

    /**
     * Called when it's necessary to create a new sector.
     * Change central fragment to Register Sector Fragment
     */
    private void addSector() {
        Fragment addSectorFragment = RegisterSectorFragment.newInstance();
        changeCenterFragment(addSectorFragment);
    }

    /**
     * Init sector view.
     * @param sectorList sector list. Retrieved at {@link #sectorListRequest()}
     */
    private void initSectorAndFilterListView(List<Sector> sectorList) {
        lvSector = (ListView) findViewById(R.id.lv_sector);

        leftSideButtonsListAdapter = new LeftSideButtonsListAdapter(activity, sectorList);
        lvSector.setAdapter(leftSideButtonsListAdapter);

        lvSector.setOnItemClickListener(getItemClickListener());
        lvSector.performItemClick(lvSector.getAdapter().getView(1, null, null), 1, lvSector.getAdapter().getItemId(1));

        lvSector.setOnItemLongClickListener(onItemLongClickListener());

        lvFilters = (ListView) findViewById(R.id.lv_alarms_filter);

        filterAdapter = new AlarmFilterAdapter(activity);
        lvFilters.setAdapter(filterAdapter);
    }

    @NonNull
    private AdapterView.OnItemClickListener getItemClickListener() {
        return new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                leftSideButtonsListAdapter.setSelectedIndex(position);
                leftSideButtonsListAdapter.getItem(position);
                leftSideButtonsListAdapter.notifyDataSetChanged();

                switch (position) {
                    case 0: {
                        sectorSelected = 0;
                        addSector();
                        break;
                    }
                    case 1: {
                        sectorSelected = 1;
                        changeCenterFragmentGridFragment(leftSideButtonsListAdapter.getItem_id(position));
                        break;
                    }
                    case 2: {
                        sectorSelected = 2;
                        changeCenterFragmentGridFragment(leftSideButtonsListAdapter.getItem_id(position));
                        break;

                    }

                    default: {
                        sectorSelected = position;
                        changeCenterFragmentGridFragment(leftSideButtonsListAdapter.getItem_id(position));
                        break;
                    }
                }
            }
        };
    }

    private AdapterView.OnItemLongClickListener onItemLongClickListener() {
        return new AdapterView.OnItemLongClickListener() {
            @Override
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                if (position > 2) {
                    if (sectorSelected == position) {
                        final Sector selectedSector = (Sector) parent.getItemAtPosition(position);
                        alertDialogSectorManagement(selectedSector);
                    } else {
                        ActivityUtils.alertDialogSimple(activity, "Alerta", "Antes de configurar um setor, você deve selecioná-lo");
                    }
                }
                return true;
            }
        };
    }

    @Override
    public void onSectorAdded() {
        this.sectorListRequest();
    }

    // Change center fragments ---------------------------------------------------------------------
    /**
     * Implemented because of {@link ChangeCenterFragmentInterface}
     * Change central fragment to a given sector list fragment
     * @param itemName name of the sector to change the central fragment
     */
    public void changeCenterFragmentGridFragment(String itemName) {
        CubeeGridFragment cubeeListFragment = CubeeGridFragment.newInstance(itemName);
        currentFragment = cubeeListFragment;

        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();
        transaction.replace(R.id.fl_fragment_center, cubeeListFragment);
        try {
            transaction.commit();
        } catch (IllegalStateException ignored) {
            Log.d(TAG, "Error to change central fragment");
        }
    }

    /**
     * Implemented because of {@link ChangeCenterFragmentInterface}
     * Change central fragment to the current sector list fragment
     */
    public void changeCenterFragmentGridFragmentToSelected() {
        checkLeftBarVisibility(currentFragment);
        changeCenterFragmentGridFragment(leftSideButtonsListAdapter.getItem_id(sectorSelected));
    }

    /**
     * Implemented because of {@link ChangeCenterFragmentInterface}
     * Change central fragment to a given fragment
     * @param fragment Fragment to assume central position
     */
    public void changeCenterFragment(Fragment fragment) {
        FragmentTransaction transaction = getSupportFragmentManager().beginTransaction();

        checkLeftBarVisibility(fragment);

        transaction.replace(R.id.fl_fragment_center, fragment);
        try {
            transaction.commit();
        } catch (IllegalStateException ignored) {
            Log.d(TAG, "Error to change central fragment");
        }
    }

    private void checkLeftBarVisibility(Fragment fragment){
        FragmentManager fm = getSupportFragmentManager();

        if(fragment instanceof AlertListFragment){
            lvSector.setVisibility(View.GONE);
            lvFilters.setVisibility(View.VISIBLE);
        }else{
            lvSector.setVisibility(View.VISIBLE);
            lvFilters.setVisibility(View.GONE);
        }
    }

    /**
     * Change central fragment to the current sector list fragment
     */
    //TODO: Change this to interface
    public void backToCurrentSectorOrGrupe(){
        AppCompatActivity appCompatActivity = (AppCompatActivity) activity;
        FragmentTransaction transaction = appCompatActivity.getSupportFragmentManager().beginTransaction();
        transaction.replace(R.id.fl_fragment_center, currentFragment);
        try {
            transaction.commit();
        } catch (IllegalStateException ignored) {
            Log.d(TAG, "Error to change central fragment");
        }
    }

    /**
     * Listener to detect when the cellphone orientation change.
     *
     * @param orientation integer representing in degrees
     */
    @Override
    public void onOrientationChange(int orientation) {
        if (orientation == 90 || orientation == 270){
            // Nothing
        } else {
            CubeeInformationFragment myFragment = (CubeeInformationFragment) getSupportFragmentManager().findFragmentByTag("CUBEE_INFORMATION_FRAGMENT");
            if (myFragment != null && myFragment.isVisible()) {
                if(!CubeeController.getInstance().getCubeeAtual().getCubeeState()){
                    remoteControlDialog();
                }
            }
        }
    }

    /**
     * Dialog to enter remote control mode by changing orientation.
     */
    private void remoteControlDialog() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            builder = new android.support.v7.app.AlertDialog.Builder(activity, android.R.style.Theme_Material_Dialog_Alert);
        } else {
            builder = new android.support.v7.app.AlertDialog.Builder(activity);
        }
        if(myDialog == null){
            showRemoteControlDialog();
        }else if(!myDialog.isShowing()){
            showRemoteControlDialog();
        }

    }

    private void showRemoteControlDialog(){
        builder.setTitle("Controle remoto")
                .setMessage("Deseja entrar no modo controle remoto?")
                .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        Intent intent = new Intent(activity, RemoteControlActivity.class);
                        startActivity(intent);
                    }
                })
                .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // do nothing
                    }
                })
                .setIcon(android.R.drawable.ic_dialog_alert);
        myDialog = builder.create();
        myDialog.show();
    }
}