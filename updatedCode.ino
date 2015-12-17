//wifi connection
#define SSID "AndroidAP"
#define PASSWORD ""

char thingSpeakAddress[] = "127.0.0.1;
String writeAPIKey = "A51OEZ0JHJG2NUDM";
const int updateThingSpeakInterval = 16 * 1000; // Time interval in milliseconds to update ThingSpeak (number of seconds * 1000 = interval)

long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;

//ligtht
int sensorPin = 5;    // select the input pin for the potentiometer

//air
int pin3 = 3;

//air
int sensorValueAir;
int airpin = A3;

//temp and wet
#include "DHT.h"
#define DHTTYPE DHT22
#define DHTPIN 2
DHT dht(DHTPIN, DHTTYPE);


//wifi
#include "uartWIFI.h"
WIFI wifi;

void setup()
{

  analogReference(EXTERNAL); //
  wifi.begin();
  bool b = wifi.Initialize(STA, SSID, PASSWORD);
  if (!b)
  {
    DebugSerial.println("Init error");
  }
  delay(5000); //make sure the module can have enough time to get an IP address
  String ipstring = wifi.showIP();
  DebugSerial.println("My IP address:");
  DebugSerial.println(ipstring); //show the ip address of module

  String wifistring = wifi.showJAP();
  DebugSerial.println(wifistring); //show the name of current wifi access port

  Serial.begin(9600);
  //DHT begin
  dht.begin();

  //air
  pinMode(pin3, OUTPUT);


}
void loop()
{

  //DebugSerial.println("host connect");
  collectData();
}
void collectData(){
  float meanTemp = 0.0;
  float meanHumid = 0.0;
  float meanAir = 0.0;
  float meanLight = 0.0;
  float meanSound = 0.0;
  for(int i = 0; i < 5 ; i++){
      meanTemp += getTemp();
      meanHumid += getHumidity();
      meanAir += getAir();
      meanLight += getLight();
      meanSound += soundLevel();
  }
  meanTemp = meanTemp/5;
  meanHumid = meanHumid/5;
  meanAir = meanAir/5;
  meanLight = meanLight/5;
  meanSound = meanSound/5;
  updateThingSpeak("temp=" + String(meanTemp) + "&wet=" + String(meanHumid) + "&gas=" + String(meanAir, DEC) + "&light=" + String(meanLight) + "&noise="+String(meanSound)+"&positionId=1");
}
/*void updateThingSpeak(String tsData){
  Serial.println(tsData);
}*/

void updateThingSpeak(String tsData)
{
  if (wifi.newMux(1, thingSpeakAddress, 80))
  {
    wifi.Send("POST /metrics HTTP/1.1\n");
    wifi.Send("Host: 127.0.0.1\n");
    wifi.Send("Connection: close\n");
    //wifi.Send("X-THINGSPEAKAPIKEY: "+writeAPIKey+"\n");
    wifi.Send("Content-Type: application/x-www-form-urlencoded\n");
    wifi.Send("Content-Length: ");
    wifi.Send(String(tsData.length()));
    wifi.Send("\n\n");

    wifi.Send(tsData);

    lastConnectionTime = millis();

    Serial.println("Success");
    wifi.closeMux();

  }
  else
  {
    failedCounter++;

    Serial.println("Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");

    lastConnectionTime = millis();
  }
}

int getLight()
{
  float meanValue = 0;
  for(int i = 0; i < 100; i++){
    meanValue += analogRead(sensorPin);
  }
  return meanValue/100;
}

float getTemp() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    return 0;
  } else {
    return t;
  }
}

float getHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    return 0;
  } else {
    return h;
  }
}

int getAir()
{
  float meanValue = 0;
  for(int i = 0; i < 100; i++){
    meanValue += analogRead(airpin);
  }
  return meanValue/100;
}

float soundLevel(){
 int sample;
 unsigned long startMillis= millis();  // Start of sample window
 unsigned int peakToPeak = 0;   // peak-to-peak level

 unsigned int signalMax = 0;
 unsigned int signalMin = 1024;

 // collect data for 50 mS
 while (millis() - startMillis < 1000)
 {
    sample = analogRead(0);
    if (sample < 1024)  // toss out spurious readings
    {
       if (sample > signalMax)
       {
          signalMax = sample;  // save just the max levels
       }
       else if (sample < signalMin)
       {
          signalMin = sample;  // save just the min levels
       }
    }
 }
 peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
 double volts = (peakToPeak * 3.3) / 1024;  // convert to volts
 double db = 27 + volts * 28.395;  
 return db;
}
