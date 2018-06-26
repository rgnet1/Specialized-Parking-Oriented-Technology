package com.example.mariocabrera.seniordesign;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;



public class ParkingNotAllowedFragment extends Fragment {

    public ParkingNotAllowedFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_parking_not_allowed, container, false);

//        // Button Exit
//        Button btnExitVerifiedPermits = (Button) v.findViewById(R.id.btnExitVerifiedPermits);
//        btnExitVerifiedPermits.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//
//                Toast.makeText(getActivity(), "Exit clicked", Toast.LENGTH_SHORT).show();
//
//
//            }
//        });

        return v;
    }
}
