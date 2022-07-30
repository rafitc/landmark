#include <SPI.h>
#include <Ethernet.h>
#include "PubSubClient.h"

#define INTERVAL    2000 // 2 sec delay between publishing

#define CLIENT_ID   "Tower_TWO_SWITCH" //client ID | Unique ID for SUMP & motor

#define T4_DOM_ON 2
#define T4_DOM_OFF 3

#define T4_FLU_ON 5
#define T4_FLU_OFF 6

#define T5_DOM_ON 7
#define T5_DOM_OFF A0

#define T5_FLU_ON A1
#define T5_FLU_OFF A2

#define T3_DOM_ON 8
#define T3_DOM_OFF 9

#define T3_FLU_ON A3
#define T3_FLU_OFF A4

//-----------------------------
#define T4_DOM_LEVELTOPIC "/towerfour/dom/level"
#define T5_DOM_LEVELTOPIC "/towerfive/dom/level"
#define T3_DOM_LEVELTOPIC "/towerthree/dom/level"

#define T4_FLU_LEVELTOPIC  "/towerfour/flu/level"
#define T5_FLU_LEVELTOPIC  "/towerfive/flu/level"
#define T3_FLU_LEVELTOPIC  "/towerthree/flu/level"
//-----------------------------
///towerfive/dom/switch

#define UNIT_1_ONLINESTATUS "/unittwo/online" //Unit one includes T4, T5, and T3. To reduce unwanted memory coneception this will be better 

#define T4_DOM_MOTORSTATUS "/towerfour/dom/motorstatus"
#define T5_DOM_MOTORSTATUS "/towerfive/dom/motorstatus"
#define T3_DOM_MOTORSTATUS "/towerthree/dom/motorstatus"

#define T4_FLU_MOTORSTATUS "/towerfour/flu/motorstatus"
#define T5_FLU_MOTORSTATUS "/towerfive/flu/motorstatus"
#define T3_FLU_MOTORSTATUS "/towerthree/flu/motorstatus"

#define T4_DOM_DASH_MOTORSTATUS "/towerfour/dom/dashStatus"
#define T5_DOM_DASH_MOTORSTATUS "/towerfive/dom/dashStatus"
#define T3_DOM_DASH_MOTORSTATUS "/towerthree/dom/dashStatus"

#define T4_FLU_DASH_MOTORSTATUS "/towerfour/flu/dashStatus"
#define T5_FLU_DASH_MOTORSTATUS "/towerfive/flu/dashStatus"
#define T3_FLU_DASH_MOTORSTATUS "/towerthree/flu/dashStatus"

// #define DRYRUNSTATUS "/sumpone/dryrunstatus"
// #define DRYRUNSTATUS "/sumpone/dryrunstatus"
// #define DRYRUNSTATUS "/sumpone/dryrunstatus"

// #define DEGENSTATUS "/sumpone/degen"
// #define DEGENSTATUS "/towerfive/degen"
// #define DEGENSTATUS "/towerthree/degen"

#define T4_MESSAGES "/towerfour/messages"
#define T5_MESSAGES "/towerfive/messages"
#define T3_MESSAGES "/towerthree/messages"


//-----Motor Condition
bool t4_dom_motorConditionFlag = false;
bool t4_flu_motorConditionFlag = false;

bool t5_dom_motorConditionFlag = false;
bool t5_flu_motorConditionFlag = false;

bool t3_dom_motorConditionFlag = false;
bool t3_flu_motorConditionFlag = false;

//--------Motor Func
bool t4_dom_motorFunctionFlag  = false;
bool t4_flu_motorFunctionFlag  = false;

bool t5_dom_motorFunctionFlag  = false;
bool t5_flu_motorFunctionFlag  = false;

bool t3_dom_motorFunctionFlag  = false;
bool t3_flu_motorFunctionFlag  = false;
//--------ETherNet Config-----

//----Falg to protect from ErrorWhilePublishing
bool t3_dom_restartFlag = false;
bool t4_dom_restartFlag = false;
bool t5_dom_restartFlag = false;

bool t3_flu_restartFlag = false;
bool t4_flu_restartFlag = false;
bool t5_flu_restartFlag = false;

int failCount = 0;
long previousMillis;

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xEE };

IPAddress ip(192, 168, 1, 87);
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
  //-----T4 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T4_DOM_MOTORSTATUS) == 0) { //for T4 DOM
    //Serial.println("T4-DOM: TurnOn Motor ");
    mqttClient.publish(T4_MESSAGES, "T4:DOM Motor ON");
    t4_dom_motorConditionFlag = true;
    if (!t4_dom_motorFunctionFlag) {
      t4_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T4_DOM_MOTORSTATUS) == 0) {
    //Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T4_MESSAGES, "T4:DOM Motor OFF");
    t4_dom_motorConditionFlag = true;
    if (t4_dom_motorFunctionFlag) {
      t4_dom_turnOff();
    }
    if(!t4_dom_restartFlag){
        t4_dom_restartFlag = true;
        t4_dom_turnOff();
    }
  }
  //-----T5 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T5_DOM_MOTORSTATUS) == 0) { //for T5 DOM
    //Serial.println("T4-DOM: TurnOn Motor ");
    mqttClient.publish(T5_MESSAGES, "T5:DOM Motor ON");
    t5_dom_motorConditionFlag = true;
    if (!t5_dom_motorFunctionFlag) {
      t5_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T5_DOM_MOTORSTATUS) == 0) {
    // Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T5_MESSAGES, "T5:DOM Motor OFF");
    t5_dom_motorConditionFlag = true;
    if (t5_dom_motorFunctionFlag) {
      t5_dom_turnOff();
    }
    if(!t5_dom_restartFlag){
        t5_dom_restartFlag = true;
        t5_dom_turnOff();
    }
  }
  //-----T3 DOM
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T3_DOM_MOTORSTATUS) == 0) { //for T3 DOM
    // Serial.println("T3-DOM: TurnOn Motor ");
    mqttClient.publish(T3_MESSAGES, "T3:DOM Motor ON");
    t3_dom_motorConditionFlag = true;
    if (!t3_dom_motorFunctionFlag) {
      t3_dom_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T3_DOM_MOTORSTATUS) == 0) {
    // Serial.println("Turn OFF Motor Command Received ");
    mqttClient.publish(T3_MESSAGES, "T3:DOM Motor ON");
    t3_dom_motorConditionFlag = true;
    if (t3_dom_motorFunctionFlag) {
      t3_dom_turnOff();
    }
    if(!t3_dom_restartFlag){
        t3_dom_restartFlag = true;
        t3_dom_turnOff();
    }
  }
  //----Flush
  //-----T4 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T4_FLU_MOTORSTATUS) == 0) { //for T3 DOM
    // Serial.println(F("T4-FLU: TurnOn Motor "));
    mqttClient.publish(T4_MESSAGES, "T4:FLU Motor ON");
    t4_flu_motorConditionFlag = true;
    if (!t4_flu_motorFunctionFlag) {
      t4_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T4_FLU_MOTORSTATUS) == 0) {
    // Serial.println(F("T4-FLU: TurnOff Motor "));
    mqttClient.publish(T4_MESSAGES, "T4-FLU: TurnOff Motor ");
    t4_flu_motorConditionFlag = true;
    if (t4_flu_motorFunctionFlag) {
      t4_flu_turnOff();
    }
    if(!t4_flu_restartFlag){
        t4_flu_restartFlag = true;
        t4_flu_turnOff();
    }
  }
  //-----T5 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T5_FLU_MOTORSTATUS) == 0) { //for T3 DOM
    Serial.println(F("T5-FLU: TurnOn Motor "));
    mqttClient.publish(T5_MESSAGES, "T5:FLU Motor ON");
    t5_flu_motorConditionFlag = true;
    if (!t5_flu_motorFunctionFlag) {
      t5_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T5_FLU_MOTORSTATUS) == 0) {
    // Serial.println(F("T5-FLU: TurnOff Motor "));
    mqttClient.publish(T5_MESSAGES, "T5-FLU: TurnOff Motor ");
    t5_flu_motorConditionFlag = true;
    if (t5_flu_motorFunctionFlag) {
      t5_flu_turnOff();
    }
    if(!t5_flu_restartFlag){
        t5_flu_restartFlag = true;
        t5_flu_turnOff();
    }
  }
  //-----T3 FLU
  if (strcmp(messageBuffer, "turnOnMotorOne") == 0 && strcmp(topic, T3_FLU_MOTORSTATUS) == 0) { //for T3 DOM
    Serial.println(F("T3-FLU: TurnOn Motor "));
    mqttClient.publish(T3_MESSAGES, "T3:FLU Motor ON");
    t3_flu_motorConditionFlag = true;
    if (!t3_flu_motorFunctionFlag) {
      t3_flu_turnOn();
    }

  }
  if (strcmp(messageBuffer, "turnOffMotorOne") == 0 && strcmp(topic, T3_FLU_MOTORSTATUS) == 0) {
    Serial.println(F("T3-FLU: TurnOff Motor "));
    mqttClient.publish(T3_MESSAGES, "T3-FLU: TurnOff Motor ");
    t3_flu_motorConditionFlag = true;
    if (t3_flu_motorFunctionFlag) {
      t3_flu_turnOff();
    }
    if(!t3_flu_restartFlag){
        t3_flu_restartFlag = true;
        t3_flu_turnOff();
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

  Serial.println(F("Motor Unit 1 - T4/T5/T3"));

  //setup MQTT client
  mqttClient.setClient(client);
  mqttClient.setCallback(callback);

  mqttClient.setServer("192.168.1.85", 1883); //for using local broker

  Serial.println(F("MQTT client configured"));

  Serial.print(F(" \n Trying to connect as a client -> "));
  int connectionflag = mqttClient.connect(CLIENT_ID);
  Serial.println(connectionflag);

  boolean r;
  r = mqttClient.subscribe(T4_DOM_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
  r = mqttClient.subscribe(T4_FLU_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
    r = mqttClient.subscribe(T5_DOM_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);
  r = mqttClient.subscribe(T5_FLU_MOTORSTATUS);
  Serial.println(F("subscribed status MOTORSTATUS"));
  Serial.println(r);

  //  r = mqttClient.subscribe(LEVELTOPIC);
  //  Serial.println("subscribed status LEVELTOPIC");
  //  Serial.println(r);
}


bool t4_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t4_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T4_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t4_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T4_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t5_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t5_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T5_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t5_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T5_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t3_dom_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t4_dom_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T3_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t3_dom_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T3_DOM_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}
//------FLU 
bool t4_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t4_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T4_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t4_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T4_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t5_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t5_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T5_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t5_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T5_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  Serial.println(F("Published Data "));
  return true;
}

bool t3_flu_sendData() {
  bool sendStatusFlag;
  char* msgBuffer;

  msgBuffer = "Online";
  sendStatusFlag = mqttClient.publish(UNIT_1_ONLINESTATUS, msgBuffer);
  if (sendStatusFlag == false) {
    Serial.println(F("Error while publishsing "));
    return false;
  }
  if (t3_flu_motorFunctionFlag) {
    msgBuffer = "ON";
    sendStatusFlag = mqttClient.publish(T3_FLU_DASH_MOTORSTATUS, msgBuffer);
    if (sendStatusFlag == false) {
      Serial.println(F("Error while publishsing "));
      return false;
    }
  }
  else if (!t3_flu_motorFunctionFlag) {
    msgBuffer = "OFF";
    sendStatusFlag = mqttClient.publish(T3_FLU_DASH_MOTORSTATUS, msgBuffer);
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
void t4_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t4_dom_motorFunctionFlag = true;
  Serial.println(F("T4 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T4_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T4_DOM_ON, !LOW);
}
void t4_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T4 -DOM motorOFF"));
  t4_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T4_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T4_DOM_OFF, !LOW);
}

void t4_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t4_flu_motorFunctionFlag = true;
  Serial.println(F("T4 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T4_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T4_FLU_ON, !LOW);
}
void t4_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T4 -FLU motorOFF"));
  t4_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T4_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T4_FLU_OFF, !LOW);
}
//
//---------------Tower Two ------
void t5_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t5_dom_motorFunctionFlag = true;
  Serial.println(F("T5 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T5_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T5_DOM_ON, !LOW);
}
void t5_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T5 -DOM motorOFF"));
  t5_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T5_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T5_DOM_OFF, !LOW);
}

void t5_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t5_flu_motorFunctionFlag = true;
  Serial.println(F("T5 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T5_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T5_FLU_ON, !LOW);
}
void t5_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T5 -FLU motorOFF"));
  t5_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T5_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T5_FLU_OFF, !LOW);
}
//---------------Tower Three ------
void t3_dom_turnOn() {
  //Turn Off by 2Second control of starter box
  t3_dom_motorFunctionFlag = true;
  Serial.println(F("T3 -DOM motorON"));
  //M1_ON_RELAY
  digitalWrite(T3_DOM_ON, !HIGH);
  delay(2000);
  digitalWrite(T3_DOM_ON, !LOW);
}
void t3_dom_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T3 -DOM motorOFF"));
  t3_dom_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T3_DOM_OFF, !HIGH);
  delay(2000);
  digitalWrite(T3_DOM_OFF, !LOW);
}

void t3_flu_turnOn() {
  //Turn Off by 2Second control of starter box
  t3_flu_motorFunctionFlag = true;
  Serial.println(F("T3 -FLU motorON"));
  //M1_ON_RELAY
  digitalWrite(T3_FLU_ON, !HIGH);
  delay(2000);
  digitalWrite(T3_FLU_ON, !LOW);
}
void t3_flu_turnOff() {
  //Turn Off by 2Second control of starter box
  Serial.println(F("T3 -FLU motorOFF"));
  t3_flu_motorFunctionFlag = false;
  //M1_OFF_RELAY
  digitalWrite(T3_FLU_OFF, !HIGH);
  delay(2000);
  digitalWrite(T3_FLU_OFF, !LOW);
}

void loop() {
  bool sendFlagcheck = false;
  if (millis() - previousMillis > INTERVAL) {
    sendFlagcheck = t4_dom_sendData();
    sendFlagcheck = t5_dom_sendData();
    sendFlagcheck = t3_dom_sendData();

    sendFlagcheck = t4_flu_sendData();
    sendFlagcheck = t5_flu_sendData();
    sendFlagcheck = t3_flu_sendData();
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
  }
  mqttClient.loop();
}