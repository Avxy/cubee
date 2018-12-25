package com.lacina.cubeeclient.activities;


import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.support.design.widget.TabLayout;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.support.v4.view.ViewPager;
import android.support.v7.app.AppCompatActivity;

import com.lacina.cubeeclient.R;
import com.lacina.cubeeclient.adapters.LoginBLEPagerAdapter;
import com.lacina.cubeeclient.interfaces.BleScanChangeable;

import butterknife.BindView;
import butterknife.ButterKnife;

/**
 * Initial tabs activity, setup adapters for Login and Ble Connection tabs.
 * Setup viewpager with tabs.
 * Ask for localization permissions.
 * Implements {@link BleScanChangeable} so you can set the activity to go when scan is successful
 **/
@SuppressWarnings("ALL")
public class LoginAndBLETabsActivity extends AppCompatActivity implements BleScanChangeable {

    @SuppressWarnings("unused")
    private String TAG = "LoginAndBleTabsA";

    /**
     * Permission code needed to ask COARSE_LOCATION permission.
     */
    private static final int MY_PERMISSIONS_CODE = 1;

    /**
     * Tab layout view
     */
    @SuppressWarnings("unused")
    @BindView(R.id.initial_tabs)
    public TabLayout tabLayout;

    /**
     * Tab layout adapter
     */
    private LoginBLEPagerAdapter adapter;

    /**
     * View Pager to connect with tab layout and allow swipe between fragment
     */
    @SuppressWarnings("unused")
    @BindView(R.id.viewPager)
    public ViewPager viewPager;

    /**
     * Field to reference context
     */
    private Context context;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        //SetUp Activity variables
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_initial_tabs_);
        context = getApplicationContext();
        ButterKnife.bind(this);

        //Add tabs to activity
        tabLayout.addTab(tabLayout.newTab());
        tabLayout.addTab(tabLayout.newTab());
        tabLayout.setTabGravity(TabLayout.GRAVITY_FILL);

        //Set adapter to control the fragments.
        adapter = new LoginBLEPagerAdapter(getSupportFragmentManager(), tabLayout.getTabCount(), context);
        viewPager.setAdapter(adapter);

        //Setup Tabs with view pager.
        tabLayout.setupWithViewPager(viewPager);

        //Name tabs
        TabLayout.Tab tabLogin = tabLayout.getTabAt(0);
        TabLayout.Tab tabBle = tabLayout.getTabAt(1);
        if (tabBle != null && tabLogin != null) {
            tabLogin.setText(R.string.login_tab);
            tabBle.setText(R.string.ble_tab);
        }


        setTabAndChangeListeners();


        //Ask for COARSE_LOCATION permission.
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (ContextCompat.checkSelfPermission(this,
                    android.Manifest.permission.ACCESS_COARSE_LOCATION)
                    != PackageManager.PERMISSION_GRANTED) {

                ActivityCompat.requestPermissions(this,
                        new String[]{android.Manifest.permission.ACCESS_COARSE_LOCATION},
                        MY_PERMISSIONS_CODE);

            }
        }

    }

    /**
     * When a device is found by scan and selected CUbeeBleCommandActivity is opened.
     * @return The class to go to.    @Override
    public Class activityToGoTo() {
        return CubeeBLECommandActivity.class;
    }
     */
    @Override
    public Class activityToGoTo() {
        return CubeeBLECommandActivity.class;
    }

    /**
     * SetUp changeListener. Make tabs and viewPager work together.
     */
    private void setTabAndChangeListeners() {
        tabLayout.setOnTabSelectedListener(new TabLayout.OnTabSelectedListener() {
            @Override
            public void onTabSelected(TabLayout.Tab tab) {
                viewPager.setCurrentItem(tab.getPosition());
            }

            @Override
            public void onTabUnselected(TabLayout.Tab tab) {

            }

            @Override
            public void onTabReselected(TabLayout.Tab tab) {

            }
        });


        viewPager.addOnPageChangeListener(new ViewPager.OnPageChangeListener() {
            @Override
            public void onPageScrolled(int position, float v, int i1) {
            }

            @Override
            public void onPageSelected(int position) {
            }

            @Override
            public void onPageScrollStateChanged(int state) {

            }
        });
    }


}




