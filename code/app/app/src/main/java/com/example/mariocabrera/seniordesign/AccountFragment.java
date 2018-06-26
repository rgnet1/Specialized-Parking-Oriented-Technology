package com.example.mariocabrera.seniordesign;


import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.widget.SwipeRefreshLayout;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;


/**
 * A simple {@link Fragment} subclass.
 */
public class AccountFragment extends Fragment implements SwipeRefreshLayout.OnRefreshListener{

    private SwipeRefreshLayout swipeRefreshLayout;
    Button transactionHistBtn;
    TextView NameView, emailView, idView, permitView, balanceView, licenseView;

    private static final String LOG_TAG = "AccountFragment";

    public AccountFragment() {
        // Required empty public constructor
    }


    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_account, container, false);
        // Display number of spots on start up
        SharedPreferences sharedPref = this.getActivity().getSharedPreferences("accountInfo", Context.MODE_PRIVATE);
        String firstNameString =  sharedPref.getString("firstName", "");
        String lastNameString =   sharedPref.getString("lastName", "");
        String emailString =      sharedPref.getString("email", "");
        String idString =         sharedPref.getString("id", "");
        String permitString =     sharedPref.getString("permit", "");
        String balanceString =    sharedPref.getString("balance", "");
        String licenseString =    sharedPref.getString("license", "");

        NameView = (TextView) v.findViewById(R.id.Name);
        //lastNameView = (TextView) v.findViewById(R.id.LastName);
        emailView = (TextView) v.findViewById(R.id.email);
        idView = (TextView) v.findViewById(R.id.ID);
        permitView = (TextView) v.findViewById(R.id.permit);
        balanceView = (TextView) v.findViewById(R.id.balance);
        licenseView = (TextView) v.findViewById(R.id.license);

        NameView.setText(firstNameString+" "+lastNameString);
//        lastNameView.setText(lastNameString);
        emailView.setText(emailString);
        idView.setText(idString);
        permitView.setText(permitString);
        balanceView.setText("$"+balanceString);
        licenseView.setText(licenseString);

      //  super.onCreate(savedInstanceState);
        transactionHistBtn = (Button)v.findViewById(R.id.transactionHistoryBtn);
        transactionHistBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Toast.makeText(getActivity(), "Transaction History",
                        Toast.LENGTH_SHORT).show();
            }
        });

        // SwipeToRefresh
        swipeRefreshLayout = (SwipeRefreshLayout) v.findViewById(R.id.swipeAccount);
        swipeRefreshLayout.setOnRefreshListener(this);

        return v;
    }

    // SwipeToRefresh
    @Override
    public void onRefresh(){

        ((MainActivity)getActivity()).updateAccount();

        if(swipeRefreshLayout.isRefreshing()){
            swipeRefreshLayout.setRefreshing(false);
        }
    }



}
