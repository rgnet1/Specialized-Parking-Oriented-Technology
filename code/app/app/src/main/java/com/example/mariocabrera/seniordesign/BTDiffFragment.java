package com.example.mariocabrera.seniordesign;

import android.content.Context;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;


public class BTDiffFragment extends Fragment{

    private static final String LOG_TAG = "BTdiffFragment";

    // ---------------------------------------------------------------------------------------------
    // Receiving Data from Activity to Fragment

    public static BTDiffFragment newInstance(String[] messageOne, String[] messageTwo) {
        BTDiffFragment ff = new BTDiffFragment();
        //Supply the construction argument for this fragment
        Bundle b = new Bundle();
        b.putStringArray("lots", messageOne);
        b.putStringArray("btArray", messageTwo);
        ff.setArguments(b);
        return(ff);
    }
    // ---------------------------------------------------------------------------------------------


    public BTDiffFragment() {
        // Required empty public constructor
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment
        View v = inflater.inflate(R.layout.fragment_btdiff, container, false);
//        Toast.makeText(getActivity(), "btDiff fragment, onCreateView()", Toast.LENGTH_LONG).show();

        // -----------------------------------------------------------------------------------------
        // Receiving Data from Activity to Fragment

        final String[] lots = getArguments().getStringArray("lots");
        final String[] btArray = getArguments().getStringArray("btArray");


        // -----------------------------------------------------------------------------------------

        ListAdapter myAdapter = new ArrayAdapter<String>(getActivity(), android.R.layout.simple_list_item_1, lots);
        ListView btListView = (ListView) v.findViewById(R.id.btListView);
        btListView.setAdapter(myAdapter);

        btListView.setOnItemClickListener(
                new AdapterView.OnItemClickListener() {
                    @Override
                    public void onItemClick(AdapterView<?> parent, View view, int position, long l) {
                        String curLot = String.valueOf(parent.getItemAtPosition(position));
                        Toast.makeText(getActivity(), String.valueOf(curLot), Toast.LENGTH_SHORT).show();
                        ((MainActivity)getActivity()).sendJSONverifyStuff(btArray[position]);
                        popBackStackTillEntry(0);
                    }

                }
        );

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
