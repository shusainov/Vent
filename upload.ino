#include <PinOutNodeMCU.h>
#include <DHT_U.h>

#include <BlynkSimpleEsp8266.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>

#define DHTPIN D7    
#define DHTTYPE           DHT22 
#define ReadingInterval   2000
DHT_Unified dht(DHTPIN, DHTTYPE);

#define VentRelayPin D0 

char auth[] = "YourBlinkAuthKey";

// Your WiFi credentials.
// Set password to "" for open networks.

char ssid[] = "MyWiFi";
char pass[] = "MyWiFiPassword";
const char* host = "esp8266-vannaya";

BlynkTimer timer;

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

float targetHumidity = 60;
float humidity = 60;
bool ventOn = false;
int ventOnTimerNumber = 15;

void setup() {
  Serial.begin(115200);
  dht.begin();
  Blynk.begin(auth, ssid, pass);
  
  MDNS.begin(host);

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host);
  
  timer.setInterval(ReadingInterval, sendSensor);
  timer.setInterval(1000, setVentStatus);
  Blynk.syncVirtual(V8);
  Blynk.virtualWrite(V3, targetHumidity);

  pinMode(VentRelayPin, OUTPUT);
  digitalWrite(VentRelayPin, HIGH); 
}

void loop() {
  httpServer.handleClient();
  Blynk.run();
  timer.run();
}

void sendSensor()
{
  sensors_event_t event;  
  dht.temperature().getEvent(&event);

  if (isnan(event.temperature)) {
    Serial.println("Error reading temperature!");
  }
  else {
    Blynk.virtualWrite(V0, event.temperature);
  }

  dht.humidity().getEvent(&event);
  
  if (isnan(event.relative_humidity)) {
    Serial.println("Error reading humidity!");
  }
  else {
    humidity = event.relative_humidity;
    Blynk.virtualWrite(V1, humidity);
  }
}

BLYNK_WRITE(V3)
{
  targetHumidity = param.asFloat(); 
  Serial.println(targetHumidity);
}

BLYNK_WRITE(V8)
{
  ventOn = param.asInt();
  
  if (ventOn) ventOnTimerNumber=timer.setTimeout(60000, setVentOnToOff);
  if (!ventOn && timer.isEnabled(ventOnTimerNumber)) 
      timer.deleteTimer(ventOnTimerNumber);
}

void setVentStatus(){
  
  if( (targetHumidity<humidity) || ventOn){
      digitalWrite(VentRelayPin, LOW);
      Serial.println("On");
    } else 
    {
      digitalWrite(VentRelayPin, HIGH);
      Serial.println("Off");
    };
}


void setVentOnToOff(){
  ventOn = 0;
  Blynk.virtualWrite(V8, 0);
}
