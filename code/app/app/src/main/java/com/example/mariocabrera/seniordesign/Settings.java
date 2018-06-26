package com.example.mariocabrera.seniordesign;

import android.content.Intent;
import android.graphics.Color;
import android.os.Bundle;
import android.support.v4.view.GravityCompat;
import android.support.v4.widget.DrawerLayout;
import android.support.v7.app.AppCompatActivity;
import android.widget.CompoundButton;
import android.widget.Switch;

public class Settings extends AppCompatActivity {

    private Switch darkToggle;
    SharedPref settings;
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        // Set Theme on Launch
        settings = new SharedPref(this);
        if(settings.loadNightModeState()){
            setTheme(R.style.DarkTheme);
        }else{
            setTheme(R.style.AppTheme);
        }


        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_settings);

        darkToggle = (Switch) findViewById(R.id.darkModeSwithId);
        if(settings.loadNightModeState()){
            darkToggle.setChecked(true);
        }

        // Toggle Listener
        darkToggle.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if(isChecked){
                    settings.setNightModeState(true);

                }else{
                    settings.setNightModeState(false);
                }
                restartApp();
            }
        });
    }

    public void restartApp(){
        Intent i = new Intent(getApplicationContext(),Settings.class);
        startActivity(i);
        finish();
    }


    @Override
    public void onBackPressed() {
            super.onBackPressed();
        // Set theme on Launch
        Intent i = new Intent(getApplicationContext(),MainActivity.class);
        startActivity(i);
    }

}
