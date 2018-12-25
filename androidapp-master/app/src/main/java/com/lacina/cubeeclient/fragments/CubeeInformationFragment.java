package com.lacina.cubeeclient.fragments;

import android.app.Activity;
import android.app.DatePickerDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v7.app.AlertDialog;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.DatePicker;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.NumberPicker;
import android.widget.TextView;

import com.android.volley.VolleyError;
import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.android.gms.tasks.Task;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.google.firebase.auth.GetTokenResult;
import com.google.gson.Gson;
import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.reflect.TypeToken;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.activities.GraphExpandActivity;
import com.lacina.cubeeclient.activities.MainActivity;
import com.lacina.cubeeclient.controllers.CubeeController;
import com.lacina.cubeeclient.interfaces.ChangeCenterFragmentInterface;
import com.lacina.cubeeclient.model.Cubee;
import com.lacina.cubeeclient.model.CubeeMeasurement;
import com.lacina.cubeeclient.serverConnection.AppConfig;
import com.lacina.cubeeclient.serverConnection.RequestQueueSingleton;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonArrayCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnGetJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.callbacks.OnPostJsonObjectCallback;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonArrayRequest;
import com.lacina.cubeeclient.serverConnection.requests.GetJsonObjectRequest;
import com.lacina.cubeeclient.serverConnection.requests.PostJsonObjectRequest;
import com.lacina.cubeeclient.utils.ActivityUtils;
import com.lacina.cubeeclient.utils.VolleyErrorUtils;

import java.lang.reflect.Type;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Activity to show information about a specific Cubee.
 */
@SuppressWarnings({"ALL", "JavaDoc"})
public class CubeeInformationFragment extends BackableFragment {

    private final String TAG = "CubeeInformationFrag";

    /**
     * Signal button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_signal_cubee)
    public Button btnSignalCubee;

    /**
     * Exit button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_exit)
    public ImageButton ibtnExit;

    /**
     * Cubee Relay status
     */
    @SuppressWarnings("unused")
    @BindView(R.id.cubee_status)
    public TextView tvCubeeStatus;

    /**
     * Cubee text view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.cubee_name)
    public TextView tvCubeeName;

    /**
     * Finish date button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_finish_date_filter)
    public Button btnFinishDateInterval;

    /**
     * Activate and deactivate cubee relay button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.activate_deactivate_cubee)
    public Button btnActiveDeactiveCubee;

    /**
     * Init date for filter select button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_init_date_filter)
    public Button btnInitDateInterval;

    /**
     * Send filter to server button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_send_filter)
    public Button btnSendFilter;

    /**
     * Edit threshold button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_threshold)
    public Button btnEditThreshold;

    /**
     * Text view to show init date to filter
     */
    @SuppressWarnings("unused")
    @BindView(R.id.tv_init_date_interval)
    public TextView tvInitIntervalDate;

    /**
     * Text view to show finish date to filter
     */
    @SuppressWarnings("unused")
    @BindView(R.id.tv_finish_date_interval)
    public TextView tvFinishIntervalDate;

    /**
     * Delete cubee button view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.btn_delete_cubee)
    ImageButton ibtnDeleteCubee;

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    private FirebaseUser firebaseUser;

    /**
     * Flag to request just once
     */
    private boolean flagFirstRequest = true;

    /**
     * User idToken to authenticate with server
     */
    private String idToken;

    /**
     * Field to reference application context.
     */
    private Context context;

    /**
     * Field to reference this activity.
     */
    private Activity activity;

    /**
     * Cubee no_sector_selected.
     */
    private Cubee cubeeAtual;

    /**
     * Timer used to loop a request to see if a command has successfully sended
     */
    private Timer timer;

    private Boolean changeCommandFlag;

    private int contTime;

    private View fragmentView;

    private ProgressDialog progressDialogDateMeasurement;

    private CubeeMeasurement cubeeMeasurement;

    private Calendar finishCalendar;

    private Calendar initCalendar;


    //Fragment Methods ----------------------------------------------------
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        this.activity = getActivity();
        this.context = activity.getApplicationContext();
        changeCommandFlag = false;

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        fragmentView = inflater.inflate(R.layout.fragment_cubee_informations, container, false);
        ButterKnife.bind(this, fragmentView);


        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();
        if (firebaseUser != null) {
            firebaseUser.getIdToken(true).addOnCompleteListener(new OnCompleteListener<GetTokenResult>() {
                public void onComplete(@NonNull Task<GetTokenResult> task) {
                    if (task.isSuccessful()) {
                        idToken = task.getResult().getToken();
                        Log.i("TOKEN", idToken);
                    } else {
                        //noinspection ThrowableResultOfMethodCallIgnored,ThrowableResultOfMethodCallIgnored
                        Log.e(TAG, "Error Get token" + (task.getException() == null ? "null exception" : task.getException().getMessage()));
                    }
                }
            });
        }

        //sendRequestCubeeMeasurement();
        initButtons();
        setCurrentDateOnView();

        return fragmentView;
    }

    /**
     * At On Resume retrieve actual cubee.
     * {@link CubeeController}
     */
    @Override
    public void onResume() {
        super.onResume();
        cubeeAtual = CubeeController.getInstance().getCubeeAtual();
        tvCubeeName.setText(cubeeAtual.getName());
        sendRequestCubeeMeasurement();
        ActivityUtils.showProgressDialog(activity, "Atualizando CUBEE");
        flagFirstRequest = true;
        initLoop();


    }

    /**
     * At On Destroy set actual cubee to null
     * {@link CubeeController}
     */
    @Override
    public void onDestroy() {
        super.onDestroy();
        CubeeController.getInstance().setCubeeAtual(null);
        cancelTimer();
    }

    @Override
    public void onPause() {
        super.onPause();
        cancelTimer();
    }

    @Override
    public void onBackButtonPressed() {
        if (activity instanceof ChangeCenterFragmentInterface) {
            ((ChangeCenterFragmentInterface) activity).changeCenterFragmentGridFragmentToSelected();
        }

    }

    //Inits Buttons---------------------------------------------------------------------------------
    private void initButtons() {
        //EXIT BUTTON
        ibtnExit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                MainActivity mainActivity = (MainActivity) activity;
                mainActivity.backToCurrentSectorOrGrupe();
                //MainActivity.backToCurrentSectorOrGrupe()
            }
        });

        //DELETE
        ibtnDeleteCubee.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                ActivityUtils.showProgressDialog(activity, "Carregando.");
                @SuppressWarnings("Convert2Diamond") Map<String, String> headers = new HashMap<String, String>();
                headers.put("idCubee", cubeeAtual.get_id());
                headers.put("idToken", idToken);
                GetJsonArrayRequest taskRequest = new GetJsonArrayRequest(onGetTasksByCubee(), AppConfig.getInstance().getTasksByCubee());
                RequestQueueSingleton.getInstance(activity).addToRequestQueue(taskRequest.getRequest(headers));
            }
        });

        //RELAY BUTTON
        btnActiveDeactiveCubee.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                @SuppressWarnings("Convert2Diamond") Map<String, String> params = new HashMap<String, String>();
                params.put("idCubee", CubeeController.getInstance().getCubeeAtual().get_id());
                params.put("idToken", idToken);
                GetJsonObjectRequest getJsonObjectRequest = new GetJsonObjectRequest(getIdEventsCallback(), AppConfig.getInstance().getIdEventsByCubee());
                RequestQueueSingleton.getInstance(activity).addToRequestQueue(getJsonObjectRequest.getRequest(params));

            }
        });


        //SENG SIGNAL
        btnSignalCubee.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                sendRequestCommandCubee(3, null);
                ActivityUtils.showToast(context, "Sinal enviado");
            }
        });


        //SET INIT DATE IN FILTER
        btnInitDateInterval.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                showCalendarPicker(initCalendar, finishCalendar, tvInitIntervalDate);
            }

        });


        //SET
        btnFinishDateInterval.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                showCalendarPicker(finishCalendar, initCalendar, tvFinishIntervalDate);
            }

        });

        //SET MESUREMENT FILTER
        btnSendFilter.setOnClickListener(new View.OnClickListener() {

            @Override
            public void onClick(View v) {
                //ActivityUtils.showToast(context, "Carregando.");

                Long interval = (finishCalendar.getTime().getTime() - initCalendar.getTime().getTime()) / (1000 * 60 * 60);
                if (finishCalendar.before(initCalendar)) {
                    ActivityUtils.showToast(context, "A data final não pode ser menor do que a data inicial.");
                } else if (interval <= 0) {
                    sendRequestCubeeMeasurement();
                } else if (interval > 0 && interval <= 48) {
                    ActivityUtils.alertDialogSimpleCallbackBtnMessages(activity, "Alerta", "Você está enviando um intervalo de tempo maior do que 1h. Portanto, sua resposta será medições por hora."
                            , "Cancel", "OK", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    sendRequestCubeeMeasurement();
                                }
                            });
                } else if (interval > 48 && (interval / 24) <= 62) {
                    ActivityUtils.alertDialogSimpleCallbackBtnMessages(activity, "Alerta", "Você está enviando um intervalo de tempo maior do que 48h. Portanto, sua resposta será medições por dia."
                            , "Cancel", "OK", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    changeFormatDate("HOUR", initCalendar, tvInitIntervalDate);
                                    changeFormatDate("HOUR", finishCalendar, tvFinishIntervalDate);
                                    sendRequestCubeeMeasurement();
                                }
                            });
                } else {
                    ActivityUtils.alertDialogSimpleCallbackBtnMessages(activity, "Alerta", "Você está enviando um intervalo de tempo maior do que 62 dias. Portanto, sua resposta será medições por mês."
                            , "Cancel", "OK", new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    changeFormatDate("DAY_OF_MONTH", initCalendar, tvInitIntervalDate);
                                    changeFormatDate("DAY_OF_MONTH", finishCalendar, tvFinishIntervalDate);
                                    sendRequestCubeeMeasurement();
                                }
                            });
                }

            }

        });


        //EDIT TRESHOLD
        btnEditThreshold.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(activity);
                LayoutInflater inflater = activity.getLayoutInflater();
                final View dialogView = inflater.inflate(R.layout.set_threshold_custom_dialog, null);
                dialogBuilder.setView(dialogView);

                final EditText edtUpperThreshold = (EditText) dialogView.findViewById(R.id.edtv_upper_threshold);
                final EditText edtLowerThreshold = (EditText) dialogView.findViewById(R.id.edtv_lower_threshold);

                dialogBuilder.setTitle("Limites");
                dialogBuilder.setMessage("Por favor, insira os limites superiores e inferiores nos campos abaixo.");

                if (cubeeAtual.getUpperThreshold() != null && cubeeAtual.getLowerThreshold() != null) {
                    Log.i(TAG, cubeeAtual.toString());
                    edtLowerThreshold.setText(cubeeAtual.getLowerThreshold() + "");
                    edtUpperThreshold.setText(cubeeAtual.getUpperThreshold() + "");
                }


                dialogBuilder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        final String upperThreshold = edtUpperThreshold.getText().toString().trim();
                        final String lowerThreshold = edtLowerThreshold.getText().toString().trim();
                        if (edtLowerThreshold.getText().toString().isEmpty() || edtLowerThreshold.getText().toString().isEmpty()) {
                            ActivityUtils.alertDialogSimple(activity, "ALERTA", "Existem campos que não foram preenchidos.");
                            //dialog.dismiss();
                        } else {
                            Integer upperThresholdInteger;
                            Integer lowerThresholdInteger;
                            try {
                                upperThresholdInteger = Integer.parseInt(upperThreshold);
                                lowerThresholdInteger = Integer.parseInt(lowerThreshold);

                                if (lowerThresholdInteger > upperThresholdInteger) {
                                    ActivityUtils.alertDialogSimple(activity, "ALERTA", "O limite inferior não pode ser menor que o limite superior");
                                    //dialog.dismiss();
                                } else {
                                    sendThresholdLimits(upperThreshold, lowerThreshold);
                                }
                            } catch (Exception e) {
                                ActivityUtils.alertDialogSimple(activity, "Alerta", "Por favor, digite números inteiros nos campos.");
                            }
                        }
                    }
                });
                dialogBuilder.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        //pass
                    }
                });
                AlertDialog b = dialogBuilder.create();
                b.show();

            }
        });
    }

    /**
     * Request to set threshold limits at server
     * CALLBACK: {@link #onPostThresholdLimits()}
     * @param upperThreshold Upper limit, if this limit is surpassed a notification is sended
     * @param lowerThreshold Lower limit, if this limit is surpassed a notification is sended
     */
    private void sendThresholdLimits(String upperThreshold, String lowerThreshold) {
        ActivityUtils.showProgressDialog(activity, "Enviando dados");
        Map<String, String> params = new HashMap<>();
        params.put("idToken", idToken);
        params.put("idCubee", cubeeAtual.get_id());
        params.put("upperThreshold", upperThreshold);
        params.put("lowerThreshold", lowerThreshold);
        PostJsonObjectRequest postJsonObjectRequest = new PostJsonObjectRequest(onPostThresholdLimits(), AppConfig.getInstance().postThreshold());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postJsonObjectRequest.getRequest(params));
    }


    private OnPostJsonObjectCallback onPostThresholdLimits() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                Gson gson = new Gson();
                Cubee cubee = gson.fromJson(jsonObject, Cubee.class);
                CubeeController.getInstance().setCubeeAtual(cubee);
                Log.i(TAG, "Cubee thresholds = " + cubee.toString());
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.showToast(context, "Limiares alterados com sucesso");
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.showToast(context, "Erro ao alterar os limiares");

            }
        };
    }

    private void showCalendarPicker(final Calendar cal1, final Calendar cal2, final TextView tv) {

        changeFormatDate("", initCalendar, tvInitIntervalDate);
        changeFormatDate("", finishCalendar, tvFinishIntervalDate);


        DatePickerDialog dialog = new DatePickerDialog(activity, new DatePickerDialog.OnDateSetListener() {
            @Override
            public void onDateSet(DatePicker view, int year, int month, int dayOfMonth) {

                Calendar c = (Calendar) cal1.clone();
                c.set(Calendar.DAY_OF_MONTH, dayOfMonth);
                c.set(Calendar.MONTH, month);
                c.set(Calendar.YEAR, year);
                c.set(Calendar.HOUR_OF_DAY, 0);
                c.set(Calendar.MINUTE, 0);
                c.set(Calendar.SECOND, 0);
                c.set(Calendar.MILLISECOND, 0);

                Calendar c2 = (Calendar) cal2.clone();
                c2.set(Calendar.HOUR_OF_DAY, 0);
                c2.set(Calendar.MINUTE, 0);
                c2.set(Calendar.SECOND, 0);
                c2.set(Calendar.MILLISECOND, 0);

                showHourPicker(dayOfMonth, month, year, cal1, tv);
            }
        }, cal1.get(Calendar.YEAR), cal1.get(Calendar.MONTH), cal1.get(Calendar.DAY_OF_MONTH));
        dialog.show();


    }

    private void showHourPicker(final int dayOfMonth, final int month, final int year, final Calendar c1, final TextView tv) {


        final Dialog d = new Dialog(activity);
        d.setTitle("Horas");
        d.setContentView(R.layout.dialog_number_picker);
        Button btmSetHour = (Button) d.findViewById(R.id.btm_set_hour);
        Button btmCancel = (Button) d.findViewById(R.id.btm_cancel);
        final NumberPicker np = (NumberPicker) d.findViewById(R.id.numberPicker);
        np.setMaxValue(23); // max value 23
        np.setMinValue(0);   // min value 0
        np.setWrapSelectorWheel(false);
        np.setValue(c1.get(Calendar.HOUR_OF_DAY));
        d.setCanceledOnTouchOutside(false);
        d.setCancelable(false);

        final int oldHour = c1.get(Calendar.HOUR_OF_DAY);
        np.setOnValueChangedListener(new NumberPicker.OnValueChangeListener() {
            @Override
            public void onValueChange(NumberPicker picker, int oldVal, int newVal) {
                c1.set(Calendar.HOUR_OF_DAY, newVal);
            }
        });
        btmSetHour.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                c1.set(Calendar.DAY_OF_MONTH, dayOfMonth);
                c1.set(Calendar.MONTH, month);
                c1.set(Calendar.YEAR, year);
                c1.set(Calendar.MINUTE, 0);
                c1.set(Calendar.SECOND, 0);
                c1.set(Calendar.MILLISECOND, 0);


                int monthString = c1.get(Calendar.MONTH) + 1;
                tv.setText(c1.get(Calendar.DAY_OF_MONTH) + "/" + monthString + "/" + c1.get(Calendar.YEAR) + " - " + c1.get(Calendar.HOUR_OF_DAY) + "h");
                d.dismiss();

            }
        });

        btmCancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                c1.set(Calendar.HOUR_OF_DAY, oldHour);
                d.dismiss(); // dismiss the dialog

            }
        });
        d.show();


    }

    @SuppressWarnings("IfCanBeSwitch")
    private void changeFormatDate(String paraterToErase, Calendar c, TextView tv) {

        String result;
        int month = c.get(Calendar.MONTH) + 1;

        if (paraterToErase.equals("")) {
            result = c.get(Calendar.DAY_OF_MONTH) + "/" + month + "/" + c.get(Calendar.YEAR) + " - " + c.get(Calendar.HOUR_OF_DAY) + "h";
        } else if (paraterToErase.equals("HOUR")) {
            c.set(Calendar.HOUR_OF_DAY, 0);
            result = c.get(Calendar.DAY_OF_MONTH) + "/" + month + "/" + c.get(Calendar.YEAR);
        } else {
            c.set(Calendar.HOUR_OF_DAY, 0);
            result = month + "/" + c.get(Calendar.YEAR);
        }


        tv.setText(result);

    }

    private void deleteCubee() {
        Map<String, String> params = new HashMap<>();
        params.put("idCubee", cubeeAtual.get_id());
        params.put("idToken", idToken);
        //params.put("idEvent", idEvent);
        PostJsonObjectRequest deleteCubee = new PostJsonObjectRequest(onDeleteCubeeCallback(), AppConfig.getInstance().deleteCubee());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(deleteCubee.getRequest(params));
        ActivityUtils.showProgressDialog(activity, "Excluindo CUBEE");
    }

    /**
     * Callbacks to the request.
     * If Sucess: Deelete the CUBEE, show a message to user and go to the previous activity
     * If Error:  Don't delethe the CUBEE and show message to user.
     **/
    private OnPostJsonObjectCallback onDeleteCubeeCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {
                ActivityUtils.cancelProgressDialog();
                ActivityUtils.alertDialogSimple(activity, "CUBEE Excluído", "CUBEE " + cubeeAtual.getName() + " excluído com sucesso");
                MainActivity mainActivity = (MainActivity) activity;
                mainActivity.backToCurrentSectorOrGrupe();

                //MainActivity.backToCurrentSectorOrGrupe();
            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                String message = VolleyErrorUtils.getGenericMessage(volleyError, activity);
                Log.d(TAG, message);
                ActivityUtils.alertDialogSimple(activity, "ERRO", "Erro ao excluir o CUBEE " + cubeeAtual.getName() +
                        ". Por favor, tente novamente");
            }


        };
    }

    //Graphic---------------------------------------------------------------------------------------

    private void initGraphic(View view) {


        CubeeMeasurement cubeeMeasurement = CubeeController.getInstance().getCubeeMeasurement();


        LineChart lineChart = (LineChart) view.findViewById(R.id.chart);
        //ArrayList<String> labels = new ArrayList<String>();
        ArrayList<Entry> entries = new ArrayList<>();
        for (int i = 0; i < cubeeMeasurement.getValuesMeasurements().size(); i++) {
            try{
                entries.add(new Entry(Float.parseFloat(cubeeMeasurement.getValuesMeasurements().get(i)), i));
            }catch (Exception e){
                //OK
            }
        }
        LineDataSet dataset = new LineDataSet(entries, "# of Calls");

        LineData data = new LineData(cubeeMeasurement.getKeyMeasurements(), dataset);
        dataset.setColor(Color.BLACK);

        //dataset.setColors(ColorTemplate.COLORFUL_COLORS); //
        lineChart.getXAxis().setTextColor(Color.WHITE);
        lineChart.getAxisLeft().setTextColor(Color.WHITE);
        lineChart.getAxisRight().setTextColor(Color.WHITE);

        lineChart.setDescription("");
        dataset.setDrawFilled(true);

        lineChart.setData(data);
        lineChart.animateY(5000);

        lineChart.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent i = new Intent(activity, GraphExpandActivity.class);
                activity.startActivity(i);

            }
        });


    }

    //Requests -----------------------------------------------------------------------------

    /**
     * ON CLICK - Activate actual cubee
     * {@link CubeeController}
     *
     * @param jsonElements
     */
    private void activateCubee(JsonArray jsonElements) {
        changeCommandFlag = true;
        contTime = 0;
        initLoop();
        ActivityUtils.showProgressDialog(activity, "Ativando CUBEE");
        sendRequestCommandCubee(1, jsonElements);
    }

    private void setCurrentDateOnView() {


        initCalendar = Calendar.getInstance();
        finishCalendar = Calendar.getInstance();
        // set current date into textview
        String initDataString = initCalendar.get(Calendar.DAY_OF_MONTH) + "/" + (initCalendar.get(Calendar.MONTH) + 1) + "/" + initCalendar.get(Calendar.YEAR) + " - " + initCalendar.get(Calendar.HOUR_OF_DAY) + "h";
        tvInitIntervalDate.setText(initDataString);
        tvFinishIntervalDate.setText(initDataString);


    }

    private void deactivateCubee(JsonArray jsonElements) {
        changeCommandFlag = true;
        contTime = 0;
        initLoop();
        ActivityUtils.showProgressDialog(activity, "Desativando CUBEE");
        sendRequestCommandCubee(2, jsonElements);
    }

    /**
     * Create and send request to set AppCommand flag.
     *
     * @param command      true for activate cubee and false to deactivate actual cubee
     * @param jsonElements
     */
    private void sendRequestCommandCubee(final Integer command, JsonArray jsonElements) {
        if (command != 1) {
            changeCommandFlag = true;
        }
        Map<String, String> params = new HashMap<>();

        if (jsonElements != null) {
            params.put("listOfIdEvents", jsonElements.toString());
        }
        params.put("idCubee", cubeeAtual.get_id());
        params.put("idToken", idToken);
        params.put("command", Integer.toString(command));
        PostJsonObjectRequest postCubeeActivate = new PostJsonObjectRequest(onPostCubeeCommandCallback(), AppConfig.getInstance().setCommandCubee());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(postCubeeActivate.getRequest(params));
    }

    private void refreshActualCubee() {
        Map<String, String> headers = new HashMap<>();
        headers.put("idToken", idToken);
        headers.put("idCubee", cubeeAtual.get_id());
        GetJsonObjectRequest getCubeeByIdRequest = new GetJsonObjectRequest(onGetCubeeByIdCallback(), AppConfig.getInstance().getCubeeById());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(getCubeeByIdRequest.getRequest(headers));
    }

    private void sendRequestCubeeMeasurement() {
        progressDialogDateMeasurement = new ProgressDialog(activity);
        progressDialogDateMeasurement.setMessage("Carregando.");
        progressDialogDateMeasurement.setCanceledOnTouchOutside(false);
        progressDialogDateMeasurement.show();


        int initMonth = initCalendar.get(Calendar.MONTH) + 1;
        String initDateString = initMonth + "/" + initCalendar.get(Calendar.DAY_OF_MONTH) + "/" + initCalendar.get(Calendar.YEAR);
        int finishMonth = finishCalendar.get(Calendar.MONTH) + 1;
        String finishDateString = finishMonth + "/" + finishCalendar.get(Calendar.DAY_OF_MONTH) + "/" + finishCalendar.get(Calendar.YEAR);


        @SuppressWarnings("Convert2Diamond") Map<String, String> headers = new HashMap<String, String>();
        headers.put("idCubee", CubeeController.getInstance().getCubeeAtual().get_id());
        headers.put("idToken", idToken);
        headers.put("initDate", initDateString);
        headers.put("initHour", initCalendar.get(Calendar.HOUR_OF_DAY) + "");

        headers.put("finishDate", finishDateString);
        headers.put("finishHour", finishCalendar.get(Calendar.HOUR_OF_DAY) + "");

        GetJsonObjectRequest getJsonArrayRequest = new GetJsonObjectRequest(onGetMeasurementCallback(), AppConfig.getInstance().getCubeeMeasurement());
        RequestQueueSingleton.getInstance(activity).addToRequestQueue(getJsonArrayRequest.getRequest(headers));
    }

    //Callbacks ----------------------------------------------------------------------------

    private OnGetJsonObjectCallback onGetMeasurementCallback() {
        return new OnGetJsonObjectCallback() {
            @Override
            public void onGetJsonObjectCallbackSuccess(JsonObject jsonMeasurement) {
                Gson gson = new Gson();
                Type type = new TypeToken<List<String>>() {
                }.getType();
                ArrayList<String> labels = gson.fromJson(jsonMeasurement.get("keys"), type);
                ArrayList<String> values = gson.fromJson(jsonMeasurement.get("values"), type);
                cubeeMeasurement = new CubeeMeasurement(cubeeAtual.get_id(), labels, values);
                CubeeController.getInstance().setCubeeMeasurement(cubeeMeasurement);
                progressDialogDateMeasurement.cancel();

                if (values.size() == 0) {
                    ActivityUtils.showToastLong(context, "Você não tem medições no intervalo de tempo definido.");
                }
                initGraphic(fragmentView);
            }

            @Override
            public void onGetJsonObjectCallbackError(VolleyError volleyError) {
                progressDialogDateMeasurement.cancel();
                String myMessage = "Um erro ao recuperar medições ocorreu. Por favor, tente novamente. ";
                ActivityUtils.showToastLong(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));

            }
        };
    }

    private OnGetJsonObjectCallback getIdEventsCallback() {
        return new OnGetJsonObjectCallback() {
            @Override
            public void onGetJsonObjectCallbackSuccess(JsonObject jsonObject) {
                final JsonArray jsonElements = jsonObject.getAsJsonArray("listOfIdEvents");
                if (jsonElements.size() == 0) {
                    toggleCubee(null);
                } else {
                    String message = "Você possui eventos acontecendo neste momento. Se você continuar estes eventos serão desabilitados.";
                    if (jsonElements.size() == 1) {
                        message = "Você possui um evento acontecendo neste momento. Se você continuar este evento sera desabilitada.";
                    }

                    ActivityUtils.alertDialogSimpleDualButton(activity, "Alerta", message, new DialogInterface.OnClickListener() {
                        @Override
                        public void onClick(DialogInterface dialog, int which) {
                            toggleCubee(jsonElements);
                        }
                    });
                }

            }

            @Override
            public void onGetJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao recuperar os eventos. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }
        };
    }

    private void toggleCubee(JsonArray jsonElements) {
        if (cubeeAtual.getCubeeState()) {
            deactivateCubee(jsonElements);
        } else {
            activateCubee(jsonElements);
        }
    }

    private OnGetJsonObjectCallback onGetCubeeByIdCallback() {
        return new OnGetJsonObjectCallback() {
            @Override
            public void onGetJsonObjectCallbackSuccess(JsonObject jsonObject) {
                Gson gson = new Gson();
                Cubee cubee = gson.fromJson(jsonObject, Cubee.class);


                if (!cubeeAtual.getCubeeState().equals(cubee.getCubeeState())) {
                    cubeeAtual = cubee;
                    if (cubeeAtual.getCubeeState()) {
                        ActivityUtils.showToast(context, "CUBEE ativado com sucesso.");
                    } else {
                        ActivityUtils.showToast(context, "CUBEE desativado com sucesso.");
                    }
                    changeTVStatus();
                    ActivityUtils.cancelProgressDialog();
                    contTime = 0;
                    timer.cancel();
                    cubeeAtual = cubee;
                    CubeeController.getInstance().setCubeeAtual(cubee);
                }

                if (flagFirstRequest) {
                    changeTVStatus();
                    ActivityUtils.cancelProgressDialog();
                    flagFirstRequest = false;
                }
            }

            @Override
            public void onGetJsonObjectCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                String myMessage = "Erro ao recuperar CUBEE. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
                MainActivity mainActivity = (MainActivity) activity;
                mainActivity.backToCurrentSectorOrGrupe();

            }

        };
    }

    /**
     * Callbacks to the request.
     * If Sucess: Change the CUBEE command and show a message to user
     * If Error:  Show message to user.
     **/
    private OnPostJsonObjectCallback onPostCubeeCommandCallback() {
        return new OnPostJsonObjectCallback() {
            @Override
            public void onPostJsonObjectCallbackSuccess(JsonObject jsonObject) {

            }

            @Override
            public void onPostJsonObjectCallbackError(VolleyError volleyError) {
                String myMessage = "Erro ao enviar comando para o CUBEE. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }

        };
    }

    private OnGetJsonArrayCallback onGetTasksByCubee() {
        return new OnGetJsonArrayCallback() {
            //String idEvent = null;
            @Override
            public void onGetJsonArrayCallbackSuccess(JsonArray jsonArray) {
                ActivityUtils.cancelProgressDialog();

                String message = "Se você deseja realmente excluir o CUBEE ";


                if (jsonArray.size() != 0) {
                    // idEvent= jsonArray.get(0).getAsJsonObject().get("idEvent").getAsString();
                    message = "Este CUBEE está presente em algum(s) agendamento(s). Se você deseja realmente excluir o CUBEE ";
                    //Log.i(TAG, idEvent);

                }
                //Log.i(TAG, jsonArray.size()+"");

                ActivityUtils.alertDialogSimpleDualButton(activity, "Alerta!", message + cubeeAtual.getName()
                        + ", clique em OK.", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        //deleteCubee(idEvent);
                        deleteCubee();
                    }
                });


            }

            @Override
            public void onGetJsonArrayCallbackError(VolleyError volleyError) {
                ActivityUtils.cancelProgressDialog();
                String myMessage = "Erro ao recuperar tarefas. ";
                ActivityUtils.showToast(activity, VolleyErrorUtils.decideRightMessage(volleyError, myMessage, activity, false));
            }
        };
    }

//Anothers Functions ----------------------------------------------------------------------

    private void changeTVStatus() {
        if (cubeeAtual.getCubeeState()) {
            tvCubeeStatus.setText("CUBEE Ativado");
            btnActiveDeactiveCubee.setText("Desativar");

        } else {
            tvCubeeStatus.setText("CUBEE Desativado");
            btnActiveDeactiveCubee.setText("Ativar");
        }
    }

    private void initLoop() {
        if (timer != null) {
            timer.cancel();
        }
        timer = new Timer();
        // This timer task will be executed every 1 sec.
        timer.schedule(new TimerTask() {
            @Override
            public void run() {
                if (idToken != null) {
                    refreshActualCubee();
                }
                Log.d("DEBUG", Boolean.toString(changeCommandFlag));
                Log.d("DEBUG", Integer.toString(contTime));
                if (changeCommandFlag) {
                    contTime++;
                    if (contTime == 10) {
                        ActivityUtils.cancelProgressDialog();
                        ActivityUtils.showToastOutActivity(context, "Erro ao mudar estado do CUBEE");
                    }
                }

            }
        }, 0, 3000);
    }

    /**
     * Cancel loop
     */
    private void cancelTimer() {
        if (timer != null) {
            timer.cancel();
        }
    }
}
