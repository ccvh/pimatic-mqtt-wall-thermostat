#include <GxEPD.h>
#include <GxGDEP015OC1/GxGDEP015OC1.cpp>  // 1.54" b/w

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>

#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>

GxIO_Class io(SPI, SS, 17, 16);
GxEPD_Class display(io, 16, 4);

#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "******";
const char* password =  "********";
const char* mqttServer = "192.168.178.100";
const int mqttPort = 1883;
const char* mqttUser = "*****";
const char* mqttPassword = "*****";

char RealTemp[50];
char Setpoint[50];
String Value;
char sendPayload[50];

int PushbuttonUpPin = 13;
int PushbuttonDownPin = 12;


WiFiClient espClient;
PubSubClient client(espClient);

void DrawText(int x, int y, String Text, const GFXfont* f) {
  display.setFont(f);
  display.setCursor(x, y);
  display.println(Text);  
}

void ClearDisp() {
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
}

void setup() {
  pinMode(PushbuttonUpPin, INPUT); 
  pinMode(PushbuttonDownPin, INPUT); 
  
  display.init();
  display.setTextColor(GxEPD_BLACK);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password);
 
  while(WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.println("WiFi connecting...");

    DrawText(5, 15, "WiFi connecting...", &FreeMonoBold9pt7b);
    display.update();
  }
  
  DrawText(5, 33, "Connected to WiFi!", &FreeMonoBold9pt7b);
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  
  while (!client.connected()) {
    Serial.println("MQTT connecting...");
    DrawText(5, 69, "MQTT connecting...", &FreeMonoBold9pt7b);
    display.update();

    if (client.connect("Thermostat Bad", mqttUser, mqttPassword )) {
      Serial.println("connected");
      DrawText(5, 87, "MQTT connected!", &FreeMonoBold9pt7b);
      DrawText(5, 105, "Wait for publish...", &FreeMonoBold9pt7b);
      display.update(); 
    }
    else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }

  client.subscribe("Temperatur/Bad/Setpoint");  
  client.subscribe("Temperatur/Bad/RealTemp");  
}

 
void callback(char* topic, byte* payload, unsigned int length) {
  
  if(strcmp(topic, "Temperatur/Bad/RealTemp")==0)
  {
    RealTemp[0] = (char)0;
    Serial.println("Received Temperatur/Bad/RealTemp");
    
    for (int i = 0; i < length; i++) {
      RealTemp[i] = payload[i];
    }
  }
  
  if(strcmp(topic, "Temperatur/Bad/Setpoint")==0)
  {
    Setpoint[0] = (char)0;
    Serial.println("Received Temperatur/Bad/Setpoint");

    for (int j = 0; j < length; j++) {
      Setpoint[j] = payload[j];
    } 

  }
  String SetpointPr = String(Setpoint);
  String RealTempPr = String(RealTemp);

  int rt_strlen = strlen(RealTemp);
  int sp_strlen = strlen(Setpoint);
  int x_sp;
  int x_rt;
  
  if(sp_strlen == 2){
    x_sp = 95;    
  }
  else {
    x_sp = 45;
  }
  
  if(rt_strlen == 2){
    x_rt = 95; 
  }
  else {
    x_rt = 45;
  }

  ClearDisp();
  
  DrawText(5, 15, "Raumtemperatur", &FreeMonoBold9pt7b);
  DrawText(x_rt, 65, RealTempPr, &FreeMonoBold24pt7b);
  display.drawCircle(165, 140, 3, GxEPD_BLACK);
  display.drawCircle(165, 140, 2, GxEPD_BLACK);
  
  DrawText(5, 110, "Thermostat", &FreeMonoBold9pt7b);
  DrawText(x_sp, 170, SetpointPr, &FreeMonoBold24pt7b);
  display.drawCircle(165, 35, 3, GxEPD_BLACK);
  display.drawCircle(165, 35, 2, GxEPD_BLACK);

  display.drawRect(0, 0, 200, 200, GxEPD_BLACK);
  display.drawLine(0, 95, 200, 95, GxEPD_BLACK);

  display.fillTriangle(175, 120, 185, 110, 195, 120, GxEPD_BLACK);
  display.fillTriangle(175, 165, 185, 175, 195, 165, GxEPD_BLACK); 
  
  display.update();
}

void loop() {
  client.loop();
  
  int UpButtonValue = digitalRead(PushbuttonUpPin);
  int DownButtonValue = digitalRead(PushbuttonDownPin);
  
  if(UpButtonValue == HIGH)
  {  
    float newValue = atof(Setpoint) + 0.5;;
    dtostrf(newValue, 3, 1, sendPayload);

    client.publish("Temperatur/Bad/Setpoint", sendPayload);
    delay(300);
  }
  if(DownButtonValue == HIGH)
    { 
    float newValue = atof(Setpoint) - 0.5;
    dtostrf(newValue, 3, 1, sendPayload);
        
    client.publish("Temperatur/Bad/Setpoint", sendPayload);
    delay(300);
  }
}
