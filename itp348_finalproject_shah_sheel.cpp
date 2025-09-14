
#include "Particle.h"
SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler(LOG_LEVEL_INFO);


#define BLYNK_TEMPLATE_ID "TMPL22qVV82t6"
#define BLYNK_TEMPLATE_NAME "Final Project ITP 348 Helmet"
#define BLYNK_AUTH_TOKEN "NzY8037vUJyFV7_IrTycrS9Su3xYF15G"
#include <blynk.h>

#include <PulseSensorAmped.h>

// defining prats here 

// for PIR sensors
const int MOTION_PIN1 = D2;
const int MOTION_PIN2 = D3;
const int MOTION_PIN3 = D4;
const int MOTION_PIN4 = D5;  

// for all LEDs
const int LED_PIN1 = D1;
const int LED_PIN2 = D0;
const int MEDICAL_PIN = A1;
const int LED_PIN3 = D8;

// for vibration motors
const int VIBR_PIN1 = D6;
const int VIBR_PIN2 = D10;
const int VIBR_PIN3 = D7;

// for Pulse Sensor
const int pulseSignalPin = A0;

PulseSensor PulseSensorAmped;

double currentBPM = 0;

// This is connected to the button. when we press, it sends an alert in the Blynk app and to the email on the account. 
BLYNK_WRITE(V0) {
    if (param.asInt() == 1) {
        Blynk.logEvent("backup_request", "The soldier has requested backup");
    }
}


// Same thing as V0 but for the medical assistance instead. Also lights up a red LED to signify to the soldier that it's gone through. 
BLYNK_WRITE(V1) {
    if (param.asInt() == 1) {
        Blynk.logEvent("medical_assistance", "The soldier has requested medical assistance");
        Serial.println("Calling for Medical Help");
        digitalWrite(MEDICAL_PIN, HIGH);
    } else {
        digitalWrite(MEDICAL_PIN, LOW);
    }
}


// This does a couple of things 
// Gets the pulse sensor working, which allows me to store the BPM data as a variable, so I can display it in the app, and Initial State
// Also, if the heart rate is too high, it vibrates all 3 motors at once, so the soldier knows that the HR is too high. 

void PulseSensorAmped_data(int BPM, int IBI) {
    currentBPM = BPM;
    Serial.print ("BPM:");
    Serial.print (BPM);
    Serial.print(" IBI: ");
    Serial.println(IBI);

    Blynk.virtualWrite(V2, BPM);  // Sends BPM to Blynk app on virtual pin V2

    if (BPM > 150) {
        digitalWrite(VIBR_PIN1, HIGH);
        digitalWrite(VIBR_PIN2, HIGH);
        digitalWrite(VIBR_PIN3, HIGH);
    } else {
        digitalWrite(VIBR_PIN1, LOW);
        digitalWrite(VIBR_PIN2, LOW);
        digitalWrite(VIBR_PIN3, LOW);
    }

}

// just brought this in from the Particle website
void myHandler(const char *event, const char *data) {
  // Handle the integration response
}

// Setting up all the variables. 
void setup() {
    Serial.begin(9600);
    pinMode(MOTION_PIN1, INPUT);
    pinMode(MOTION_PIN2, INPUT);
    pinMode(MOTION_PIN3, INPUT);
    pinMode(MOTION_PIN4, INPUT);
    pinMode(LED_PIN1, OUTPUT);
    pinMode(LED_PIN2, OUTPUT);
    pinMode(MEDICAL_PIN, OUTPUT);
    pinMode(LED_PIN3, OUTPUT);
    pinMode(VIBR_PIN1, OUTPUT);
    pinMode(VIBR_PIN2, OUTPUT);
    pinMode(VIBR_PIN3, OUTPUT);
    Blynk.config(BLYNK_AUTH_TOKEN);
    Blynk.connect();
    delay(10000);  // So PIR sensor can "warm up"; helps with accuracy of the readings

    analogRead(pulseSignalPin);
    PulseSensorAmped.attach(pulseSignalPin);
    PulseSensorAmped.start();

    Particle.subscribe(System.deviceID() + "/hook-response/BPM/", myHandler, MY_DEVICES);

}


unsigned long lastPublishTime = 0;  
const unsigned long publishInterval = 60000;  

void loop() {
    Blynk.run();

    // PIR sensor logic
    int PROXIMITY_1 = digitalRead(MOTION_PIN1);
    int PROXIMITY_2 = digitalRead(MOTION_PIN2);
    int PROXIMITY_3 = digitalRead(MOTION_PIN3);
    int PROXIMITY_4 = digitalRead(MOTION_PIN4);

    if (PROXIMITY_1 == HIGH) {
        Serial.println("Motion detected in FRONT!");
        digitalWrite(LED_PIN1, HIGH);
        digitalWrite(VIBR_PIN1, HIGH);
    } else {
        digitalWrite(LED_PIN1, LOW);
        digitalWrite(VIBR_PIN1, LOW);
    }

    if (PROXIMITY_2 == HIGH) {
        Serial.println("Motion detected in RIGHT!");
        digitalWrite(LED_PIN2, HIGH);
        digitalWrite(VIBR_PIN2, HIGH);
    } else {
        digitalWrite(LED_PIN2, LOW);
        digitalWrite(VIBR_PIN2, LOW);
    }

    if (PROXIMITY_3 == HIGH) {
        Serial.println("Motion detected in LEFT!");
        digitalWrite(LED_PIN3, HIGH);
        digitalWrite(VIBR_PIN3, HIGH);
    } else {
        digitalWrite(LED_PIN3, LOW);
        digitalWrite(VIBR_PIN3, LOW);
    }

    if (PROXIMITY_4 == HIGH) {
        Serial.println("Motion detected in BACK!");
    }

    PulseSensorAmped.process();  

    // Adds the BPM value every minute to the Initial State chart. 
    if (millis() - lastPublishTime >= publishInterval) {
        lastPublishTime = millis();
        String data = String(currentBPM);
        Particle.publish("BPM", data, PRIVATE);
    }
}
