#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>

char ssid[] = "TelstraE78C95";
char pass[] = "mnnu9vh7x7";


int echopin = 2;
int trigpin = 4;
int LED = 8;

WiFiClient wClient;
MqttClient mqttClient(wClient);

const char broker[] = "broker.emqx.io";
int port = 1883;
const char pat_subject[] = "SIT210/pat";
const char waves_subject[] = "SIT210/waves";
const char message[] = "working";

bool subscribed = false;
bool pat = false;

void setup() {
  // Initialize serial and wait for port to open:
  
  Serial.begin(9600);

  while(!Serial) delay(1);

  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);

  Serial.println("Attempting to connect to wifi");
  while (WiFi.begin(ssid, pass) != WL_CONNECTED)
  {
    Serial.print(",");
    delay(5000);
  }

  Serial.println("Connected to WiFi");
  Serial.println();

  Serial.println("Attempting to connect to MQTT");
  Serial.println(broker);

  if(!mqttClient.connect(broker, port)){
    Serial.print("Can't connect to MQTT broker: error code = ");
    Serial.println(mqttClient.connectError());

    while(1);
  }
  Serial.println("Connected to MQTT");
  Serial.println();

  
  mqttClient.subscribe(pat_subject);
  mqttClient.subscribe(waves_subject);
}

void loop() {  
  if (subscribed == false){
    sensor();
  }
  else{
    led();
  }
}

void led(){
  int Msize = mqttClient.parseMessage();
  Serial.print(Msize);
  if (Msize){
    Serial.print("Works");
    Serial.print("Received message with the subject");
    Serial.print(mqttClient.messageTopic());
    Serial.print(" , length ");
    Serial.print(Msize);
    Serial.println(" bytes:");

    while (mqttClient.available()){
      Serial.print((char)mqttClient.read());
    }

    Serial.println();
    int i = 0;
    if (pat == false)
    {
      while (i < 3){
        digitalWrite(LED, HIGH);
        delay(700);
        digitalWrite(LED, LOW);
        delay(700);
        i++;
      }
    }
    else
    {
      digitalWrite(LED, HIGH);
      delay(3000);
      digitalWrite(LED, LOW);
    }

    Serial.println();
  }
  subscribed = false;
}

void sensor(){
  mqttClient.poll();
  delay(500);

  digitalWrite(trigpin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigpin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigpin, LOW);
  
  float time = pulseIn(echopin, HIGH);
  float cm = (time * 0.0343) / 2;
  Serial.print("length from sensor: ");
  Serial.println(cm);
  if (cm < 25 && cm > 10){        
    pat = false;
    mqttClient.beginMessage(waves_subject);
    mqttClient.println(waves_subject);
    mqttClient.print("hello: ");
    mqttClient.endMessage();
    subscribed = true;
    delay(500);
    pat = false;
  }
  else if (cm < 10){
    pat = true;
    mqttClient.beginMessage(pat_subject);
    mqttClient.println(pat_subject);
    mqttClient.print("patted: ");
    mqttClient.endMessage();
    subscribed = true;
    delay(500);
  }
}
