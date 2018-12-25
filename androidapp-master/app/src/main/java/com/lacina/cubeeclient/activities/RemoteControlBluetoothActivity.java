package com.lacina.cubeeclient.activities;

import android.app.Activity;
import android.content.DialogInterface;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.RadioButton;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.model.InstantCommandBluetooth;
import com.lacina.cubeeclient.utils.SensorOrientationChangeNotifier;

import org.greenrobot.eventbus.EventBus;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;

public class RemoteControlBluetoothActivity extends AppCompatActivity implements SensorOrientationChangeNotifier.Listener {

    /**
     * Each Radio Button below represents an output in a state.
     */

    @BindView(R.id.control_btnA1)
    public RadioButton btnA1;

    @BindView(R.id.control_btnA2)
    public RadioButton btnA2;

    @BindView(R.id.control_btnB1)
    public RadioButton btnB1;

    @BindView(R.id.control_btnB2)
    public RadioButton btnB2;

    @BindView(R.id.control_btnC1)
    public RadioButton btnC1;

    @BindView(R.id.control_btnC2)
    public RadioButton btnC2;

    @BindView(R.id.control_btnD1)
    public RadioButton btnD1;

    @BindView(R.id.control_btnD2)
    public RadioButton btnD2;

    /**
     * Array List that will contain all radio buttons
     */
    private ArrayList<RadioButton> buttons;

    /**
     * Number of outputs(radio buttons).
     */
    private static final int OUTPUTS_SIZE = 8;

    /**
     * Reference to this activity.
     */
    private Activity activity;

    /**
     * Flags to identify if a radio button is selected.
     */
    private Boolean A1Flag, A2Flag, B1Flag, B2Flag, C1Flag, C2Flag, D1Flag, D2Flag;

    /**
     * AlertDialog builder to create dialogs.
     */
    private android.support.v7.app.AlertDialog.Builder builder;

    private AlertDialog myDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_remote_control_bluetooth);

        ButterKnife.bind(this);
        activity = this;

        populateArrayButtons();

        onClicks();
    }

    /**
     * Populate an ArrayList with all RadioButtons
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
     * This method has all OnClickListeners.
     */
    private void onClicks() {

        A1Flag = false;
        btnA1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                A1Flag = !A1Flag;
                btnA1.setChecked(A1Flag);
                sendOutput();
            }
        });

        A2Flag = false;
        btnA2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                A2Flag = !A2Flag;
                btnA2.setChecked(A2Flag);
                sendOutput();
            }
        });

        B1Flag = false;
        btnB1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                B1Flag = !B1Flag;
                btnB1.setChecked(B1Flag);
                sendOutput();
            }
        });

        B2Flag = false;
        btnB2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                B2Flag = !B2Flag;
                btnB2.setChecked(B2Flag);
                sendOutput();
            }
        });

        C1Flag = false;
        btnC1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                C1Flag = !C1Flag;
                btnC1.setChecked(C1Flag);
                sendOutput();
            }
        });

        C2Flag = false;
        btnC2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                C2Flag = !C2Flag;
                btnC2.setChecked(C2Flag);
                sendOutput();
            }
        });

        D1Flag = false;
        btnD1.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                D1Flag = !D1Flag;
                btnD1.setChecked(D1Flag);
                sendOutput();
            }
        });

        D2Flag = false;
        btnD2.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                D2Flag = !D2Flag;
                btnD2.setChecked(D2Flag);
                sendOutput();
            }
        });
    }

    /**
     * Send output by EventBus
     */
    private void sendOutput() {
        int currentOutput = getCurrentOutputValue();

        EventBus.getDefault().post(new InstantCommandBluetooth(currentOutput));
    }

    /**
     * According to Radio Buttons outputs, create a Int value based on binary system.
     *
     * @return Int: a value that corresponds to Radio Buttons outputs.
     */
    public int getCurrentOutputValue() {
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

    /**
     * Listener to detect when the cellphone orientation change.
     *
     * @param orientation integer representing in degrees
     */
    @Override
    public void onOrientationChange(int orientation) {
        if (orientation == 90 || orientation == 270){
            remoteControlDialog();
        } else {
            // Do nothing
        }
    }

    /**
     * Dialog to exit remote control mode by changing orientation.
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

    /**
     * Dialog when user try to exit.
     */
    public void showRemoteControlDialog(){
        builder.setTitle("Controle remoto")
                .setMessage("Deseja sair do modo controle remoto?")
                .setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        finish();
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

    @Override
    protected void onResume() {
        super.onResume();
        SensorOrientationChangeNotifier.getInstance(this).addListener(this);
    }

    @Override
    protected void onPause() {
        super.onPause();
        SensorOrientationChangeNotifier.getInstance(this).remove(this);
    }
}
