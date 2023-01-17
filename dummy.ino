
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <TinyGPSPlus.h>

//Setting up Ultrasonic Sensor
#define trigPin1 4
#define echoPin1 5

#define trigPin2 0
#define echoPin2 2

#define trigPin3 14 //d6 trigger
#define echoPin3 12 //d5 echo

#define trigPin4 13 //d7 trigger
#define echoPin4 15 //d8 echo

#define MIN_DISTANCE 20 //Total minimum distance of LeftSensor and RightSensor
#define MAX_DISTANCE 80 //Total maximum distance of LeftSensor and RightSensor

TinyGPSPlus gps;



long duration, distance, RightSensor,BackSensor,FrontSensor,LeftSensor;

//Initialize the wifi ssid and password
const char* ssid = "0108-2.4G";
const char* password = "0137519887";

//Initializing Bin Id
const String bin_id = "SWM-47500-1";

//Waste Management WebApp config
const String serverName = "http://103.253.145.174/api/";

const String PATH_CREATE_LOG = "log/store?";
const String PATH_UPDATE_BIN = "bin/update/" + bin_id + "?";


String title, message, current_location , status , action ,location_GPS; 

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;

bool API_FULL_SENT ,API_FALL_SENT = false;

long TotalDistance, percentage , holdPercentage = 0 ; 
 

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200);

  pinMode(trigPin1, OUTPUT);   //Start Setting up ultrasonic sensor 
  pinMode(echoPin1, INPUT);

  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  pinMode(trigPin4, OUTPUT);
  pinMode(echoPin4, INPUT);   //End Setting up ultrasonic sensor


  pinMode(LED_BUILTIN, OUTPUT); //Setup built in LED.


  WiFi.begin(ssid, password);   //Connect to wifi 
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
  //Get Location

  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    if (gps.encode(Serial2.read())){
        location_GPS = String(gps.location.lat(), 6) + "," + (gps.location.lng(), 6);
        displayInfo();
    }

    //Detect the content inside the bin
    SonarSensor(trigPin1, echoPin1);
    RightSensor = distance;
    SonarSensor(trigPin2, echoPin2);
    LeftSensor = distance;

    //Calculate average of distance to get percentage
    TotalDistance = RightSensor + LeftSensor;
    percentage = distance_to_percentage(TotalDistance);

    if(percentage < 1){
      percentage = 0;
    }
    if(percentage >100){
      percentage = 100;
    }

    //Detect when bin fall
    SonarSensor(trigPin3, echoPin3);
    FrontSensor = distance;
    SonarSensor(trigPin4, echoPin4);
    BackSensor = distance;


    Serial.print("L: ");
    Serial.print(LeftSensor);
    Serial.print(" - R: ");
    Serial.print(RightSensor);
    Serial.print(" - Percentage: ");
    Serial.print(100 - percentage);
    Serial.println(" %");

    if(percentage < 30 && percentage != 0){  //Reset API_FULL_SENT gate when the bin is emptied.
      API_FULL_SENT = false;
    }

    if(API_FULL_SENT == false){
      if(percentage = 100){
        title = "Waste bin is ready to collect (100% usage)" ; 
        message = "Please collect the waste" ; 
        current_location = location_GPS ;
        status = "FULL";
        action = PATH_CREATE_LOG;
        call_API(title, message, current_location , status, bin_id,action);
        Serial.print("Dumpster full API sent");
        API_FULL_SENT = true;
      }
    }

    Serial.print(" - Front Sensor :  ");
    Serial.print(FrontSensor);
    Serial.print(" - Back Sensor :  ");
    Serial.print(BackSensor);


    if(API_FALL_SENT == true) {
      if(BackSensor > 70 && FrontSensor > 70){
        API_FALL_SENT = false;  //Reset API_FALL_SENT gate when the bin is restored.
      }
    }

    if(API_FALL_SENT == false){
      if(BackSensor <= 5 || FrontSensor <= 5){
          title = "Waste bin fall" ; 
          message = "Please send help" ; 
          current_location = location_GPS ;
          status = "FALL";
          action = PATH_CREATE_LOG;
          call_API(title, message, current_location , status, bin_id,action);
          Serial.print("Dumpster fall API sent");
          API_FALL_SENT = true; 
      }
    }

    String get_percentage = String(percentage);
    update_content(get_percentage, PATH_UPDATE_BIN); // Update content with API request
    lastTime = millis();
  }
}

double distance_to_percentage(double distance) {
    return (distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE) * 100;
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

void call_API(String title, String message, String current_location , String status , String bin_id, String action){
      WiFiClient client;
      HTTPClient http;
      
      String API = serverName + action;
      // Your Domain name with URL path or IP address with path
      http.begin(client, API);

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

void update_content(String percentage, String action){
      WiFiClient client;
      HTTPClient http;
      
      String API = serverName + action;
      // Your Domain name with URL path or IP address with path
      http.begin(client, API);

      // Specify content-type header
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      // Data to send with HTTP POST
      String httpRequestData = "percentage=" + percentage;
      // Send HTTP POST request
      int httpResponseCode = http.POST(httpRequestData);
      Serial.println(httpRequestData);

      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
        
      // Free resources
      http.end();
}

void updateSerial(){
  delay(500);
  while (Serial.available())  {
    Serial2.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (Serial2.available())  {
    Serial.write(Serial2.read());//Forward what Software Serial received to Serial Port
  }
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid()){
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }
}