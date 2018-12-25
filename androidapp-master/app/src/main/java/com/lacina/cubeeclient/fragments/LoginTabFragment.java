package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.support.v4.widget.SwipeRefreshLayout;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;

//import com.facebook.CallbackManager;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.LoginEmailPasswordActivity;
import com.lacina.cubeeclient.activities.MainActivity;
import com.lacina.cubeeclient.activities.RegisterUserActivity;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetCheckConnectionServerCallback;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.ConnectionUtils;

import butterknife.BindView;
import butterknife.ButterKnife;


@SuppressWarnings({"ALL", "JavaDoc"})
public class LoginTabFragment extends Fragment {

    private static final String KEY_ERROR = "KEY_ERROR";

    /**
     * Tag used to auxiliate the application log
     */
    private static final String TAG = "LoginActivityLauncher";

    private static final String AUTENTICATION_ERROR = "Autentication error";

    /**
     * Refresh layout view used to update the fragment informations
     */
    @SuppressWarnings("unused")
    @BindView(R.id.refresh)
    public SwipeRefreshLayout refreshLayout;

    /**
     * Field to represent this aplicationContext
     */
    private Context context;

    /**
     * Field to represent this activity
     */
    private Activity activity;

    @SuppressWarnings("unused")
    private Fragment fragment;

    /**
     * The entry point of the Firebase Authentication SDK.
     **/
    private FirebaseAuth mAuth;

    /**
     * Button view to login
     */
    private View mLoginButton;

    /**
     * Button view to create account
     */
    private View mCreateAccountButton;

    private final FirebaseAuth.AuthStateListener mAuthListener = new FirebaseAuth.AuthStateListener() {
        @Override
        public void onAuthStateChanged(@NonNull FirebaseAuth firebaseAuth) {
            FirebaseUser user = firebaseAuth.getCurrentUser();
            if (user != null) {
                Log.d(TAG, "onAuthStateChanged:signed_in:" + user.getUid());
                updateUI(user);
            } else {
                // User is signed out
                Log.d(TAG, "onAuthStateChanged:signed_out");
            }
        }
    };

    /**
     * Bind the view and set the click listeners
     *
     * @param inflater
     * @param container
     * @param savedInstanceState
     * @return
     */
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        final View fragmentView = inflater.inflate(R.layout.fragment_login, container, false);
        activity = getActivity();
        context = activity.getApplicationContext();
//        FacebookSdk.sdkInitialize(getActivity().getApplicationContext());
        ButterKnife.bind(this, fragmentView);

        fragment = this;

        refreshLayout.setOnRefreshListener(new SwipeRefreshLayout.OnRefreshListener() {
            @Override
            public void onRefresh() {
                checkConection();
                refreshLayout.setRefreshing(false);
            }
        });

        refreshLayout.setColorSchemeResources(R.color.colorAccent);


        mAuth = FirebaseAuth.getInstance();


        //VIEWS
        mLoginButton = fragmentView.findViewById(R.id.login_button);
        mCreateAccountButton = fragmentView.findViewById(R.id.create_account);

        //Clicked on sing up
        mLoginButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(context, LoginEmailPasswordActivity.class);
                startActivity(intent);
            }
        });

        //default register
        mCreateAccountButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(context, RegisterUserActivity.class);
                startActivity(intent);
            }
        });


        checkConection();
        return fragmentView;

    }

    /**
     * If you at any momment the token expires the user is redirected for this fragment to login again.
     **/
    @Override
    public void onResume() {
        super.onResume();
        if (activity.getIntent().getExtras() != null && !activity.getIntent().getExtras().isEmpty()) {
            String extra = activity.getIntent().getExtras().getString(KEY_ERROR);
            if (extra != null) {
                if (extra.equals(AUTENTICATION_ERROR)) {
                    ActivityUtils.alertDialogSimple(activity, getString(R.string.erro_autenticantion_title), getString(R.string.error_autentication_message));
                }
            }
        }
    }


    /**
     * If user is not null and server is on goes to the MainActivity.
     */
    private void updateUI(FirebaseUser user) {
        if (user != null) {
            ConnectionUtils.checkServerConnection(activity, new OnGetCheckConnectionServerCallback() {
                @Override
                public void onGetCheckConnectionServerResponse(boolean response) {
                    if(response){
                        Intent intent = new Intent(context, MainActivity.class);
                        FirebaseUser currentUser = mAuth.getCurrentUser();
                        if (currentUser != null) {
                            String uid = currentUser.getUid();
                            intent.putExtra("user_id", uid);
                            activity.startActivity(intent);
                            activity.finish();
                        }

                    }
                }
            });

        }
    }

    @Override
    public void onStart() {
        super.onStart();
        mAuth.addAuthStateListener(mAuthListener);
    }

    @Override
    public void onStop() {
        super.onStop();
        //noinspection ConstantConditions
        if (mAuthListener != null) {
            mAuth.removeAuthStateListener(mAuthListener);
        }
    }

    /**
     * Check if the server is on
     */
    private void checkConection() {
        ConnectionUtils.checkServerConnection(activity, new OnGetCheckConnectionServerCallback() {
            @Override
            public void onGetCheckConnectionServerResponse(boolean response) {
                activateLogin(response);

            }
        });
    }

    /**
     * Activate the buttons is the server is on.
     */
    private void activateLogin(Boolean activate) {
        mCreateAccountButton.setClickable(activate);
        mLoginButton.setClickable(activate);

        if (activate) {
            mCreateAccountButton.setAlpha(1f);
            mLoginButton.setAlpha(1f);
        } else {
            ActivityUtils.showToast(context, "Não foi possível se conectar com o servidor. Por favor, tente novamente.");
            mCreateAccountButton.setAlpha((float) 0.2);
            mLoginButton.setAlpha((float) 0.2);

        }

    }

}

