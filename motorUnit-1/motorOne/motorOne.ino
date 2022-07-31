#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

#define INTERVAL    2000 // 2 sec delay between publishing

#define CLIENT_ID   "Tower_ONE_SWITCH" //client ID | Unique ID for SUMP & motor

#define T1_DOM_ON 2
#define T1_DOM_OFF 3

#define T1_FLU_ON 5
#define T1_FLU_OFF 6

#define T2_DOM_ON 7
#define T2_DOM_OFF 19

#define T2_FLU_ON 18
#define T2_FLU_OFF 16

#define T6_DOM_ON 17
#define T6_DOM_OFF 14

#define T6_FLU_ON 9
#define T6_FLU_OFF 8

//-----------------------------
#define T1_DOM_LEVELTOPIC "/towerone/dom/level"#define T2_DOM_LEVELTOPIC "/towertwo/dom/level"
#define T6_DOM_LEVELTOPIC "/towersix/dom/level"

#define T1_FLU_LEVELTOPIC  "/towerone/flu/level"
#define T2_FLU_LEVELTOPIC  "/towertwo/flu/level"
#define T6_FLU_LEVELTOPIC  "/towersix/flu/level"
//-----------------------------
///towertwo/dom/switch

#define UNIT_1_ONLINESTATUS "/unitone/online" //Unit one includes T1, T2, and T6. To reduce unwanted memory coneception this will be better 

#define T1_DOM_MOTORSTATUS "/towerone/dom/motorstatus"
#define T2_DOM_MOTORSTATUS "/towertwo/dom/motorstatus"
#define T6_DOM_MOTORSTATUS "/towersix/dom/motorstatus"

#define T1_FLU_MOTORSTATUS "/towerone/flu/motorstatus"
#define T2_FLU_MOTORSTATUS "/towertwo/flu/motorstatus"
#define T6_FLU_MOTORSTATUS "/towersix/flu/motorstatus"

#define T1_DOM_DASH_MOTORSTATUS "/towerone/dom/dashStatus"
#define T2_DOM_DASH_MOTORSTATUS "/towertwo/dom/dashStatus"
#define T6_DOM_DASH_MOTORSTATUS "/towersix/dom/dashStatus"

#define T1_FLU_DASH_MOTORSTATUS "/towerone/flu/dashStatus"
#define T2_FLU_DASH_MOTORSTATUS "/towertwo/flu/dashStatus"
#define T6_FLU_DASH_MOTORSTATUS "/towersix/flu/dashStatus"

// #define DRYRUNSTATUS "/sumpone/dryrunstatus"
// #define DRYRUNSTATUS "/sumpone/dryrunstatus"
// #define DRYRUNSTATUS "/sumpone/dryrunstatus"

// #define DEGENSTATUS "/sumpone/degen"
// #define DEGENSTATUS "/towertwo/degen"
// #define DEGENSTATUS "/towersix/degen"

#define T1_MESSAGES "/towerone/messages"
#define T2_MESSAGES "/towertwo/messages"
#define T6_MESSAGES "/towersix/messages"


//-----Motor Condition
bool t1_dom_motorConditionFlag = false;
bool t1_flu_motorConditionFlag = false;

bool t2_dom_motorConditionFlag = false;
bool t2_flu_motorConditionFlag = false;

bool t6_dom_motorConditionFlag = false;
bool t6_flu_motorConditionFlag = false;

//--------Motor Func
bool t1_dom_motorFunctionFlag  = true;
bool t1_flu_motorFunctionFlag  = false;

bool t2_dom_motorFunctionFlag  = false;
bool t2_flu_motorFunctionFlag  = false;

bool t6_dom_motorFunctionFlag  = false;
bool t6_flu_motorFunctionFlag  = false;
//--------ETherNet Config-----

//---------FLU
bool t1_dom_dataReceived = false;
bool t2_dom_dataReceived = false;
bool t6_dom_dataReceived = false;

bool t1_flu_dataReceived = false;
bool t2_flu_dataReceived = false;
bool t6_flu_dataReceived = false;


int failCount = 0;
long previousMillis;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xAF, 0xDE, 0xEE };

IPAddress ip(192, 168, 1, 97);
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
    mqttClient.publish(T1_MESSAGES, "T1:DOM Motor ON");
    t1_dom_motorConditionFlag = true;
    if (!t1_dom_motorFunctionFlag) {
      t1_dom_turnOn();
    }
  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T1_DOM_MOTORSTATUS) == 0) {
    // Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T1_MESSAGES, "Motor ONE OFF");
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
    mqttClient.publish(T2_MESSAGES, "T1:DOM Motor ON");
    t2_dom_motorConditionFlag = true;
    if (!t2_dom_motorFunctionFlag) {
      t2_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T2_DOM_MOTORSTATUS) == 0) {
    // Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T2_MESSAGES, "Motor ONe OFFF");
    t2_dom_motorConditionFlag = true;
    if (t2_dom_motorFunctionFlag) {
      t2_dom_turnOff();
    }
    if(!t2_dom_dataReceived){
      t2_dom_turnOff();
      t2_dom_dataReceived = true;
    }
  }
  //-----T6 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T6_DOM_MOTORSTATUS) == 0) { //for T6 DOM
    // Serial.println("T1-DOM: TurnOn Motor ");
    mqttClient.publish(T6_MESSAGES, "T1:DOM Motor ON");
    t6_dom_motorConditionFlag = true;
    if (!t6_dom_motorFunctionFlag) {
      t6_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T6_DOM_MOTORSTATUS) == 0) {
    // Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T6_MESSAGES, "Motor ONe OFFF");
    t6_dom_motorConditionFlag = true;
    if (t6_dom_motorFunctionFlag) {
      t6_dom_turnOff();
    }
    if(!t6_dom_dataReceived){
      t6_dom_turnOff();
      t6_dom_dataReceived = true;
    }
  }
  //----Flush
  //-----T1 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T1_FLU_MOTORSTATUS) == 0) { //for T6 DOM
    // Serial.println(F("T1-FLU: TurnOn Motor "));
    mqttClient.publish(T1_MESSAGES, "T1:FLU Motor ON");
    t1_flu_motorConditionFlag = true;
    if (!t1_flu_motorFunctionFlag) {
      t1_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T1_FLU_MOTORSTATUS) == 0) {
    // Serial.println(F("T1-FLU: TurnOff Motor "));
    mqttClient.publish(T1_MESSAGES, "Motor ONe OFFF");
    t1_flu_motorConditionFlag = true;
    if (t1_flu_motorFunctionFlag) {
      t1_flu_turnOff();
    }
    if(!t1_flu_dataReceived){
      t1_flu_turnOff();
      t1_flu_dataReceived = true;
    }
  }
  //-----T2 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T2_FLU_MOTORSTATUS) == 0) { //for T6 DOM
    // Serial.println(F("T2-FLU: TurnOn Motor "));
    mqttClient.publish(T2_MESSAGES, "T2:FLU Motor ON");
    t2_flu_motorConditionFlag = true;
    if (!t2_flu_motorFunctionFlag) {
      t2_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T2_FLU_MOTORSTATUS) == 0) {
    // Serial.println(F("T2-FLU: TurnOff Motor "));
    mqttClient.publish(T2_MESSAGES, "Motor ONe OFFF");
    t2_flu_motorConditionFlag = true;
    if (t2_flu_motorFunctionFlag) {
      t2_flu_turnOff();
    }
    if(!t2_flu_dataReceived){
      t2_flu_turnOff();
      t2_flu_dataReceived = true;
    }
  }
  //-----T6 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T6_FLU_MOTORSTATUS) == 0) { //for T6 DOM
    // Serial.println(F("T6-FLU: TurnOn Motor "));
    mqttClient.publish(T6_MESSAGES, "T6:FLU Motor ON");
    t6_flu_motorConditionFlag = true;
    if (!t6_flu_motorFunctionFlag) {
      t6_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T6_FLU_MOTORSTATUS) == 0) {
    // Serial.println(F("T6-FLU: TurnOff Motor "));
    mqttClient.publish(T6_MESSAGES, "Motor ONe OFFF");
    t6_flu_motorConditionFlag = true;
    if (t6_flu_motorFunctionFlag) {
      t6_flu_turnOff();
    }
    if(!t6_flu_dataReceived){
      t6_flu_turnOff();
      t6_flu_dataReceived = true;
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

  Serial.println(F("Motor Unit 1 - T1/T2/T6"));

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
  r = mqttClient.subscribe(T1_FLU_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
  r = mqttClient.subscribe(T2_DOM_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
  r = mqttClient.subscribe(T2_FLU_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
  r = mqttClient.subscribe(T6_DOM_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
  r = mqttClient.subscribe(T6_FLU_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);

  //  r = mqttClient.subscribe(LEVELTOPIC);
  //  Serial.println("subscribed status LEVELTOPIC");
  //  Serial.println(r);
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
//------FLU 
bool t1_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t1_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T1_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t1_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T1_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t2_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t2_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T2_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t2_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T2_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t6_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t6_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T6_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t6_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T6_FLU_DASH_MOTORSTATUS, msgBuffer);
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
  Serial.println(F("T1 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T1_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T1_DOM_ON, !LOW);
}
void t1_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T1 -DOM motorOFF"));
  t1_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T1_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T1_DOM_OFF, !LOW);
//  delay(2000);
}

void t1_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t1_flu_motorFunctionFlag = true;
  Serial.println(F("T1 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T1_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T1_FLU_ON, !LOW);
}
void t1_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T1 -FLU motorOFF"));
  t1_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T1_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T1_FLU_OFF, !LOW);
}
//
//---------------Tower Two ------
void t2_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t2_dom_motorFunctionFlag = true;
  Serial.println(F("T2 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T2_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T2_DOM_ON, !LOW);
}
void t2_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T2 -DOM motorOFF"));
  t2_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T2_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T2_DOM_OFF, !LOW);
}

void t2_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t2_flu_motorFunctionFlag = true;
  Serial.println(F("T2 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T2_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T2_FLU_ON, !LOW);
}
void t2_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T2 -FLU motorOFF"));
  t2_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T2_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T2_FLU_OFF, !LOW);
}
//---------------Tower Three ------
void t6_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t6_dom_motorFunctionFlag = true;
  Serial.println(F("T6 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T6_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T6_DOM_ON, !LOW);
}
void t6_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T6 -DOM motorOFF"));
  t6_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T6_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T6_DOM_OFF, !LOW);
}

void t6_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t6_flu_motorFunctionFlag = true;
  Serial.println(F("T6 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T6_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T6_FLU_ON, !LOW);
}
void t6_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T6 -FLU motorOFF"));
  t6_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T6_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T6_FLU_OFF, !LOW);
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

    sendFlagcheck = t1_flu_sendData();
    sendFlagcheck = t2_flu_sendData();
    sendFlagcheck = t6_flu_sendData();
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