package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.os.Build;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;

import com.android.volley.VolleyError;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonObject;
import com.ikovac.timepickerwithseconds.MyTimePickerDialog;
import com.ikovac.timepickerwithseconds.TimePicker;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.Db9Rule;
import com.lacina.cubeeclient.model.State;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.ConnectionUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Activity to select areas and times to setup a new DB9 Rule
 */
@SuppressWarnings("ALL")
public class SelectAreasActivity extends AppCompatActivity {

    private static final int INITIAL_PAGE = 1;

    private static final int OUTPUTS_SIZE = 8;

    private static final String DEFAULT_INITIAL_TIME = "00:00:00";

    private static final String ON = "1";

    private static final String OFF = "0";

    private static final String SECONDS_ERROR_KEY = "seconds error";

    private static final String SAME_OUTPUT_ERROR_KEY = "same output error";

    private final String STATES_KEY = "states:";

    private final String TIME_KEY = "time:";

    private final String TOTAL_SECONDS = "totalSeconds:";

    /**
     * Each Radio Button below represents an output in a state.
     */

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.btnA1)
    public RadioButton btnA1;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.btnA2)
    public RadioButton btnA2;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.btnB1)
    public RadioButton btnB1;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.btnB2)
    public RadioButton btnB2;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.btnC1)
    public RadioButton btnC1;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.btnC2)
    public RadioButton btnC2;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.btnD1)
    public RadioButton btnD1;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.btnD2)
    public RadioButton btnD2;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.next)
    public ImageButton next;

    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.previous)
    public ImageButton previous;

    /**
     * Image button to delete a page.
     */
    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.delete_state)
    public ImageButton delete;

    /**
     * Button to send states.
     */
    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.finish)
    public Button finish;

    /**
     * Button to set the period of time that the current state will work.
     */
    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.stateTime)
    public Button stateTime;

    /**
     * TextView to show the number of pages.
     */
    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.currentPage)
    public TextView currentPageText;

    /**
     * Button to redirect to another activity that will have all DB9 Rules.
     */
    @SuppressWarnings({"WeakerAccess", "CanBeFinal", "unused"})
    @BindView(R.id.float_btn_select_areas)
    public FloatingActionButton goToListDB9rules;

    /**
     * Array List that will contain all radio buttons
     */
    private ArrayList<RadioButton> buttons;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    /**
     * Reference to this activity.
     */
    private Activity activity;

    /**
     * Flags to identify if a radio button is selected
     */
    private Boolean A1Flag, A2Flag, B1Flag, B2Flag, C1Flag, C2Flag, D1Flag, D2Flag;

    /**
     * Current state page
     */
    private int currentPage;

    /**
     * Identify how many pages were created.
     */
    private int currentMaxPage = 1;

    /**
     * Limit of pages
     */
    private final int maxPage = 5;

    /**
     * SharedPreferences to save informations about states.
     */
    private SharedPreferences sharedPreferences;

    private SharedPreferences.Editor editor;

    /**
     * AlertDialog builder
     */
    private AlertDialog.Builder builder;

    private AlertDialog.Builder builder_dialog_name;

    /**
     * idToken of firebase to authenticate with server.
     */
    private String idToken;

    /**
     * Current cubee selected.
     */
    private Cubee cubeeSelected;

    // Activity Methods ----------------------------------------------------------------------------
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_select_areas);

        ButterKnife.bind(this);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        activity = this;

        getBundleInformations();

        currentPage = INITIAL_PAGE;

        setupDialogs();
        populateArrayButtons();
        checkPageLimit();

        sharedPreferences = activity.getPreferences(Context.MODE_PRIVATE);
        editor = sharedPreferences.edit();
        editor.clear().commit();

        getIdToken();
    }

    /**
     * Get bundle informations and change floating button visibility according to current activity.
     * If cubeeSelected != null, user came from MainActivity.
     * If cubeeSelected == null, user came from ListDB9RulesActivity.
     *
     * PS: Button to add a new DB9 Rule in ListDB9RulesActivity was removed(setted to invisible in case of need to change).
     */
    private void getBundleInformations() {
        Bundle extras = getIntent().getExtras();
        if (extras != null) {
            cubeeSelected = (Cubee) extras.get("cubeeSelected");
            if(cubeeSelected != null){
                goToListDB9rules.setVisibility(View.VISIBLE);
            }else{
                goToListDB9rules.setVisibility(View.GONE);
            }
        }
    }

    /**
     * This method has all OnClickListeners.
     */
    private void onClicks() {

        next.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (currentPage != maxPage && currentPage != currentMaxPage) {
                    saveCurrentState();
                    currentPage++;
                    onChangePage();
                    currentPageText.setText(currentPage + "/" + currentMaxPage);
                } else if (currentPage == currentMaxPage && currentMaxPage < maxPage) {
                    if (stateTime.getText().equals(DEFAULT_INITIAL_TIME)) {
                        ActivityUtils.showToast(activity,
                                "Antes de prosseguir, preencha o campo de duração do estado");
                    } else {
                        showNewStateDialog();
                    }

                }
            }
        });

        previous.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (currentPage != INITIAL_PAGE) {
                    saveCurrentState();
                    currentPage--;
                    onChangePage();
                    currentPageText.setText(currentPage + "/" + currentMaxPage);
                }
            }
        });

        finish.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                saveCurrentState();
                if (validateAllStates()) {
                    setName();
                }
            }
        });

        delete.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showRemoveStateDialog();
            }
        });

        stateTime.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MyTimePickerDialog mTimePicker = new MyTimePickerDialog(activity, new MyTimePickerDialog.OnTimeSetListener() {
                    @Override
                    public void onTimeSet(TimePicker view, int hourOfDay, int minute, int seconds) {
                        stateTime.setText(String.format("%02d", hourOfDay) + ":" +
                                String.format("%02d", minute) + ":" + String.format("%02d", seconds));
                    }
                }, 0, 0, 0, true);
                mTimePicker.show();
            }
        });

        A1Flag = false;
        btnA1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                A1Flag = !A1Flag;
                btnA1.setChecked(A1Flag);
            }
        });

        A2Flag = false;
        btnA2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                A2Flag = !A2Flag;
                btnA2.setChecked(A2Flag);
            }
        });

        B1Flag = false;
        btnB1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                B1Flag = !B1Flag;
                btnB1.setChecked(B1Flag);
            }
        });

        B2Flag = false;
        btnB2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                B2Flag = !B2Flag;
                btnB2.setChecked(B2Flag);
            }
        });

        C1Flag = false;
        btnC1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                C1Flag = !C1Flag;
                btnC1.setChecked(C1Flag);
            }
        });

        C2Flag = false;
        btnC2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                C2Flag = !C2Flag;
                btnC2.setChecked(C2Flag);
            }
        });

        D1Flag = false;
        btnD1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                D1Flag = !D1Flag;
                btnD1.setChecked(D1Flag);
            }
        });

        D2Flag = false;
        btnD2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                D2Flag = !D2Flag;
                btnD2.setChecked(D2Flag);
            }
        });

        goToListDB9rules.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(activity, ListDb9RulesActivity.class);
                intent.putExtra("cubeeSelected", cubeeSelected);
                startActivity(intent);
            }
        });
    }

    /**
     * Firebase authentication
     */
    private void getIdToken() {
        firebaseUser.getIdToken(true)
                .addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                    public void onComplete(@NonNull Task<GetTokenResult> task) {
                        if (task.isSuccessful()) {
                            idToken = task.getResult().getToken();
                            onClicks();
                        } else {
                            ConnectionUtils.logout(activity);
                        }
                    }
                });
    }

    /**
     * Instantiate AlertDialog Builder.
     */
    private void setupDialogs() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            builder = new AlertDialog.Builder(activity, android.R.style.Theme_Material_Dialog_Alert);
            builder_dialog_name = new AlertDialog.Builder(activity, android.R.style.Theme_Material_Dialog_Alert);
        } else {
            builder = new AlertDialog.Builder(activity);
            builder_dialog_name = new AlertDialog.Builder(activity);
        }
    }

    /**
     * Create an Array List with all radio buttons
     */
    private void populateArrayButtons() {
        buttons = new ArrayList<>();
        buttons.add(btnA1);
        buttons.add(btnA2);
        buttons.add(btnB1);
        buttons.add(btnB2);
        buttons.add(btnC1);
        buttons.add(btnC2);
        buttons.add(btnD1);
        buttons.add(btnD2);
    }
    /**
     * Save outputs and time of the current state(page) on Shared Preferences.
     */
    private void saveCurrentState() {
        int currentOutput = getCurrentOutputValue();

        String currentTime = (String) stateTime.getText();
        String[] time = currentTime.split(":");

        int hours = Integer.parseInt(time[0]);
        int minutes = Integer.parseInt(time[1]);
        int seconds = Integer.parseInt(time[2]);
        int totalSeconds = seconds + (minutes * 60) + (hours * 3600);

        editor.putInt(STATES_KEY + currentPage, currentOutput);
        editor.putString(TIME_KEY + currentPage, currentTime);
        editor.putInt(TOTAL_SECONDS + currentPage, totalSeconds);
        editor.commit();
    }

    // Requests ------------------------------------------------------------------------------------

    /**
     * Send DB9 rule to server.
     * <p>
     * Callback: {@link #onPostDB9Rule()}
     */
    private void sendRule(String ruleName) {
        Map<String, String> params = new HashMap<>();

        ArrayList<State> states = createStates();
        Db9Rule rule = new Db9Rule(ruleName, states);

        Gson gson = new Gson();
        String jsonEvent = gson.toJson(rule);

        params.put("jsonDB9Rule", jsonEvent);
        params.put("idToken", idToken);

        PostJsonObjectRequest postJsonObjectRequest = new PostJsonObjectRequest(onPostDB9Rule(), AppConfig.getInstance().registerDb9Rule());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postJsonObjectRequest.getRequest(params));
    }

    /**
     * Callback after posting a DB9 Rule, will get id of the new rule and set to current cubee selected.
     *
     * @return OnPostJsonObjectCallBack
     */
    private OnPostJsonObjectCallback onPostDB9Rule() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                if(cubeeSelected != null){
                    try {
                        JSONObject object = new JSONObject(String.valueOf(jsonObject));
                        String ruleId = object.getString("_id");
                        addRuleToCubee(ruleId);
                    } catch (JSONException e) {
                        e.printStackTrace();
                    }
                }else {
                    ActivityUtils.showToast(activity, "Regra cadastrada com sucesso.");
                    ActivityUtils.cancelProgressDialog();
                    finish();
                }
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao cadastrar a regra.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }
        };
    }

    /**
     * Add a DB9 Rule to a Cubee.
     *
     * @param rule A DB9 Rule to be added to a Cubee.
     *             Callback: {@link #onSetDB9Rule()}
     */
    private void addRuleToCubee(String ruleId) {
        Map<String, String> params = new HashMap<>();

        params.put("ruleId", ruleId);
        params.put("idToken", idToken);
        params.put("cubeeId", cubeeSelected.get_id());

        PostJsonObjectRequest postJsonObjectRequest = new PostJsonObjectRequest(onSetDB9Rule(), AppConfig.getInstance().setDB9ToCubee());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postJsonObjectRequest.getRequest(params));
    }

    /**
     * Callback of adding a DB9 Rule to a Cubee.
     *
     * @return OnPostJsonObjectCallback
     */
    private OnPostJsonObjectCallback onSetDB9Rule() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.showToast(activity, "Regra cadastrada com sucesso.");
                ActivityUtils.cancelProgressDialog();
                finish();
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao cadastrar a regra.";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }
        };
    }

    // Checks --------------------------------------------------------------------------------------

    /**
     * Validate if all states are correct filled.
     *
     * @return Boolean: if true, all states are correct filled, if false aren't.
     */
    private boolean validateAllStates() {
        int currentOutput;
        int totalSeconds;
        int previousOutput = -1;
        for (int i = INITIAL_PAGE; i <= currentMaxPage; i++) {
            currentOutput = sharedPreferences.getInt(STATES_KEY + i, 0);
            totalSeconds = sharedPreferences.getInt(TOTAL_SECONDS + i, 0);
            if (totalSeconds == 0) {
                showFinishErrorDialog(SECONDS_ERROR_KEY);
                return false;
            } else if (currentOutput == previousOutput) {
                showFinishErrorDialog(SAME_OUTPUT_ERROR_KEY);
                return false;
            }
            previousOutput = currentOutput;
        }
        return true;
    }

    /**
     * Change buttons visibility according to the current page.
     */
    private void checkPageLimit() {
        if (currentPage == INITIAL_PAGE) {
            delete.setVisibility(View.INVISIBLE);
            previous.setVisibility(View.INVISIBLE);
        } else if (currentPage == maxPage) {
            delete.setVisibility(View.VISIBLE);
            next.setVisibility(View.INVISIBLE);
        } else if (currentPage == currentMaxPage) {
            delete.setVisibility(View.VISIBLE);
            previous.setVisibility(View.VISIBLE);
            next.setVisibility(View.VISIBLE);
        } else {
            delete.setVisibility(View.INVISIBLE);
            previous.setVisibility(View.VISIBLE);
            next.setVisibility(View.VISIBLE);
        }
    }

    /**
     * Change Radio Buttons outputs according to a binary string.
     *
     * @param binaryString 8 digit binary string(Ex: 00101001).
     */
    private void setOutputs(String binaryString) {
        String[] mString = binaryString.split("");
        int index = 1;

        for (RadioButton btn : buttons) {
            if (mString[index].equals(ON)) {
                btn.setChecked(true);
            } else if (mString[index].equals(OFF)) {
                btn.setChecked(false);
            }
            index++;
        }
        updateFlags();
    }

    /**
     * Update radio Buttons Flags according to current outputs.
     */
    private void updateFlags() {
        A1Flag = btnA1.isChecked();
        A2Flag = btnA2.isChecked();
        B1Flag = btnB1.isChecked();
        B2Flag = btnB2.isChecked();
        C1Flag = btnC1.isChecked();
        C2Flag = btnC2.isChecked();
        D1Flag = btnD1.isChecked();
        D2Flag = btnD2.isChecked();
    }

    /**
     * Set previous saved state time.
     */
    private void checkStateTime() {
        String currentTime = sharedPreferences.getString(TIME_KEY + currentPage, null);
        if (currentTime != null) {
            stateTime.setText(currentTime);
        } else {
            stateTime.setText(DEFAULT_INITIAL_TIME);
        }
    }

    /**
     * Set all Radio Buttons to false.
     */
    private void setAllStatesFalse() {
        for (RadioButton btn : buttons) {
            btn.setChecked(false);
        }
        updateFlags();
    }

    /**
     * This method is called to check which will be the next page
     * and what need to change in consequence of it.
     */
    private void onChangePage() {
        checkPageLimit();
        setAllStatesFalse(); // Reset flags
        loadState();
    }

    /**
     * Load previous state of current page.
     */
    private void loadState() {
        int currentOutput = sharedPreferences.getInt(STATES_KEY + currentPage, 0);

        String binaryString = Integer.toBinaryString(currentOutput);
        String binary8digits = fillBinaryString(binaryString);
        setOutputs(binary8digits);

        checkStateTime();
    }

    /**
     * This method construct a 8 digits binary string.
     *
     * @param binaryString Binary string to be filled.
     * @return 8 digit binary string.
     */
    private String fillBinaryString(String binaryString) {
        String temp = "";
        for (int i = 0; i < 8 - binaryString.length(); i++) {
            temp += "0";
        }
        temp += binaryString;

        return temp;
    }

    /**
     * Remove state dialog.
     */
    private void showRemoveStateDialog() {
        builder.setTitle("Excluir estado")
                .setMessage("Tem certeza que deseja excluir este estado?")
                .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        currentPage--;
                        currentMaxPage--;
                        onChangePage();
                        currentPageText.setText(currentPage + "/" + currentMaxPage);
                    }
                })
                .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // do nothing
                    }
                })
                .setIcon(android.R.drawable.ic_dialog_alert)
                .show();
    }

    /**
     * New state dialog.
     */
    private void showNewStateDialog() {
        builder.setTitle("Novo estado")
                .setMessage("Deseja adicionar outro estado?")
                .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        saveCurrentState();
                        currentMaxPage++;
                        currentPage++;
                        onChangePage();
                        currentPageText.setText(currentPage + "/" + currentMaxPage);
                    }
                })
                .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // do nothing
                    }
                })
                .setIcon(android.R.drawable.ic_dialog_alert)
                .show();
    }

    /**
     * This method show up a dialog to set the rule's name.
     */
    private void setName() {
        final EditText input = new EditText(this);
        LinearLayout.LayoutParams lp = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.MATCH_PARENT);
        input.setLayoutParams(lp);
        input.setTextColor(Color.WHITE);

        builder_dialog_name.setView(input);

        builder_dialog_name.setTitle("Nome")
                .setMessage("Digite um nome para a regra DB9")
                .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        String name = input.getText().toString();
                        if (name.length() > 0) {
                            sendRule(name);
                        } else {
                            ActivityUtils.showToast(activity, "O nome da regra deve conter pelo menos um caractere");
                        }
                    }
                })
                .setNegativeButton(android.R.string.no, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        // do nothing
                    }
                })
                .setIcon(0)
                .show();

    }

    /**
     * Create an Array List with all states after user press 'Finish'.
     *
     * @return ArrayList<State> states: contains all states.
     */
    private ArrayList<State> createStates() {
        int currentOutput;
        int totalSeconds;
        ArrayList<State> states = new ArrayList<>();
        for (int i = INITIAL_PAGE; i <= currentMaxPage; i++) {
            currentOutput = sharedPreferences.getInt(STATES_KEY + i, 0);
            totalSeconds = sharedPreferences.getInt(TOTAL_SECONDS + i, 0);
            State state = new State(totalSeconds, currentOutput);
            states.add(state);
        }

        return states;
    }

    /**
     * Finish erros dialogs.
     * SECONDS_ERROR: All state's times must be different then 0.
     * SAME_OUTPUT_ERROR: Two states in sequence must have different outputs.
     *
     * @param errorKey A string that will identify the type of error to show correct dialog.
     */
    private void showFinishErrorDialog(String errorKey) {
        if (errorKey.equals(SECONDS_ERROR_KEY)){
            builder.setTitle("Erro ao registrar regra")
                    .setMessage("Por favor, preencha a duração de todos os estados.")
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // do nothing
                        }
                    })
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .show();
        } else if (errorKey.equals(SAME_OUTPUT_ERROR_KEY)){
            builder.setTitle("Erro ao registrar regra")
                    .setMessage("Não é possível registrar dois estados com saídas iguais em sequência.")
                    .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                        public void onClick(DialogInterface dialog, int which) {
                            // do nothing
                        }
                    })
                    .setIcon(android.R.drawable.ic_dialog_alert)
                    .show();
        }
    }

    /**
     * According to Radio Buttons outputs, create a Int value based on binary system.
     *
     * @return Int: a value that corresponds to Radio Buttons outputs.
     */
    private int getCurrentOutputValue() {
        int byteValue = 0;
        int pot = OUTPUTS_SIZE - 1;
        for (RadioButton btn : buttons) {
            if (btn.isChecked()) {
                byteValue += Math.pow(2, pot);
            }
            pot--;
        }
        return byteValue;
    }


}
