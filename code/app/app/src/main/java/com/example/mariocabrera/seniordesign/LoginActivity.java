package com.example.mariocabrera.seniordesign;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.VolleyLog;
import com.android.volley.toolbox.JsonObjectRequest;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;
import java.util.logging.Logger;

/**
 * A login screen that offers login via email/password.
 */
public class LoginActivity extends AppCompatActivity {

    private static final String LOG_TAG = "LoginActivity";

    //----------------------------------------------------------------------------------------------
    // Variables

    EditText emailInput;
    EditText passwordInput;
    TextView loginTextView;
    Button email_sign_in_button;
    //----------------------------------------------------------------------------------------------


    //----------------------------------------------------------------------------------------------
    // HTTP Post Request

    String serverURLlogin = "https://car-management-66420.appspot.com/login";
    String serverURLmobileinfo = "https://car-management-66420.appspot.com/mobileinfo";
    android.app.AlertDialog.Builder builder;

    // Use link below to check the status for each spot (Database must be on)
    // http://car-management-66420.appspot.com/Database

    //----------------------------------------------------------------------------------------------

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getSupportActionBar().hide();

//        Toast.makeText(LoginActivity.this, "LoginActivity, onCreate()", Toast.LENGTH_LONG).show();

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            Window w = getWindow();
            w.setFlags(WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS, WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS);
        }


        setContentView(R.layout.activity_login);
        final ProgressBar bar = (ProgressBar) findViewById(R.id.progressBar);
        bar.getIndeterminateDrawable().setColorFilter(
                getResources().getColor(R.color.colorWhite),
                android.graphics.PorterDuff.Mode.SRC_IN);
        bar.setVisibility(View.GONE);
        emailInput = (EditText) findViewById(R.id.email);
        passwordInput = (EditText) findViewById(R.id.password);
        loginTextView = (TextView) findViewById(R.id.loginTextView);

        // Button functionality
        email_sign_in_button = (Button) findViewById(R.id.email_sign_in_button);
        email_sign_in_button.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {

                bar.setVisibility(View.VISIBLE);
                String email = emailInput.getText().toString();
                String password = passwordInput.getText().toString();

                // Verify Credentials
                sendLoginVerify(email, password, bar);
                for(int i = 0; i < 100;++i) {
                    bar.setProgress(i);
                }
            }

        });

    }

    // Save the users log in info
    public void saveInfo(String email, String password){

        Toast.makeText(LoginActivity.this, "LoginActivity, saveInfo()", Toast.LENGTH_LONG).show();

        // Save to SharedPreferences
        SharedPreferences sharedPref = getSharedPreferences("userInfo", Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = sharedPref.edit();
        editor.putString("email", email);
        editor.putString("password", password);
        editor.apply();

        if((email.compareTo("") == 0) || (password.compareTo("") == 0)){
            // If SharedPreferences are empty, and user inputs blank credentials,
            // toast that inputs are invalid
            Toast.makeText(this, "Please enter valid username and password", Toast.LENGTH_SHORT).show();
        } else{

            Toast.makeText(LoginActivity.this, "Logged In!", Toast.LENGTH_LONG).show();

            // Save the user's login status
            SharedPreferences sharedPrefUser = getSharedPreferences("login", Context.MODE_PRIVATE);
            SharedPreferences.Editor editorUser = sharedPrefUser.edit();
            editorUser.putString("loginStatus", "true");
            editorUser.apply();

            // Go to MainActivity
            Intent myIntent = new Intent(this, MainActivity.class);
            startActivity(myIntent);
        }
    }

    //----------------------------------------------------------------------------------------------
    // HTTP Login JSON Request: Verify

    public void sendLoginVerify(final String email, final String password, final ProgressBar bar) {
//        Toast.makeText(LoginActivity.this, "LoginActivity, sendLoginVerify()", Toast.LENGTH_LONG).show();
        builder = new android.app.AlertDialog.Builder(LoginActivity.this);
        JSONObject parameters = null;
        final Map<String, String> postParam = new HashMap<String, String>();

        postParam.put("email", email);
        postParam.put("password", password);
        parameters = new JSONObject(postParam);

        String server_url = serverURLlogin;

        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
                server_url, parameters,
                new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Parses current balance out of JSON response //
                        String jsonString = response.toString();
                        try {
                            JSONObject jObject = new JSONObject(jsonString);
                            String status = jObject.getString("login");

                            if(status.compareTo("verified") == 0){
                                requestAccountInfo(email, password);
                                bar.setVisibility(View.GONE);
                            } else if(status.compareTo("failed") == 0){
                                bar.setVisibility(View.GONE);
                                builder.setTitle("Log In Failed");
                                builder.setMessage("Username or password incorrect.\n");
                                android.app.AlertDialog alertDialog = builder.create();
                                alertDialog.show();
                            } else{
                                bar.setVisibility(View.GONE);
                                builder.setTitle("Developer: Check Log In \"sendLoginVerify\"");
                                builder.setMessage("Cloud JSON Input: " + postParam.toString() +
                                        "\n\nCloud String Response:\n" + response.toString());
                                android.app.AlertDialog alertDialog = builder.create();
                                alertDialog.show();
                            }

                        } catch (JSONException e) {
                            e.printStackTrace();
                            builder.setTitle("Developer Error: sendLoginVerify() catch block, LoginActivity");
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
                Toast.makeText(LoginActivity.this, error.toString(), Toast.LENGTH_LONG).show();
            }
        }) {

            // Passing login request header
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                HashMap<String, String> headers = new HashMap<String, String>();
                headers.put("Message_Type", "Login");
                return headers;
            }

        };

        jsonObjReq.setTag(LOG_TAG);
        MySingleton.getInstance(LoginActivity.this).addTorequestque(jsonObjReq);
    }
    //----------------------------------------------------------------------------------------------



    //----------------------------------------------------------------------------------------------
    // HTTP POST Request: Account Info

    public void requestAccountInfo(final String userEmail, final String userPassword) {
//        Toast.makeText(LoginActivity.this, "LoginActivity, requestAccountInfo()", Toast.LENGTH_LONG).show();
        builder = new android.app.AlertDialog.Builder(LoginActivity.this);
        JSONObject parameters = null;
        final Map<String, String> postParam = new HashMap<String, String>();

        postParam.put("email", userEmail);
        postParam.put("password", userPassword);
        parameters = new JSONObject(postParam);

        String server_url = serverURLmobileinfo;

        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
                server_url, parameters,
                new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Parses values out of JSON response //
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
                            saveInfo(userEmail, userPassword);

                        } catch (JSONException e) {
                            e.printStackTrace();

                            // Display an alert if an error with the JSON occurred
                            builder.setTitle("Developer Error: requestAccountInfo() catch block, LoginActivity");
                            builder.setMessage("Cloud JSON Input: " + postParam.toString() +
                                    "\n\nCloud String Response:\n" + response.toString());
                            android.app.AlertDialog alertDialog = builder.create();
                            alertDialog.show();
                        }
                    }
                }, new Response.ErrorListener() {

            @Override
            public void onErrorResponse(VolleyError error) {

                // Toast any error response
                VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
                Toast.makeText(LoginActivity.this, error.toString(), Toast.LENGTH_LONG).show();
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
        MySingleton.getInstance(LoginActivity.this).addTorequestque(jsonObjReq);
    }


    //----------------------------------------------------------------------------------------------
    // Close app when back button is pressed
    @Override
    public void onBackPressed(){
        Intent a = new Intent(Intent.ACTION_MAIN);
        a.addCategory(Intent.CATEGORY_HOME);
        a.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(a);
    }


}



