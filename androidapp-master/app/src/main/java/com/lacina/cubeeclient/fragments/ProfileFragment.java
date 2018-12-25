package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.FragmentTransaction;
import android.util.Log;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.User;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.old.OnGetUserInfoCallback;
import com.lacina.cubeeclient.serverConnection.requests.old.GetUserInfoRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;

import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;


/**
 * Fragment for control User Profile.
 * Connects with the server for get info.
 **/
@SuppressWarnings({"ALL", "JavaDoc"})
public class ProfileFragment extends BackableFragment {

    /**
     * Text view for the user name
     */
    @SuppressWarnings("unused")
    @BindView(R.id.name_register)
    public TextView tvNameView;

    /**
     * Text view for the user email
     */
    @SuppressWarnings("unused")
    @BindView(R.id.actv_email_register)
    public TextView tvEmailView;

    /**
     * Text view for the user thelephone
     */
    @SuppressWarnings("unused")
    @BindView(R.id.number_register)
    public TextView tvThelephoneView;

    /**
     * Button view to click for edit the user profile
     */
    @SuppressWarnings("unused")
    @BindView(R.id.edit_button)
    public Button btnEdit;

    /**
     * Image view for the user
     */
    @SuppressWarnings("unused")
    @BindView(R.id.cubee_icon)
    public ImageView imgUser;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    /**
     * idToken used in request to autenticate with backend
     */
    private String idToken;

    /**
     * Tag used to auxiliate the application log
     */
    private final String TAG = "ProfileFrag";


    @SuppressWarnings("unused")
    public ProfileFragment() {
    }


    public static ProfileFragment newInstance() {
        return new ProfileFragment();
    }

    /**
     * Set the firebaseUser
     *
     * @param savedInstanceState
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true)
                    .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                        public void onComplete(@NonNull Task<GetTokenResult> task) {
                            if (task.isSuccessful()) {
                                idToken = task.getResult().getToken();
                                Map<String, String> headers = new HashMap<>();
                                headers.put("idToken", idToken);
                                GetUserInfoRequest getUserInfo = new GetUserInfoRequest(getUserInfoCallback(), AppConfig.getInstance().getUser());
                                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getUserInfo.getRequest(headers));
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
     * Callback for get info
     * If sucess show info
     * If error show message to user
     **/
    private OnGetUserInfoCallback getUserInfoCallback() {
        return new OnGetUserInfoCallback() {
            @Override
            public void onGetUserInfoCallbackSuccess(User user) {
                if (user != null) {
                    tvNameView.setText(user.getName());
                    tvEmailView.setText(user.getEmail());
                    tvThelephoneView.setText(user.getTelephone());
                }
            }

            @Override
            public void onGetUserInfoCallbackError(String message) {
                ActivityUtils.showToast(activity, "Erro ao se conectar com o servidor. Tente de novo.");

            }

        };
    }

    /**
     * Bind the view and set the fields
     *
     * @param inflater
     * @param container
     * @param savedInstanceState
     * @return
     */
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        View fragmentView = inflater.inflate(R.layout.fragment_profile, container, false);
        activity = getActivity();
        ButterKnife.bind(this, fragmentView);
        createClickLinesters();

        fragmentView.setFocusableInTouchMode(true);
        fragmentView.requestFocus();
        fragmentView.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View v, int keyCode, KeyEvent event) {
                if (keyCode == KeyEvent.KEYCODE_BACK) {
                    Log.i(TAG, "onKey Back listener is working!!!");
                    FragmentTransaction ft = getFragmentManager().beginTransaction();
                    ft.replace(R.id.fl_fragment_center, CubeeGridFragment.newInstance(null));
                    ft.commit();
                    return true;
                } else {
                    return false;
                }
            }
        });

        return fragmentView;

    }

    /**
     * On click listener for the edit button
     */
    private void createClickLinesters() {
        btnEdit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                FragmentTransaction ft = getFragmentManager().beginTransaction();
                ft.replace(R.id.fl_fragment_center, EditProfileFragment.newInstance());
                ft.commit();
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
