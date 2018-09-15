
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
WebServer server ( 80 );

#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"

/*************************************
  ESP32 D1 R32 pin use:
    GPIO 33  =   Encoder
    GPIO 32  =   Encoder

    GPIO 26  =   Motor driver
    GPIO 25  =   Motor driver
    GPIO 17  =   Motor driver

    GPIO 27  =   homeSwitch
    GPIO 14  =   limitSwitch

    GPIO 10  =   LED1
    GPIO 11  =   LED2
    GPIO 12  =   LED3
    GPIO 13  =   LED4
    GPIO 13  =   LED5
***************************************/

//-------------- ENCODER SETUP ---------------//
#define ROTARY_ENCODER_A_PIN 33
#define ROTARY_ENCODER_B_PIN 32
#define ROTARY_ENCODER_BUTTON_PIN 25
#define ROTARY_ENCODER_VCC_PIN 27 /*put -1 of Rotary encoder Vcc is connected directly to 3,3V; else you can use declared output pin for powering rotary encoder */

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN);

//-------------- MOTOR CONTOLE SETUP ---------------//
#define enB 17
#define in1 25
#define in2 26
int rotDirection = 0;

//-------------- PWM VARIABEL SETUP ---------------//
int LEDfreq = 1500;
int motorFreq = 4000;

int resolution = 8;   // 8 bit res. (0-255)
byte motorChannel = 0;

#define GPIO12 12
#define GPIO13 13
#define GPIO5 5
#define GPIO23 23
#define GPIO19 16

byte ledChannel1 = 1;
byte ledChannel2 = 2;
byte ledChannel3 = 3;
byte ledChannel4 = 4;
byte ledChannel5 = 5;

//-------------- PHYSICAL LIMIT SWITCHES ---------------//
#define homeSwitch 27
#define limitSwitch 14

//-------------- ENCODER LIMIT VALUES ---------------//
const int upperThreshold = 1250;
const int lowerThreshold  = 950;
const int encoderMax = 2150;

//-------------- WIFI VARIABEL SETUP ---------------//
const char *ssid = "GreatestPiecesofArt";
const char *password = " ";

// Variable to store the HTTP request
String header;

bool toggleVal = HIGH;
bool sendDataState = HIGH;
bool state = LOW;
bool displayInputData = HIGH;

int motor_speed = 255;

char incomingByte;
byte LEDpwm = 50;  //fade value for LED strips
int LED;

//------------ VALUES FOR WEB SERVER -------------//
String position = "Waiting for Input...";
int16_t encoderValue;
String msg0 = "waiting for input...";
String msg1 = "waiting for input...";


void handleRoot() {

  String html = "<!DOCTYPE html> <html> <head> <meta name=\"viewport\" content=\"width=device-width, minimumscale=1.0, maximum-scale=1.0, initial-scale=1\" /> <style> body { text-align: center; background-color: #ccc7c7; margin: auto; width: 60%; } #dataVals { display: block; margin-top: 5%; } .convertButton { clear: both; position: relative; -moz-box-shadow: 0px 1px 0px 0px #fff6af; -webkit-box-shadow: 0px 1px 0px 0px #fff6af; box-shadow: 0px 1px 0px 0px #fff6af; background: -webkit-gradient(linear, left top, left bottom, color-stop(0.05, #ffec64), color-stop(1, #ffab23)); background: -moz-linear-gradient(top, #ffec64 5%, #ffab23 100%); background: -webkit-linear-gradient(top, #ffec64 5%, #ffab23 100%); background: -o-linear-gradient(top, #ffec64 5%, #ffab23 100%); background: -ms-linear-gradient(top, #ffec64 5%, #ffab23 100%); background: linear-gradient(to bottom, #ffec64 5%, #ffab23 100%); filter: progid:DXImageTransform.Microsoft.gradient(startColorstr='#ffec64', endColorstr='#ffab23', GradientType=0); background-color: #ffec64; -moz-border-radius: 6px; -webkit-border-radius: 6px; border-radius: 6px; border: 1px solid #ffaa22; display: inline-block; cursor: pointer; color: #333333; font-family: Arial; font-size: 21px; padding: 14px 69px; text-decoration: none; text-shadow: 0px 1px 0px #ffee66; } .convertButton:hover { background: -webkit-gradient(linear, left top, left bottom, color-stop(0.05, #ffab23), color-stop(1, #ffec64)); background: -moz-linear-gradient(top, #ffab23 5%, #ffec64 100%); background: -webkit-linear-gradient(top, #ffab23 5%, #ffec64 100%); background: -o-linear-gradient(top, #ffab23 5%, #ffec64 100%); background: -ms-linear-gradient(top, #ffab23 5%, #ffec64 100%); background: linear-gradient(to bottom, #ffab23 5%, #ffec64 100%); filter: progid:DXImageTransform.Microsoft.gradient(startColorstr='#ffab23', endColorstr='#ffec64', GradientType=0); background-color: #ffab23; } .convertButton:active { position: relative; top: 1px; } .convertButton:active { position: relative; top: 1px; } #curtain, #encoder { background-color: white; padding: 1%; width: 12%; display: block; margin-left: auto; margin-right: auto; } #msgCore0, #msgCore1 { background-color: white; padding: 1%; width: 45%; display: block; margin-left: auto; margin-right: auto; } </style> </head> <body> <br> <br> <h1>How Said that Art Couldn't also have a WebServer?</h1> <h2>This is an Access Point Web Server Hosted on the Second Core of an ESP32 for the Greatest Piece of Art Installation</h2> <h3>Created by Christoffer Vincent W. Thon, DTU Skylab Denmark, 27-02-2018</h3> <br> <p><a href=\"/toggle\"><button class=\"convertButton\" id=\"viewButton\"> Curtains IN / OUT </button></a></p> <!--<a onClick=\"changeState()\" class=\"convertButton\" id=\"viewButton\">Curtains IN/OUT</a>--> <div id=\"dataVals\"> <h2>Curtain Position:</h2> <div id=\"curtain\">";
  html += position;
  html += "</div> <h2>Encoder Value:</h2> <div id=\"encoder\">";
  html += encoderValue;
  html += "</div> <h2>Messages From ESP32 Core 0:</h2> <div id=\"msgCore0\">";
  html += msg0;
  html += "</div> <h2>Messages From ESP32 Core 1:</h2> <div id=\"msgCore1\">";
  html += msg1;
  html += "</div> </div> <br> <br> <br> <p>If you Find this project Interesting - Please Visit my Website for more Info</p> <p>http://creative-engineering.org/</p> <script> var changed = false; function changeState() { if (changed == false) { console.log(\"state = TRUE\"); changed = true; } else { console.log(\"state = FALSE\"); changed = false; } } function loadDoc() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function () { if (this.readyState == 4 && this.status == 200) { var obj = JSON.parse(this.responseText); document.getElementById(\"curtain\").innerHTML = obj.data[0].dataValue; document.getElementById(\"encoder\").innerHTML = obj.data[1].dataValue; document.getElementById(\"msgCore0\").innerHTML = obj.data[2].dataValue; document.getElementById(\"msgCore1\").innerHTML = obj.data[3].dataValue; } }; xhttp.open(\"GET\", \"/data\", true); xhttp.send(); } var timedEvent = setInterval(function () { loadDoc(); }, 100); </script> </body> </html>";

  server.send(200, "text/html", html);
}

void getData() {

  String text2 = "{\"data\":[";
  text2 += "{\"dataValue\":\"";
  text2 += position;
  text2 += "\"},";
  text2 += "{\"dataValue\":\"";
  text2 += encoderValue;
  text2 += "\"},";
  text2 += "{\"dataValue\":\"";
  text2 += msg0;
  text2 += "\"},";
  text2 += "{\"dataValue\":\"";
  text2 += msg1;
  text2 += "\"}";
  text2 += "]}";

  server.send(200, "text/html", text2);
}

void toggle () {
  if (toggleVal) {
    state = HIGH;
    //motor_speed = 255;
    Serial.println("Going out...");
    toggleVal = LOW;
  }
  else if (!toggleVal) {
    state = LOW;
    //motor_speed = 255;
    Serial.println("Going in...");
    toggleVal = HIGH;
  }
  Serial.println("you pushed that a button!");
  String MSG = "<!DOCTYPE html> <html> <head> <title>toggle</title> </head> <body> <script> goBack(); function goBack() { window.history.go(-1); } </script> </body> </html>";
  server.send(200, "text/html", MSG);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }
  server.send ( 404, "text/plain", message );
}

void rotary_loop() {
  //first lets handle rotary encoder button click
  if (rotaryEncoder.currentButtonState() == BUT_RELEASED) {
    //we can process it here or call separate function like:
  }

  int16_t encoderDelta = rotaryEncoder.encoderChanged();

  //optionally we can ignore whenever there is no change
  if (encoderDelta == 0) return;

  if (encoderDelta != 0) {
    encoderValue = rotaryEncoder.readEncoder();
  }
}

TaskHandle_t xTask1, xTask2;

void setup() {

  Serial.begin(112500);
  // create the queue which size can contains 5 elements of Data
  //xQueue = xQueueCreate(5, sizeof(Data));

  xTaskCreatePinnedToCore(
    GIPOTask,           // Task function.
    "GIPOTask",        // name of task.
    10000,                    // Stack size of task
    NULL,                     // parameter of the task
    1,                        // priority of the task
    &xTask1,                // Task handle to keep track of created task
    0);                    // pin task to core 0

  delay(500);

  xTaskCreatePinnedToCore(
    serverTask,           // Task function.
    "serverTask",        // name of task.
    20000,                    // Stack size of task
    NULL,                     // parameter of the task
    2,                        // priority of the task
    &xTask2,            // Task handle to keep track of created task
    1);                 // pin task to core 1
}

void loop() { } //not needed scene we are running tasks on spefefic cores

void GIPOTask( void * parameter ) {

  Serial.print("Init Encoder...");
  msg0 = "init encoder...";
  rotaryEncoder.begin();
  rotaryEncoder.setBoundaries(0, encoderMax, false); //minValue, maxValue, cycle values (when max go to min and vice versa)
  Serial.println("Done!");

  Serial.print("Init Output Pins...");
  msg0 = "init output pins...";
  setPins();
  Serial.println("Done!");

  msg0 = "wait 5 seconds to init server on other core!";
  delay(5000);        //needed for the other core to start up first
  digitalWrite(19, HIGH); //turning on motor driver module

  msg0 = "going to home position...";
  Serial.print(msg0);

  while ( digitalRead(homeSwitch) == LOW ) { //go home before looping
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(motorChannel, 150); // inset homing value in "dutyCycle"
  }
  encoderValue = 0;
  msg0 = "beginning loop!";

  for (;;) {

    msg0 = "GPIOTask running...";

    rotary_loop(); //encoder update
    //Serial.print("encoderValue: "); Serial.println(encoderValue);

    incomingSerialData();
    motorContole();
    updateLEDs();

    //------------ Reduce speed before limit impact -------------//
    if ( encoderValue < lowerThreshold && !state ) {   // Home impact
      int encoderValueIn = constrain(encoderValue, 500, lowerThreshold); 
      motor_speed = map(encoderValueIn, lowerThreshold, 500, 255, 190); 
      //motor_speed = constrain(motor_speed, 155, 255);

      position = "HOME"; //update position for web server
      Serial.print("motor_speed: "); Serial.println(motor_speed);

      if (digitalRead(homeSwitch))
      {
        motor_speed = 255;
        encoderValue = 0;
      }
    }
    if ( encoderValue > upperThreshold && state ) {   //Limit impact
      int encoderValueOut = constrain(encoderValue, upperThreshold, 1850);
      motor_speed = map(encoderValueOut, upperThreshold, 1850, 255, 190);
      //motor_speed = constrain(motor_speed, 160, 255);

      Serial.print("motor_speed: "); Serial.println(motor_speed);

      position = "LIMIT"; //update position for web server

      if (digitalRead(limitSwitch))
      {
        motor_speed = 255;
        encoderValue = encoderMax;
      }
    }
  }
}

void serverTask( void * parameter )
{
  msg1 = "setting up access point...";
  accessPointSetup();

  msg1 = "hosting web server!";
  for (;;) {
    msg1 = "serverTask running...";
    server.handleClient();  //server update
  }
}

void setPins() {
  //-------------- H-BRIDGE SETUP ---------------//
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  //-------------- SWITCH SETUP ---------------//
  pinMode(homeSwitch, INPUT_PULLUP);
  pinMode(limitSwitch, INPUT_PULLUP);
  //---------------- LED SETUP -----------------//
  pinMode(GPIO12, OUTPUT);
  pinMode(GPIO13, OUTPUT);
  pinMode(GPIO5,  OUTPUT);
  pinMode(GPIO23, OUTPUT);
  pinMode(GPIO19, OUTPUT);
  //------- Motor driver module turn on pin -------//
  pinMode(19, OUTPUT);

  //-------------- PWM MOTOR SETUP ---------------//
  ledcSetup(motorChannel, motorFreq, resolution);
  ledcAttachPin(enB, motorChannel);

  //-------------- PWM LED SETUP ---------------//
  ledcSetup(ledChannel1, LEDfreq, resolution);
  ledcAttachPin(GPIO12, ledChannel1);

  ledcSetup(ledChannel2, LEDfreq, resolution);
  ledcAttachPin(GPIO13, ledChannel2);

  ledcSetup(ledChannel3, LEDfreq, resolution);
  ledcAttachPin(GPIO5,  ledChannel3);

  ledcSetup(ledChannel4, LEDfreq, resolution);
  ledcAttachPin(GPIO23, ledChannel4);

  ledcSetup(ledChannel5, LEDfreq, resolution);
  ledcAttachPin(GPIO19, ledChannel5);

}

void incomingSerialData()
{
  while ( Serial.available() > 0 ) {
    incomingByte = Serial.read();
    Serial.println(incomingByte);
    motor_speed = 255;

    /***************** State change & LED set *****************/
    if ( incomingByte == 49 ) {  // recieve 1
      state = HIGH;
      Serial.print("incomingByte: "); Serial.println(incomingByte);
    }
    else if ( incomingByte == 48 ) { // recieve 0
      state = LOW;
      Serial.print("incomingByte: "); Serial.println(incomingByte);
    }
  }
}

void updateLEDs()
{
  LED = map(encoderValue, 500, encoderMax, 0, 5);
  Serial.print("LED: "); Serial.println(LED);

  if (LED <= 0) {
    ledcWrite(ledChannel1, 0);
    ledcWrite(ledChannel2, 0);
    ledcWrite(ledChannel3, 0);
    ledcWrite(ledChannel4, 0);
    ledcWrite(ledChannel5, 0);
  }

  if (LED > 0) {
    ledcWrite(ledChannel1, 50);
  }
  else {
    ledcWrite(ledChannel1, 0);
  }
  if (LED > 1) {
    ledcWrite(ledChannel2, LEDpwm);
  }
  else {
    ledcWrite(ledChannel2, 0);
  }
  if (LED > 2) {
    ledcWrite(ledChannel3, LEDpwm);
  }
  else {
    ledcWrite(ledChannel3, 0);
  }
  if (LED > 3) {
    ledcWrite(ledChannel4, LEDpwm);
  }
  else {
    ledcWrite(ledChannel4, 0);
  }
  if (LED > 4) {
    ledcWrite(ledChannel5, LEDpwm);
  }
  else {
    ledcWrite(ledChannel5, 0);
  }
}

void motorContole()
{
  /*********************** Motor Control ***********************/
  if ( state == HIGH  && digitalRead(limitSwitch) == LOW ) { //limitSwitch is pullUp
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    ledcWrite(motorChannel, motor_speed); // inset homing value in "dutyCycle"
    // Serial.println("Forwards");
  }
  else if ( state == LOW  && digitalRead(homeSwitch) == LOW ) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(motorChannel, motor_speed); // inset homing value in "dutyCycle"
    //Serial.println("Backwards");
  }
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(motorChannel, 0); // inset homing value in "dutyCycle"
    // Serial.println("Full Stop!");
  }
}

void accessPointSetup() {

  WiFi.begin ( ssid, password );

  Serial.println();
  Serial.print("Configuring access point...");

  WiFi.softAP( ssid, password );
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  if ( MDNS.begin ( "esp32" ) ) {
    Serial.println ( "MDNS responder started" );
  }

  server.on ( "/", handleRoot );
  server.on("/data", getData);
  server.on("/toggle", toggle);;

  server.onNotFound ( handleNotFound );

  server.begin();
  Serial.println ("HTTP server started");
}

