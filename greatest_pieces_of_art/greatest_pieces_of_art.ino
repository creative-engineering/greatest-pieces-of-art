
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
WebServer server ( 80 );

#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"

/************************************
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
*************************************/

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
int freq = 5000;
int resolution = 8;   // 8 bit res. (0-255)
byte motorChannel = 0;

#define GPIO12 12
#define GPIO13 13
#define GPIO5 5
#define GPIO23 23
#define GPIO19 19

byte ledChannel1 = 1;
byte ledChannel2 = 2;
byte ledChannel3 = 3;
byte ledChannel4 = 4;
byte ledChannel5 = 5;

//-------------- PHYSICAL LIMIT SWITCHES ---------------//
#define homeSwitch 27
#define limitSwitch 14

//-------------- ENCODER LIMIT VALUES ---------------//
const int upperThreshold = 3000;
const int lowerThreshold  = 2000;
const int encoderMax = 5000;

//-------------- WIFI VARIABEL SETUP ---------------//
const char *ssid = "GreatestPiecesofArt";
const char *password = " ";

bool toggleVal = HIGH;
bool sendDataState = HIGH;
bool state = LOW;
int16_t encoderValue = 0;
int motor_speed = 255;
char incomingByte;
int LED;
int counter = 0;

void handleRoot() {
  int  LDRReading = 10;
  String html = "<!DOCTYPE html> <html> <head><meta name=\"viewport\" content=\"width=device-width, minimumscale=1.0, maximum-scale=1.0, initial-scale=1\" /><style> body {text-align: center; max-width: 400px; margin: 10px auto;} #updateView { max-width: 400px; display: none; margin-top: 30px; } #dataVals { max-width: 400px; display: block; margin-top: 30px; } .convertButton { clear: both; position:relative; margin-top: 30px; -moz-box-shadow: 0px 1px 0px 0px #fff6af; -webkit-box-shadow: 0px 1px 0px 0px #fff6af; box-shadow: 0px 1px 0px 0px #fff6af; background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #ffec64), color-stop(1, #ffab23)); background:-moz-linear-gradient(top, #ffec64 5%, #ffab23 100%); background:-webkit-linear-gradient(top, #ffec64 5%, #ffab23 100%); background:-o-linear-gradient(top, #ffec64 5%, #ffab23 100%); background:-ms-linear-gradient(top, #ffec64 5%, #ffab23 100%); background:linear-gradient(to bottom, #ffec64 5%, #ffab23 100%); filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ffec64', endColorstr='#ffab23',GradientType=0); background-color:#ffec64; -moz-border-radius:6px; -webkit-border-radius:6px; border-radius:6px; border:1px solid #ffaa22; display:inline-block; cursor:pointer; color:#333333; font-family:Arial; font-size:21px; padding:14px 69px; text-decoration:none; text-shadow:0px 1px 0px #ffee66; } .convertButton:hover { background:-webkit-gradient(linear, left top, left bottom, color-stop(0.05, #ffab23), color-stop(1, #ffec64)); background:-moz-linear-gradient(top, #ffab23 5%, #ffec64 100%); background:-webkit-linear-gradient(top, #ffab23 5%, #ffec64 100%); background:-o-linear-gradient(top, #ffab23 5%, #ffec64 100%); background:-ms-linear-gradient(top, #ffab23 5%, #ffec64 100%); background:linear-gradient(to bottom, #ffab23 5%, #ffec64 100%); filter:progid:DXImageTransform.Microsoft.gradient(startColorstr='#ffab23', endColorstr='#ffec64',GradientType=0); background-color:#ffab23; } .convertButton:active { position:relative; top:1px; }</style></head> <body> <a onClick=\"changeView()\" class=\"convertButton\" id=\"viewButton\">Update Rate</a> <div id=\"dataVals\"> <h1>Light Level</h1> <div id=\"light\">";
  html += LDRReading;
  html += "</div><h1>Count Number</h1><div id=\"countNum\">";
  //  html += count;
  html += "</div> </div> <div id=\"updateView\"> <label for=\"fader\">Update rate in Milliseconds</label><br /> <input type=\"range\" style=\"width: 300px\" min=\"200\" max=\"5000\" value=\"2000\" id=\"fader\" step=\"1\" oninput=\"outputUpdate(value)\"> <output for=\"fader\" id=\"volume\">2000</output></div> <script> var changed = false; function changeView(){ if(changed == false){ document.getElementById('updateView').style.display = 'block'; document.getElementById('dataVals').style.display = 'none'; document.getElementById('viewButton').innerHTML = \"Show Data\"; changed = true; } else{ document.getElementById('updateView').style.display = 'none'; document.getElementById('dataVals').style.display = 'block'; document.getElementById('viewButton').innerHTML = \"Update Rate\"; changed = false; } } function loadDoc() { var xhttp = new XMLHttpRequest(); xhttp.onreadystatechange = function() { if (this.readyState == 4 && this.status == 200) { var obj = JSON.parse(this.responseText); document.getElementById(\"light\").innerHTML = obj.data[0].dataValue; document.getElementById(\"countNum\").innerHTML = obj.data[1].dataValue; } }; xhttp.open(\"GET\", \"/data\", true); xhttp.send(); } var timedEvent = setInterval(function(){ loadDoc(); }, 500); function outputUpdate(vol) { clearInterval(timedEvent); timedEvent = setInterval(function(){ loadDoc(); }, vol); document.querySelector('#volume').value = vol; delayVal = vol; } </script> </body> </html>";
  html += "<br><h1>Curtain Control</h1><p><a href=\"toggle\"><button>Toggle</button></a>&nbsp;</p>";
  //count++;
  server.send(200, "text/html", html);
}

void getData() {
  //This is a JSON formatted string that will be served. You can change the values to whatever like.
  // {"data":[{"dataValue":"1024"},{"dataValue":"23"}]} This is essentially what is will output you can add more if you like
  // int LDRReading = 10;
  String text2 = "{\"data\":[";
  text2 += "{\"dataValue\":\"";
  text2 += encoderValue;
  text2 += "\"},";
  text2 += "{\"dataValue\":\"";
  //text2 += count;
  text2 += "\"}";
  text2 += "]}";
  server.send(200, "text/html", text2);
  //count++;
}

void toggle () {

  if (toggleVal == HIGH) {
    // digitalWrite(led, HIGH);
    toggleVal = LOW;
  }
  else if (toggleVal == LOW) {
    // digitalWrite(led, LOW);
    toggleVal = HIGH;
  }

  Serial.println("you pushed a button!");
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

  //lets see if anything changed
  int16_t encoderDelta = rotaryEncoder.encoderChanged();

  //optionally we can ignore whenever there is no change
  if (encoderDelta == 0) return;

  //for some cases we only want to know if value is increased or decreased (typically for menu items)
  if (encoderDelta > 0) Serial.print("+");
  if (encoderDelta < 0) Serial.print("-");

  //if value is changed compared to our last read
  if (encoderDelta != 0) {
    //now we need current value
    encoderValue = rotaryEncoder.readEncoder();
    //process new value. Here is simple output.
    // Serial.print("Value: ");
    // Serial.println(encoderValue);
  }
}

/* structure that hold data*/
typedef struct {
  int sender;
  char *msg;
  // int counter;
} Data;

/* this variable hold queue handle */
xQueueHandle xQueue;
TaskHandle_t xTask1;
TaskHandle_t xTask2;

void setup() {

  Serial.begin(112500);
  /* create the queue which size can contains 5 elements of Data */
  xQueue = xQueueCreate(5, sizeof(Data));

  xTaskCreatePinnedToCore(
    sendTask,           /* Task function. */
    "sendTask",        /* name of task. */
    10000,                    /* Stack size of task */
    NULL,                     /* parameter of the task */
    1,                        /* priority of the task */
    &xTask1,                /* Task handle to keep track of created task */
    0);                    /* pin task to core 0 */
  xTaskCreatePinnedToCore(
    receiveTask,           /* Task function. */
    "receiveTask",        /* name of task. */
    10000,                    /* Stack size of task */
    NULL,                     /* parameter of the task */
    1,                        /* priority of the task */
    &xTask2,            /* Task handle to keep track of created task */
    1);                 /* pin task to core 1 */
}

void loop() {}

void sendTask( void * parameter ) {

  delay(2000); //what a bit before running core 0 task
  /* keep the status of sending data */
  BaseType_t xStatus;
  /* time to block the task until the queue has free space */
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  /* create data to send */
  Data data;
  /* sender 1 has id is 1 */
  data.sender = 1;
  // data.counter = 10;

  Serial.print("Init Encoder...");
  rotaryEncoder.begin();
  rotaryEncoder.setBoundaries(0, 5000, false); //minValue, maxValue, cycle values (when max go to min and vice versa)
  Serial.println(" Done!");

  Serial.print("Init Output Pins...");
  setPins();
  Serial.println(" Done!");

  for (;;) {

    rotary_loop(); //encoder update

    counter++;

    /***************** Reduce speed before limit impact *****************/
    if ( encoderValue <= lowerThreshold && state == LOW /*&& stateFlag == HIGH */ ) {     // Home impact
      int encoderValueIn = constrain(encoderValue, 800, lowerThreshold);
      motor_speed = map(encoderValueIn, lowerThreshold, 800, 255, 140);
      motor_speed = constrain(motor_speed, 140, 255);

      //Serial.print("motor_speed IN: ");
      //Serial.println(motor_speed);

      if (digitalRead(homeSwitch))
      {
        motor_speed = 255;
        encoderValue = 0;
      }
    }

    if ( encoderValue >= upperThreshold && state /*&& stateFlag == LOW*/ ) {      //Limit impact
      int encoderValueOut = constrain(encoderValue, upperThreshold, encoderMax);
      motor_speed = map(encoderValueOut, upperThreshold, 5000 - 800, 255, 140);
      motor_speed = constrain(motor_speed, 140, 255);

      //Serial.print("motor_speed OUT: ");
      // Serial.println(motor_speed);

      if ( digitalRead(limitSwitch))
      {
        motor_speed = 255;
        encoderValue = 5000;
      }
    }

    LED = map(encoderValue, 0, 5000, 0, 5);
    //Serial.print("LED value: ");
    // Serial.println(LED);

    if (LED == 0) {
      ledcWrite(ledChannel1, 0);
      ledcWrite(ledChannel2, 0);
      ledcWrite(ledChannel3, 0);
      ledcWrite(ledChannel4, 0);
      ledcWrite(ledChannel5, 0);
    }
    if (LED == 1) {
      ledcWrite(ledChannel1, 200);
    }
    if (LED == 2) {
      ledcWrite(ledChannel2, 200);
    }
    if (LED == 3) {
      ledcWrite(ledChannel3, 200);
    }
    if (LED == 4) {
      ledcWrite(ledChannel4, 200);
    }
    if (LED == 5) {
      ledcWrite(ledChannel5, 200);
    }

    /***************** Serial input data *****************/
    while ( Serial.available() > 0 ) {
      incomingByte = Serial.read();
      Serial.println(incomingByte);
    }
    /***************** State change & LED set *****************/
    if ( incomingByte == 49 ) {  // recieve 1
      state = HIGH;
    }
    else if ( incomingByte == 48 ) { // recieve 0
      state = LOW;
    }

    /*********************** Motor Control ***********************/
    if ( state == HIGH  && digitalRead(limitSwitch) == LOW ) { //limitSwitch is pullUp
      digitalWrite(in1, HIGH);
      digitalWrite(in2, LOW);
      ledcWrite(motorChannel, motor_speed); // inset homing value in "dutyCycle"
      Serial.println("forwards");
    }
    else if ( state == LOW  && digitalRead(homeSwitch) == LOW ) {
      digitalWrite(in1, LOW);
      digitalWrite(in2, HIGH);
      ledcWrite(motorChannel, motor_speed); // inset homing value in "dutyCycle"
      Serial.println("backwards");
    }
    else {
      digitalWrite(in1, LOW);
      digitalWrite(in2, LOW);
      ledcWrite(motorChannel, 0); // inset homing value in "dutyCycle"
    }

    //-------------- SEND DATA TO CORE 1 ---------------//
    if (sendDataState) {
      sendDataState = LOW;

      Serial.print("sendTask run on core "); Serial.print(xTaskGetAffinity(xTask1)); Serial.println(" is sending data");
      data.msg = (char *)malloc(20);
      memset(data.msg, 0, 20);
      memcpy(data.msg, "hello world", strlen("hello world"));
      /* send data to front of the queue */
      xStatus = xQueueSendToFront( xQueue, &data, xTicksToWait );
      /* check whether sending is ok or not */
      if ( xStatus == pdPASS ) {
        /* increase counter of sender 1 */
        Serial.println("sendTask sent data");
      }
    }
  }
  vTaskDelete( NULL );
}


void receiveTask( void * parameter )
{
  /* keep the status of receiving data */
  BaseType_t xStatus;
  /* time to block the task until data is available */
  const TickType_t xTicksToWait = pdMS_TO_TICKS(100);
  Data data;

  accessPointSetup();

  for (;;) {
    /* receive data from the queue */
    xStatus = xQueueReceive( xQueue, &data, xTicksToWait );
    /* check whether receiving is ok or not */
    if (xStatus == pdPASS) {
      Serial.print("receiveTask run on core");
      /* get the core that the task was pinned to */
      Serial.print(xTaskGetAffinity(xTask2));
      /* print the data to terminal */
      Serial.print(" got data: ");
      Serial.print("sender = ");
      Serial.print(data.sender);
      Serial.print(" msg = ");
      Serial.print(data.msg);
      // Serial.print(" counter = ");
      // Serial.println(data.counter);
      free(data.msg);
      Serial.println("you got mail!");
    }
    Serial.println(counter);
    server.handleClient();  // server update
    //Serial.println("Handling Client...");
  }
  vTaskDelete( NULL );
}

void setPins() {
  //-------------- H-BRIDGE SETUP ---------------//
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  //-------------- SWITCH SETUP ---------------//
  pinMode(homeSwitch, INPUT_PULLUP);
  pinMode(limitSwitch, INPUT_PULLUP);
  //-------------- LED SETUP ---------------//
  pinMode(GPIO12, OUTPUT);
  pinMode(GPIO13, OUTPUT);
  pinMode(GPIO5,  OUTPUT);
  pinMode(GPIO23, OUTPUT);
  pinMode(GPIO19, OUTPUT);

  //-------------- PWM MOTOR SETUP ---------------//
  ledcSetup(motorChannel, freq, resolution);
  ledcAttachPin(enB, motorChannel);
  //-------------- PWM LED SETUP ---------------//
  ledcSetup(ledChannel1, freq, resolution);
  ledcAttachPin(GPIO12, ledChannel1);

  ledcSetup(ledChannel2, freq, resolution);
  ledcAttachPin(GPIO13, ledChannel2);

  ledcSetup(ledChannel3, freq, resolution);
  ledcAttachPin(GPIO5,  ledChannel3);

  ledcSetup(ledChannel4, freq, resolution);
  ledcAttachPin(GPIO23, ledChannel4);

  ledcSetup(ledChannel5, freq, resolution);
  ledcAttachPin(GPIO19, ledChannel5);

  // Set "home" rotation direction and home curtains
  // Serial.print("I'm Going To Home Pos");
  while (digitalRead(homeSwitch) == LOW) { //go home before looping
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    ledcWrite(motorChannel, 140); // inset homing value in "dutyCycle"
    // Serial.print(".");
  }
  Serial.println("");
}

void accessPointSetup() {

  WiFi.begin ( ssid, password );

  Serial.println();
  Serial.print("Configuring access point...");

  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  if ( MDNS.begin ( "esp32" ) ) {
    Serial.println ( "MDNS responder started" );
  }

  server.on ( "/", handleRoot );
  server.on("/data", getData);
  server.on("/toggle", toggle);
  server.onNotFound ( handleNotFound );

  server.begin();
  Serial.println ("HTTP server started");
}

