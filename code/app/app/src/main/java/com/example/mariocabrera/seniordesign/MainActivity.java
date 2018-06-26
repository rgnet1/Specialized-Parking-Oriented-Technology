package com.example.mariocabrera.seniordesign;

import android.Manifest;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.ColorStateList;
import android.graphics.Color;
import android.media.MediaPlayer;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.support.annotation.NonNull;
import android.support.annotation.RequiresApi;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.NavigationView;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.ActionBarDrawerToggle;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.SpannableString;
import android.text.style.TextAppearanceSpan;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.view.View;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.VolleyLog;
import com.android.volley.toolbox.JsonObjectRequest;

import org.json.JSONException;
import org.json.JSONObject;
import org.w3c.dom.Text;

import java.util.HashMap;
import java.util.Map;
import java.util.Random;

public class MainActivity extends AppCompatActivity
        implements NavigationView.OnNavigationItemSelectedListener{

    private static final String LOG_TAG = "MainActivity";


    // ---------------------------------------------------------------------------------------------
    // Variables

    android.app.AlertDialog.Builder builder;
    NavigationView navigationView;

    // BlueTooth //
    private final static int REQUEST_ENABLE_BT = 1;
    private BluetoothManager btManager;
    private BluetoothAdapter btAdapter;
    private Handler scanHandler = new Handler();
    private int scan_interval_ms = 5000;    // 5000 = 20 secconds supposedly
    private boolean isScanning = false;
    private static final int PERMISSION_REQUEST_COARSE_LOCATION = 456;
    static final char[] hexArray = "0123456789ABCDEF".toCharArray();
    // BlueTotth differentiation
    String[] btArray = new String[10];
    static int scanToggle = 0;
    static int scanCount = 0;
    int count = 0;

    // HTTP Post Request strings //
    String serverURLverify = "https://car-management-66420.appspot.com/mobileapp";
    String serverURLpayment = "https://car-management-66420.appspot.com/payment";
    String serverURLblueTooth = "https://car-management-66420.appspot.com/bluetooth";
    static int toggleSendJSON = 1;
    // Use link below to check the status for each spot (Database must be on)
    // http://car-management-66420.appspot.com/Database

    // HTTP Get Request strings //
    String serverURLspots = "https://car-management-66420.appspot.com/spots?Parking_Lot=";
    String serverURLmobileinfo = "https://car-management-66420.appspot.com/mobileinfo";
    // serverURLspots format:
    // "https://car-management-66420.appspot.com/spots?Parking_Lot=EastRemote";


    // Passing Data from Activity to Fragment //
    public String uuidTwelve = "defaultTwelve";
    public String uuid = "default";

    // Settings Prefrences
    SharedPref settings;



    // ---------------------------------------------------------------------------------------------
    // Sounds for Action Button

    MediaPlayer mySong;

    private void StopOverlap() {
        if (mySong != null) {
            try {
                if (mySong.isPlaying()) {
                    mySong.release();
                }
            } catch (IllegalStateException e) {
                // do nothing
            }
        }
    }
    // ---------------------------------------------------------------------------------------------


    // ---------------------------------------------------------------------------------------------
    // Log In

    public boolean isUserLoggedIn() {

        SharedPreferences sharedPref = getSharedPreferences("login", Context.MODE_PRIVATE);
        String loginStatus = sharedPref.getString("loginStatus", "");

        if (loginStatus.compareTo("true") == 0) {
            return true;
        } else {
            return false;
        }
    }
    // ---------------------------------------------------------------------------------------------


    // ---------------------------------------------------------------------------------------------
    // BlueTooth

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String permissions[], @NonNull int[] grantResults) {
//        Toast.makeText(MainActivity.this, "onRequestPermision()", Toast.LENGTH_LONG).show();
        switch (requestCode) {
            case PERMISSION_REQUEST_COARSE_LOCATION: {
                if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    // Permission granted, yay! Start the Bluetooth device scan.
                } else {
                    // Alert the user that this application requires the location permission to perform the scan.
                }
            }
        }
    }
    // ---------------------------------------------------------------------------------------------

    // ---------------------------------------------------------------------------------------------
    // Update Fragments

    public void updateMapA(){
//        Toast.makeText(MainActivity.this, "updateMapA()", Toast.LENGTH_LONG).show();
        SharedPreferences userSharedPref = getSharedPreferences("userInfo", Context.MODE_PRIVATE);
        String email = userSharedPref.getString("email", "");
        String password = userSharedPref.getString("password", "");
        requestAccountInfoUpdate(email, password, "MapA");
    }

    public void updateMapB(){
//        Toast.makeText(MainActivity.this, "updateMapB()", Toast.LENGTH_LONG).show();
        SharedPreferences userSharedPref = getSharedPreferences("userInfo", Context.MODE_PRIVATE);
        String email = userSharedPref.getString("email", "");
        String password = userSharedPref.getString("password", "");
        requestAccountInfoUpdate(email, password, "MapB");
    }

    public void updateAccount(){
//        Toast.makeText(MainActivity.this, "updateAccount()", Toast.LENGTH_LONG).show();
        SharedPreferences userSharedPref = getSharedPreferences("userInfo", Context.MODE_PRIVATE);
        String email = userSharedPref.getString("email", "");
        String password = userSharedPref.getString("password", "");
        requestAccountInfoUpdate(email, password, "Account");
    }

    public void rebuildMapA(){
        setTitle("West Remote");
        MapAFragment fragment = new MapAFragment();
        android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
        fragmentTransaction.replace(R.id.frame, fragment, "fragment map A");
        fragmentTransaction.commit();
    }

    public void rebuildMapB(){
        setTitle("East Remote");
        MapBFragment fragment = null;
        SharedPreferences sharedPref = getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
        String userid = sharedPref.getString("id", "");
        fragment = new MapBFragment().newInstance(userid, uuidTwelve);
        android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
        fragmentTransaction.replace(R.id.frame, fragment, "fragment map B");
        fragmentTransaction.commit();
    }

    public void rebuildAccount(){
        setTitle("Account");
        AccountFragment fragment = new AccountFragment();
        android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
        fragmentTransaction.replace(R.id.frame, fragment, "fragment account");
        fragmentTransaction.commit();
    }

    public void requestAccountInfoUpdate(String email, String password, final String fragment) {
//        Toast.makeText(MainActivity.this, "requestAccountInfoUpdate()", Toast.LENGTH_LONG).show();
        builder = new android.app.AlertDialog.Builder(MainActivity.this);
        JSONObject parameters = null;
        final Map<String, String> postParam = new HashMap<String, String>();

        postParam.put("email", email);
        postParam.put("password", password);
        parameters = new JSONObject(postParam);

        String server_url = serverURLmobileinfo;


        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
                server_url, parameters,
                new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Parses current balance out of JSON response //
                        String jsonString = response.toString();
                        try {
                            JSONObject jObject = new JSONObject(jsonString);
                            String status = jObject.getString("status");
                            String firstName = jObject.getString("First_Name");
                            String lastName = jObject.getString("Last_Name");
                            String email = jObject.getString("E_mail");
                            String license = jObject.getString("License_Plate");
                            String spots = jObject.getString("Spots");
                            String permit = jObject.getString("Permit_Type");
                            String balance = jObject.getString("Account_Balance");
                            String loginStatus = jObject.getString("Log_In_Status");
                            String parkingLot = jObject.getString("Parking_Lot");
                            String id = jObject.getString("id");

                            // Save Account Info to Shared Preferences
                            SharedPreferences sharedPref = getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
                            SharedPreferences.Editor editor = sharedPref.edit();
                            editor.putString("firstName", firstName);
                            editor.putString("lastName", lastName);
                            editor.putString("email", email);
                            editor.putString("id", id);
                            editor.putString("permit", permit);
                            editor.putString("balance", balance);
                            editor.putString("license", license);
                            //
                            editor.putString("status", status);
                            editor.putString("spots", spots);
                            editor.putString("loginStatus", loginStatus);
                            editor.putString("parkingLot", parkingLot);
                            editor.apply();

                            if (fragment.compareTo("MapA") == 0) {
                                rebuildMapA();
                            } else if (fragment.compareTo("MapB") == 0) {
                                rebuildMapB();
                            } else if (fragment.compareTo("Account") == 0){
                                rebuildAccount();
                            }  else {
                                builder.setTitle("Error in Refresh");
                                builder.setMessage("Developer: Check requestAccountInfoUpdate() " +
                                        "in MainActivity()");
                                android.app.AlertDialog alertDialog = builder.create();
                                alertDialog.show();
                            }

                        } catch (JSONException e) {
                            e.printStackTrace();
                            builder.setTitle("Developer Error: requestAccountInfo catch block");
                            builder.setMessage("Cloud JSON Input: " + postParam.toString() +
                                    "\n\nCloud String Response:\n" + response.toString());
                            android.app.AlertDialog alertDialog = builder.create();
                            alertDialog.show();
                        }

                    }
                }, new Response.ErrorListener() {

            @Override
            public void onErrorResponse(VolleyError error) {
                VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
                Toast.makeText(MainActivity.this, error.toString(), Toast.LENGTH_LONG).show();
            }
        }) {
            // Passing mobileinfo request header
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                HashMap<String, String> headers = new HashMap<String, String>();
                headers.put("Message_Type", "Info");
                return headers;
            }
        };

        jsonObjReq.setTag(LOG_TAG);
        MySingleton.getInstance(MainActivity.this).addTorequestque(jsonObjReq);
    }

    //----------------------------------------------------------------------------------------------


    // ---------------------------------------------------------------------------------------------


    @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
    @Override
    protected void onCreate(Bundle savedInstanceState) {

//        Toast.makeText(MainActivity.this, "onCreate()", Toast.LENGTH_LONG).show();

        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        // -----------------------------------------------------------------------------------------
        // Menu
//        TextView navSite = (TextView) findViewById(R.id.navSite);
////        navSite.setText("site");



        // -----------------------------------------------------------------------------------------
        // Refresh on Startup

        requestSpots("East");
        requestSpots("West");

        // Update Account Info
        // Display number of spots on start up
        SharedPreferences sharedPref = getSharedPreferences("userInfo", Context.MODE_PRIVATE);
        String email = sharedPref.getString("email", "");
        String password = sharedPref.getString("password", "");
        requestAccountInfo(email, password);

        // Ramzey   = 1395938
        // Jon      = 1429392
        // Mario    = 1427605
        // Travis   = 1463728
//        userid = sharedPref.getString("id", "");
//        Toast.makeText(MainActivity.this, userid, Toast.LENGTH_SHORT).show();

        // -----------------------------------------------------------------------------------------


        // -----------------------------------------------------------------------------------------
        // Log In

        boolean logInFlag = isUserLoggedIn();
        if (logInFlag == false) {
//            Toast.makeText(MainActivity.this, "onCreate(), go to LoginActivity", Toast.LENGTH_LONG).show();
            Intent myIntent = new Intent(this, LoginActivity.class);
            startActivity(myIntent);
        }

        // -----------------------------------------------------------------------------------------
        // Set theme on Launch
        settings = new SharedPref(this);
        if (settings.loadNightModeState()) {
            setTheme(R.style.DarkTheme);
        } else {
            setTheme(R.style.AppTheme);
        }

        // -----------------------------------------------------------------------------------------
        // BlueTooth
        btManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        btAdapter = btManager.getAdapter();
        scanHandler.post(scanRunnable);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            requestPermissions(new String[]{Manifest.permission.ACCESS_COARSE_LOCATION}, PERMISSION_REQUEST_COARSE_LOCATION);
        }

        // -----------------------------------------------------------------------------------------
        // Automatically turn on BlueTooth

        if (btAdapter != null) {
            // Device supports Bluetooth
            if (!btAdapter.isEnabled()) {
                // Bluetooth isn't enabled, so enable it.
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
//                Toast.makeText(MainActivity.this, "adatpter in onCreate()", Toast.LENGTH_LONG).show();
            }
        }
        // -----------------------------------------------------------------------------------------

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        fab.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                // Snackbar.make(view, "Replace with your own action", Snackbar.LENGTH_LONG).setAction("Action", null).show();

                //----------------------------------------------------------------------------------
                // HTTP Post Request
                toggleSendJSON = 1;
                scanToggle = 0;
//                sendJSONverifyStuff();
                // ----------------------------------------------------------------------------------

                // ---------------------------------------------------------------------------------
                // Sounds
                StopOverlap();  // stops sound overlaps
                Random rand = new Random();
                int randomNum = rand.nextInt(40);    // Gives n such that 0 <= n < 34

                if (randomNum == 0) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.a_cp_has_been_lost);
                } else if (randomNum == 1) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.a_cp_is_under_hostile_control);
                } else if (randomNum == 2) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.a_cp_is_under_republic_control);
                } else if (randomNum == 3) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.another_cp_for_the_republic);
                } else if (randomNum == 4) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.cp_under_attack);
                } else if (randomNum == 5) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.for_the_chancellor);
                } else if (randomNum == 6) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.for_the_repubic);
                } else if (randomNum == 7) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.frill_in_heck_its_a_super);
                } else if (randomNum == 8) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.hostiles_have_gained_a_cp);
                } else if (randomNum == 9) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.just_like_the_simulation);
                } else if (randomNum == 10) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.no_one_messes_with_the_501st);
                } else if (randomNum == 11) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.our_reinforcements_are_being_depleted);
                } else if (randomNum == 12 || randomNum >34) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.rockets);
                } else if (randomNum == 13) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.set_em_up_and_knock_em_down);
                } else if (randomNum == 14) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.super_battledroid_take_him_down);
                } else if (randomNum == 15) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.take_out_that_super);
                } else if (randomNum == 16) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.thats_another_cp_under_republic_control);
                } else if (randomNum == 17) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.the_bigger_they_are_the_harder_they_fall);
                } else if (randomNum == 18) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.the_enemy_has_taken_a_cp);
                } else if (randomNum == 19) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.the_republic_cant_spare_reinforcements);
                } else if (randomNum == 20) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.the_republic_has_retaken_a_cp);
                } else if (randomNum == 21) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.the_republic_is_in_control_of_a_cp);
                } else if (randomNum == 22) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.theyre_flanking_us);
                } else if (randomNum == 23) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.theyre_losing_reinforcements);
                } else if (randomNum == 24) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.theyve_sent_in_the_rollers);
                } else if (randomNum == 25) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.theyve_sent_in_the_supers);
                } else if (randomNum == 26) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.watch_that_duel_blade);
                } else if (randomNum == 27) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.watch_that_light_saber);
                } else if (randomNum == 28) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.watch_those_blasters);
                } else if (randomNum == 29) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.watch_your_flanks);
                } else if (randomNum == 30) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.we_cant_keep_losing_cps);
                } else if (randomNum == 31) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.weve_got_droids);
                } else if (randomNum == 32) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.weve_got_seps);
                } else if (randomNum == 33) {
                    mySong = MediaPlayer.create(getApplicationContext(), R.raw.weve_lost_control_of_a_cp);
                }
                mySong.start();
                // ---------------------------------------------------------------------------------
            }
        });

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        ActionBarDrawerToggle toggle = new ActionBarDrawerToggle(
                this, drawer, toolbar, R.string.navigation_drawer_open, R.string.navigation_drawer_close);
        drawer.addDrawerListener(toggle);
        toggle.syncState();

        navigationView = (NavigationView) findViewById(R.id.nav_view);
        navigationView.setNavigationItemSelectedListener(this);

        View headerView = navigationView.getHeaderView(0);
        TextView navUser = (TextView) headerView.findViewById(R.id.navUser);
        TextView navEmail = (TextView) headerView.findViewById(R.id.navEmail);

        SharedPreferences sharedPrefMenu = getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
        String firstNameString =  sharedPrefMenu.getString("firstName", "");
        String lastNameString =   sharedPrefMenu.getString("lastName", "");
        String emailString =      sharedPref.getString("email", "");
        navUser.setText(firstNameString + " " + lastNameString);
        navEmail.setText(emailString);

        //------------------set Theme on Launch---------------------------------------------------
        settings = new SharedPref(this);
        int[] navTextColor;
        //Change Menu Text Color
        Menu menu = navigationView.getMenu();
        MenuItem tools= menu.findItem(R.id.menuTextColor);
        SpannableString s = new SpannableString(tools.getTitle());
        if (settings.loadNightModeState()) {
            navigationView.setBackgroundColor(getResources().getColor(R.color.darkBackgroundColor));
            navTextColor = new int[] {
                    Color.WHITE,
                    Color.WHITE,
                    Color.WHITE,
                    Color.WHITE
            };
            s.setSpan(new TextAppearanceSpan(this, R.style.DarkTheme), 0, s.length(), 0);
        } else {
            setTheme(R.style.AppTheme);
            navigationView.setBackgroundColor(getResources().getColor(R.color.lightBackgroundColor));
            navTextColor = new int[] {
                    Color.BLACK,
                    Color.BLACK,
                    Color.BLACK,
                    Color.BLACK
            };
            s.setSpan(new TextAppearanceSpan(this, R.style.AppTheme), 0, s.length(), 0);
        }


        //                     Set up Color states for menu color
        /**
         * start of code configuration for color of text of your Navigation Drawer / Menu based on state
         */
        int[][] state = new int[][] {
                new int[] {-android.R.attr.state_enabled}, // disabled
                new int[] {android.R.attr.state_enabled}, // enabled
                new int[] {-android.R.attr.state_checked}, // unchecked
                new int[] { android.R.attr.state_pressed}  // pressed

        };



        ColorStateList colorStateList1 = new ColorStateList(state, navTextColor);


        // FOR NAVIGATION VIEW ITEM ICON COLOR
        int[][] states = new int[][] {
                new int[] {-android.R.attr.state_enabled}, // disabled
                new int[] {android.R.attr.state_enabled}, // enabled
                new int[] {-android.R.attr.state_checked}, // unchecked
                new int[] { android.R.attr.state_pressed}  // pressed

        };

        ColorStateList colorStateList2 = new ColorStateList(states, navTextColor);
        navigationView.setItemTextColor(colorStateList1);
        navigationView.setItemIconTintList(colorStateList2);
        tools.setTitle(s);

        // Make FragmentMapA as default screen
        MapAFragment fragment = new MapAFragment();
        android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
        fragmentTransaction.replace(R.id.frame, fragment, "fragment map A");
        fragmentTransaction.commit();
    }


    //----------------------------------------------------------------------------------------------
    // Shared Preferences

    // lot = "East" or "West"
    // folder = "WestInfo"      or "EastInfo"
    // file   = "spotsLeftWest" or "spotsLeftEast"

    public void saveSpots(String lot, String spots) {
        String folder = lot + "Info";
        String file = "spotsLeft" + lot;
//        Toast.makeText(MainActivity.this, "saveSpots()", Toast.LENGTH_LONG).show();

        SharedPreferences sharedPref = getSharedPreferences(folder, Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPref.edit();
        editor.putString(file, spots);
        editor.apply();
    }


    //----------------------------------------------------------------------------------------------
    // HTTP GET Request: Empty Spots (all of them)

    // Type into String either "East" or "West"

    public void requestSpots(final String lot) {
//        Toast.makeText(MainActivity.this, "requestSpots()", Toast.LENGTH_LONG).show();

        builder = new android.app.AlertDialog.Builder(MainActivity.this);
        String server_url = serverURLspots.concat(lot).concat("Remote");

        JsonObjectRequest jsonObjReq = new JsonObjectRequest
                (Request.Method.GET, server_url, (String) null, new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Parses spot out of JSON request //
                        String jsonString = response.toString();
                        try {
                            JSONObject jObject = new JSONObject(jsonString);
                            String spots = jObject.getString("spots");
                            saveSpots(lot, spots);

                        } catch (JSONException e) {
                            e.printStackTrace();
                            builder.setTitle("Developer Error: requestSpots() catch block");
                            builder.setMessage("Developer: Check requestSpots() " +
                                    "in MainActivity()");
                            android.app.AlertDialog alertDialog = builder.create();
                            alertDialog.show();
                        }


                    }
                }, new Response.ErrorListener() {

                    @Override
                    public void onErrorResponse(VolleyError error) {
                        VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
                        Toast.makeText(MainActivity.this, error.toString(), Toast.LENGTH_LONG).show();

                    }
                }) {

            // Passing payment request header
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                HashMap<String, String> headers = new HashMap<String, String>();
                headers.put("Message_Type", "Spots");
                return headers;
            }
        };

        // Access the RequestQueue through your singleton class.
        jsonObjReq.setTag(LOG_TAG);
        MySingleton.getInstance(MainActivity.this).addTorequestque(jsonObjReq);
    }

    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    // HTTP POST Request: Account Info

    public void requestAccountInfo(String email, String password) {
//        Toast.makeText(MainActivity.this, "requestAccountInfo()", Toast.LENGTH_LONG).show();

        builder = new android.app.AlertDialog.Builder(MainActivity.this);
        JSONObject parameters = null;
        final Map<String, String> postParam = new HashMap<String, String>();

        postParam.put("email", email);
        postParam.put("password", password);
        parameters = new JSONObject(postParam);

        String server_url = serverURLmobileinfo;


        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
                server_url, parameters,
                new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Parses current balance out of JSON response //
                        String jsonString = response.toString();
                        try {
                            JSONObject jObject = new JSONObject(jsonString);
                            String status = jObject.getString("status");
                            String firstName = jObject.getString("First_Name");
                            String lastName = jObject.getString("Last_Name");
                            String email = jObject.getString("E_mail");
                            String license = jObject.getString("License_Plate");
                            String spots = jObject.getString("Spots");
                            String permit = jObject.getString("Permit_Type");
                            String balance = jObject.getString("Account_Balance");
                            String loginStatus = jObject.getString("Log_In_Status");
                            String parkingLot = jObject.getString("Parking_Lot");
                            String id = jObject.getString("id");

                            // Save Account Info to Shared Preferences
                            SharedPreferences sharedPref = getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
                            SharedPreferences.Editor editor = sharedPref.edit();
                            editor.putString("firstName", firstName);
                            editor.putString("lastName", lastName);
                            editor.putString("email", email);
                            editor.putString("id", id);
                            editor.putString("permit", permit);
                            editor.putString("balance", balance);
                            editor.putString("license", license);
                            //
                            editor.putString("status", status);
                            editor.putString("spots", spots);
                            editor.putString("loginStatus", loginStatus);
                            editor.putString("parkingLot", parkingLot);
                            editor.apply();


                        } catch (JSONException e) {
                            e.printStackTrace();
                            builder.setTitle("Developer Error: requestAccountInfo() catch block");
                            builder.setMessage("Cloud JSON Input: " + postParam.toString() +
                                    "\n\nCloud String Response:\n" + response.toString());
                            android.app.AlertDialog alertDialog = builder.create();
                            alertDialog.show();
                        }

                    }
                }, new Response.ErrorListener() {

            @Override
            public void onErrorResponse(VolleyError error) {
                VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
                Toast.makeText(MainActivity.this, error.toString(), Toast.LENGTH_LONG).show();
            }
        }) {

            // Passing mobileinfo request header
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                HashMap<String, String> headers = new HashMap<String, String>();
                headers.put("Message_Type", "Info");
                return headers;
            }
        };

        jsonObjReq.setTag(LOG_TAG);
        MySingleton.getInstance(MainActivity.this).addTorequestque(jsonObjReq);
    }

    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    // HTTP POST JSON Request: Verify with Waiting

    public void sendJSONverifyStuff(String uuidTwelve) {
//        Toast.makeText(MainActivity.this, "sendJSONverifyStuff()", Toast.LENGTH_LONG).show();
        builder = new android.app.AlertDialog.Builder(MainActivity.this);
        JSONObject parameters = null;
        final Map<String, String> postParam = new HashMap<String, String>();

        SharedPreferences sharedPref = getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
        String userid = sharedPref.getString("id", "");

        postParam.put("bt_string", uuidTwelve);
        postParam.put("userid", userid);
        parameters = new JSONObject(postParam);

        // Toast.makeText(MainActivity.this, postParam.toString(), Toast.LENGTH_LONG).show();
        String server_url = serverURLverify;

        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
                server_url, parameters,
                new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Parses current balance out of JSON response //
                        String jsonString = response.toString();
                        try {
                            JSONObject jObject = new JSONObject(jsonString);
                            String status = jObject.getString("status");

                            if (status.compareTo("verified") == 0) {
                                builder.setTitle("Verified!");
                                builder.setMessage(
//                                        "Cloud JSON Input: " + postParam.toString() +
//                                        "\n\nCloud String Response:\n" + response.toString() +
                                        "Your parking has been verified!\n\nHave a nice day.   :)");
                                android.app.AlertDialog alertDialog = builder.create();
                                alertDialog.show();
                            } else if (status.compareTo("illegal") == 0) {
                                builder.setTitle("Not verified");
                                builder.setMessage(
//                                        "Cloud JSON Input: " + postParam.toString() +
//                                        "\n\nCloud String Response:\n" + response.toString() +
                                        "Please leave the parking space soon to avoid an illegal parking status."
                                        + "\n\nHave a nice day.   :)");
                                android.app.AlertDialog alertDialog = builder.create();
                                alertDialog.show();
                            } else if (status.compareTo("payment") == 0) {
                                PaymentOptionFragment fragment = new PaymentOptionFragment();
                                android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
                                fragmentTransaction.add(R.id.frame, fragment, "fragment payment option");
                                fragmentTransaction.addToBackStack(null);
                                fragmentTransaction.commit();
                            } else if (status.compareTo("Spot not busy") == 0) {
                                builder.setTitle("Spot Not Busy");
                                builder.setMessage(
//                                        "Cloud JSON Input: " + postParam.toString() +
//                                        "\n\nCloud String Response:\n" + response.toString() +
                                        "Please try parking closer to the sensor\n\nThank you!   :)");
                                android.app.AlertDialog alertDialog = builder.create();
                                alertDialog.show();
                            } else {
                                builder.setTitle("Not verified!");
                                builder.setMessage("Please try again");
                                android.app.AlertDialog alertDialog = builder.create();
                                alertDialog.show();
                            }

                        } catch (JSONException e) {
                            e.printStackTrace();
                            builder.setTitle("Developer Error: sendJSONverifyStuff() catch block");
                            builder.setMessage("Cloud JSON Input: " + postParam.toString() +
                                    "\n\nCloud String Response:\n" + response.toString());
                            android.app.AlertDialog alertDialog = builder.create();
                            alertDialog.show();
                        }

//                        // Includes entire JSON response in alert //
//                        builder.setTitle("Input & Output");
//                        builder.setMessage("Cloud JSON Input: " + postParam.toString() + "\n\nCloud String Response:\n" + response.toString());
//                        android.app.AlertDialog alertDialog = builder.create();
//                        alertDialog.show();

                    }
                }, new Response.ErrorListener() {

            @Override
            public void onErrorResponse(VolleyError error) {
                VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
                Toast.makeText(MainActivity.this, error.toString(), Toast.LENGTH_LONG).show();
                builder.setTitle("Verification Error");
                builder.setMessage("We couldn't read your parking unit. " +
                        "Please press the action button on the lower right corner " +
                        "to try again.\n\nThank you.   :)");
                android.app.AlertDialog alertDialog = builder.create();
                alertDialog.show();
            }
        }) {

            // Passing mobileapp request header
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                HashMap<String, String> headers = new HashMap<String, String>();
                headers.put("Message_Type", "Verify");
                return headers;
            }

        };

        jsonObjReq.setTag(LOG_TAG);
        MySingleton.getInstance(MainActivity.this).addTorequestque(jsonObjReq);
    }
    //----------------------------------------------------------------------------------------------

    //----------------------------------------------------------------------------------------------
    // HTTP POST JSON Request: Payment with Designated Alerts

    public void sendJSONpaymentStuff() {
//        Toast.makeText(MainActivity.this, "sendJSONpaymentStuff", Toast.LENGTH_LONG).show();
        builder = new android.app.AlertDialog.Builder(MainActivity.this);
        JSONObject parameters = null;
        final Map<String, String> postParam = new HashMap<String, String>();

        SharedPreferences sharedPref = getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
        String userid = sharedPref.getString("id", "");

        postParam.put("bt_string", uuidTwelve);
        postParam.put("userid", userid);
        parameters = new JSONObject(postParam);

        // Toast.makeText(MainActivity.this, postParam.toString(), Toast.LENGTH_LONG).show();
        String server_url = serverURLpayment;

        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
                server_url, parameters,
                new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Parses current balance out of JSON response //
                        String jsonString = response.toString();
                        try {
                            JSONObject jObject = new JSONObject(jsonString);
                            String currentBalance = jObject.getString("current_balance");

                            builder.setTitle("Payment Confirmed!");
                            builder.setMessage("Cloud JSON Input: " + postParam.toString() +
                                    "\n\nYour New Balance: $" + currentBalance);
                            android.app.AlertDialog alertDialog = builder.create();
                            alertDialog.show();

                        } catch (JSONException e) {
                            e.printStackTrace();
                            builder.setTitle("Developer Error: sendJSONpaymentStuff() catch block");
                            builder.setMessage("Cloud JSON Input: " + postParam.toString() +
                                    "\n\nCloud String Response:\n" + response.toString());
                            android.app.AlertDialog alertDialog = builder.create();
                            alertDialog.show();
                        }

//                        // Includes entire JSON response in alert //
//                        builder.setTitle("Input & Output");
//                        builder.setMessage("Cloud JSON Input: " + postParam.toString() + "\n\nCloud String Response:\n" + response.toString());
//                        android.app.AlertDialog alertDialog = builder.create();
//                        alertDialog.show();

                    }
                }, new Response.ErrorListener() {

            @Override
            public void onErrorResponse(VolleyError error) {
                VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
                Toast.makeText(MainActivity.this, error.toString(), Toast.LENGTH_LONG).show();
                builder.setTitle("Payment Error");
                builder.setMessage("Please Try Again");
                android.app.AlertDialog alertDialog = builder.create();
                alertDialog.show();
            }
        }) {

            // Passing payment request header
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                HashMap<String, String> headers = new HashMap<String, String>();
                headers.put("Message_Type", "Payment");
                return headers;
            }
        };

        jsonObjReq.setTag(LOG_TAG);
        MySingleton.getInstance(MainActivity.this).addTorequestque(jsonObjReq);
    }
    //----------------------------------------------------------------------------------------------

    // Called in PaymentOtionFragment
    // when the user denies the payment option
    public void paymentUnconfirmedAlert() {
//        Toast.makeText(MainActivity.this, "paymentUnconfirmedAlert()", Toast.LENGTH_LONG).show();
        builder.setTitle("Payment Unconfirmed");
        builder.setMessage("Please leave the parking space soon to avoid an illegal parking status."
                + "\n\nHave a nice day.   :)");
        android.app.AlertDialog alertDialog = builder.create();
        alertDialog.show();
    }

    // ---------------------------------------------------------------------------------------------


    // BlueTooth

    private BluetoothAdapter.LeScanCallback leScanCallback = new BluetoothAdapter.LeScanCallback() {
        @Override
        public void onLeScan(final BluetoothDevice device, final int rssi, final byte[] scanRecord) {
            int startByte = 2;
            boolean patternFound = false;
            while (startByte <= 5) {
                if (((int) scanRecord[startByte + 2] & 0xff) == 0x02 && // Identifies an iBeacon
                        ((int) scanRecord[startByte + 3] & 0xff) == 0x15) { // Identifies correct data length
                    patternFound = true;
                    break;
                }
                startByte++;
            }

//            Toast.makeText(MainActivity.this, "leScanCallBack()", Toast.LENGTH_SHORT).show();

            if (patternFound) {

                Log.i(LOG_TAG, "patternFound!");
                //Convert to hex String
                byte[] uuidBytes = new byte[16];
                System.arraycopy(scanRecord, startByte + 4, uuidBytes, 0, 16);
                String hexString = bytesToHex(uuidBytes);

                // UUID detection
                uuid = hexString.substring(0, 8) + "-" +
                        hexString.substring(8, 12) + "-" +
                        hexString.substring(12, 16) + "-" +
                        hexString.substring(16, 20) + "-" +
                        hexString.substring(20, 32);

                // major
                final int major = (scanRecord[startByte + 20] & 0xff) * 0x100 + (scanRecord[startByte + 21] & 0xff);

                // minor
                final int minor = (scanRecord[startByte + 22] & 0xff) * 0x100 + (scanRecord[startByte + 23] & 0xff);

                Log.i(LOG_TAG, "UUID: " + uuid + ", major: " + major + ", minor" + minor);
                uuidTwelve = uuid.substring(uuid.length() - 12);


                // ---------------------------------------------------------------------------------
                // Unique BT Strings

                if (toggleSendJSON == 1) {
                    boolean matchFound = false;
                    if (scanToggle == 0) {
                        scanCount++;
                        if (count < 10) {
                            if (count == 0) {
                                btArray[count] = uuidTwelve;
                                count++;
                            } else {
                                for (int i = 0; i < count; i++) {
                                    if ((btArray[i] != null) && (uuidTwelve.compareTo(btArray[i]) == 0)) {
                                        matchFound = true;
                                    }
                                }
                                // only insert if a match was not found.
                                if (matchFound == false) {
                                    // do nothing
                                    btArray[count] = uuidTwelve;
                                    count++;
                                }
                            }
                        }
                        // Print array
                        if ((count > 1) && (scanCount > 10)) {
                            btRequestSpots(btArray);
                            scanToggle = 1;
                            toggleSendJSON = 0;
                        } else if (scanCount > 10) {
                            sendJSONverifyStuff(uuidTwelve);
                            scanToggle = 1;
                            toggleSendJSON = 0;
                        }
                    }
                }

                // ---------------------------------------------------------------------------------
            }

        }
    };

    private static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    private Runnable scanRunnable = new Runnable() {
        @RequiresApi(api = Build.VERSION_CODES.JELLY_BEAN_MR2)
        @Override
        public void run() {

            if (isScanning) {
                if (btAdapter != null) {
                    btAdapter.stopLeScan(leScanCallback);
                }
            } else {
                if (btAdapter != null) {
                    btAdapter.startLeScan(leScanCallback);
                }
            }

            isScanning = !isScanning;

            scanHandler.postDelayed(this, scan_interval_ms);
        }
    };
    // ---------------------------------------------------------------------------------------------

    // ---------------------------------------------------------------------------------------------
    // BlueTooth Differentiation

    public void pickBT(String[] lots, String[] btArray) {
//        Toast.makeText(MainActivity.this, "pickBT()", Toast.LENGTH_LONG).show();
        BTDiffFragment fragment = null;
        fragment = new BTDiffFragment().newInstance(lots, btArray);
        android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
        fragmentTransaction.add(R.id.frame, fragment, "fragment btdiff");
        fragmentTransaction.addToBackStack(null);
        fragmentTransaction.commit();
    }

    //----------------------------------------------------------------------------------------------
    // HTTP POST Request: BlueTooth Differentiation

    public void btRequestSpots(final String[] btArray) {
//        Toast.makeText(MainActivity.this, "btRequestSpots()", Toast.LENGTH_LONG).show();
        builder = new android.app.AlertDialog.Builder(MainActivity.this);
        JSONObject parameters = null;
        final Map<String, String> postParam = new HashMap<String, String>();

        String server_url = serverURLblueTooth;

        String key = "bt_string";
        for (int i = 0; i < btArray.length; i++) {
            if (btArray[i] != null) {
                postParam.put(key + String.valueOf(i + 1), btArray[i]);
            }
        }

        parameters = new JSONObject(postParam);

        final int finalCount = count;

        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
                server_url, parameters,
                new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Parses current balance out of JSON response //
                        String jsonString = response.toString();
                        try {
                            JSONObject jObject = new JSONObject(jsonString);

                            String bt_string = "";
                            String lot_string = "";
                            String tempBt = "";
                            String tempLot = "";
                            int numElements = 0;

                            // Parse out any bluetooth strings that are not in the database "Not Found"
                            // lot_string holds concatenated database reponses (i.e. EastRemote:3)
                            // bt_string holds cancatenated bt_string numbers (i.e. 1 in "bt_string1")
                            for (int i = 1; i <= finalCount; i++) {
                                tempBt = "bt_string" + String.valueOf(i);
                                tempLot = jObject.getString(tempBt);
                                if(tempLot.compareTo("Not Found") != 0){
                                    lot_string = lot_string.concat(tempLot + ";");
                                    bt_string = bt_string.concat(String.valueOf(i) + ";");
                                    numElements++;
                                }
                            }

                            // Get ready to tokenize concatenated strings
                            String[] lots = new String[numElements];
                            String[] btIndexArray = new String[numElements];
                            int i;

                            // Insert lot_string tokens into an array
                            i = 0;
                            for (String retval : lot_string.split(";")) {
                                lots[i] = retval;
                                i++;
                            }

                            // Insert bt_string tokens into an array
                            i = 0;
                            for (String retval : bt_string.split(";")) {
                                btIndexArray[i] = retval;
                                i++;
                            }

                            // Create an array that holds the actual bluetooth uuids that were found
                            String[] btArrayTemp = new String[numElements];
                            int index;
                            for(i = 0; i < numElements; i++){
                                index = Integer.valueOf(btIndexArray[i]) - 1;
                                btArrayTemp[i] = btArray[index];
                            }

                            // Input lots array and corresponding uuids into pickBT()
                            pickBT(lots, btArrayTemp);

                        } catch (JSONException e) {
                            e.printStackTrace();
                            builder.setTitle("Developer Error: btRequestSpots() catch block");
                            builder.setMessage("Cloud JSON Input: " + postParam.toString() +
                                    "\n\nCloud String Response:\n" + response.toString());
                            android.app.AlertDialog alertDialog = builder.create();
                            alertDialog.show();
                        }

//                        // Includes entire JSON response in alert //
//                        builder.setTitle("Input & Output");
//                        builder.setMessage("Cloud JSON Input: " + postParam.toString()
//                                + "\n\nCloud String Response:\n" + response.toString());
//                        android.app.AlertDialog alertDialog = builder.create();
//                        alertDialog.show();
                    }
                }, new Response.ErrorListener() {

            @Override
            public void onErrorResponse(VolleyError error) {
                VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
                Toast.makeText(MainActivity.this, error.toString(), Toast.LENGTH_LONG).show();
            }
        }) {

            // Passing bluetooth request header
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                HashMap<String, String> headers = new HashMap<String, String>();
                headers.put("Message_Type", "Bluetooth");
                return headers;
            }
        };

        jsonObjReq.setTag(LOG_TAG);
        MySingleton.getInstance(MainActivity.this).addTorequestque(jsonObjReq);
    }










//    String[] fakebtArray = {"fake1", "886788678867", "fake2", "859485948594", "fake3"};
//
//    public void fakebtRequestSpots(final String[] btArray) {
////        Toast.makeText(MainActivity.this, "btRequestSpots()", Toast.LENGTH_LONG).show();
//        builder = new android.app.AlertDialog.Builder(MainActivity.this);
//        JSONObject parameters = null;
//        final Map<String, String> postParam = new HashMap<String, String>();
//
//        String server_url = serverURLblueTooth;
//
//        String key = "bt_string";
//        for (int i = 0; i < fakebtArray.length; i++) {
//            if (fakebtArray[i] != null) {
//                postParam.put(key + String.valueOf(i + 1), fakebtArray[i]);
//            }
//        }
//
//        parameters = new JSONObject(postParam);
//
//        final int finalCount = count;
//
//        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
//                server_url, parameters,
//                new Response.Listener<JSONObject>() {
//
//                    @Override
//                    public void onResponse(JSONObject response) {
//
//                        // Parses current balance out of JSON response //
//                        String jsonString = response.toString();
//                        try {
//                            JSONObject jObject = new JSONObject(jsonString);
//
//                            String bt_string = "";
//                            String lot_string = "";
//                            String status = null;
//                            String tempBt = "";
//                            String tempLot = "";
//                            int numElements = 0;
//
//                            for(int i = 1; i <= fakebtArray.length; i++){
//                                tempBt = "bt_string" + String.valueOf(i);
//                                tempLot = jObject.getString(tempBt);
//                                if(tempLot.compareTo("Not Found") != 0){
//                                    lot_string = lot_string.concat(tempLot + ";");
//
//                                    bt_string = bt_string.concat(String.valueOf(i) + ";");
//                                    numElements++;
//                                }
//                            }
//
//                            String[] lots = new String[numElements];
//                            String[] btIndexArray = new String[numElements];
//                            int i;
//
//                            i = 0;
//                            for (String retval : lot_string.split(";")) {
//                                lots[i] = retval;
//                                i++;
//                            }
//
//                            i = 0;
//                            for (String retval : bt_string.split(";")) {
//                                btIndexArray[i] = retval;
//                                i++;
//                            }
//
//                            String[] btArrayTemp = new String[numElements];
//                            int index;
//                            for(i = 0; i < numElements; i++){
//                                index = Integer.valueOf(btIndexArray[i]) - 1;
//                                btArrayTemp[i] = fakebtArray[index];
//                            }
//
//                            pickBT(lots, btArrayTemp);
//
//
////                            // Includes entire JSON response in alert //
////                            builder.setTitle("Input & Output");
////                            builder.setMessage("Cloud JSON Input: " + postParam.toString()
////                                    + "\n\nCloud String Response:\n" + response.toString()
////                                    + "\n\nCustom Response:\nbt - " + bt_string + ",lot - " + lot_string + "..." + numElements
////                                    + "\n\nArrays:\nlots - " + lots[0] + ", " + lots[1]
////                                    + "\nbtIndexArray - " + btIndexArray[0] + ", " + btIndexArray[1]
////                                    + "\nbtArray - " + btArrayTemp[0] + ", " + btArrayTemp[1]);
////                            android.app.AlertDialog alertDialog = builder.create();
////                            alertDialog.show();
//
//
////                            // Assuming Case 2 in notes // Jon
////                            // int numErrors = 0;   // Jon
////                            for (int i = 1; i <= finalCount; i++) {
//////                                status = jObject.getString("status");
//////                                if(!status.equals("error")){
//////                                    String temp = jObject.getString("bt_string" + String.valueOf(i));
//////                                    bt_string = bt_string.concat(temp + ";");
//////                                } else{
//////                                    i--;
//////                                }
////                                // if(String.valueOf(i).compareTo("error") == 0){   // Jon
////                                String temp = jObject.getString("bt_string" + String.valueOf(i));
////                                bt_string = bt_string.concat(temp + ";");
////                                // } else {         // Jon
////                                //    numErrors++;  // Jon
////                                // }                // Jon
////                            }
////
////                            // use "finalCount - numErrors" for array sizes
////                            String[] lots = new String[finalCount];
////                            String[] btArrayTemp = new String[finalCount];
////                            int i = 0;
////                            for (String retval : bt_string.split(";")) {
////                                lots[i] = retval;
////                                btArrayTemp[i] = fakebtArray[i];
////                                i++;
////                            }
////                            pickBT(lots, btArrayTemp);
//
//                        } catch (JSONException e) {
//
//                            e.printStackTrace();
//                            builder.setTitle("Catch BTdiff");
//                            builder.setMessage("Cloud JSON Input: " + postParam.toString() +
//                                    "\n\nCloud String Response:\n" + response.toString());
//                            android.app.AlertDialog alertDialog = builder.create();
//                            alertDialog.show();
//                        }
//
////                        // Includes entire JSON response in alert //
////                        builder.setTitle("Input & Output");
////                        builder.setMessage("Cloud JSON Input: " + postParam.toString()
////                                + "\n\nCloud String Response:\n" + response.toString());
////                        android.app.AlertDialog alertDialog = builder.create();
////                        alertDialog.show();
//                    }
//                }, new Response.ErrorListener() {
//
//            @Override
//            public void onErrorResponse(VolleyError error) {
//                VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
//                Toast.makeText(MainActivity.this, error.toString(), Toast.LENGTH_LONG).show();
//            }
//        }) {
//
//            // Passing bluetooth request header
//            @Override
//            public Map<String, String> getHeaders() throws AuthFailureError {
//                HashMap<String, String> headers = new HashMap<String, String>();
//                headers.put("Message_Type", "Bluetooth");
//                return headers;
//            }
//        };
//
//        jsonObjReq.setTag(LOG_TAG);
//        MySingleton.getInstance(MainActivity.this).addTorequestque(jsonObjReq);
//    }

    // ---------------------------------------------------------------------------------------------


    @Override
    public void onBackPressed() {

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);

        if (drawer.isDrawerOpen(GravityCompat.START)) {
            drawer.closeDrawer(GravityCompat.START);
        }
        // set Theme on Launch
        // settings = new SharedPref(this);
        else if (settings.loadNightModeState()) {
            setTheme(R.style.DarkTheme);
        } else if(!settings.loadNightModeState()) {
            setTheme(R.style.AppTheme);
        } else{
            Intent a = new Intent(Intent.ACTION_MAIN);
            a.addCategory(Intent.CATEGORY_HOME);
            a.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
            startActivity(a);
        }


//        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
//        if (drawer.isDrawerOpen(GravityCompat.START)) {
//            drawer.closeDrawer(GravityCompat.START);
//        } else {
//            super.onBackPressed();
//        }
//        // set Theme on Launch
//        // settings = new SharedPref(this);
//        if (settings.loadNightModeState()) {
//            setTheme(R.style.DarkTheme);
//        } else {
//            setTheme(R.style.AppTheme);
//        }

        Intent a = new Intent(Intent.ACTION_MAIN);
        a.addCategory(Intent.CATEGORY_HOME);
        a.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(a);
    }

    @Override
    protected void onResume() {
        super.onResume();
        //set Theme on Launch
        settings = new SharedPref(this);
        if (settings.loadNightModeState()) {
            setTheme(R.style.DarkTheme);
            navigationView.setBackgroundColor(getResources().getColor(R.color.darkBackgroundColor));
        } else {
            setTheme(R.style.AppTheme);
            navigationView.setBackgroundColor(getResources().getColor(R.color.lightBackgroundColor));
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        if (id == R.id.action_log_out) {

            // Reset User Info
            SharedPreferences sharedPrefUser = getSharedPreferences("login", Context.MODE_PRIVATE);
            SharedPreferences.Editor editorUser = sharedPrefUser.edit();
            editorUser.putString("loginStatus", "false");
            editorUser.apply();
            Toast.makeText(getApplicationContext(), "logging out", Toast.LENGTH_LONG).show();
            Intent myIntent = new Intent(this, LoginActivity.class);
            startActivity(myIntent);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }


    @SuppressWarnings("StatementWithEmptyBody")
    @Override
    public boolean onNavigationItemSelected(MenuItem item) {
        // Handle navigation view item clicks here.
        int id = item.getItemId();

        // Set theme on Launch
        if (settings.loadNightModeState()) {
            setTheme(R.style.DarkTheme);
        } else {
            setTheme(R.style.AppTheme);
        }

        if (id == R.id.nav_map_A) {
            setTitle("West Remote");
            MapAFragment fragment = new MapAFragment();
            android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
            fragmentTransaction.replace(R.id.frame, fragment, "fragment map A");
            fragmentTransaction.commit();
        } else if (id == R.id.nav_map_B) {
            setTitle("East Remote");
            MapBFragment fragment = null;
            SharedPreferences sharedPref = getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
            String userid = sharedPref.getString("id", "");
            fragment = new MapBFragment().newInstance(userid, uuidTwelve);
            android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
            fragmentTransaction.replace(R.id.frame, fragment, "fragment map B");
            fragmentTransaction.commit();
        } else if (id == R.id.nav_account) {
            setTitle("Account");
            AccountFragment fragment = new AccountFragment();
            android.support.v4.app.FragmentTransaction fragmentTransaction = getSupportFragmentManager().beginTransaction();
            fragmentTransaction.replace(R.id.frame, fragment, "fragment account");
            fragmentTransaction.commit();
        } else if (id == R.id.nav_settings) {
            setTitle("Settings");
            Intent i = new Intent(getApplicationContext(), Settings.class);
            startActivity(i);
            finish();
        }

        DrawerLayout drawer = (DrawerLayout) findViewById(R.id.drawer_layout);
        drawer.closeDrawer(GravityCompat.START);
        return true;
    }


}



















