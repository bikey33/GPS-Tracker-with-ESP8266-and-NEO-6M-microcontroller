#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <TinyGPS.h>
#include<SoftwareSerial.h>
//Declare object of class HTTPClient
HTTPClient http;
WiFiClient client;

TinyGPS gps;
SoftwareSerial ss(D3, D4);//rx,tx

const char *ssid ="wall";
const char *password ="12345678";
float flat=0, flon=0;
int v_id=2;

//Web/Server address to read/write from 
const char *host = "http://4wheel.space/api/locations";   //your IP/web server address
void GPS_Read(void){
   bool newData = false;
  unsigned long chars;
  unsigned short sentences, failed;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (ss.available()>0)
    {
      char c = ss.read();
      Serial.write(c); // uncomment this line if you want to see the GPS data flowing
      if (gps.encode(c)) // Did a new valid sentence come in?
        newData = true;
    }
  }

  if (newData)
  {
    unsigned long age;
    gps.f_get_position(&flat, &flon, &age);
    Serial.print("LAT=");
    Serial.print(flat == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flat, 6);
    Serial.print(" LON=");
    Serial.print(flon == TinyGPS::GPS_INVALID_F_ANGLE ? 0.0 : flon, 6);
    Serial.print(" SAT=");
    Serial.print(gps.satellites() == TinyGPS::GPS_INVALID_SATELLITES ? 0 : gps.satellites());
    Serial.print(" PREC=");
    Serial.print(gps.hdop() == TinyGPS::GPS_INVALID_HDOP ? 0 : gps.hdop());
  }
  
  gps.stats(&chars, &sentences, &failed);
  Serial.print(" CHARS=");
  Serial.print(chars);
  Serial.print(" SENTENCES=");
  Serial.print(sentences);
  Serial.print(" CSUM ERR=");
  Serial.println(failed);
  if (chars == 0)
    Serial.println("** No characters received from GPS: check wiring **");
}

void setup() {
  delay(1000);
  Serial.begin(9600);
  ss.begin(9600);
  delay(1000);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot

  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
}

void loop() {
  
  //Prepare data
  String latitude,longitude, postData;
  GPS_Read();// Call Function to obtain latitude and longitude from a GPS module
  latitude=String(flat,6);//Float to String conversion
  longitude=String(flon,6);//Float to String conversion
  String vehicle_id=String(v_id);
  //prepare request
  postData = "latitude=" + latitude + "&longitude=" + longitude+"&vehicle_id="+vehicle_id ;
  http.begin(client,host);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  int httpCode = http.POST(postData);//Sending Post Request
  String payload = http.getString();

  Serial.println(httpCode);
  Serial.println(payload);
  http.end();
  delay(48000);
}
