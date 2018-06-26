package com.example.mariocabrera.seniordesign;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.VolleyLog;
import com.android.volley.toolbox.JsonObjectRequest;

import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;

import static android.app.FragmentManager.POP_BACK_STACK_INCLUSIVE;


public class PermitValidFragment extends Fragment {


    //----------------------------------------------------------------------------------------------
    // HTTP Post Request

    private static final String LOG_TAG = "Permit Valid";
    String server_url = "http://car-management-66420.appspot.com/mobileapp";
//    String server_url = "http://car-management-66420.appspot.com/payment";
    android.app.AlertDialog.Builder builder;

    //----------------------------------------------------------------------------------------------


    public PermitValidFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_permit_valid, container, false);

        // Button Exit
        Button btnExitVerifiedPermits = (Button) v.findViewById(R.id.btnExitVerifiedPermits);
        btnExitVerifiedPermits.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                Toast.makeText(getActivity(), "Exit clicked", Toast.LENGTH_SHORT).show();
                sendJSON();

//                // Works...but crashes
//                // https://www.youtube.com/watch?v=eUG3VWnXFtg
//                // https://www.programcreek.com/java-api-examples/?class=android.support.v4.app.FragmentManager&method=popBackStack
//                FragmentManager manager = getActivity().getSupportFragmentManager();
//                manager.popBackStack(null, POP_BACK_STACK_INCLUSIVE);


//                FragmentManager manager = getActivity().getSupportFragmentManager();
//                manager.popBackStack("fragment map B", POP_BACK_STACK_INCLUSIVE);

                FragmentManager manager = getActivity().getSupportFragmentManager();
                manager.popBackStack();



//                final PermitValidFragment fragment = new PermitValidFragment();
//                FragmentManager manager = getActivity().getSupportFragmentManager();
//                android.support.v4.app.FragmentTransaction trans = manager.beginTransaction();
//                trans.remove(fragment);
//                trans.commit();
//                manager.popBackStack();

//                FragmentManager fm = getActivity().getSupportFragmentManager();
//                fm.popBackStack(FragmentManager.POP_BACK_STACK_INCLUSIVE);
//                fm.popBackStack("PermitValidFragment", FragmentManager.POP_BACK_STACK_INCLUSIVE);


//                MapAFragment fragment = new MapAFragment();
//                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
//                ft.replace(R.id.frame, fragment, "fragment map A");
//                ft.commit();

//                String userid = "1429392";
//                String uuidTwelve = "defaultTwelve";
//
//                MapBFragment fragment = null;
//                fragment = new MapBFragment().newInstance(userid, uuidTwelve);
//                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
//                ft.replace(R.id.frame, fragment, "fragment map B");
//                ft.commit();

//                if (getFragmentManager().getBackStackEntryCount() == 0) {
//                    getActivity().finish();
//                } else {
//                    getFragmentManager().popBackStack();
//                }

//                getActivity().getFragmentManager().beginTransaction().remove(fragment).commit();

//                getActivity().getSupportFragmentManager().popBackStackImmediate();\


//                FragmentActivity myContext = null;
//
//                final FragmentManager fragManager = myContext.getSupportFragmentManager();
//                while (fragManager.getBackStackEntryCount() != 0) {
//                    fragManager.popBackStackImmediate();
//                }

//                MapAFragment fragment = new MapAFragment();
//                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
//                ft.replace(R.id.frame, fragment, "fragment map A");
//                ft.commit();

//                MapAFragment fragment = new MapAFragment();
//                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
//                ft.replace(R.id.frame, fragment, "fragment map A");
//                ft.commit();

//                // This portion works
//                String userid = "1429392";
//                String uuidTwelve = "defaultTwelve";
//                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
//                ft.replace(R.id.frame, new MapBFragment().newInstance(userid, uuidTwelve), "waiting fragment");
//                ft.commit();


            }
        });

        return v;
    }

    //----------------------------------------------------------------------------------------------
    // HTTP JSON Request

    public void sendJSON() {
        builder = new android.app.AlertDialog.Builder(getActivity());

        final Map<String, String> postParam = new HashMap<String, String>();
        postParam.put("bt_string", "173817381738");   // 173817381738
        postParam.put("userid", "1429392");
        JSONObject parameters = new JSONObject(postParam);
        // Toast.makeText(MainActivity.this, postParam.toString(), Toast.LENGTH_LONG).show();

        JsonObjectRequest jsonObjReq = new JsonObjectRequest(Request.Method.POST,
                server_url, parameters,
                new Response.Listener<JSONObject>() {

                    @Override
                    public void onResponse(JSONObject response) {

                        // Toast.makeText(MainActivity.this, response.toString(), Toast.LENGTH_LONG).show();

                        builder.setTitle("Input & Output");
                        builder.setMessage("Cloud JSON Input: " + postParam.toString() + "\n\nCloud String Response:\n" + response.toString());
                        android.app.AlertDialog alertDialog = builder.create();
                        alertDialog.show();


                    }
                }, new Response.ErrorListener() {

            @Override
            public void onErrorResponse(VolleyError error) {
                VolleyLog.d(LOG_TAG, "Error: " + error.getMessage());
                Toast.makeText(getActivity(), error.toString(), Toast.LENGTH_LONG).show();
            }
        }) {
            // Passing some request headers //

            // mobileapp header
//            @Override
//            public Map<String, String> getHeaders() throws AuthFailureError {
//                HashMap<String, String> headers = new HashMap<String, String>();
//                headers.put("Message_Type", "Verify");
//                return headers;
//            }

            // payment header
            @Override
            public Map<String, String> getHeaders() throws AuthFailureError {
                HashMap<String, String> headers = new HashMap<String, String>();
                headers.put("Message_Type", "Payment");
                return headers;
            }
        };

        jsonObjReq.setTag(LOG_TAG);
        MySingleton.getInstance(getActivity()).addTorequestque(jsonObjReq);
    }
    //----------------------------------------------------------------------------------------------



}
