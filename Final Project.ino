#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

//Setting up Ultrasonic Sensor
#define trigPin1 4
#define echoPin1 5

#define trigPin2 0
#define echoPin2 2

#define trigPin3 14 //d6 trigger
#define echoPin3 12 //d5 echo

#define trigPin4 13 //d7 trigger
#define echoPin4 15 //d8 echo


// #define trigPin3 9
// #define echoPin3 10

long duration, distance, RightSensor,BackSensor,FrontSensor,LeftSensor;
//End setting up Ultrasonic Sensor

const char* ssid = "0108-2.4G";
const char* password = "0137519887";

//Initializing Bin Id
const char* bin_id = "1001";

//Your Domain name with URL path or IP address with path
const char* serverName = "http://103.253.145.174/api/log/store?";
String title, message, current_location , status ; 

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 1000;

bool _25percent, _50percent = false;

void setup() {
  Serial.begin(115200);

  //Start Setting up ultrasonic sensor 
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);

  // pinMode(trigPin3, OUTPUT);
  // pinMode(echoPin3, INPUT);
  //End Setting up ultrasonic sensor

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 5 seconds (timerDelay variable), it will take 5 seconds before publishing the first reading.");
}

void loop() {

  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    SonarSensor(trigPin1, echoPin1);
    RightSensor = distance;
    SonarSensor(trigPin2, echoPin2);
    LeftSensor = distance;
    SonarSensor(trigPin3, echoPin3);
    FrontSensor = distance;
    SonarSensor(trigPin4, echoPin4);
    BackSensor = distance;

    if(_25percent == false){
      if(LeftSensor < 20 ){
        title = "Dumpster Content" ; 
        message = "OK" ; 
        current_location = "Kuala Lumpur" ;
        status = "Dumpster is currently in 25% usage";
        call_API(title, message, current_location , status, bin_id);
        Serial.print("Dumpster is 25%");

        _25percent = true; 
      }
    }

    if(_50percent == false){
      if(RightSensor < 20 ){
        title = "Dumpster Content" ; 
        message = "OK" ; 
        current_location = "Kuala Lumpur" ;
        status = "Dumpster is currently in 50% usage";
        call_API(title, message, current_location , status, bin_id);
        Serial.print("Dumpster is 50%");

        _50percent = true; 
      }
    }

    Serial.print("Left Sensor : ");
    Serial.print(LeftSensor);
    Serial.print(" - Right Sensor :  ");
    Serial.println(RightSensor);
    Serial.print(" - Front Sensor :  ");
    Serial.print(FrontSensor);
    Serial.print(" - Back Sensor :  ");
    Serial.print(BackSensor);

    lastTime = millis();
  }
}

void SonarSensor(int trigPin,int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;

}

void call_API(String title, String message, String current_location , String status , String bin_id){
      WiFiClient client;
      HTTPClient http;
      
      // Your Domain name with URL path or IP address with path
      http.begin(client, serverName);
  
      // If you need Node-RED/server authentication, insert user and password below
      //http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
  
      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "title=" + title + "&message=" + message + "&current_location=" + current_location + "&status=" + status + "&bin_id=" + bin_id; 
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      Serial.println(httpRequestData);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
}


    // if(LeftSensor < 20){
    //   Serial.print("Dumpster is almost full");
    // }
    // Check WiFi connection status
    // if(WiFi.status()== WL_CONNECTED){

    //     Serial.println("WiFi Connected");
    //     title = "Rosak" ; 
    //     message = "From Arduiono : Tolong baiki" ; 
    //     current_location = "Kuala Lumpur" ;
    //     status = "0108";
    //     // String httpRequestData = "title=" + title + "&message=" + message + "&current_location=" + current_location + "&status=" + status + "&bin_id=" + bin_id; 
        
    //     call_API(title, message, current_location , status, bin_id);


    //     // Your Domain name with URL path or IP address with path
    //     // http.begin(client, serverName);
    
    //     // // Specify content-type header
    //     // http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    //     // // Data to send with HTTP POST
    //     // // Send HTTP POST request
    //     // Serial.println(httpRequestData); 
    //     // int httpResponseCode = http.POST(httpRequestData);
    //     // Serial.print("HTTP Response code: ");
    //     // Serial.println(httpResponseCode);     
    //}
    // else {
    //   Serial.println("WiFi Disconnected");
    // }