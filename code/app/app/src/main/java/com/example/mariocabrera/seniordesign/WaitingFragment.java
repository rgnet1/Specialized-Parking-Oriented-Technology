package com.example.mariocabrera.seniordesign;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.Toast;


public class WaitingFragment extends Fragment {


    public WaitingFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_waiting, container, false);

        // -----------------------------------------------------------------------------------------
        // Receiving Data from Fragment to Fragment
        Bundle bundleArgs = getArguments();
        String useridRec = bundleArgs.getString("userid");
        String uuidRec = bundleArgs.getString("uuidTwelve");
        Toast.makeText(getActivity(), "userid: " + useridRec + "\nuuid: " + uuidRec, Toast.LENGTH_SHORT).show();

        // -----------------------------------------------------------------------------------------

        // Button Valid Permit
        Button btnValidPermit = (Button) v.findViewById(R.id.btnValidPermit);

        btnValidPermit.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(getActivity(), "Valid Permit", Toast.LENGTH_SHORT).show();
                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
                ft.add(R.id.frame, new PermitValidFragment(), "fragment permit valid");
                ft.addToBackStack("fragment permit valid");
                ft.commit();
//                getActivity().getFragmentManager().popBackStack();


            }
        });

        // Button Payment Option
        Button btnPaymentOption = (Button) v.findViewById(R.id.btnPaymentOption);
        btnPaymentOption.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                // ---------------------------------------------------------------------------------
                // Passing Data from Fragment to Fragment
                String messageOne = getArguments().getString("userid");
                String messageTwo = getArguments().getString("uuidTwelve");
                Bundle bundle = new Bundle();
                bundle.putString("userid", messageOne);
                bundle.putString("uuidTwelve", messageTwo);
                PaymentOptionFragment paymentOptionFragment = new PaymentOptionFragment();
                paymentOptionFragment.setArguments(bundle);
                // ---------------------------------------------------------------------------------


                Toast.makeText(getActivity(), "Payment Option", Toast.LENGTH_SHORT).show();
                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
                ft.add(R.id.frame, paymentOptionFragment, "fragment payment option");
                ft.addToBackStack(null);
                ft.commit();

            }
        });

        // Button Parking Not Allowed
        Button btnParkingNotAllowed = (Button) v.findViewById(R.id.btnParkingNotAllowed);
        btnParkingNotAllowed.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(getActivity(), "Parking Not Allowed", Toast.LENGTH_SHORT).show();
                final android.support.v4.app.FragmentTransaction ft = getFragmentManager().beginTransaction();
                ft.add(R.id.frame, new ParkingNotAllowedFragment(), "fragment parking not allowed");
                ft.addToBackStack(null);
                ft.commit();

            }
        });

        return v;
    }



}
