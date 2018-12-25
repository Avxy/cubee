package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.view.View;
import android.widget.AutoCompleteTextView;
import android.widget.EditText;

import com.android.volley.VolleyError;
import com.google.firebase.auth.FirebaseAuth;
import com.google.gson.JsonObject;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;

/**
 * Activity for register a user.
 * <p>
 * This activity is user to show a registrations form and send registration to server
 *
 **/
@SuppressWarnings({"ALL", "JavaDoc"})
public class RegisterUserActivity extends AppCompatActivity {

    /**
     * Edit text view to input user email
     */
    @SuppressWarnings("unused")
    @BindView(R.id.actv_email_register)
    public AutoCompleteTextView actvEmail;

    /**
     * Edit text view to input user password
     */
    @SuppressWarnings("unused")
    @BindView(R.id.ed_password_register)
    public EditText edtPassword;

    /**
     * Edit text view to input user password to confirm
     */
    @SuppressWarnings("unused")
    @BindView(R.id.confirm_password_register)
    public EditText edtConfirmPassword;

    /**
     * Edit text view to input user thelephone
     */
    @SuppressWarnings("unused")
    @BindView(R.id.number_register)
    public EditText edtThelephoneView;

    /**
     * Edit text view to input user name
     */
    @SuppressWarnings("unused")
    @BindView(R.id.name_register)
    public AutoCompleteTextView actvNameView;

    /**
     * Button view to click for register user
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_register_user)
    public View btnRegister;

    /**
     * Field to reference this aplicationContext
     */
    private Context context;

    /**
     * Field to reference this activity
     */
    private Activity activity;

    /**
     * Firebase authentication and registration object reference
     */
    @SuppressWarnings("unused")
    private FirebaseAuth firebaseAuth;

    /**
     * Bind view and set fields
     * @param savedInstanceState
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_sing_up);
        context = getApplicationContext();
        activity = this;
        firebaseAuth = FirebaseAuth.getInstance();
        ButterKnife.bind(this);
    }

    /**
     * ON CLICK - Register user
     */
    @SuppressWarnings("unused")
    @OnClick(R.id.btn_register_user)
    public void clickRegister() {
        attemptSingUp();
    }

    /**
     * Method to validate the form and send the registration if no error occurs at validation.
     **/
    private void attemptSingUp() {
        // Reset errors.
        actvEmail.setError(null);
        edtPassword.setError(null);
        actvNameView.setError(null);
        actvNameView.setError(null);
        edtConfirmPassword.setError(null);

        // Store values at the time of the login attempt.
        String email = actvEmail.getText().toString().trim();
        String password = edtPassword.getText().toString().trim();
        String confirmPassowrd = edtConfirmPassword.getText().toString().trim();
        String name = actvNameView.getText().toString().trim();
        String number = edtThelephoneView.getText().toString().trim();

        boolean cancel = false;
        View focusView = null;

        // Check for a valid password, if the user entered one.
        if (!TextUtils.isEmpty(password) && !isPasswordValid(password)) {
            edtPassword.setError(getString(R.string.error_invalid_password));
            focusView = edtPassword;
            cancel = true;
        }

        if (!password.equals(confirmPassowrd)) {
            edtConfirmPassword.setError(getString(R.string.confirm_password_error));
            focusView = edtConfirmPassword;
            cancel = true;
        }


        // Check for a valid email address.
        if (TextUtils.isEmpty(email)) {
            actvEmail.setError(getString(R.string.error_field_required));
            focusView = actvEmail;
            cancel = true;
        } else if (!isEmailValid(email)) {
            actvEmail.setError(getString(R.string.error_invalid_email));
            focusView = actvEmail;
            cancel = true;
        }

        // Check for a valid name.
        if (TextUtils.isEmpty(name)) {
            actvNameView.setError(getString(R.string.error_field_required));
            focusView = actvNameView;
            cancel = true;
        }

        //Check for a valid number
        if (TextUtils.isEmpty(number)) {
            edtThelephoneView.setError(getString(R.string.error_field_required));
            focusView = edtThelephoneView;
            cancel = true;
        }

        if (cancel) {
            focusView.requestFocus();
        } else {
            sendRegistration(email, name, password, number);
        }
    }

    /**
     * Send the segistrations to server
     *
     * @param email     User email
     * @param name      User name
     * @param password  User passwprd
     * @param telephone User telephone
     **/
    private void sendRegistration(String email, String name, String password, String telephone) {
        Map<String, String> params = new HashMap<>();
        params.put("name", name);
        params.put("email", email);
        params.put("telephone", telephone);
        params.put("password", password);

        ActivityUtils.showProgressDialog(activity, "Cadastrando usuário.");

        PostJsonObjectRequest postUserRegister = new PostJsonObjectRequest(postUserRegisterCallbacks(), AppConfig.getInstance().registerUser());
        RequestQueueSingleton.getInstance(this).addToRequestQueue(postUserRegister.getRequest(params));
    }

    /**
     * Method to register callbacks on server post register user operations
     * <p>
     * If Server Operation Sucess: Show toast and start LoginPasswordActivity
     * If Server Operation Error: Show toast
     **/
    private OnPostJsonObjectCallback postUserRegisterCallbacks() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.showToast(context, getString(R.string.register_user_success));
                Intent intent = new Intent(context, LoginAndBLETabsActivity.class);
                startActivity(intent);
                finish();
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                String myMessage = getString(R.string.register_user_error);
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, true));
            }
        };
    }

    /**
     * Email validation logic
     * @param email Email to verify
     * @return Return true if is a valid email, false otherwise
     **/
    private boolean isEmailValid(String email) {
        //TODO: Replace this with your own logic
        return email.contains("@");
    }

    /**
     * Password validation logic
     * @param password
     * @return Return true if is a valid password, false otherwise
     **/
    private boolean isPasswordValid(String password) {
        //TODO: Replace this with your own logic..
        return password.length() > 4;
    }

}
