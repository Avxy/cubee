package com.lacina.cubeeclient.activities;

import android.graphics.Color;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;
import com.google.firebase.auth.FirebaseAuth;
import com.google.firebase.auth.FirebaseUser;
import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.controllers.CubeeController;
import com.lacina.cubeeclient.model.CubeeMeasurement;

import java.util.ArrayList;

/**
 * Activity to show Graphic information about a specific CUBEE with more details.
 */
@SuppressWarnings("ALL")
public class GraphExpandActivity extends AppCompatActivity {

    @SuppressWarnings("unused")
    private String TAG = "GraphExpandActv";

    /**
     * Firebase user profile information object.
     * Contain helper methods to o change or retrieve profile information,
     * as well as to manage that user's authentication state.
     */
    @SuppressWarnings("unused")
    private FirebaseUser firebaseUser;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        //SetUp variables.
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_expand_graph);
        firebaseUser = FirebaseAuth.getInstance().getCurrentUser();

        //Init Graphic
        initGraphic();

    }

    /**
     * Get mesurements from controller and draws the graphic.
     */
    private void initGraphic() {
        CubeeMeasurement cubeeMeasurement = CubeeController.getInstance().getCubeeMeasurement();
        LineChart lineChart = (LineChart) findViewById(R.id.chart);
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
        //dataset.setColors(ColorTemplate.COLORFUL_COLORS);
        dataset.setColor(Color.BLACK);
        dataset.setDrawFilled(true);

        lineChart.setData(data);
        lineChart.animateY(5000);
    }

}
