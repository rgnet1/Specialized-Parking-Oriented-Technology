package com.example.mariocabrera.seniordesign;


import android.content.Context;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.graphics.PorterDuff;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.widget.SwipeRefreshLayout;
import android.support.v7.widget.CardView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;


/**
 * A simple {@link Fragment} subclass.
 */

// If you need buttons, add "implements View.OnClickListener" before parantheses below
public class MapAFragment extends Fragment implements SwipeRefreshLayout.OnRefreshListener {

    static int toggleUpdate = 1;
    ProgressBar prg;

    private SwipeRefreshLayout swipeRefreshLayout;
//    TextView numSpotsA, curLotA, curSpotA;
    TextView numSpotsA, curLotA, curSpotA, numSpotsATitle, curLotATitle;
    private static final String LOG_TAG = "MapAFragment";

    public MapAFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_map_a, container, false);

        SharedPreferences sharedPref = this.getActivity().getSharedPreferences("WestInfo", Context.MODE_PRIVATE);
        String spots = sharedPref.getString("spotsLeftWest", "");

        // Progress bar
        prg = (ProgressBar) v.findViewById(R.id.progressBar);
        double s = Integer.parseInt(spots);
        s =  (1 - (s/20)) * 100;
        String fullPercentage = String.valueOf((int) s);
        Log.d("APPPPPP","the your message will go here" + fullPercentage);
        prg.setProgress((int) s);
        if( s > 79){
            prg.getProgressDrawable().setColorFilter(
                    Color.RED, PorterDuff.Mode.SRC_IN
            );
        }


        numSpotsA = (TextView) v.findViewById(R.id.NumSpotsA);
        numSpotsATitle = (TextView) v.findViewById((R.id.numSpotsATitle));
        numSpotsATitle.setText("West Remote");
        numSpotsA.setText("Open Spots: " + spots+"\n("+fullPercentage+"% full)");

        // Display User's current spot
        SharedPreferences sharedPrefSpot = this.getActivity().getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
        String curSpot = sharedPrefSpot.getString("spots", "None");
        String curLot = sharedPrefSpot.getString("parkingLot", "None");
        curLotA = (TextView) v.findViewById(R.id.LotA);
        curSpotA = (TextView) v.findViewById(R.id.SpotA);
        curLotATitle = (TextView) v.findViewById(R.id.curLotATitle);

        //Assign Text
        curLotATitle.setText("Your Parking Location");
        curLotA.setText(curLot);
        curSpotA.setText("You are in spot: " + curSpot);

        // Don't display the User card if its not the right lot
        // or if the user is not in a spot
        CardView userCard = (CardView) v.findViewById(R.id.userCardA);
        if(curLot.equals("None")|| !curLot.equals("WestRemote")){
            userCard.setVisibility(v.GONE);
        }else{
            userCard.setVisibility(v.VISIBLE);
        }

        // -----------------------------------------------------------------------------------------

        // SwipeToRefresh
        swipeRefreshLayout = (SwipeRefreshLayout) v.findViewById(R.id.swipeMapA);
        swipeRefreshLayout.setOnRefreshListener(this);

        if(toggleUpdate == 1){
            toggleUpdate = 0;
            onRefresh();
        }
        return v;
    }

    // SwipeToRefresh
    @Override
    public void onRefresh(){
//        Toast.makeText(getActivity(), "MapA, onRefresh()", Toast.LENGTH_LONG).show();
        ((MainActivity)getActivity()).updateMapA();

        if(swipeRefreshLayout.isRefreshing()){
            swipeRefreshLayout.setRefreshing(false);
        }
    }
}


