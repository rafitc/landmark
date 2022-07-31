#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

#define INTERVAL    2000 // 2 sec delay between publishing

#define CLIENT_ID   "Tower_ONE_SWITCH" //client ID | Unique ID for SUMP & motor

//-----RELAY PINS
#define T1_DOM_ON 2 //SumpneTurnOn
#define T1_DOM_OFF 3 ////SumpneTurnOff

#define T2_DOM_ON 7 //sumptwoTurnOne
#define T2_DOM_OFF 19 //sumptwoTurnOne

#define T6_DOM_ON 17 //StandbyOn | Not using now
#define T6_DOM_OFF 14//StandbyOff | Not using now

#define UNIT_1_ONLINESTATUS "/sump/unitone/online" //Unit one includes T1, T2, and T6. To reduce unwanted memory coneception this will be better 

#define T1_DOM_MOTORSTATUS "/sumpone/motorstatus"
#define T2_DOM_MOTORSTATUS "/sumptwo/motorstatus"
#define T6_DOM_MOTORSTATUS "/standby/motorstatus" //Standby | Not using now

#define T1_DOM_DASH_MOTORSTATUS "/sumpone/dashStatus"
#define T2_DOM_DASH_MOTORSTATUS "/sumptwo/dashStatus"
#define T6_DOM_DASH_MOTORSTATUS "/standby/dashStatus" //Standby | Not using now

#define T1_MESSAGES "/sumpone/messages"
#define T2_MESSAGES "/sumptwo/messages"
#define T6_MESSAGES "/standby/messages" //Standby | Not using now


//-----Motor Condition
bool t1_dom_motorConditionFlag = false;
bool t2_dom_motorConditionFlag = false;
bool t6_dom_motorConditionFlag = false;
//--------Motor Func
bool t1_dom_motorFunctionFlag  = true;

bool t2_dom_motorFunctionFlag  = false;

bool t6_dom_motorFunctionFlag  = false;
//--------ETherNet Config-----

//---------FLU
bool t1_dom_dataReceived = false;
bool t2_dom_dataReceived = false;
bool t6_dom_dataReceived = false;

int failCount = 0;
long previousMillis;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xAF, 0xDE, 0xEE };

IPAddress ip(192, 168, 1, 84);
IPAddress myDns(8, 8, 8, 8);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetClient client;
PubSubClient mqttClient;

//--------------resetFunction
void(* resetFunc) (void) = 0;

//--------------CallBack
void callback(char* topic, byte* payload, unsigned int length) {
  char messageBuffer[30];  //somewhere to put the message
  int i = 0;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("-----Message-----------\n");
  Serial.print("Message:");


  memcpy(messageBuffer, payload, length);  //copy in the payload
  messageBuffer[length] = '\0';  //convert copied payload to a C style string
  Serial.println(messageBuffer);  //print the buffer if you want to

  Serial.println("-----");
  //-----T1 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T1_DOM_MOTORSTATUS) == 0) { //for T1 DOM
    //Serial.println("T1-DOM: TurnOn Motor ");
    mqttClient.publish(T1_MESSAGES, "SUMP 1 Motor On ");
    t1_dom_motorConditionFlag = true;
    if (!t1_dom_motorFunctionFlag) {
      t1_dom_turnOn();
    }
  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T1_DOM_MOTORSTATUS) == 0) {
    // Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T1_MESSAGES, "SUMP 1 Motor Off ");
    t1_dom_motorConditionFlag = true;
    if (t1_dom_motorFunctionFlag) {
      t1_dom_turnOff();
    }
    if(!t1_dom_dataReceived){
      t1_dom_turnOff();
      t1_dom_dataReceived = true;
    }
  }
  //-----T2 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T2_DOM_MOTORSTATUS) == 0) { //for T2 DOM
    // Serial.println("T1-DOM: TurnOn Motor ");
    mqttClient.publish(T2_MESSAGES, "SUMP 2 Motor On");
    t2_dom_motorConditionFlag = true;
    if (!t2_dom_motorFunctionFlag) {
      t2_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T2_DOM_MOTORSTATUS) == 0) {
    // Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T2_MESSAGES, "SUMP 2 Motor Off");
    t2_dom_motorConditionFlag = true;
    if (t2_dom_motorFunctionFlag) {
      t2_dom_turnOff();
    }
    if(!t2_dom_dataReceived){
      t2_dom_turnOff();
      t2_dom_dataReceived = true;
    }
  }
  //---NOT USING NOW
  //-----T6 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T6_DOM_MOTORSTATUS) == 0) { //for T6 DOM
    // Serial.println("T1-DOM: TurnOn Motor ");
    mqttClient.publish(T6_MESSAGES, "StandBy ON");
    t6_dom_motorConditionFlag = true;
    if (!t6_dom_motorFunctionFlag) {
      t6_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T6_DOM_MOTORSTATUS) == 0) {
    // Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T6_MESSAGES, "standby OFF");
    t6_dom_motorConditionFlag = true;
    if (t6_dom_motorFunctionFlag) {
      t6_dom_turnOff();
    }
    if(!t6_dom_dataReceived){
      t6_dom_turnOff();
      t6_dom_dataReceived = true;
    }
  }
}

void setup() {
  Serial.begin(9600);
  for (int i = 2; i <= 9; i++) { //pin config of relay
    pinMode(i, OUTPUT);
    digitalWrite(i, !LOW);
  }
  for (int i = 14; i <= 19; i++) { //pin config of relay
    pinMode(i, OUTPUT);
    digitalWrite(i, !LOW);
  }
  //-----ethernet setup
  // start the Ethernet connection:
  Serial.println(F("Initialize Ethernet with DHCP:"));
  Ethernet.begin(mac, ip, myDns, gateway, subnet);
  Serial.print("  DHCP assigned IP ");
  Serial.println(Ethernet.localIP());
  // give the Ethernet shield a second to initialize:
  delay(1000);

  Serial.println(F("SUMP TANK MOTOR"));

  //setup MQTT client
  mqttClient.setClient(client);
  mqttClient.setCallback(callback);

  mqttClient.setServer("192.168.1.85", 1883); //for using local broker

  Serial.println(F("MQTT client configured"));

  Serial.print(F(" \n Trying to connect as a client -> "));
  int connectionflag = mqttClient.connect(CLIENT_ID);
  Serial.println(connectionflag);

  boolean r;
  r = mqttClient.subscribe(T1_DOM_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
  r = mqttClient.subscribe(T2_DOM_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
  r = mqttClient.subscribe(T6_DOM_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
}


bool t1_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t1_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T1_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t1_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T1_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}



bool t2_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t2_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T2_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t2_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T2_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t6_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t1_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T6_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t6_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T6_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

void listenData() {
  //Listen TOPIC for necessory conditions
}
//---------------Tower One ------
void t1_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t1_dom_motorFunctionFlag = true;
  Serial.println(F("SUMP ONE MOTOR ON"));
  //M1_ON_RELAY
  digitalWrite(T1_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T1_DOM_ON, !LOW);
}
void t1_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("SUMP ONE MOTOR OFF"));
  t1_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T1_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T1_DOM_OFF, !LOW);
//  delay(2000);
}
//
//---------------Tower Two ------
void t2_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t2_dom_motorFunctionFlag = true;
  Serial.println(F("SUMP TWO MOTOR ON"));
  //M1_ON_RELAY
  digitalWrite(T2_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T2_DOM_ON, !LOW);
}
void t2_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("SUMP TWO MOTOR OFF"));
  t2_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T2_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T2_DOM_OFF, !LOW);
}
//---------------Tower Three ------
void t6_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t6_dom_motorFunctionFlag = true;
  Serial.println(F("STANDBY MOTOR ON"));
  //M1_ON_RELAY
  digitalWrite(T6_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T6_DOM_ON, !LOW);
}
void t6_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("STANDBY MOTOR OFF"));
  t6_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T6_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T6_DOM_OFF, !LOW);
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print(F("Attempting MQTT connection..."));
    // Attempt to connect
    if (mqttClient.connect(CLIENT_ID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
//      mqttClient.publish(ONLINESTATUS, "Online");
    }
    else {
      Serial.print("failed, rc=");
      int state = mqttClient.state();
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}
void loop() {
   mqttClient.loop();
  bool sendFlagcheck = false;
  if (millis() - previousMillis > INTERVAL) {
    sendFlagcheck = t1_dom_sendData();
    sendFlagcheck = t2_dom_sendData();
    sendFlagcheck = t6_dom_sendData();
    if (sendFlagcheck == false) {
      failCount ++;
    }
    else if (sendFlagcheck == true) {
      failCount = 0;
    }
    previousMillis = millis();
  }
  if (failCount > 5) {
    Serial.println(F("Resetting conection"));
    resetFunc();
    //reconnect();
  }
}