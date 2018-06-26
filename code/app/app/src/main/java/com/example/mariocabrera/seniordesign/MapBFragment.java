package com.example.mariocabrera.seniordesign;


import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.os.Bundle;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.support.annotation.NonNull;
import android.support.v4.app.Fragment;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.CardView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;
import android.app.FragmentManager;
import android.app.FragmentTransaction;

import com.android.volley.AuthFailureError;
import com.android.volley.Request;
import com.android.volley.Response;
import com.android.volley.VolleyError;
import com.android.volley.VolleyLog;
import com.android.volley.toolbox.JsonObjectRequest;
import com.android.volley.toolbox.StringRequest;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;

/**
 * A simple {@link Fragment} subclass.
 */

// If you need buttons, add "implements View.OnClickListener" before parantheses below
public class MapBFragment extends Fragment implements SwipeRefreshLayout.OnRefreshListener{

    private SwipeRefreshLayout swipeRefreshLayout;
//    Button btn1B, btn1C, btn2B;
    TextView numSpotsB, curLotB, curSpotB, numSpotsBTitle, curLotBTitle;
    private static final String LOG_TAG = "MapBFragment";

    // ---------------------------------------------------------------------------------------------
    // Receiving Data from Activity to Fragment

//    public String userid = "defaultUser";
//    public String uuidTwelve = "defaultTwelve";

    public static MapBFragment newInstance(String messageOne, String messageTwo) {

        MapBFragment ff = new MapBFragment();
        //Supply the construction argument for this fragment
        Bundle b = new Bundle();
        b.putString("userid", messageOne);
        b.putString("uuidTwelve", messageTwo);
        ff.setArguments(b);
        return(ff);
    }
    // ---------------------------------------------------------------------------------------------


    public MapBFragment() {
        // Required empty public constructor
    }

    View v;
    ProgressBar prg;
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        v = inflater.inflate(R.layout.fragment_map_b, container, false);
//        Toast.makeText(getActivity(), "MapA, onCreateView()", Toast.LENGTH_LONG).show();

//        // Button B Stuff
//        btn1B = (Button)v.findViewById(R.id.btn1B);
//        btn1B.setOnClickListener((View.OnClickListener) this);
//
//        btn1C = (Button)v.findViewById(R.id.btn1C);
//        btn1C.setOnClickListener(this);
//
//        btn2B = (Button)v.findViewById(R.id.btn2B);
//        btn2B.setOnClickListener(this);


        // -----------------------------------------------------------------------------------------
        // Receiving Data from Activity to Fragment

        //TextView tv=(TextView) v.findViewById(R.id.text1);
        String uuidRec = getArguments().getString("uuidTwelve");
        String useridRec = (String) getArguments().getString("userid");
//        btn2B.setText(useridRec + ", " + uuidRec);
        // -----------------------------------------------------------------------------------------

        // -----------------------------------------------------------------------------------------
        // Display number of spots on start up

        SharedPreferences sharedPref = this.getActivity().getSharedPreferences("EastInfo", Context.MODE_PRIVATE);
        String spots = sharedPref.getString("spotsLeftEast", "");


        // Progress bar
        prg = (ProgressBar) v.findViewById(R.id.progressBar);
        double s = Integer.parseInt(spots);
        s =  (1 - (s/51)) * 100;
        String fullPercentage = String.valueOf((int) s);
        //Log.d("APPPPPP","the your message will go here" + fullPercentage);
        prg.setProgress((int) s);
        //prg.setProgress(66);
        if( s > 79){
            prg.getProgressDrawable().setColorFilter(
                    Color.RED, PorterDuff.Mode.SRC_IN
            );
        }

        numSpotsB = (TextView) v.findViewById(R.id.NumSpotsB);
        numSpotsBTitle = (TextView) v.findViewById((R.id.numSpotsBTitle));
        numSpotsBTitle.setText("East Remote");
        numSpotsB.setText("Open Spots: " + spots+"\n("+fullPercentage+"% full)");
        //numSpotsB.setText("Open Spots: " + spots+"\n(66% full)");

        // Display User's current spot
        SharedPreferences sharedPrefSpot = this.getActivity().getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
        String curSpot = sharedPrefSpot.getString("spots", "None");
        String curLot = sharedPrefSpot.getString("parkingLot", "None");
        curLotB = (TextView) v.findViewById(R.id.LotB);
        curSpotB = (TextView) v.findViewById(R.id.SpotB);
        curLotBTitle = (TextView) v.findViewById(R.id.curLotBTitle);

        //Assign Text
        curLotBTitle.setText("Your Parking Location");
        curLotB.setText(curLot);
        curSpotB.setText("You are in spot: "+curSpot);

        //Don't display the User card if its not the right lot
        //or if the user is not in a spot
        CardView userCard = (CardView) v.findViewById(R.id.userCardB);
        if(curLot.equals("None") || !(curLot.equals("EastRemote"))){
            userCard.setVisibility(v.GONE);
        }else{
            userCard.setVisibility(v.VISIBLE);
        }
        // -----------------------------------------------------------------------------------------


        // SwipeToRefresh
        swipeRefreshLayout = (SwipeRefreshLayout) v.findViewById(R.id.swipeMapB);
        swipeRefreshLayout.setOnRefreshListener(this);

        return v;
    }




//    // Button Stuff
//    @Override
//    public void onClick(View view) {
//        switch(view.getId()){
//            case R.id.btn1B:
//
//
//                Toast.makeText(getActivity(), "Button 1B clicked", Toast.LENGTH_SHORT).show();
//                String[] btArray = {"886788678867"};
//                ((MainActivity)getActivity()).btRequestSpots(btArray);
//
//                // Start Popups
////                Intent intent = new Intent(getActivity(), WaitingPop.class);
////                getActivity().startActivity(intent);
////                break;
//
//                // Request and Update number of empty spots
////                ((MainActivity)getActivity()).requestSpots("East");
////                String spots = ((MainActivity)getActivity()).accessSpots("East");
////                Toast.makeText(getActivity(), spots, Toast.LENGTH_LONG).show();
////                TextView numSpotsB = (TextView) getView().findViewById(R.id.NumSpotsB);
////                numSpotsB.setText(spots);
////                ((MainActivity)getActivity()).requestSpots("West");   // Map A
//
//
//
//
//
//                break;
//
////            case R.id.btn1C:
////
////                // Update Account Info
////                // Display number of spots on start up
//////                SharedPreferences sharedPref = this.getActivity().getSharedPreferences("userInfo", Context.MODE_PRIVATE);
//////                String email = sharedPref.getString("email", "");
//////                String password = sharedPref.getString("password", "");
//////                ((MainActivity)getActivity()).requestAccountInfo(email, password);
////
////                break;
////
////            case R.id.btn2B:
////
////                // ---------------------------------------------------------------------------------
////                // Passing Data from Fragment to Fragment
////                String messageOne = getArguments().getString("userid");
////                String messageTwo = getArguments().getString("uuidTwelve");
////                Bundle bundle = new Bundle();
////                bundle.putString("userid", messageOne);
////                bundle.putString("uuidTwelve", messageTwo);
////                WaitingFragment waitingFragment = new WaitingFragment();
////                waitingFragment.setArguments(bundle);
////                // ---------------------------------------------------------------------------------
////
////                Toast.makeText(getActivity(), "Button 2B clicked", Toast.LENGTH_SHORT).show();
////
////                // Change fragment
////                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
////                ft.add(R.id.frame, waitingFragment, "fragment waiting");
////                ft.addToBackStack("fragment waiting");
////                ft.commit();
////                break;
//
//        }
//    }


    // SwipeToRefresh
    @Override
    public void onRefresh(){

//        Toast.makeText(getActivity(), "MapB, onRefresh()", Toast.LENGTH_LONG).show();
        ((MainActivity)getActivity()).updateMapB();

        if(swipeRefreshLayout.isRefreshing()){
            swipeRefreshLayout.setRefreshing(false);
        }
    }

}
