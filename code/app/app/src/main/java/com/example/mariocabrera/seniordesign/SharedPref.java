package com.example.mariocabrera.seniordesign;

import android.content.Context;
import android.content.SharedPreferences;

public class SharedPref {
    SharedPreferences settings;
    public SharedPref(Context context){
        settings = context.getSharedPreferences("settings", Context.MODE_PRIVATE);
    }

    //this will save the NightMode state: True or False
    public void setNightModeState(Boolean state){
        SharedPreferences.Editor editor = settings.edit();
        editor.putBoolean("NightMode",state);
        editor.commit();
    }

    //this method will load the Night Mode State
    public Boolean loadNightModeState(){
        Boolean state = settings.getBoolean("NightMode",false);
        return state;
    }
}

