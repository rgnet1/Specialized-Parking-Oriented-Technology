package com.example.mariocabrera.seniordesign;

import android.content.Context;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
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

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashMap;
import java.util.Map;


public class PaymentOptionFragment extends Fragment {

    private static final String LOG_TAG = "PaymentOptionFragment";

    public String useridRec;
    public String uuidRec;

    //----------------------------------------------------------------------------------------------
    // HTTP Post Request

    String serverURLverify = "http://car-management-66420.appspot.com/mobileapp";
    String serverURLpayment = "http://car-management-66420.appspot.com/payment";
    android.app.AlertDialog.Builder builder;
    static int toggleSendJSON = 1;

    // Use link below to check the status for each spot (Database must be on)
    // http://car-management-66420.appspot.com/Database

    //----------------------------------------------------------------------------------------------


    public PaymentOptionFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_payment_option, container, false);

        // Button Yes
        Button btnPaymentYes = (Button) v.findViewById(R.id.btnPaymentYes);
        btnPaymentYes.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {


                ((MainActivity)getActivity()).sendJSONpaymentStuff();
                popBackStackTillEntry(0);

            }
        });

        // Button No
        Button btnPaymentNo = (Button) v.findViewById(R.id.btnPaymentNo);
        btnPaymentNo.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                ((MainActivity)getActivity()).paymentUnconfirmedAlert();
                popBackStackTillEntry(0);

            }
        });

        return v;
    }

    public void popBackStackTillEntry(int entryIndex) {

        if (getActivity().getSupportFragmentManager() == null) {
            return;
        }
        if (getActivity().getSupportFragmentManager().getBackStackEntryCount() <= entryIndex) {
            return;
        }
        FragmentManager.BackStackEntry entry = getActivity().getSupportFragmentManager().getBackStackEntryAt(
                entryIndex);
        if (entry != null) {
            getActivity().getSupportFragmentManager().popBackStackImmediate(entry.getId(),
                    FragmentManager.POP_BACK_STACK_INCLUSIVE);
        }

    }


}
